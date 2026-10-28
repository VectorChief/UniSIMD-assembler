/******************************************************************************/
/* Copyright (c) 2013-2026 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_Z64_H
#define RT_RTARCH_Z64_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_z64.h: Implementation of s390x 64-bit BASE instructions.
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
 * stack_st - applies [mov] to stack from full register (push)
 * stack_ld - applies [mov] to full register from stack (pop)
 * stack_sa - applies [mov] to stack from all full registers
 * stack_la - applies [mov] to all full registers from stack
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

#include "rtarch_z32.h"
#include "rtarch_z32_128x1v1.h"
#include "rtarch_z32_128x2v1.h"
#include "rtarch_z64_128x1v1.h"
#include "rtarch_z64_128x2v1.h"
#include "rtarch_zHB.h"
#include "rtarch_zHB_128x1v1.h"
#include "rtarch_zHB_128x2v1.h"
#include "rtarch_zQF_128x1v1.h"
#include "rtarch_zQF_128x2v1.h"

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

#if (defined RT_Z64)

/******************************************************************************/
/**********************************   BASE   **********************************/
/******************************************************************************/

/* mov (D = S)
 * set-flags: no */

#define movzx_ri(RD, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RD),0x01,VAL(IS)))

#define movzx_mi(MD, DD, IS)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    REG(MD), VAL(DD), A1(DD), EMPTY2)   \
        EMIT6(MIM(0xC2, SPxx,0x04,0x08)) /* SP -= 8 */                      \
        EMIT6(0xE30000000024 | MSM(TMxx,   SPxx,0x00))                      \
        EMIT6(MIM(0xC0, TMxx,   0x01,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MD),REG(MD), VAL(DD), B1(DD), P1(DD)))  \
        EMIT6(0xE30000000004 | MSM(TMxx,   SPxx,0x00))                      \
        EMIT6(MIM(0xC2, SPxx,0x08,0x08)) /* SP += 8 */

#define movzx_rr(RD, RS)                                                    \
        EMITW(MGM(0x04, REG(RD),REG(RS),0x00))

#define movwn_rr(RD, RS)       /* move 32-bit to 64-bit with sign-extend */ \
        EMITW(MGM(0x14, REG(RD),REG(RS),0x00))

#define movwz_rr(RD, RS)       /* move 32-bit to 64-bit with zero-extend */ \
        EMITW(MGM(0x16, REG(RD),REG(RS),0x00))

#define movzx_ld(RD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x04, REG(RD),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define movwn_ld(RD, MS, DS)   /* load 32-bit to 64-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x14, REG(RD),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define movwz_ld(RD, MS, DS)   /* load 32-bit to 64-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x16, REG(RD),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define movzx_st(RS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    REG(MD), VAL(DD), A1(DD), EMPTY2)   \
        EMIT6(MDM(0x24, REG(RS),MOD(MD),REG(MD), VAL(DD), B1(DD), P1(DD)))


#define movzx_rj(RD, IT, IS)     /* IT - upper 32-bit, IS - lower 32-bit */ \
        EMIT6(MIM(0xC0, REG(RD),0x01,VAL(IS)))                              \
        EMIT6(MIM(0xC0, REG(RD),0x08,VAL(IT)))

#define movzx_mj(MD, DD, IT, IS) /* IT - upper 32-bit, IS - lower 32-bit */ \
        AUW(SIB(MD),  EMPTY,  EMPTY,    REG(MD), VAL(DD), A1(DD), EMPTY2)   \
        EMIT6(MIM(0xC2, SPxx,0x04,0x08)) /* SP -= 8 */                      \
        EMIT6(0xE30000000024 | MSM(TMxx,   SPxx,0x00))                      \
        EMIT6(MIM(0xC0, TMxx,   0x01,VAL(IS)))                              \
        EMIT6(MIM(0xC0, TMxx,   0x08,VAL(IT)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MD),REG(MD), VAL(DD), B1(DD), P1(DD)))  \
        EMIT6(0xE30000000004 | MSM(TMxx,   SPxx,0x00))                      \
        EMIT6(MIM(0xC2, SPxx,0x08,0x08)) /* SP += 8 */

/* and (G = G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define andzx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x0B,VAL(IS)))                              \
        EMIT6(MIM(0xC0, REG(RG),0x0A,   0x00))

#define andzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x0B,VAL(IS)))                              \
        EMIT6(MIM(0xC0, TMxx,   0x0A,   0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define andzx_rr(RG, RS)                                                    \
        EMITW(MGM(0x80, REG(RG),REG(RS),0x00))

#define andzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x80, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define andzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0x80, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define andzx_mr(MG, DG, RS)                                                \
        andzx_st(W(RS), W(MG), W(DG))


#define andzxZri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x0A,   0x00))                              \
        EMIT6(MIM(0xC0, REG(RG),0x0B,VAL(IS)))

#define andzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x0A,   0x00))                              \
        EMIT6(MIM(0xC0, TMxx,   0x0B,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define andzxZrr(RG, RS)                                                    \
        EMITW(MGM(0x80, REG(RG),REG(RS),0x00))

#define andzxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x80, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define andzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0x80, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define andzxZmr(MG, DG, RS)                                                \
        andzxZst(W(RS), W(MG), W(DG))

/* ann (G = ~G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define annzx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x06,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x0B,VAL(IS)))                              \
        EMIT6(MIM(0xC0, REG(RG),0x0A,   0x00))

#define annzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x06,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x0B,VAL(IS)))                              \
        EMIT6(MIM(0xC0, TMxx,   0x0A,   0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#if RT_Z64 >= 15 /* only on z15 and beyond */

#define annzx_rr(RG, RS)                                                    \
        EMITW(MGM(0xE5, REG(RG),REG(RS),REG(RG)))

#define annzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0xE5, REG(RG),TMxx,   REG(RG)))

#define annzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0xE5, TMxx,   REG(RS),TMxx))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#else /* RT_Z64 >= 15 */

#define annzx_rr(RG, RS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x06,0xFFFFFFFF))                           \
        EMITW(MGM(0x80, REG(RG),REG(RS),0x00))

#define annzx_ld(RG, MS, DS)                                                \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x06,0xFFFFFFFF))                           \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x80, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define annzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x06,0xFFFFFFFF))                           \
        EMITW(MGM(0x80, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#endif /* RT_Z64 >= 15 */

#define annzx_mr(MG, DG, RS)                                                \
        annzx_st(W(RS), W(MG), W(DG))


#define annzxZri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x0A,   0x00))                              \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x0B,VAL(IS)))                              \

#define annzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x0A,   0x00))                              \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x0B,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#if RT_Z64 >= 15 /* only on z15 and beyond */

#define annzxZrr(RG, RS)                                                    \
        EMITW(MGM(0xE5, REG(RG),REG(RS),REG(RG)))

#define annzxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0xE5, REG(RG),TMxx,   REG(RG)))

#define annzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0xE5, TMxx,   REG(RS),TMxx))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#else /* RT_Z64 >= 15 */

#define annzxZrr(RG, RS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x06,0xFFFFFFFF))                           \
        EMITW(MGM(0x80, REG(RG),REG(RS),0x00))

#define annzxZld(RG, MS, DS)                                                \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x06,0xFFFFFFFF))                           \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x80, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define annzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x06,0xFFFFFFFF))                           \
        EMITW(MGM(0x80, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#endif /* RT_Z64 >= 15 */

#define annzxZmr(MG, DG, RS)                                                \
        annzxZst(W(RS), W(MG), W(DG))

/* orr (G = G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define orrzx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x0D,VAL(IS)))

#define orrzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x0D,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define orrzx_rr(RG, RS)                                                    \
        EMITW(MGM(0x81, REG(RG),REG(RS),0x00))

#define orrzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x81, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define orrzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0x81, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define orrzx_mr(MG, DG, RS)                                                \
        orrzx_st(W(RS), W(MG), W(DG))


#define orrzxZri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x0D,VAL(IS)))

#define orrzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x0D,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define orrzxZrr(RG, RS)                                                    \
        EMITW(MGM(0x81, REG(RG),REG(RS),0x00))

#define orrzxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x81, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define orrzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0x81, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define orrzxZmr(MG, DG, RS)                                                \
        orrzxZst(W(RS), W(MG), W(DG))

/* orn (G = ~G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define ornzx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x06,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x0D,VAL(IS)))

#define ornzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x06,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x0D,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#if RT_Z64 >= 15 /* only on z15 and beyond */

#define ornzx_rr(RG, RS)                                                    \
        EMITW(MGM(0x65, REG(RG),REG(RS),REG(RG)))

#define ornzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0x65, REG(RG),TMxx,   REG(RG)))

#define ornzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0x65, TMxx,   REG(RS),TMxx))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#else /* RT_Z64 >= 15 */

#define ornzx_rr(RG, RS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x06,0xFFFFFFFF))                           \
        EMITW(MGM(0x81, REG(RG),REG(RS),0x00))

#define ornzx_ld(RG, MS, DS)                                                \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x06,0xFFFFFFFF))                           \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x81, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define ornzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x06,0xFFFFFFFF))                           \
        EMITW(MGM(0x81, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#endif /* RT_Z64 >= 15 */

#define ornzx_mr(MG, DG, RS)                                                \
        ornzx_st(W(RS), W(MG), W(DG))


#define ornzxZri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x06,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x0D,VAL(IS)))

#define ornzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x06,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x0D,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#if RT_Z64 >= 15 /* only on z15 and beyond */

#define ornzxZrr(RG, RS)                                                    \
        EMITW(MGM(0x65, REG(RG),REG(RS),REG(RG)))

#define ornzxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0x65, REG(RG),TMxx,   REG(RG)))

#define ornzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0x65, TMxx,   REG(RS),TMxx))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#else /* RT_Z64 >= 15 */

#define ornzxZrr(RG, RS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x06,0xFFFFFFFF))                           \
        EMITW(MGM(0x81, REG(RG),REG(RS),0x00))

#define ornzxZld(RG, MS, DS)                                                \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x06,0xFFFFFFFF))                           \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x81, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define ornzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x06,0xFFFFFFFF))                           \
        EMITW(MGM(0x81, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#endif /* RT_Z64 >= 15 */

#define ornzxZmr(MG, DG, RS)                                                \
        ornzxZst(W(RS), W(MG), W(DG))

/* xor (G = G ^ S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define xorzx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,VAL(IS)))

#define xorzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define xorzx_rr(RG, RS)                                                    \
        EMITW(MGM(0x82, REG(RG),REG(RS),0x00))

#define xorzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x82, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define xorzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0x82, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define xorzx_mr(MG, DG, RS)                                                \
        xorzx_st(W(RS), W(MG), W(DG))


#define xorzxZri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,VAL(IS)))

#define xorzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define xorzxZrr(RG, RS)                                                    \
        EMITW(MGM(0x82, REG(RG),REG(RS),0x00))

#define xorzxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x82, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define xorzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0x82, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define xorzxZmr(MG, DG, RS)                                                \
        xorzxZst(W(RS), W(MG), W(DG))

/* not (G = ~G)
 * set-flags: no */

#define notzx_rx(RG)                                                        \
        EMITW(0xB2220000 | TPxx << 4)                                       \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x06,0xFFFFFFFF))                           \
        EMITH(0x0400 | TPxx << 4)

#define notzx_mx(MG, DG)                                                    \
        EMITW(0xB2220000 | TPxx << 4)                                       \
        EMIT6(MIM(0xC2, SPxx,0x04,0x08)) /* SP -= 8 */                      \
        EMIT6(0xE30000000024 | MSM(TPxx,   SPxx,0x00))                      \
        EMIT6(MIM(0xC2, SPxx,0x04,0x08)) /* SP -= 8 */                      \
        EMIT6(0xE30000000024 | MSM(TMxx,   SPxx,0x00))                      \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x06,0xFFFFFFFF))                           \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(0xE30000000004 | MSM(TMxx,   SPxx,0x00))                      \
        EMIT6(MIM(0xC2, SPxx,0x08,0x08)) /* SP += 8 */                      \
        EMIT6(0xE30000000004 | MSM(TPxx,   SPxx,0x00))                      \
        EMIT6(MIM(0xC2, SPxx,0x08,0x08)) /* SP += 8 */                      \
        EMITH(0x0400 | TPxx << 4)

/* neg (G = -G)
 * set-flags: undefined (*_*), yes (*Z*) */

#define negzx_rx(RG)                                                        \
        EMITW(MGM(0x03, REG(RG),REG(RG),0x00))

#define negzx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0x03, TMxx,   TMxx,   0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))


#define negzxZrx(RG)                                                        \
        EMITW(MGM(0x03, REG(RG),REG(RG),0x00))

#define negzxZmx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0x03, TMxx,   TMxx,   0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

/* add (G = G + S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define addzx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC2, REG(RG),0x08,VAL(IS)))

#define addzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC2, TMxx,   0x08,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define addzx_rr(RG, RS)                                                    \
        EMITW(MGM(0x08, REG(RG),REG(RS),0x00))

#define addzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x08, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define addwn_ld(RG, MS, DS)    /* add 32-bit to 64-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x18, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define addwz_ld(RG, MS, DS)    /* add 32-bit to 64-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x1A, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define addzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0x08, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define addzx_mr(MG, DG, RS)                                                \
        addzx_st(W(RS), W(MG), W(DG))


#define addzxZri(RG, IS)                                                    \
        EMIT6(MIM(0xC2, REG(RG),0x08,VAL(IS)))                              \
        EMITW(MGM(0x02, REG(RG),REG(RG),0x00))         /* <- set flags (Z) */

#define addzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC2, TMxx,   0x08,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0x02, TMxx,   TMxx,   0x00))         /* <- set flags (Z) */

#define addzxZrr(RG, RS)                                                    \
        EMITW(MGM(0x08, REG(RG),REG(RS),0x00))                              \
        EMITW(MGM(0x02, REG(RG),REG(RG),0x00))         /* <- set flags (Z) */

#define addzxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x08, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0x02, REG(RG),REG(RG),0x00))         /* <- set flags (Z) */

#define addwnZld(RG, MS, DS)    /* add 32-bit to 64-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x18, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0x02, REG(RG),REG(RG),0x00))         /* <- set flags (Z) */

#define addwzZld(RG, MS, DS)    /* add 32-bit to 64-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x1A, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0x02, REG(RG),REG(RG),0x00))         /* <- set flags (Z) */

#define addzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0x08, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0x02, TMxx,   TMxx,   0x00))         /* <- set flags (Z) */

#define addzxZmr(MG, DG, RS)                                                \
        addzxZst(W(RS), W(MG), W(DG))

/* sub (G = G - S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define subzx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC2, REG(RG),0x04,VAL(IS)))

#define subzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC2, TMxx,   0x04,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define subzx_rr(RG, RS)                                                    \
        EMITW(MGM(0x09, REG(RG),REG(RS),0x00))

#define subzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x09, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define subwn_ld(RG, MS, DS)  /* sub 32-bit from 64-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x19, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define subwz_ld(RG, MS, DS)  /* sub 32-bit from 64-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x1B, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define subzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0x09, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define subzx_mr(MG, DG, RS)                                                \
        subzx_st(W(RS), W(MG), W(DG))


#define subzxZri(RG, IS)                                                    \
        EMIT6(MIM(0xC2, REG(RG),0x04,VAL(IS)))                              \
        EMITW(MGM(0x02, REG(RG),REG(RG),0x00))         /* <- set flags (Z) */

#define subzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC2, TMxx,   0x04,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0x02, TMxx,   TMxx,   0x00))         /* <- set flags (Z) */

#define subzxZrr(RG, RS)                                                    \
        EMITW(MGM(0x09, REG(RG),REG(RS),0x00))                              \
        EMITW(MGM(0x02, REG(RG),REG(RG),0x00))         /* <- set flags (Z) */

#define subzxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x09, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0x02, REG(RG),REG(RG),0x00))         /* <- set flags (Z) */

#define subwnZld(RG, MS, DS)  /* sub 32-bit from 64-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x19, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0x02, REG(RG),REG(RG),0x00))         /* <- set flags (Z) */

#define subwzZld(RG, MS, DS)  /* sub 32-bit from 64-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x1B, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0x02, REG(RG),REG(RG),0x00))         /* <- set flags (Z) */

#define subzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0x09, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0x02, TMxx,   TMxx,   0x00))         /* <- set flags (Z) */

#define subzxZmr(MG, DG, RS)                                                \
        subzxZst(W(RS), W(MG), W(DG))

/* shl (G = G << S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlzx_rx(RG)                     /* reads Recx for shift count */   \
        EMIT6(MTM(0x0D, REG(RG),TEcx,   0x00))

#define shlzx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x0D, TMxx,   TEcx,   0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shlzx_ri(RG, IS)                                                    \
        EMIT6(MTM(0x0D, REG(RG),0x00,VAL(IS)))

#define shlzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x0D, TMxx,   0x00,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shlzx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMIT6(MTM(0x0D, REG(RG),REG(RS),0x00))

#define shlzx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x04, TDxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MTM(0x0D, REG(RG),TDxx,   0x00))

#define shlzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x0D, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shlzx_mr(MG, DG, RS)                                                \
        shlzx_st(W(RS), W(MG), W(DG))


#define shlzxZrx(RG)                     /* reads Recx for shift count */   \
        EMIT6(MTM(0x0D, REG(RG),TEcx,   0x00))                              \
        EMITW(0xA70F0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define shlzxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x0D, TMxx,   TEcx,   0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA70F0000 | TMxx << 20)                 /* <- set flags (Z) */

#define shlzxZri(RG, IS)                                                    \
        EMIT6(MTM(0x0D, REG(RG),0x00,VAL(IS)))                              \
        EMITW(0xA70F0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define shlzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x0D, TMxx,   0x00,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA70F0000 | TMxx << 20)                 /* <- set flags (Z) */

#define shlzxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMIT6(MTM(0x0D, REG(RG),REG(RS),0x00))                              \
        EMITW(0xA70F0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define shlzxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x04, TDxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MTM(0x0D, REG(RG),TDxx,   0x00))                              \
        EMITW(0xA70F0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define shlzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x0D, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA70F0000 | TMxx << 20)                 /* <- set flags (Z) */

#define shlzxZmr(MG, DG, RS)                                                \
        shlzxZst(W(RS), W(MG), W(DG))

/* shr (G = G >> S), unsigned (logical)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrzx_rx(RG)                     /* reads Recx for shift count */   \
        EMIT6(MTM(0x0C, REG(RG),TEcx,   0x00))

#define shrzx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x0C, TMxx,   TEcx,   0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrzx_ri(RG, IS)                                                    \
        EMIT6(MTM(0x0C, REG(RG),0x00,VAL(IS)))

#define shrzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x0C, TMxx,   0x00,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrzx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMIT6(MTM(0x0C, REG(RG),REG(RS),0x00))

#define shrzx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x04, TDxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MTM(0x0C, REG(RG),TDxx,   0x00))

#define shrzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x0C, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrzx_mr(MG, DG, RS)                                                \
        shrzx_st(W(RS), W(MG), W(DG))


#define shrzxZrx(RG)                     /* reads Recx for shift count */   \
        EMIT6(MTM(0x0C, REG(RG),TEcx,   0x00))                              \
        EMITW(0xA70F0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define shrzxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x0C, TMxx,   TEcx,   0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA70F0000 | TMxx << 20)                 /* <- set flags (Z) */

#define shrzxZri(RG, IS)                                                    \
        EMIT6(MTM(0x0C, REG(RG),0x00,VAL(IS)))                              \
        EMITW(0xA70F0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define shrzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x0C, TMxx,   0x00,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA70F0000 | TMxx << 20)                 /* <- set flags (Z) */

#define shrzxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMIT6(MTM(0x0C, REG(RG),REG(RS),0x00))                              \
        EMITW(0xA70F0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define shrzxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x04, TDxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MTM(0x0C, REG(RG),TDxx,   0x00))                              \
        EMITW(0xA70F0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define shrzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x0C, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA70F0000 | TMxx << 20)                 /* <- set flags (Z) */

#define shrzxZmr(MG, DG, RS)                                                \
        shrzxZst(W(RS), W(MG), W(DG))

/* shr (G = G >> S), signed (arithmetic)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrzn_rx(RG)                     /* reads Recx for shift count */   \
        EMIT6(MTM(0x0A, REG(RG),TEcx,   0x00))

#define shrzn_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x0A, TMxx,   TEcx,   0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrzn_ri(RG, IS)                                                    \
        EMIT6(MTM(0x0A, REG(RG),0x00,VAL(IS)))

#define shrzn_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x0A, TMxx,   0x00,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrzn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMIT6(MTM(0x0A, REG(RG),REG(RS),0x00))

#define shrzn_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x04, TDxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MTM(0x0A, REG(RG),TDxx,   0x00))

#define shrzn_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x0A, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrzn_mr(MG, DG, RS)                                                \
        shrzn_st(W(RS), W(MG), W(DG))


#define shrznZrx(RG)                     /* reads Recx for shift count */   \
        EMIT6(MTM(0x0A, REG(RG),TEcx,   0x00))                              \
        EMITW(0xA70F0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define shrznZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x0A, TMxx,   TEcx,   0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA70F0000 | TMxx << 20)                 /* <- set flags (Z) */

#define shrznZri(RG, IS)                                                    \
        EMIT6(MTM(0x0A, REG(RG),0x00,VAL(IS)))                              \
        EMITW(0xA70F0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define shrznZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x0A, TMxx,   0x00,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA70F0000 | TMxx << 20)                 /* <- set flags (Z) */

#define shrznZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMIT6(MTM(0x0A, REG(RG),REG(RS),0x00))                              \
        EMITW(0xA70F0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define shrznZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x04, TDxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MTM(0x0A, REG(RG),TDxx,   0x00))                              \
        EMITW(0xA70F0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define shrznZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x0A, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA70F0000 | TMxx << 20)                 /* <- set flags (Z) */

#define shrznZmr(MG, DG, RS)                                                \
        shrznZst(W(RS), W(MG), W(DG))

/* ror (G = G >> S | G << 32 - S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define rorzx_rx(RG)                     /* reads Recx for shift count */   \
        EMITH(MRM(0x17, TDxx,   TDxx,   0x00))                              \
        EMITH(MRM(0x1B, TDxx,   TEcx,   0x00))                              \
        EMIT6(MTM(0x1C, REG(RG),TDxx,   0x00))

#define rorzx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        negwx_rx(Recx)                                                      \
        EMIT6(MTM(0x1C, TMxx,   TEcx,   0x00))                              \
        negwx_rx(Recx)                                                      \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define rorzx_ri(RG, IS)                                                    \
        EMIT6(MTM(0x1C, REG(RG),0x00,   (64-VAL(IS)) & 0x3F))

#define rorzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x1C, TMxx,   0x00,   (64-VAL(IS)) & 0x3F))               \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define rorzx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITH(MRM(0x17, TDxx,   TDxx,   0x00))                              \
        EMITH(MRM(0x1B, TDxx,   REG(RS),0x00))                              \
        EMIT6(MTM(0x1C, REG(RG),TDxx,   0x00))

#define rorzx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x17, TDxx,   TDxx,   0x00))                              \
        EMITH(MRM(0x1B, TDxx,   TMxx,   0x00))                              \
        EMIT6(MTM(0x1C, REG(RG),TDxx,   0x00))

#define rorzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        negwx_rx(W(RS))                                                     \
        EMIT6(MTM(0x1C, TMxx,   REG(RS),0x00))                              \
        negwx_rx(W(RS))                                                     \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define rorzx_mr(MG, DG, RS)                                                \
        rorzx_st(W(RS), W(MG), W(DG))


#define rorzxZrx(RG)                     /* reads Recx for shift count */   \
        EMITH(MRM(0x17, TDxx,   TDxx,   0x00))                              \
        EMITH(MRM(0x1B, TDxx,   TEcx,   0x00))                              \
        EMIT6(MTM(0x1C, REG(RG),TDxx,   0x00))                              \
        EMITW(0xA70F0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define rorzxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        negwx_rx(Recx)                                                      \
        EMIT6(MTM(0x1C, TMxx,   TEcx,   0x00))                              \
        negwx_rx(Recx)                                                      \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA70F0000 | TMxx << 20)                 /* <- set flags (Z) */

#define rorzxZri(RG, IS)                                                    \
        EMIT6(MTM(0x1C, REG(RG),0x00,   (64-VAL(IS)) & 0x3F))               \
        EMITW(0xA70F0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define rorzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x1C, TMxx,   0x00,   (64-VAL(IS)) & 0x3F))               \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA70F0000 | TMxx << 20)                 /* <- set flags (Z) */

#define rorzxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITH(MRM(0x17, TDxx,   TDxx,   0x00))                              \
        EMITH(MRM(0x1B, TDxx,   REG(RS),0x00))                              \
        EMIT6(MTM(0x1C, REG(RG),TDxx,   0x00))                              \
        EMITW(0xA70F0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define rorzxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x17, TDxx,   TDxx,   0x00))                              \
        EMITH(MRM(0x1B, TDxx,   TMxx,   0x00))                              \
        EMIT6(MTM(0x1C, REG(RG),TDxx,   0x00))                              \
        EMITW(0xA70F0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define rorzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        negwx_rx(W(RS))                                                     \
        EMIT6(MTM(0x1C, TMxx,   REG(RS),0x00))                              \
        negwx_rx(W(RS))                                                     \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA70F0000 | TMxx << 20)                 /* <- set flags (Z) */

#define rorzxZmr(MG, DG, RS)                                                \
        rorzxZst(W(RS), W(MG), W(DG))

/* mul (G = G * S)
 * set-flags: undefined */

#define mulzx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC2, REG(RG),0x00,VAL(IS)))

#define mulzx_rr(RG, RS)                                                    \
        EMITW(MGM(0xED, REG(RG),REG(RS),REG(RG)))

#define mulzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x0C, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define mulwn_ld(RG, MS, DS)  /* mul 64-bit with 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x1C, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define mulwz_ld(RG, MS, DS)  /* mul 64-bit with 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x51, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0xED, REG(RG),TMxx,   REG(RG)))


#define mulzx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(MGM(0x86, TEdx,REG(RS),   0x00))

#define mulzx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x86, TEdx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))


#define mulzn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(MGM(0xEC, TEdx,REG(RS),   TEdx))

#define mulzn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x84, TEdx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))


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


#define prezx_xx()   /* to be placed right before divzx_x* or remzx_xx */   \
        movzx_ri(Redx, IC(0))        /* to prepare Redx for int-divide */

#define prezn_xx()   /* to be placed right before divzn_x* or remzn_xx */   \
                                     /* to prepare Redx for int-divide */


#define divzx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(MGM(0x87, TEdx,REG(RS),   0x00))

#define divzx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x87, TEdx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))


#define divzn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(MGM(0x0D, TEdx,REG(RS),   0x00))

#define divzn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x0D, TEdx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))


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


#define remzx_xx() /* to be placed before divzx_x*, but after prezx_xx */   \
                                     /* to prepare for rem calculation */

#define remzx_xr(RS)        /* to be placed immediately after divzx_xr */   \
                                     /* to produce remainder Redx<-rem */

#define remzx_xm(MS, DS)    /* to be placed immediately after divzx_xm */   \
                                     /* to produce remainder Redx<-rem */


#define remzn_xx() /* to be placed before divzn_x*, but after prezn_xx */   \
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
        EMIT6(MIM(0xC0, TDxx,   0x01,VAL(IT)))                              \
        CXR(cc, MOD(RS), TDxx,   lb)

#define cmjzx_mi(MS, DS, IT, cc, lb)                                        \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MIM(0xC0, TDxx,   0x01,VAL(IT)))                              \
        CXR(cc, %%r0,   TDxx,    lb)

#define cmjzx_rr(RS, RT, cc, lb)                                            \
        CXR(cc, MOD(RS), MOD(RT), lb)

#define cmjzx_rm(RS, MT, DT, cc, lb)                                        \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MT),REG(MT), VAL(DT), B1(DT), P1(DT)))  \
        CXR(cc, MOD(RS), %%r0,   lb)

#define cmjwn_rm(RS, MT, DT, cc, lb)   /* cmj 64/32-bit with sign-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMIT6(MDM(0x14, TMxx,   MOD(MT),REG(MT), VAL(DT), B1(DT), P1(DT)))  \
        CXR(cc, MOD(RS), %%r0,   lb)

#define cmjwz_rm(RS, MT, DT, cc, lb)   /* cmj 64/32-bit with zero-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMIT6(MDM(0x16, TMxx,   MOD(MT),REG(MT), VAL(DT), B1(DT), P1(DT)))  \
        CXR(cc, MOD(RS), %%r0,   lb)

#define cmjzx_mr(MS, DS, RT, cc, lb)                                        \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        CXR(cc, %%r0,   MOD(RT), lb)

#define cmjwn_mr(MS, DS, RT, cc, lb)   /* cmj 32/64-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x14, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        CXR(cc, %%r0,   MOD(RT), lb)

#define cmjwz_mr(MS, DS, RT, cc, lb)   /* cmj 32/64-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x16, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        CXR(cc, %%r0,   MOD(RT), lb)

/* cmp (flags = S ? T)
 * set-flags: yes */

#define cmpzx_ri(RS, IT)                                                    \
        EMITW(MGM(0x04, TLxx,   REG(RS),0x00))                              \
        EMIT6(MIM(0xC0, TRxx,   0x01,VAL(IT)))

#define cmpzx_mi(MS, DS, IT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x04, TLxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MIM(0xC0, TRxx,   0x01,VAL(IT)))

#define cmpzx_rr(RS, RT)                                                    \
        EMITW(MGM(0x04, TRxx,   REG(RT),0x00))                              \
        EMITW(MGM(0x04, TLxx,   REG(RS),0x00))

#define cmpzx_rm(RS, MT, DT)                                                \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMIT6(MDM(0x04, TRxx,   MOD(MT),REG(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(MGM(0x04, TLxx,   REG(RS),0x00))

#define cmpwn_rm(RS, MT, DT)    /* cmp 64-bit to 32-bit with sign-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMIT6(MDM(0x14, TRxx,   MOD(MT),REG(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(MGM(0x04, TLxx,   REG(RS),0x00))

#define cmpwz_rm(RS, MT, DT)    /* cmp 64-bit to 32-bit with zero-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMIT6(MDM(0x16, TRxx,   MOD(MT),REG(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(MGM(0x04, TLxx,   REG(RS),0x00))

#define cmpzx_mr(MS, DS, RT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x04, TLxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0x04, TRxx,   REG(RT),0x00))

#define cmpwn_mr(MS, DS, RT)    /* cmp 32-bit to 64-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x14, TLxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0x04, TRxx,   REG(RT),0x00))

#define cmpwz_mr(MS, DS, RT)    /* cmp 32-bit to 64-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x16, TLxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0x04, TRxx,   REG(RT),0x00))

#endif /* (defined RT_Z64) */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/************************ internal definitions for cmj ************************/

#define RXJ0(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(cgrje,   r1, r2, lb) ASM_END

#define RXJ1(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(cgrjne,  r1, r2, lb) ASM_END

#define RXJ2(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(clgrjl,  r1, r2, lb) ASM_END

#define RXJ3(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(clgrjle, r1, r2, lb) ASM_END

#define RXJ4(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(clgrjh,  r1, r2, lb) ASM_END

#define RXJ5(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(clgrjhe, r1, r2, lb) ASM_END

#define RXJ6(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(cgrjl,   r1, r2, lb) ASM_END

#define RXJ7(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(cgrjle,  r1, r2, lb) ASM_END

#define RXJ8(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(cgrjh,   r1, r2, lb) ASM_END

#define RXJ9(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(cgrjhe,  r1, r2, lb) ASM_END

#define CXR(cc, r1, r2, lb)                                                 \
        RX##cc(r1, r2, lb)

#endif /* RT_RTARCH_Z64_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

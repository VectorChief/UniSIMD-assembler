/******************************************************************************/
/* Copyright (c) 2013-2021 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X32_H
#define RT_RTARCH_X32_H

#define RT_BASE_REGS        16

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_x32.h: Implementation of x86_64 32-bit BASE instructions.
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

/* structural */

#define REX(rxg, rxm) /* W0 */                                              \
        EMITB(0x40 | (rxg) << 2 | (rxm))

#define REW(rxg, rxm) /* W1 */                                              \
        EMITB(0x48 | (rxg) << 2 | (rxm))

#define MRM(reg, mod, rem)                                                  \
        EMITB((mod) << 6 | (reg) << 3 | (rem))

#define AUX(sib, cdp, cim)  sib  cdp  cim

#if   (defined RT_X32)

#ifdef RT_BASE_COMPAT_BMI
#undef  RT_X32
#define RT_X32 RT_BASE_COMPAT_BMI /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */
#endif /* RT_BASE_COMPAT_BMI */

#define ADR                                                                 \
        EMITB(0x67)

#define x67 1

#elif (defined RT_X64)

#ifdef RT_BASE_COMPAT_BMI
#undef  RT_X64
#define RT_X64 RT_BASE_COMPAT_BMI /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */
#endif /* RT_BASE_COMPAT_BMI */

#define ADR

#define x67 0

#endif /* defined (RT_X32, RT_X64) */

/* mandatory escape prefix for some opcodes (must preceed rex) */
#define ESC                                                                 \
        EMITB(0x66)

/* fwait instruction for legacy processors (fix for fstcw) */
#define FWT                                                                 \
        EMITB(0x9B)

/* 3-byte VEX prefix with full customization (W0) */
#define VEX(rxg, rxm, ren, len, pfx, aux)                                   \
        EMITB(0xC4)                                                         \
        EMITB(0x00 | (1 - (rxg)) << 7 | 1 << 6 | (1 - (rxm)) << 5 | (aux))  \
        EMITB(0x00 | (len) << 2 | (0x0F - (ren)) << 3 | (pfx))

/* 3-byte VEX prefix with full customization (W1) */
#define VEW(rxg, rxm, ren, len, pfx, aux)                                   \
        EMITB(0xC4)                                                         \
        EMITB(0x00 | (1 - (rxg)) << 7 | 1 << 6 | (1 - (rxm)) << 5 | (aux))  \
        EMITB(0x80 | (len) << 2 | (0x0F - (ren)) << 3 | (pfx))

/* 4-byte EVEX prefix with full customization (W0, K0) */
#define EVX(rxg, rxm, ren, len, pfx, aux)                                   \
        EMITB(0x62)                                                         \
        EMITB(0x00 | (1-((rxg)&1))<<7|(2-((rxg)&2))<<3|(3-(rxm))<<5|(aux))  \
        EMITB(0x04 | (15-((ren)&15))<<3 | (pfx))                            \
        EMITB(0x00 | (16-((ren)&16))>>1 | (len) << 5)

/* 4-byte EVEX prefix with full customization (W1, K0) */
#define EVW(rxg, rxm, ren, len, pfx, aux)                                   \
        EMITB(0x62)                                                         \
        EMITB(0x00 | (1-((rxg)&1))<<7|(2-((rxg)&2))<<3|(3-(rxm))<<5|(aux))  \
        EMITB(0x84 | (15-((ren)&15))<<3 | (pfx))                            \
        EMITB(0x00 | (16-((ren)&16))>>1 | (len) << 5)

/* 4-byte EVEX prefix with full customization (W0, K0), fp16 */
#define EFX(rxg, rxm, ren, len, pfx, aux)                                   \
        EMITB(0x62)                                                         \
        EMITB(0x04 | (1-((rxg)&1))<<7|(2-((rxg)&2))<<3|(3-(rxm))<<5|(aux))  \
        EMITB(0x04 | (15-((ren)&15))<<3 | (pfx))                            \
        EMITB(0x00 | (16-((ren)&16))>>1 | (len) << 5)

/* 4-byte EVEX prefix with full customization (W1, K0), fp16 */
#define EFW(rxg, rxm, ren, len, pfx, aux)                                   \
        EMITB(0x62)                                                         \
        EMITB(0x04 | (1-((rxg)&1))<<7|(2-((rxg)&2))<<3|(3-(rxm))<<5|(aux))  \
        EMITB(0x84 | (15-((ren)&15))<<3 | (pfx))                            \
        EMITB(0x00 | (16-((ren)&16))>>1 | (len) << 5)

/* 4-byte EVEX prefix with full customization (W0, K1, Z0) */
#define EKX(rxg, rxm, ren, len, pfx, aux)                                   \
        EMITB(0x62)                                                         \
        EMITB(0x00 | (1-((rxg)&1))<<7|(2-((rxg)&2))<<3|(3-(rxm))<<5|(aux))  \
        EMITB(0x04 | (15-((ren)&15))<<3 | (pfx))                            \
        EMITB(0x01 | (16-((ren)&16))>>1 | (len) << 5)

/* 4-byte EVEX prefix with full customization (W1, K1, Z0) */
#define EKW(rxg, rxm, ren, len, pfx, aux)                                   \
        EMITB(0x62)                                                         \
        EMITB(0x00 | (1-((rxg)&1))<<7|(2-((rxg)&2))<<3|(3-(rxm))<<5|(aux))  \
        EMITB(0x84 | (15-((ren)&15))<<3 | (pfx))                            \
        EMITB(0x01 | (16-((ren)&16))>>1 | (len) << 5)

/* 4-byte EVEX prefix with full customization (W0, K1, Z1) */
#define EZX(rxg, rxm, ren, len, pfx, aux)                                   \
        EMITB(0x62)                                                         \
        EMITB(0x00 | (1-((rxg)&1))<<7|(2-((rxg)&2))<<3|(3-(rxm))<<5|(aux))  \
        EMITB(0x04 | (15-((ren)&15))<<3 | (pfx))                            \
        EMITB(0x81 | (16-((ren)&16))>>1 | (len) << 5)

/* 4-byte EVEX prefix with full customization (W1, K1, Z1) */
#define EZW(rxg, rxm, ren, len, pfx, aux)                                   \
        EMITB(0x62)                                                         \
        EMITB(0x00 | (1-((rxg)&1))<<7|(2-((rxg)&2))<<3|(3-(rxm))<<5|(aux))  \
        EMITB(0x84 | (15-((ren)&15))<<3 | (pfx))                            \
        EMITB(0x81 | (16-((ren)&16))>>1 | (len) << 5)

/* 4-byte EVEX prefix with full customization (W0, B1, RM) */
#define ERX(rxg, rxm, ren, erm, pfx, aux)                                   \
        EMITB(0x62)                                                         \
        EMITB(0x00 | (1-((rxg)&1))<<7|(2-((rxg)&2))<<3|(3-(rxm))<<5|(aux))  \
        EMITB(0x04 | (15-((ren)&15))<<3 | (pfx))                            \
        EMITB(0x10 | (16-((ren)&16))>>1 | (erm) << 5)

/* 4-byte EVEX prefix with full customization (W1, B1, RM) */
#define ERW(rxg, rxm, ren, erm, pfx, aux)                                   \
        EMITB(0x62)                                                         \
        EMITB(0x00 | (1-((rxg)&1))<<7|(2-((rxg)&2))<<3|(3-(rxm))<<5|(aux))  \
        EMITB(0x84 | (15-((ren)&15))<<3 | (pfx))                            \
        EMITB(0x10 | (16-((ren)&16))>>1 | (erm) << 5)

/* 4-byte EVEX prefix with full customization (W0, B1, RM), fp16 */
#define EGX(rxg, rxm, ren, erm, pfx, aux)                                   \
        EMITB(0x62)                                                         \
        EMITB(0x04 | (1-((rxg)&1))<<7|(2-((rxg)&2))<<3|(3-(rxm))<<5|(aux))  \
        EMITB(0x04 | (15-((ren)&15))<<3 | (pfx))                            \
        EMITB(0x10 | (16-((ren)&16))>>1 | (erm) << 5)

/* 4-byte EVEX prefix with full customization (W1, B1, RM), fp16 */
#define EGW(rxg, rxm, ren, erm, pfx, aux)                                   \
        EMITB(0x62)                                                         \
        EMITB(0x04 | (1-((rxg)&1))<<7|(2-((rxg)&2))<<3|(3-(rxm))<<5|(aux))  \
        EMITB(0x84 | (15-((ren)&15))<<3 | (pfx))                            \
        EMITB(0x10 | (16-((ren)&16))>>1 | (erm) << 5)

/* selectors  */

#define RXB(reg, mod, sib)  ((reg) >> 3 & 0x03) /* full-reg-extension-bit */
#define RMB(reg, mod, sib)  ((reg) >> 3 | 0x02) /* reg-bank-extension-bit */
#define REG(reg, mod, sib)  ((reg) >> 0 & 0x07) /* register, lower 3-bits */
#define REH(reg, mod, sib)  (((reg) & 0x07)+ 8) /* 2nd 8-reg-bank, 4-bits */
#define REI(reg, mod, sib)  (((reg) & 0x07)+16) /* 3rd 8-reg-bank, 5-bits */
#define REJ(reg, mod, sib)  (((reg) & 0x07)+24) /* 4th 8-reg-bank, 5-bits */
#define REN(reg, mod, sib)  (reg) /* 3rd operand, full-reg-bank, 4/5-bits */
#define REM(reg, mod, sib)  (((reg) & 0x0F)+16) /* 2nd 16-reg-bank 5-bits */
#define MOD(reg, mod, sib)  mod
#define SIB(reg, mod, sib)  sib

#define VAL(val, typ, cmd)  val
#define VYL(val, typ, cmd)  ((val) | 0x10)
#define VXL(val, typ, cmd)  ((val) | 0x20)
#define VZL(val, typ, cmd)  ((val) | 0x40)
#define VSL(val, typ, cmd)  ((val) | 0x80)
#define VTL(val, typ, cmd)  ((val) | 0xC0)
#define TYP(val, typ, cmd)  typ
#define CMD(val, typ, cmd)  cmd

/* register-bank pass-through selectors */

#define V(reg, mod, sib)   (reg+8), mod, sib
#define X(reg, mod, sib)  (reg+16), mod, sib
#define Z(reg, mod, sib)  (reg+24), mod, sib

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/* registers    REG,  MOD,  SIB */

#define Reax    0x00, 0x03, EMPTY
#define Recx    0x01, 0x03, EMPTY
#define Redx    0x02, 0x03, EMPTY
#define Rebx    0x03, 0x03, EMPTY
#define Rebp    0x05, 0x03, EMPTY
#define Resi    0x06, 0x03, EMPTY
#define Redi    0x07, 0x03, EMPTY
#define Reg8    0x08, 0x03, EMPTY
#define Reg9    0x09, 0x03, EMPTY
#define RegA    0x0A, 0x03, EMPTY
#define RegB    0x0B, 0x03, EMPTY
#define RegC    0x0C, 0x03, EMPTY
#define RegD    0x0D, 0x03, EMPTY
#define RegE    0x0E, 0x03, EMPTY

/* addressing   REG,  MOD,  SIB */

#define Oeax    0x00, 0x00, EMPTY       /* [eax] */

#define Mecx    0x01, 0x02, EMPTY       /* [ecx + DP] */
#define Medx    0x02, 0x02, EMPTY       /* [edx + DP] */
#define Mebx    0x03, 0x02, EMPTY       /* [ebx + DP] */
#define Mebp    0x05, 0x02, EMPTY       /* [ebp + DP] */
#define Mesi    0x06, 0x02, EMPTY       /* [esi + DP] */
#define Medi    0x07, 0x02, EMPTY       /* [edi + DP] */
#define Meg8    0x08, 0x02, EMPTY       /* [r8d + DP] */
#define Meg9    0x09, 0x02, EMPTY       /* [r9d + DP] */
#define MegA    0x0A, 0x02, EMPTY       /* [r10d + DP] */
#define MegB    0x0B, 0x02, EMPTY       /* [r11d + DP] */
#define MegC    0x0C, 0x02, EMPTY       /* [r12d + DP] */
#define MegD    0x0D, 0x02, EMPTY       /* [r13d + DP] */
#define MegE    0x0E, 0x02, EMPTY       /* [r14d + DP] */

#define Iecx    0x04, 0x02, EMITB(0x01) /* [ecx + eax*1 + DP] */
#define Iedx    0x04, 0x02, EMITB(0x02) /* [edx + eax*1 + DP] */
#define Iebx    0x04, 0x02, EMITB(0x03) /* [ebx + eax*1 + DP] */
#define Iebp    0x04, 0x02, EMITB(0x05) /* [ebp + eax*1 + DP] */
#define Iesi    0x04, 0x02, EMITB(0x06) /* [esi + eax*1 + DP] */
#define Iedi    0x04, 0x02, EMITB(0x07) /* [edi + eax*1 + DP] */
#define Ieg8    0x0C, 0x02, EMITB(0x00) /* [r8d + eax*1 + DP] */
#define Ieg9    0x0C, 0x02, EMITB(0x01) /* [r9d + eax*1 + DP] */
#define IegA    0x0C, 0x02, EMITB(0x02) /* [r10d + eax*1 + DP] */
#define IegB    0x0C, 0x02, EMITB(0x03) /* [r11d + eax*1 + DP] */
#define IegC    0x0C, 0x02, EMITB(0x04) /* [r12d + eax*1 + DP] */
#define IegD    0x0C, 0x02, EMITB(0x05) /* [r13d + eax*1 + DP] */
#define IegE    0x0C, 0x02, EMITB(0x06) /* [r14d + eax*1 + DP] */

#define Jecx    0x04, 0x02, EMITB(0x41) /* [ecx + eax*2 + DP] */
#define Jedx    0x04, 0x02, EMITB(0x42) /* [edx + eax*2 + DP] */
#define Jebx    0x04, 0x02, EMITB(0x43) /* [ebx + eax*2 + DP] */
#define Jebp    0x04, 0x02, EMITB(0x45) /* [ebp + eax*2 + DP] */
#define Jesi    0x04, 0x02, EMITB(0x46) /* [esi + eax*2 + DP] */
#define Jedi    0x04, 0x02, EMITB(0x47) /* [edi + eax*2 + DP] */
#define Jeg8    0x0C, 0x02, EMITB(0x40) /* [r8d + eax*2 + DP] */
#define Jeg9    0x0C, 0x02, EMITB(0x41) /* [r9d + eax*2 + DP] */
#define JegA    0x0C, 0x02, EMITB(0x42) /* [r10d + eax*2 + DP] */
#define JegB    0x0C, 0x02, EMITB(0x43) /* [r11d + eax*2 + DP] */
#define JegC    0x0C, 0x02, EMITB(0x44) /* [r12d + eax*2 + DP] */
#define JegD    0x0C, 0x02, EMITB(0x45) /* [r13d + eax*2 + DP] */
#define JegE    0x0C, 0x02, EMITB(0x46) /* [r14d + eax*2 + DP] */

#define Kecx    0x04, 0x02, EMITB(0x81) /* [ecx + eax*4 + DP] */
#define Kedx    0x04, 0x02, EMITB(0x82) /* [edx + eax*4 + DP] */
#define Kebx    0x04, 0x02, EMITB(0x83) /* [ebx + eax*4 + DP] */
#define Kebp    0x04, 0x02, EMITB(0x85) /* [ebp + eax*4 + DP] */
#define Kesi    0x04, 0x02, EMITB(0x86) /* [esi + eax*4 + DP] */
#define Kedi    0x04, 0x02, EMITB(0x87) /* [edi + eax*4 + DP] */
#define Keg8    0x0C, 0x02, EMITB(0x80) /* [r8d + eax*4 + DP] */
#define Keg9    0x0C, 0x02, EMITB(0x81) /* [r9d + eax*4 + DP] */
#define KegA    0x0C, 0x02, EMITB(0x82) /* [r10d + eax*4 + DP] */
#define KegB    0x0C, 0x02, EMITB(0x83) /* [r11d + eax*4 + DP] */
#define KegC    0x0C, 0x02, EMITB(0x84) /* [r12d + eax*4 + DP] */
#define KegD    0x0C, 0x02, EMITB(0x85) /* [r13d + eax*4 + DP] */
#define KegE    0x0C, 0x02, EMITB(0x86) /* [r14d + eax*4 + DP] */

#define Lecx    0x04, 0x02, EMITB(0xC1) /* [ecx + eax*8 + DP] */
#define Ledx    0x04, 0x02, EMITB(0xC2) /* [edx + eax*8 + DP] */
#define Lebx    0x04, 0x02, EMITB(0xC3) /* [ebx + eax*8 + DP] */
#define Lebp    0x04, 0x02, EMITB(0xC5) /* [ebp + eax*8 + DP] */
#define Lesi    0x04, 0x02, EMITB(0xC6) /* [esi + eax*8 + DP] */
#define Ledi    0x04, 0x02, EMITB(0xC7) /* [edi + eax*8 + DP] */
#define Leg8    0x0C, 0x02, EMITB(0xC0) /* [r8d + eax*8 + DP] */
#define Leg9    0x0C, 0x02, EMITB(0xC1) /* [r9d + eax*8 + DP] */
#define LegA    0x0C, 0x02, EMITB(0xC2) /* [r10d + eax*8 + DP] */
#define LegB    0x0C, 0x02, EMITB(0xC3) /* [r11d + eax*8 + DP] */
#define LegC    0x0C, 0x02, EMITB(0xC4) /* [r12d + eax*8 + DP] */
#define LegD    0x0C, 0x02, EMITB(0xC5) /* [r13d + eax*8 + DP] */
#define LegE    0x0C, 0x02, EMITB(0xC6) /* [r14d + eax*8 + DP] */

/* immediate    VAL,  TYP,  CMD            (all immediate types are unsigned) */
/* full-size IW type is only applicable within cmdw* subset, can set sign-bit */
/* within cmdz* subset use of IW type is limited to movzx_rj/_mj instructions */

#define  IC(im) (im), 0x02, EMITB((im) & 0x7F) /* drop sign-ext (zero on ARM) */
#define  IB(im) (im), 0x00, EMITW((im) & 0xFF) /* drop sign-ext (32-bit word) */
#define  IM(im) (im), 0x00, EMITW((im) & 0xFFF) /* native AArch64 add/sub/cmp */
#define  IG(im) (im), 0x00, EMITW((im) & 0x7FFF) /* native MIPS64 add/sub/cmp */
#define  IH(im) (im), 0x00, EMITW((im) & 0xFFFF) /* second native on all ARMs */
#define  IV(im) (im), 0x00, EMITW((im) & 0x7FFFFFFF)  /* native x64 long mode */
#define  IW(im) (im), 0x00, EMITW((im) & 0xFFFFFFFF)  /* for cmdw* subset, *j */

/* displacement VAL,  TYP,  CMD         (all displacement types are unsigned) */
/* public scalable DP/DE/DF/DG/DH/DV definitions are now provided in rtbase.h */
/* as D* are used for BASE and SIMD instructions, only limits are SIMD-scaled */

#define _DP(dp) (dp), 0x00, EMITW((dp) & 0xFFF)   /* native on all ARMs, MIPS */
#define _DE(dp) (dp), 0x00, EMITW((dp) & 0x1FFF) /* AArch64 256-bit SVE ld/st */
#define _DF(dp) (dp), 0x00, EMITW((dp) & 0x3FFF) /* native AArch64 BASE ld/st */
#define _DG(dp) (dp), 0x00, EMITW((dp) & 0x7FFF)  /* native MIPS64 BASE ld/st */
#define _DH(dp) (dp), 0x00, EMITW((dp) & 0xFFFF) /* second native on all ARMs */
#define _DV(dp) (dp), 0x00, EMITW((dp) & 0x7FFFFFFF)  /* native x64 long mode */
#define  PLAIN  0x00, 0x00, EMPTY    /* special type for Oeax addressing mode */

/* triplet pass-through wrapper */

#define W(p1, p2, p3)       p1,  p2,  p3

/******************************************************************************/
/**********************************   BASE   **********************************/
/******************************************************************************/

/* mov (D = S)
 * set-flags: no */

#define movwx_ri(RD, IS)                                                    \
        REX(0,       RXB(RD)) EMITB(0xC7)                                   \
        MRM(0x00,    MOD(RD), REG(RD))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITW(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define movwx_mi(MD, DD, IS)                                                \
    ADR REX(0,       RXB(MD)) EMITB(0xC7)                                   \
        MRM(0x00,    MOD(MD), REG(MD))   /* truncate IC with TYP below */   \
        AUX(SIB(MD), CMD(DD), EMITW(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define movwx_rr(RD, RS)                                                    \
        REX(RXB(RD), RXB(RS)) EMITB(0x8B)                                   \
        MRM(REG(RD), MOD(RS), REG(RS))

#define movhn_rr(RD, RS)      /* move 16-bit to 32/64-bit w/ sign-extend */ \
        REW(RXB(RD), RXB(RS)) EMITB(0x0F) EMITB(0xBF)                       \
        MRM(REG(RD), MOD(RS), REG(RS))

#define movhz_rr(RD, RS)      /* move 16-bit to 32/64-bit w/ zero-extend */ \
        REW(RXB(RD), RXB(RS)) EMITB(0x0F) EMITB(0xB7)                       \
        MRM(REG(RD), MOD(RS), REG(RS))

#define movbn_rr(RD, RS)      /* move  8-bit to 32/64-bit w/ sign-extend */ \
        REW(RXB(RD), RXB(RS)) EMITB(0x0F) EMITB(0xBE)                       \
        MRM(REG(RD), MOD(RS), REG(RS))

#define movbz_rr(RD, RS)      /* move  8-bit to 32/64-bit w/ zero-extend */ \
        REW(RXB(RD), RXB(RS)) EMITB(0x0F) EMITB(0xB6)                       \
        MRM(REG(RD), MOD(RS), REG(RS))

#define movwx_ld(RD, MS, DS)                                                \
    ADR REX(RXB(RD), RXB(MS)) EMITB(0x8B)                                   \
        MRM(REG(RD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movhn_ld(RD, MS, DS)  /* load 16-bit to 32/64-bit w/ sign-extend */ \
    ADR REW(RXB(RD), RXB(MS)) EMITB(0x0F) EMITB(0xBF)                       \
        MRM(REG(RD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movhz_ld(RD, MS, DS)  /* load 16-bit to 32/64-bit w/ zero-extend */ \
    ADR REW(RXB(RD), RXB(MS)) EMITB(0x0F) EMITB(0xB7)                       \
        MRM(REG(RD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movbn_ld(RD, MS, DS)  /* load  8-bit to 32/64-bit w/ sign-extend */ \
    ADR REW(RXB(RD), RXB(MS)) EMITB(0x0F) EMITB(0xBE)                       \
        MRM(REG(RD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movbz_ld(RD, MS, DS)  /* load  8-bit to 32/64-bit w/ zero-extend */ \
    ADR REW(RXB(RD), RXB(MS)) EMITB(0x0F) EMITB(0xB6)                       \
        MRM(REG(RD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movwx_st(RS, MD, DD)                                                \
    ADR REX(RXB(RS), RXB(MD)) EMITB(0x89)                                   \
        MRM(REG(RS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)


#define movwx_rj(RD, IT, IS)     /* IT - upper 32-bit, IS - lower 32-bit */ \
        movwx_ri(W(RD), W(IS))

#define movwx_mj(MD, DD, IT, IS) /* IT - upper 32-bit, IS - lower 32-bit */ \
        movwx_mi(W(MD), W(DD), W(IS))

/* and (G = G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define andwx_ri(RG, IS)                                                    \
        andwxZri(W(RG), W(IS))

#define andwx_mi(MG, DG, IS)                                                \
        andwxZmi(W(MG), W(DG), W(IS))

#define andwx_rr(RG, RS)                                                    \
        andwxZrr(W(RG), W(RS))

#define andwx_ld(RG, MS, DS)                                                \
        andwxZld(W(RG), W(MS), W(DS))

#define andwx_st(RS, MG, DG)                                                \
        andwxZst(W(RS), W(MG), W(DG))

#define andwx_mr(MG, DG, RS)                                                \
        andwx_st(W(RS), W(MG), W(DG))


#define andwxZri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x04,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IS))

#define andwxZmi(MG, DG, IS)                                                \
    ADR REX(0,       RXB(MG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x04,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), CMD(IS))

#define andwxZrr(RG, RS)                                                    \
        REX(RXB(RG), RXB(RS)) EMITB(0x23)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define andwxZld(RG, MS, DS)                                                \
    ADR REX(RXB(RG), RXB(MS)) EMITB(0x23)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define andwxZst(RS, MG, DG)                                                \
    ADR REX(RXB(RS), RXB(MG)) EMITB(0x21)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define andwxZmr(MG, DG, RS)                                                \
        andwxZst(W(RS), W(MG), W(DG))

/* ann (G = ~G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define annwx_ri(RG, IS)                                                    \
        annwxZri(W(RG), W(IS))

#define annwx_mi(MG, DG, IS)                                                \
        annwxZmi(W(MG), W(DG), W(IS))

#define annwx_rr(RG, RS)                                                    \
        annwxZrr(W(RG), W(RS))

#define annwx_ld(RG, MS, DS)                                                \
        annwxZld(W(RG), W(MS), W(DS))

#define annwx_st(RS, MG, DG)                                                \
        annwxZst(W(RS), W(MG), W(DG))

#define annwx_mr(MG, DG, RS)                                                \
        annwx_st(W(RS), W(MG), W(DG))


#define annwxZri(RG, IS)                                                    \
        notwx_rx(W(RG))                                                     \
        andwxZri(W(RG), W(IS))

#define annwxZmi(MG, DG, IS)                                                \
        notwx_mx(W(MG), W(DG))                                              \
        andwxZmi(W(MG), W(DG), W(IS))

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define annwxZrr(RG, RS)                                                    \
        notwx_rx(W(RG))                                                     \
        andwxZrr(W(RG), W(RS))

#define annwxZld(RG, MS, DS)                                                \
        notwx_rx(W(RG))                                                     \
        andwxZld(W(RG), W(MS), W(DS))

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define annwxZrr(RG, RS)                                                    \
        VEX(RXB(RG), RXB(RS), REN(RG), 0, 0, 2) EMITB(0xF2)                 \
        MRM(REG(RG), MOD(RS), REG(RS))

#define annwxZld(RG, MS, DS)                                                \
        VEX(RXB(RG), RXB(MS), REN(RG), 0, 0, 2) EMITB(0xF2)                 \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define annwxZst(RS, MG, DG)                                                \
        notwx_mx(W(MG), W(DG))                                              \
        andwxZst(W(RS), W(MG), W(DG))

#define annwxZmr(MG, DG, RS)                                                \
        annwxZst(W(RS), W(MG), W(DG))

/* orr (G = G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define orrwx_ri(RG, IS)                                                    \
        orrwxZri(W(RG), W(IS))

#define orrwx_mi(MG, DG, IS)                                                \
        orrwxZmi(W(MG), W(DG), W(IS))

#define orrwx_rr(RG, RS)                                                    \
        orrwxZrr(W(RG), W(RS))

#define orrwx_ld(RG, MS, DS)                                                \
        orrwxZld(W(RG), W(MS), W(DS))

#define orrwx_st(RS, MG, DG)                                                \
        orrwxZst(W(RS), W(MG), W(DG))

#define orrwx_mr(MG, DG, RS)                                                \
        orrwx_st(W(RS), W(MG), W(DG))


#define orrwxZri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x01,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IS))

#define orrwxZmi(MG, DG, IS)                                                \
    ADR REX(0,       RXB(MG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x01,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), CMD(IS))

#define orrwxZrr(RG, RS)                                                    \
        REX(RXB(RG), RXB(RS)) EMITB(0x0B)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define orrwxZld(RG, MS, DS)                                                \
    ADR REX(RXB(RG), RXB(MS)) EMITB(0x0B)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define orrwxZst(RS, MG, DG)                                                \
    ADR REX(RXB(RS), RXB(MG)) EMITB(0x09)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define orrwxZmr(MG, DG, RS)                                                \
        orrwxZst(W(RS), W(MG), W(DG))

/* orn (G = ~G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define ornwx_ri(RG, IS)                                                    \
        ornwxZri(W(RG), W(IS))

#define ornwx_mi(MG, DG, IS)                                                \
        ornwxZmi(W(MG), W(DG), W(IS))

#define ornwx_rr(RG, RS)                                                    \
        ornwxZrr(W(RG), W(RS))

#define ornwx_ld(RG, MS, DS)                                                \
        ornwxZld(W(RG), W(MS), W(DS))

#define ornwx_st(RS, MG, DG)                                                \
        ornwxZst(W(RS), W(MG), W(DG))

#define ornwx_mr(MG, DG, RS)                                                \
        ornwx_st(W(RS), W(MG), W(DG))


#define ornwxZri(RG, IS)                                                    \
        notwx_rx(W(RG))                                                     \
        orrwxZri(W(RG), W(IS))

#define ornwxZmi(MG, DG, IS)                                                \
        notwx_mx(W(MG), W(DG))                                              \
        orrwxZmi(W(MG), W(DG), W(IS))

#define ornwxZrr(RG, RS)                                                    \
        notwx_rx(W(RG))                                                     \
        orrwxZrr(W(RG), W(RS))

#define ornwxZld(RG, MS, DS)                                                \
        notwx_rx(W(RG))                                                     \
        orrwxZld(W(RG), W(MS), W(DS))

#define ornwxZst(RS, MG, DG)                                                \
        notwx_mx(W(MG), W(DG))                                              \
        orrwxZst(W(RS), W(MG), W(DG))

#define ornwxZmr(MG, DG, RS)                                                \
        ornwxZst(W(RS), W(MG), W(DG))

/* xor (G = G ^ S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define xorwx_ri(RG, IS)                                                    \
        xorwxZri(W(RG), W(IS))

#define xorwx_mi(MG, DG, IS)                                                \
        xorwxZmi(W(MG), W(DG), W(IS))

#define xorwx_rr(RG, RS)                                                    \
        xorwxZrr(W(RG), W(RS))

#define xorwx_ld(RG, MS, DS)                                                \
        xorwxZld(W(RG), W(MS), W(DS))

#define xorwx_st(RS, MG, DG)                                                \
        xorwxZst(W(RS), W(MG), W(DG))

#define xorwx_mr(MG, DG, RS)                                                \
        xorwx_st(W(RS), W(MG), W(DG))


#define xorwxZri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x06,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IS))

#define xorwxZmi(MG, DG, IS)                                                \
    ADR REX(0,       RXB(MG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x06,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), CMD(IS))

#define xorwxZrr(RG, RS)                                                    \
        REX(RXB(RG), RXB(RS)) EMITB(0x33)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define xorwxZld(RG, MS, DS)                                                \
    ADR REX(RXB(RG), RXB(MS)) EMITB(0x33)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define xorwxZst(RS, MG, DG)                                                \
    ADR REX(RXB(RS), RXB(MG)) EMITB(0x31)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define xorwxZmr(MG, DG, RS)                                                \
        xorwxZst(W(RS), W(MG), W(DG))

/* not (G = ~G)
 * set-flags: no */

#define notwx_rx(RG)                                                        \
        REX(0,       RXB(RG)) EMITB(0xF7)                                   \
        MRM(0x02,    MOD(RG), REG(RG))

#define notwx_mx(MG, DG)                                                    \
    ADR REX(0,       RXB(MG)) EMITB(0xF7)                                   \
        MRM(0x02,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

/* neg (G = -G)
 * set-flags: undefined (*_*), yes (*Z*) */

#define negwx_rx(RG)                                                        \
        negwxZrx(W(RG))

#define negwx_mx(MG, DG)                                                    \
        negwxZmx(W(MG), W(DG))


#define negwxZrx(RG)                                                        \
        REX(0,       RXB(RG)) EMITB(0xF7)                                   \
        MRM(0x03,    MOD(RG), REG(RG))

#define negwxZmx(MG, DG)                                                    \
    ADR REX(0,       RXB(MG)) EMITB(0xF7)                                   \
        MRM(0x03,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

/* add (G = G + S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define addwx_ri(RG, IS)                                                    \
        addwxZri(W(RG), W(IS))

#define addwx_mi(MG, DG, IS)                                                \
        addwxZmi(W(MG), W(DG), W(IS))

#define addwx_rr(RG, RS)                                                    \
        addwxZrr(W(RG), W(RS))

#define addwx_ld(RG, MS, DS)                                                \
        addwxZld(W(RG), W(MS), W(DS))

#define addhn_ld(RG, MS, DS)    /* add 16-bit to 32-bit with sign-extend */ \
        addhnZld(W(RG), W(MS), W(DS))

#define addhz_ld(RG, MS, DS)    /* add 16-bit to 32-bit with zero-extend */ \
        addhzZld(W(RG), W(MS), W(DS))

#define addbn_ld(RG, MS, DS)    /* add  8-bit to 32-bit with sign-extend */ \
        addbnZld(W(RG), W(MS), W(DS))

#define addbz_ld(RG, MS, DS)    /* add  8-bit to 32-bit with zero-extend */ \
        addbzZld(W(RG), W(MS), W(DS))

#define addwx_st(RS, MG, DG)                                                \
        addwxZst(W(RS), W(MG), W(DG))

#define addwx_mr(MG, DG, RS)                                                \
        addwx_st(W(RS), W(MG), W(DG))


#define addwxZri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x00,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IS))

#define addwxZmi(MG, DG, IS)                                                \
    ADR REX(0,       RXB(MG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x00,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), CMD(IS))

#define addwxZrr(RG, RS)                                                    \
        REX(RXB(RG), RXB(RS)) EMITB(0x03)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define addwxZld(RG, MS, DS)                                                \
    ADR REX(RXB(RG), RXB(MS)) EMITB(0x03)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define addhnZld(RG, MS, DS)    /* add 16-bit to 32-bit with sign-extend */ \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xBF)                       \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)                                        \
        REX(RXB(RG),       1) EMITB(0x03)                                   \
        MRM(REG(RG),    0x03,    0x07)

#define addhzZld(RG, MS, DS)    /* add 16-bit to 32-bit with zero-extend */ \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xB7)                       \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)                                        \
        REX(RXB(RG),       1) EMITB(0x03)                                   \
        MRM(REG(RG),    0x03,    0x07)

#define addbnZld(RG, MS, DS)    /* add  8-bit to 32-bit with sign-extend */ \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xBE)                       \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)                                        \
        REX(RXB(RG),       1) EMITB(0x03)                                   \
        MRM(REG(RG),    0x03,    0x07)

#define addbzZld(RG, MS, DS)    /* add  8-bit to 32-bit with zero-extend */ \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xB6)                       \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)                                        \
        REX(RXB(RG),       1) EMITB(0x03)                                   \
        MRM(REG(RG),    0x03,    0x07)

#define addwxZst(RS, MG, DG)                                                \
    ADR REX(RXB(RS), RXB(MG)) EMITB(0x01)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define addwxZmr(MG, DG, RS)                                                \
        addwxZst(W(RS), W(MG), W(DG))

/* sub (G = G - S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define subwx_ri(RG, IS)                                                    \
        subwxZri(W(RG), W(IS))

#define subwx_mi(MG, DG, IS)                                                \
        subwxZmi(W(MG), W(DG), W(IS))

#define subwx_rr(RG, RS)                                                    \
        subwxZrr(W(RG), W(RS))

#define subwx_ld(RG, MS, DS)                                                \
        subwxZld(W(RG), W(MS), W(DS))

#define subhn_ld(RG, MS, DS)  /* sub 16-bit from 32-bit with sign-extend */ \
        subhnZld(W(RG), W(MS), W(DS))

#define subhz_ld(RG, MS, DS)  /* sub 16-bit from 32-bit with zero-extend */ \
        subhzZld(W(RG), W(MS), W(DS))

#define subbn_ld(RG, MS, DS)  /* sub  8-bit from 32-bit with sign-extend */ \
        subbnZld(W(RG), W(MS), W(DS))

#define subbz_ld(RG, MS, DS)  /* sub  8-bit from 32-bit with zero-extend */ \
        subbzZld(W(RG), W(MS), W(DS))

#define subwx_st(RS, MG, DG)                                                \
        subwxZst(W(RS), W(MG), W(DG))

#define subwx_mr(MG, DG, RS)                                                \
        subwx_st(W(RS), W(MG), W(DG))


#define subwxZri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x05,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IS))

#define subwxZmi(MG, DG, IS)                                                \
    ADR REX(0,       RXB(MG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x05,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), CMD(IS))

#define subwxZrr(RG, RS)                                                    \
        REX(RXB(RG), RXB(RS)) EMITB(0x2B)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define subwxZld(RG, MS, DS)                                                \
    ADR REX(RXB(RG), RXB(MS)) EMITB(0x2B)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define subhnZld(RG, MS, DS)  /* sub 16-bit from 32-bit with sign-extend */ \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xBF)                       \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)                                        \
        REX(RXB(RG),       1) EMITB(0x2B)                                   \
        MRM(REG(RG),    0x03,    0x07)

#define subhzZld(RG, MS, DS)  /* sub 16-bit from 32-bit with zero-extend */ \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xB7)                       \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)                                        \
        REX(RXB(RG),       1) EMITB(0x2B)                                   \
        MRM(REG(RG),    0x03,    0x07)

#define subbnZld(RG, MS, DS)  /* sub  8-bit from 32-bit with sign-extend */ \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xBE)                       \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)                                        \
        REX(RXB(RG),       1) EMITB(0x2B)                                   \
        MRM(REG(RG),    0x03,    0x07)

#define subbzZld(RG, MS, DS)  /* sub  8-bit from 32-bit with zero-extend */ \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xB6)                       \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)                                        \
        REX(RXB(RG),       1) EMITB(0x2B)                                   \
        MRM(REG(RG),    0x03,    0x07)

#define subwxZst(RS, MG, DG)                                                \
    ADR REX(RXB(RS), RXB(MG)) EMITB(0x29)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define subwxZmr(MG, DG, RS)                                                \
        subwxZst(W(RS), W(MG), W(DG))

/* shl (G = G << S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shlwx_rx(RG)                     /* reads Recx for shift count */   \
        shlwxZrx(W(RG))

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define shlwx_rx(RG)                     /* reads Recx for shift count */   \
        VEX(RXB(RG), RXB(RG),    0x01, 0, 1, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define shlwx_mx(MG, DG)                 /* reads Recx for shift count */   \
        shlwxZmx(W(MG), W(DG))

#define shlwx_ri(RG, IS)                                                    \
        shlwxZri(W(RG), W(IS))

#define shlwx_mi(MG, DG, IS)                                                \
        shlwxZmi(W(MG), W(DG), W(IS))

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shlwx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shlwxZrr(W(RG), W(RS))

#define shlwx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shlwxZld(W(RG), W(MS), W(DS))

#define shlwx_st(RS, MG, DG)                                                \
        shlwxZst(W(RS), W(MG), W(DG))

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define shlwx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        VEX(RXB(RG), RXB(RG), REN(RS), 0, 1, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#define shlwx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
    ADR REX(0x01,    RXB(MS)) EMITB(0x8B)                                   \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)                                        \
        VEX(RXB(RG), RXB(RG),    0x0F, 0, 1, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#define shlwx_st(RS, MG, DG)                                                \
    ADR VEX(0x01,    RXB(MG), REN(RS), 0, 1, 2) EMITB(0xF7)                 \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)                                        \
    ADR REX(0x01,    RXB(MG)) EMITB(0x89)                                   \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define shlwx_mr(MG, DG, RS)                                                \
        shlwx_st(W(RS), W(MG), W(DG))


#define shlwxZrx(RG)                     /* reads Recx for shift count */   \
        REX(0,       RXB(RG)) EMITB(0xD3)                                   \
        MRM(0x04,    MOD(RG), REG(RG))                                      \

#define shlwxZmx(MG, DG)                 /* reads Recx for shift count */   \
    ADR REX(0,       RXB(MG)) EMITB(0xD3)                                   \
        MRM(0x04,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define shlwxZri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0xC1)                                   \
        MRM(0x04,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shlwxZmi(MG, DG, IS)                                                \
    ADR REX(0,       RXB(MG)) EMITB(0xC1)                                   \
        MRM(0x04,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMITB(VAL(IS)))

#define shlwxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movwx_rr(Recx, W(RS))                                               \
        shlwxZrx(W(RG))                                                     \
        stack_ld(Recx)

#define shlwxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movwx_ld(Recx, W(MS), W(DS))                                        \
        shlwxZrx(W(RG))                                                     \
        stack_ld(Recx)

#define shlwxZst(RS, MG, DG)                                                \
        stack_st(Recx)                                                      \
        movwx_rr(Recx, W(RS))                                               \
        shlwxZmx(W(MG), W(DG))                                              \
        stack_ld(Recx)

#define shlwxZmr(MG, DG, RS)                                                \
        shlwxZst(W(RS), W(MG), W(DG))

/* shr (G = G >> S), unsigned (logical)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shrwx_rx(RG)                     /* reads Recx for shift count */   \
        shrwxZrx(W(RG))

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define shrwx_rx(RG)                     /* reads Recx for shift count */   \
        VEX(RXB(RG), RXB(RG),    0x01, 0, 3, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define shrwx_mx(MG, DG)                 /* reads Recx for shift count */   \
        shrwxZmx(W(MG), W(DG))

#define shrwx_ri(RG, IS)                                                    \
        shrwxZri(W(RG), W(IS))

#define shrwx_mi(MG, DG, IS)                                                \
        shrwxZmi(W(MG), W(DG), W(IS))

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shrwx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrwxZrr(W(RG), W(RS))

#define shrwx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shrwxZld(W(RG), W(MS), W(DS))

#define shrwx_st(RS, MG, DG)                                                \
        shrwxZst(W(RS), W(MG), W(DG))

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define shrwx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        VEX(RXB(RG), RXB(RG), REN(RS), 0, 3, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#define shrwx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
    ADR REX(0x01,    RXB(MS)) EMITB(0x8B)                                   \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)                                        \
        VEX(RXB(RG), RXB(RG),    0x0F, 0, 3, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#define shrwx_st(RS, MG, DG)                                                \
    ADR VEX(0x01,    RXB(MG), REN(RS), 0, 3, 2) EMITB(0xF7)                 \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)                                        \
    ADR REX(0x01,    RXB(MG)) EMITB(0x89)                                   \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define shrwx_mr(MG, DG, RS)                                                \
        shrwx_st(W(RS), W(MG), W(DG))


#define shrwxZrx(RG)                     /* reads Recx for shift count */   \
        REX(0,       RXB(RG)) EMITB(0xD3)                                   \
        MRM(0x05,    MOD(RG), REG(RG))                                      \

#define shrwxZmx(MG, DG)                 /* reads Recx for shift count */   \
    ADR REX(0,       RXB(MG)) EMITB(0xD3)                                   \
        MRM(0x05,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define shrwxZri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0xC1)                                   \
        MRM(0x05,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shrwxZmi(MG, DG, IS)                                                \
    ADR REX(0,       RXB(MG)) EMITB(0xC1)                                   \
        MRM(0x05,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMITB(VAL(IS)))

#define shrwxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movwx_rr(Recx, W(RS))                                               \
        shrwxZrx(W(RG))                                                     \
        stack_ld(Recx)

#define shrwxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movwx_ld(Recx, W(MS), W(DS))                                        \
        shrwxZrx(W(RG))                                                     \
        stack_ld(Recx)

#define shrwxZst(RS, MG, DG)                                                \
        stack_st(Recx)                                                      \
        movwx_rr(Recx, W(RS))                                               \
        shrwxZmx(W(MG), W(DG))                                              \
        stack_ld(Recx)

#define shrwxZmr(MG, DG, RS)                                                \
        shrwxZst(W(RS), W(MG), W(DG))

/* shr (G = G >> S), signed (arithmetic)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shrwn_rx(RG)                     /* reads Recx for shift count */   \
        REX(0,       RXB(RG)) EMITB(0xD3)                                   \
        MRM(0x07,    MOD(RG), REG(RG))                                      \

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define shrwn_rx(RG)                     /* reads Recx for shift count */   \
        VEX(RXB(RG), RXB(RG),    0x01, 0, 2, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define shrwn_mx(MG, DG)                 /* reads Recx for shift count */   \
    ADR REX(0,       RXB(MG)) EMITB(0xD3)                                   \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define shrwn_ri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0xC1)                                   \
        MRM(0x07,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shrwn_mi(MG, DG, IS)                                                \
    ADR REX(0,       RXB(MG)) EMITB(0xC1)                                   \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMITB(VAL(IS)))

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shrwn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movwx_rr(Recx, W(RS))                                               \
        shrwn_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrwn_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movwx_ld(Recx, W(MS), W(DS))                                        \
        shrwn_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrwn_st(RS, MG, DG)                                                \
        stack_st(Recx)                                                      \
        movwx_rr(Recx, W(RS))                                               \
        shrwn_mx(W(MG), W(DG))                                              \
        stack_ld(Recx)

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define shrwn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        VEX(RXB(RG), RXB(RG), REN(RS), 0, 2, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#define shrwn_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
    ADR REX(0x01,    RXB(MS)) EMITB(0x8B)                                   \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)                                        \
        VEX(RXB(RG), RXB(RG),    0x0F, 0, 2, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#define shrwn_st(RS, MG, DG)                                                \
    ADR VEX(0x01,    RXB(MG), REN(RS), 0, 2, 2) EMITB(0xF7)                 \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)                                        \
    ADR REX(0x01,    RXB(MG)) EMITB(0x89)                                   \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define shrwn_mr(MG, DG, RS)                                                \
        shrwn_st(W(RS), W(MG), W(DG))


#define shrwnZrx(RG)                     /* reads Recx for shift count */   \
        REX(0,       RXB(RG)) EMITB(0xD3)                                   \
        MRM(0x07,    MOD(RG), REG(RG))                                      \

#define shrwnZmx(MG, DG)                 /* reads Recx for shift count */   \
    ADR REX(0,       RXB(MG)) EMITB(0xD3)                                   \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define shrwnZri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0xC1)                                   \
        MRM(0x07,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shrwnZmi(MG, DG, IS)                                                \
    ADR REX(0,       RXB(MG)) EMITB(0xC1)                                   \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMITB(VAL(IS)))

#define shrwnZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movwx_rr(Recx, W(RS))                                               \
        shrwnZrx(W(RG))                                                     \
        stack_ld(Recx)

#define shrwnZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movwx_ld(Recx, W(MS), W(DS))                                        \
        shrwnZrx(W(RG))                                                     \
        stack_ld(Recx)

#define shrwnZst(RS, MG, DG)                                                \
        stack_st(Recx)                                                      \
        movwx_rr(Recx, W(RS))                                               \
        shrwnZmx(W(MG), W(DG))                                              \
        stack_ld(Recx)

#define shrwnZmr(MG, DG, RS)                                                \
        shrwnZst(W(RS), W(MG), W(DG))

/* ror (G = G >> S | G << 32 - S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define rorwx_rx(RG)                     /* reads Recx for shift count */   \
        rorwxZrx(W(RG))

#define rorwx_mx(MG, DG)                 /* reads Recx for shift count */   \
        rorwxZmx(W(MG), W(DG))

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define rorwx_ri(RG, IS)                                                    \
        rorwxZri(W(RG), W(IS))

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define rorwx_ri(RG, IS)                                                    \
        VEX(RXB(RG), RXB(RG),    0x00, 0, 3, 3) EMITB(0xF0)                 \
        MRM(REG(RG), MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define rorwx_mi(MG, DG, IS)                                                \
        rorwxZmi(W(MG), W(DG), W(IS))

#define rorwx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        rorwxZrr(W(RG), W(RS))

#define rorwx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        rorwxZld(W(RG), W(MS), W(DS))

#define rorwx_st(RS, MG, DG)                                                \
        rorwxZst(W(RS), W(MG), W(DG))

#define rorwx_mr(MG, DG, RS)                                                \
        rorwx_st(W(RS), W(MG), W(DG))


#define rorwxZrx(RG)                     /* reads Recx for shift count */   \
        REX(0,       RXB(RG)) EMITB(0xD3)                                   \
        MRM(0x01,    MOD(RG), REG(RG))                                      \

#define rorwxZmx(MG, DG)                 /* reads Recx for shift count */   \
    ADR REX(0,       RXB(MG)) EMITB(0xD3)                                   \
        MRM(0x01,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define rorwxZri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0xC1)                                   \
        MRM(0x01,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))

#define rorwxZmi(MG, DG, IS)                                                \
    ADR REX(0,       RXB(MG)) EMITB(0xC1)                                   \
        MRM(0x01,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMITB(VAL(IS) & 0x1F))

#define rorwxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movwx_rr(Recx, W(RS))                                               \
        rorwxZrx(W(RG))                                                     \
        stack_ld(Recx)

#define rorwxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movwx_ld(Recx, W(MS), W(DS))                                        \
        rorwxZrx(W(RG))                                                     \
        stack_ld(Recx)

#define rorwxZst(RS, MG, DG)                                                \
        stack_st(Recx)                                                      \
        movwx_rr(Recx, W(RS))                                               \
        rorwxZmx(W(MG), W(DG))                                              \
        stack_ld(Recx)

#define rorwxZmr(MG, DG, RS)                                                \
        rorwxZst(W(RS), W(MG), W(DG))

/* mul (G = G * S)
 * set-flags: undefined */

#define mulwx_ri(RG, IS)                                                    \
        REX(RXB(RG), RXB(RG)) EMITB(0x69 | TYP(IS))                         \
        MRM(REG(RG), MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IS))

#define mulwx_rr(RG, RS)                                                    \
        REX(RXB(RG), RXB(RS)) EMITB(0x0F) EMITB(0xAF)                       \
        MRM(REG(RG), MOD(RS), REG(RS))

#define mulwx_ld(RG, MS, DS)                                                \
    ADR REX(RXB(RG), RXB(MS)) EMITB(0x0F) EMITB(0xAF)                       \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mulhn_ld(RG, MS, DS)  /* mul 32-bit with 16-bit with sign-extend */ \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xBF)                       \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)                                        \
        REX(RXB(RG),       1) EMITB(0x0F) EMITB(0xAF)                       \
        MRM(REG(RG),    0x03,    0x07)

#define mulhz_ld(RG, MS, DS)  /* mul 32-bit with 16-bit with zero-extend */ \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xB7)                       \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)                                        \
        REX(RXB(RG),       1) EMITB(0x0F) EMITB(0xAF)                       \
        MRM(REG(RG),    0x03,    0x07)

#define mulbn_ld(RG, MS, DS)  /* mul 32-bit with  8-bit with sign-extend */ \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xBE)                       \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)                                        \
        REX(RXB(RG),       1) EMITB(0x0F) EMITB(0xAF)                       \
        MRM(REG(RG),    0x03,    0x07)

#define mulbz_ld(RG, MS, DS)  /* mul 32-bit with  8-bit with zero-extend */ \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xB6)                       \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)                                        \
        REX(RXB(RG),       1) EMITB(0x0F) EMITB(0xAF)                       \
        MRM(REG(RG),    0x03,    0x07)


#define mulwx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        REX(0,       RXB(RS)) EMITB(0xF7)                                   \
        MRM(0x04,    MOD(RS), REG(RS))                                      \

#define mulwx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
    ADR REX(0,       RXB(MS)) EMITB(0xF7)                                   \
        MRM(0x04,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define mulwn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        REX(0,       RXB(RS)) EMITB(0xF7)                                   \
        MRM(0x05,    MOD(RS), REG(RS))                                      \

#define mulwn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
    ADR REX(0,       RXB(MS)) EMITB(0xF7)                                   \
        MRM(0x05,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define mulwp_xr(RS)     /* Reax is in/out, prepares Redx for divwn_x* */   \
        mulwn_xr(W(RS))       /* product must not exceed operands size */

#define mulwp_xm(MS, DS) /* Reax is in/out, prepares Redx for divwn_x* */   \
        mulwn_xm(W(MS), W(DS))/* product must not exceed operands size */

/* div (G = G / S)
 * set-flags: undefined */

#define divwx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movwx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movwx_rr(Reax, W(RG))                                               \
        prewx_xx()                                                          \
        divwx_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Redx)                                                      \
        movwx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divwx_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movwx_rr(Reax, W(RG))                                               \
        prewx_xx()                                                          \
        divwx_xr(W(RS))                                                     \
        stack_ld(Redx)                                                      \
        movwx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divwx_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movwx_rr(Reax, W(RG))                                               \
        prewx_xx()                                                          \
        divwx_xm(W(MS), W(DS))                                              \
        stack_ld(Redx)                                                      \
        movwx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)


#define divwn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movwx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movwx_rr(Reax, W(RG))                                               \
        prewn_xx()                                                          \
        divwn_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Redx)                                                      \
        movwx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divwn_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movwx_rr(Reax, W(RG))                                               \
        prewn_xx()                                                          \
        divwn_xr(W(RS))                                                     \
        stack_ld(Redx)                                                      \
        movwx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divwn_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movwx_rr(Reax, W(RG))                                               \
        prewn_xx()                                                          \
        divwn_xm(W(MS), W(DS))                                              \
        stack_ld(Redx)                                                      \
        movwx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)


#define prewx_xx()   /* to be placed right before divwx_x* or remwx_xx */   \
        movwx_ri(Redx, IC(0))        /* to prepare Redx for int-divide */

#define prewn_xx()   /* to be placed right before divwn_x* or remwn_xx */   \
        movwx_rr(Redx, Reax)         /* to prepare Redx for int-divide */   \
        shrwn_ri(Redx, IC(31))


#define divwx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        REX(0,       RXB(RS)) EMITB(0xF7)                                   \
        MRM(0x06,    MOD(RS), REG(RS))                                      \
        AUX(EMPTY,   EMPTY,   EMPTY)

#define divwx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
    ADR REX(0,       RXB(MS)) EMITB(0xF7)                                   \
        MRM(0x06,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define divwn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        REX(0,       RXB(RS)) EMITB(0xF7)                                   \
        MRM(0x07,    MOD(RS), REG(RS))                                      \
        AUX(EMPTY,   EMPTY,   EMPTY)

#define divwn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
    ADR REX(0,       RXB(MS)) EMITB(0xF7)                                   \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define divwp_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xr(W(RS))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divwp_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xm(W(MS), W(DS))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem (G = G % S)
 * set-flags: undefined */

#define remwx_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movwx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movwx_rr(Reax, W(RG))                                               \
        prewx_xx()                                                          \
        divwx_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Reax)                                                      \
        movwx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remwx_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movwx_rr(Reax, W(RG))                                               \
        prewx_xx()                                                          \
        divwx_xr(W(RS))                                                     \
        stack_ld(Reax)                                                      \
        movwx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remwx_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movwx_rr(Reax, W(RG))                                               \
        prewx_xx()                                                          \
        divwx_xm(W(MS), W(DS))                                              \
        stack_ld(Reax)                                                      \
        movwx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)


#define remwn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movwx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movwx_rr(Reax, W(RG))                                               \
        prewn_xx()                                                          \
        divwn_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Reax)                                                      \
        movwx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remwn_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movwx_rr(Reax, W(RG))                                               \
        prewn_xx()                                                          \
        divwn_xr(W(RS))                                                     \
        stack_ld(Reax)                                                      \
        movwx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remwn_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movwx_rr(Reax, W(RG))                                               \
        prewn_xx()                                                          \
        divwn_xm(W(MS), W(DS))                                              \
        stack_ld(Reax)                                                      \
        movwx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)


#define remwx_xx() /* to be placed before divwx_x*, but after prewx_xx */   \
                                     /* to prepare for rem calculation */

#define remwx_xr(RS)        /* to be placed immediately after divwx_xr */   \
                                     /* to produce remainder Redx<-rem */

#define remwx_xm(MS, DS)    /* to be placed immediately after divwx_xm */   \
                                     /* to produce remainder Redx<-rem */


#define remwn_xx() /* to be placed before divwn_x*, but after prewn_xx */   \
                                     /* to prepare for rem calculation */

#define remwn_xr(RS)        /* to be placed immediately after divwn_xr */   \
                                     /* to produce remainder Redx<-rem */

#define remwn_xm(MS, DS)    /* to be placed immediately after divwn_xm */   \
                                     /* to produce remainder Redx<-rem */

/* arj (G = G op S, if cc G then jump lb)
 * set-flags: undefined
 * refer to individual instruction descriptions
 * to stay within special register limitations */

#define and_x   AL0
#define ann_x   AL1
#define orr_x   AL2
#define orn_x   AL3
#define xor_x   AL4
#define neg_x   AM0
#define add_x   AM1
#define sub_x   AM2
#define add_n   AM3
#define sub_n   AM4
#define add_z   AM5
#define sub_z   AM6
#define shl_x   AN0
#define shr_x   AN1
#define shr_n   AN2
#define ror_x   AN3

#define EZ_x    jezxx_lb
#define NZ_x    jnzxx_lb

#define arjwx_rx(RG, op, cc, lb)                                            \
        AR1(W(RG), op, w, Zrx)                                              \
        CMJ(cc, lb)

#define arjwx_mx(MG, DG, op, cc, lb)                                        \
        AR2(W(MG), W(DG), op, w, Zmx)                                       \
        CMJ(cc, lb)

#define arjwx_ri(RG, IS, op, cc, lb)                                        \
        AR2(W(RG), W(IS), op, w, Zri)                                       \
        CMJ(cc, lb)

#define arjwx_mi(MG, DG, IS, op, cc, lb)                                    \
        AR3(W(MG), W(DG), W(IS), op, w, Zmi)                                \
        CMJ(cc, lb)

#define arjwx_rr(RG, RS, op, cc, lb)                                        \
        AR2(W(RG), W(RS), op, w, Zrr)                                       \
        CMJ(cc, lb)

#define arjwx_ld(RG, MS, DS, op, cc, lb)                                    \
        AR3(W(RG), W(MS), W(DS), op, w, Zld)                                \
        CMJ(cc, lb)

#define arjwx_st(RS, MG, DG, op, cc, lb)                                    \
        AR3(W(RS), W(MG), W(DG), op, w, Zst)                                \
        CMJ(cc, lb)

#define arjwx_mr(MG, DG, RS, op, cc, lb)                                    \
        arjwx_st(W(RS), W(MG), W(DG), op, cc, lb)

/* cmj (flags = S ? T, if cc flags then jump lb)
 * set-flags: undefined */

#define EQ_x    jeqxx_lb
#define NE_x    jnexx_lb

#define LT_x    jltxx_lb
#define LE_x    jlexx_lb
#define GT_x    jgtxx_lb
#define GE_x    jgexx_lb

#define LT_n    jltxn_lb
#define LE_n    jlexn_lb
#define GT_n    jgtxn_lb
#define GE_n    jgexn_lb

#define cmjwx_rz(RS, cc, lb)                                                \
        cmjwx_ri(W(RS), IC(0), cc, lb)

#define cmjwx_mz(MS, DS, cc, lb)                                            \
        cmjwx_mi(W(MS), W(DS), IC(0), cc, lb)

#define cmjwx_ri(RS, IT, cc, lb)                                            \
        cmpwx_ri(W(RS), W(IT))                                              \
        CMJ(cc, lb)

#define cmjwx_mi(MS, DS, IT, cc, lb)                                        \
        cmpwx_mi(W(MS), W(DS), W(IT))                                       \
        CMJ(cc, lb)

#define cmjwx_rr(RS, RT, cc, lb)                                            \
        cmpwx_rr(W(RS), W(RT))                                              \
        CMJ(cc, lb)

#define cmjwx_rm(RS, MT, DT, cc, lb)                                        \
        cmpwx_rm(W(RS), W(MT), W(DT))                                       \
        CMJ(cc, lb)

#define cmjhn_rm(RS, MT, DT, cc, lb)   /* cmj 32/16-bit with sign-extend */ \
        cmphn_rm(W(RS), W(MT), W(DT))                                       \
        CMJ(cc, lb)

#define cmjhz_rm(RS, MT, DT, cc, lb)   /* cmj 32/16-bit with zero-extend */ \
        cmphz_rm(W(RS), W(MT), W(DT))                                       \
        CMJ(cc, lb)

#define cmjbn_rm(RS, MT, DT, cc, lb)   /* cmj 32/8-bit  with sign-extend */ \
        cmpbn_rm(W(RS), W(MT), W(DT))                                       \
        CMJ(cc, lb)

#define cmjbz_rm(RS, MT, DT, cc, lb)   /* cmj 32/8-bit  with zero-extend */ \
        cmpbz_rm(W(RS), W(MT), W(DT))                                       \
        CMJ(cc, lb)

#define cmjwx_mr(MS, DS, RT, cc, lb)                                        \
        cmpwx_mr(W(MS), W(DS), W(RT))                                       \
        CMJ(cc, lb)

#define cmjhn_mr(MS, DS, RT, cc, lb)   /* cmj 16/32-bit with sign-extend */ \
        cmphn_mr(W(MS), W(DS), W(RT))                                       \
        CMJ(cc, lb)

#define cmjhz_mr(MS, DS, RT, cc, lb)   /* cmj 16/32-bit with zero-extend */ \
        cmphz_mr(W(MS), W(DS), W(RT))                                       \
        CMJ(cc, lb)

#define cmjbn_mr(MS, DS, RT, cc, lb)   /* cmj  8/32-bit with sign-extend */ \
        cmpbn_mr(W(MS), W(DS), W(RT))                                       \
        CMJ(cc, lb)

#define cmjbz_mr(MS, DS, RT, cc, lb)   /* cmj  8/32-bit with zero-extend */ \
        cmpbz_mr(W(MS), W(DS), W(RT))                                       \
        CMJ(cc, lb)

/* cmp (flags = S ? T)
 * set-flags: yes */

#define cmpwx_ri(RS, IT)                                                    \
        REX(0,       RXB(RS)) EMITB(0x81 | TYP(IT))                         \
        MRM(0x07,    MOD(RS), REG(RS))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IT))

#define cmpwx_mi(MS, DS, IT)                                                \
    ADR REX(0,       RXB(MS)) EMITB(0x81 | TYP(IT))                         \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), CMD(IT))

#define cmpwx_rr(RS, RT)                                                    \
        REX(RXB(RS), RXB(RT)) EMITB(0x3B)                                   \
        MRM(REG(RS), MOD(RT), REG(RT))

#define cmpwx_rm(RS, MT, DT)                                                \
    ADR REX(RXB(RS), RXB(MT)) EMITB(0x3B)                                   \
        MRM(REG(RS), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#define cmphn_rm(RS, MT, DT)    /* cmp 32-bit to 16-bit with sign-extend */ \
    ADR REX(1,       RXB(MT)) EMITB(0x0F) EMITB(0xBF)                       \
        MRM(0x07,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)                                        \
        REX(RXB(RS),       1) EMITB(0x3B)                                   \
        MRM(REG(RS),    0x03,    0x07)

#define cmphz_rm(RS, MT, DT)    /* cmp 32-bit to 16-bit with zero-extend */ \
    ADR REX(1,       RXB(MT)) EMITB(0x0F) EMITB(0xB7)                       \
        MRM(0x07,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)                                        \
        REX(RXB(RS),       1) EMITB(0x3B)                                   \
        MRM(REG(RS),    0x03,    0x07)

#define cmpbn_rm(RS, MT, DT)    /* cmp 32-bit to  8-bit with sign-extend */ \
    ADR REX(1,       RXB(MT)) EMITB(0x0F) EMITB(0xBE)                       \
        MRM(0x07,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)                                        \
        REX(RXB(RS),       1) EMITB(0x3B)                                   \
        MRM(REG(RS),    0x03,    0x07)

#define cmpbz_rm(RS, MT, DT)    /* cmp 32-bit to  8-bit with zero-extend */ \
    ADR REX(1,       RXB(MT)) EMITB(0x0F) EMITB(0xB6)                       \
        MRM(0x07,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)                                        \
        REX(RXB(RS),       1) EMITB(0x3B)                                   \
        MRM(REG(RS),    0x03,    0x07)

#define cmpwx_mr(MS, DS, RT)                                                \
    ADR REX(RXB(RT), RXB(MS)) EMITB(0x39)                                   \
        MRM(REG(RT), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define cmphn_mr(MS, DS, RT)    /* cmp 16-bit to 32-bit with sign-extend */ \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xBF)                       \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)                                        \
        REX(RXB(RT),       1) EMITB(0x39)                                   \
        MRM(REG(RT),    0x03,    0x07)

#define cmphz_mr(MS, DS, RT)    /* cmp 16-bit to 32-bit with zero-extend */ \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xB7)                       \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)                                        \
        REX(RXB(RT),       1) EMITB(0x39)                                   \
        MRM(REG(RT),    0x03,    0x07)

#define cmpbn_mr(MS, DS, RT)    /* cmp  8-bit to 32-bit with sign-extend */ \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xBE)                       \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)                                        \
        REX(RXB(RT),       1) EMITB(0x39)                                   \
        MRM(REG(RT),    0x03,    0x07)

#define cmpbz_mr(MS, DS, RT)    /* cmp  8-bit to 32-bit with zero-extend */ \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xB6)                       \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)                                        \
        REX(RXB(RT),       1) EMITB(0x39)                                   \
        MRM(REG(RT),    0x03,    0x07)

/* ver (Mebp/inf_VER = SIMD-version)
 * set-flags: no
 * For interpretation of SIMD build flags check compatibility layer in rtzero.h
 * 0th byte - 128-bit version, 1st byte - 256-bit version, | plus _R8/_RX slots
 * 2nd byte - 512-bit version, 3rd byte - 1K4-bit version, | in upper halves */

#define cpuid_xx() /* destroys Reax, Recx, Rebx, Redx, reads Reax, Recx */  \
        EMITB(0x0F) EMITB(0xA2)     /* not portable, do not use outside */

#define verxx_xx() /* destroys Reax, Recx, Rebx, Redx, Resi, Redi */        \
        /* request cpuid:Reax=1 */                                          \
        movwx_ri(Reax, IB(1))                                               \
        cpuid_xx()                                                          \
        shrwx_ri(Redx, IB(24))                                              \
        andwx_ri(Redx, IV(0x00000004))  /* <- SSE2 to bit2 */               \
        andwx_ri(Redx, IB(1 << RT_SIMD_COMPAT_SSE))                         \
        movwx_rr(Resi, Redx)                                                \
        movwx_rr(Redx, Recx)                                                \
        shrwx_ri(Redx, IB(17))                                              \
        andwx_ri(Redx, IV(0x00000004))  /* <- SSE4.1 to bit2 */             \
        movwx_rr(Redi, Recx)                                                \
        shrwx_ri(Redi, IB(18))                                              \
        andwx_ri(Redi, IV(0x00000004))  /* <- SSE4.2 to bit2 */             \
        andwx_rr(Redx, Redi)                                                \
        orrwx_rr(Resi, Redx)                                                \
        movwx_rr(Redx, Recx)                                                \
        shrwx_ri(Redx, IB(20))                                              \
        andwx_ri(Redx, IV(0x00000100))  /* <- AVX1 to bit8 */               \
        orrwx_rr(Resi, Redx)                                                \
        movwx_rr(Redx, Recx)                                                \
        shrwx_ri(Redx, IB(8))                                               \
        andwx_ri(Redx, IV(0x00000010))  /* <- FMA3 to bit4 */               \
        orrwx_rr(Resi, Redx)                                                \
        /* request cpuid:Reax=0 to test input value Reax=7 */               \
        movwx_ri(Reax, IB(0))                                               \
        cpuid_xx()                                                          \
        subwx_ri(Reax, IB(7))                                               \
        shrwn_ri(Reax, IB(31))                                              \
        movwx_rr(Redi, Reax)                                                \
        notwx_rx(Redi)                                                      \
        /* request cpuid:Reax=7:Recx=0 */                                   \
        movwx_ri(Reax, IB(7))                                               \
        movwx_ri(Recx, IB(0))                                               \
        cpuid_xx()                                                          \
        movwx_rr(Recx, Resi)                                                \
        shlwx_ri(Recx, IB(5))        /* always require FMA3 for AVX2 */     \
        movwx_rr(Reax, Rebx)                                                \
        shrwx_ri(Reax, IB(13))     /* always require BW for AVX512DQ */     \
        movwx_ri(Redx, IV(0x00020000))                                      \
        notwx_rx(Redx)                                                      \
        orrwx_rr(Reax, Redx)                                                \
        andwx_rr(Rebx, Reax)                                                \
        movwx_rr(Reax, Rebx)                                                \
        shrwx_ri(Reax, IB(14))     /* always require VL for AVX512DQ */     \
        movwx_ri(Redx, IV(0x00020000))                                      \
        notwx_rx(Redx)                                                      \
        orrwx_rr(Reax, Redx)                                                \
        andwx_rr(Rebx, Reax)                                                \
        movwx_rr(Reax, Rebx)                                                \
        shrwn_ri(Reax, IB(31)) /* make AVX512VL extension-bit a mask */     \
        movwx_rr(Redx, Rebx)                                                \
        andwx_ri(Redx, IV(0x00030000))  /* <- AVX512 to bit16, bit17 */     \
        andwx_rr(Redx, Redi)                                                \
        orrwx_rr(Resi, Redx)                                                \
        shlwx_ri(Rebx, IB(4))                                               \
        andwx_ri(Rebx, IV(0x00000200))  /* <- AVX2 to bit9 */               \
        andwx_rr(Rebx, Redi)                                                \
        andwx_rr(Rebx, Recx)                                                \
        orrwx_rr(Resi, Rebx)                                                \
        movwx_rr(Recx, Resi)                                                \
        shrwx_ri(Recx, IB(5))                                               \
        andwx_ri(Recx, IV(0x00000008))  /* <- AVX1 to bit3 */               \
        orrwx_rr(Resi, Recx)                                                \
        movwx_rr(Recx, Resi)                                                \
        shrwx_ri(Recx, IB(4))                                               \
        andwx_ri(Recx, IV(0x00000020))  /* <- AVX2 to bit5 */               \
        orrwx_rr(Resi, Recx)                                                \
        movwx_rr(Recx, Resi)                                                \
        shlwx_ri(Recx, IB(4))                                               \
        andwx_ri(Recx, IV(0x00000040))  /* <- SSE2/4.x to bit6 */           \
        orrwx_rr(Resi, Recx)                                                \
        movwx_rr(Recx, Resi)                                                \
        shlwx_ri(Recx, IB(4))                                               \
        andwx_ri(Recx, IV(0x00003000))  /* <- AVX1/2 to bit12, bit13 */     \
        orrwx_rr(Resi, Recx)                                                \
        movwx_rr(Recx, Resi)                                                \
        shlwx_ri(Recx, IB(2))                                               \
        andwx_ri(Recx, IV(0x000C0000))  /* <- AVX512 to bit18, bit19 */     \
        orrwx_rr(Resi, Recx)                                                \
        movwx_rr(Recx, Resi)                                                \
        shlwx_ri(Recx, IB(4))                                               \
        andwx_ri(Recx, IV(0x00300000))  /* <- AVX512 to bit20, bit21 */     \
        orrwx_rr(Resi, Recx)                                                \
        movwx_rr(Recx, Resi)                                                \
        shlwx_ri(Recx, IB(8))                                               \
        andwx_ri(Recx, IV(0x03000000))  /* <- AVX512 to bit24, bit25 */     \
        orrwx_rr(Resi, Recx)                                                \
        movwx_rr(Recx, Resi)                                                \
        shlwx_ri(Recx, IB(8))                                               \
        andwx_ri(Recx, IV(0x30000000))  /* <- AVX512 to bit28, bit29 */     \
        orrwx_rr(Resi, Recx)                                                \
        movwx_rr(Recx, Resi)                                                \
        shrwx_ri(Recx, IB(16))                                              \
        andwx_ri(Recx, IV(0x00000002))  /* <- AVX512DQ+VL to bit1 */        \
        andwx_rr(Recx, Reax)                                                \
        orrwx_rr(Resi, Recx)                                                \
        movwx_rr(Recx, Resi)                                                \
        shrwx_ri(Recx, IB(6))                                               \
        andwx_ri(Recx, IV(0x00000800))  /* <- AVX512DQ+VL to bit11 */       \
        andwx_rr(Recx, Reax)                                                \
        orrwx_rr(Resi, Recx)                                                \
        movwx_st(Resi, Mebp, inf_VER)

/************************* address-sized instructions *************************/

/* adr (D = adr S)
 * set-flags: no */

#define adrxx_ld(RD, MS, DS)                                                \
    ADR REW(RXB(RD), RXB(MS)) EMITB(0x8D)                                   \
        MRM(REG(RD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

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
        REX(0,       RXB(RS)) EMITB(0xFF)   /* <- jump to address in reg */ \
        MRM(0x04,    MOD(RS), REG(RS))

#if   (defined RT_X32)

#define jmpxx_xm(MS, DS)         /* memory-targeted unconditional jump */   \
    ADR REX(1,       RXB(MS)) EMITB(0x8B)   /* <- load r15d from MS/DS */   \
        MRM(0x07,    MOD(MS), REG(MS))      /*    upper half is zeroed */   \
        AUX(SIB(MS), CMD(DS), EMPTY)        /*    as a part of 32-bit op */ \
        REX(0,             1) EMITB(0xFF)   /* <- jump to address in r15 */ \
        MRM(0x04,       0x03, 0x07)

#elif (defined RT_X64)

#define jmpxx_xm(MS, DS)         /* memory-targeted unconditional jump */   \
    ADR REX(0,       RXB(MS)) EMITB(0xFF)   /* <- jump to address in mem */ \
        MRM(0x04,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#endif /* defined (RT_X32, RT_X64) */

#define jmpxx_lb(lb)              /* label-targeted unconditional jump */   \
        ASM_BEG ASM_OP1(jmp, lb) ASM_END

#define jezxx_lb(lb)               /* setting-flags-arithmetic -> jump */   \
        ASM_BEG ASM_OP1(jz,  lb) ASM_END

#define jnzxx_lb(lb)               /* setting-flags-arithmetic -> jump */   \
        ASM_BEG ASM_OP1(jnz, lb) ASM_END

#define jeqxx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(je,  lb) ASM_END

#define jnexx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(jne, lb) ASM_END

#define jltxx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(jb,  lb) ASM_END

#define jlexx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(jbe, lb) ASM_END

#define jgtxx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(ja,  lb) ASM_END

#define jgexx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(jae, lb) ASM_END

#define jltxn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(jl,  lb) ASM_END

#define jlexn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(jle, lb) ASM_END

#define jgtxn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(jg,  lb) ASM_END

#define jgexn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(jge, lb) ASM_END

#define LBL(lb)                                          /* code label */   \
        ASM_BEG ASM_OP0(lb:) ASM_END

/************************* register-size instructions *************************/

/* stack (push stack = S, D = pop stack)
 * set-flags: no (sequence cmp/stack_la/jmp is not allowed on MIPS & POWER)
 * adjust stack pointer with 8-byte (64-bit) steps on all current targets */

#define stack_st(RS)                                                        \
        REX(0,       RXB(RS)) EMITB(0xFF)                                   \
        MRM(0x06,    MOD(RS), REG(RS))

#define stack_ld(RD)                                                        \
        REX(0,       RXB(RD)) EMITB(0x8F)                                   \
        MRM(0x00,    MOD(RD), REG(RD))

#define stack_sa()   /* save all [Reax - RegF], 15 regs in total */         \
        stack_st(Reax)                                                      \
        stack_st(Recx)                                                      \
        stack_st(Redx)                                                      \
        stack_st(Rebx)                                                      \
        stack_st(Rebp)                                                      \
        stack_st(Resi)                                                      \
        stack_st(Redi)                                                      \
        stack_st(Reg8)                                                      \
        stack_st(Reg9)                                                      \
        stack_st(RegA)                                                      \
        stack_st(RegB)                                                      \
        stack_st(RegC)                                                      \
        stack_st(RegD)                                                      \
        stack_st(RegE)                                                      \
        REX(0,             1) EMITB(0xFF)     /* <- save r15 or [RegF] */   \
        MRM(0x06,       0x03, 0x07)

#define stack_la()   /* load all [RegF - Reax], 15 regs in total */         \
        REX(0,             1) EMITB(0x8F)     /* <- load r15 or [RegF] */   \
        MRM(0x00,       0x03, 0x07)                                         \
        stack_ld(RegE)                                                      \
        stack_ld(RegD)                                                      \
        stack_ld(RegC)                                                      \
        stack_ld(RegB)                                                      \
        stack_ld(RegA)                                                      \
        stack_ld(Reg9)                                                      \
        stack_ld(Reg8)                                                      \
        stack_ld(Redi)                                                      \
        stack_ld(Resi)                                                      \
        stack_ld(Rebp)                                                      \
        stack_ld(Rebx)                                                      \
        stack_ld(Redx)                                                      \
        stack_ld(Recx)                                                      \
        stack_ld(Reax)

/******************************************************************************/
/**************************   extended double (x87)   *************************/
/******************************************************************************/

#define fpuws_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xD9)                                   \
        MRM(0x00,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define fpuws_st(MD, DD) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MD)) EMITB(0xD9)                                   \
        MRM(0x03,    MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)


#define fpuwn_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xDB)                                   \
        MRM(0x00,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define fpuwn_st(MD, DD) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MD)) EMITB(0xDB)                                   \
        MRM(0x03,    MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

#define fpuwt_st(MD, DD) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MD)) EMITB(0xDB)                                   \
        MRM(0x01,    MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)


#define addws_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xD8)                                   \
        MRM(0x00,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define subws_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xD8)                                   \
        MRM(0x04,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define sbrws_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xD8)                                   \
        MRM(0x05,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define mulws_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xD8)                                   \
        MRM(0x01,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define divws_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xD8)                                   \
        MRM(0x06,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define dvrws_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xD8)                                   \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define addes_xn(n)      /* ST(0) = ST(0) + ST(n), don't pop */             \
        EMITB(0xD8) EMITB(0xC0+(n))

#define addes_nx(n)      /* ST(n) = ST(n) + ST(0), don't pop */             \
        EMITB(0xDC) EMITB(0xC0+(n))

#define addes_np(n)      /* ST(n) = ST(n) + ST(0), pop stack */             \
        EMITB(0xDE) EMITB(0xC0+(n))


#define subes_xn(n)      /* ST(0) = ST(0) - ST(n), don't pop */             \
        EMITB(0xD8) EMITB(0xE0+(n))

#define subes_nx(n)      /* ST(n) = ST(n) - ST(0), don't pop */             \
        EMITB(0xDC) EMITB(0xE8+(n))

#define subes_np(n)      /* ST(n) = ST(n) - ST(0), pop stack */             \
        EMITB(0xDE) EMITB(0xE8+(n))


#define sbres_xn(n)      /* ST(0) = ST(n) - ST(0), don't pop */             \
        EMITB(0xD8) EMITB(0xE8+(n))

#define sbres_nx(n)      /* ST(n) = ST(0) - ST(n), don't pop */             \
        EMITB(0xDC) EMITB(0xE0+(n))

#define sbres_np(n)      /* ST(n) = ST(0) - ST(n), pop stack */             \
        EMITB(0xDE) EMITB(0xE0+(n))


#define mules_xn(n)      /* ST(0) = ST(0) * ST(n), don't pop */             \
        EMITB(0xD8) EMITB(0xC8+(n))

#define mules_nx(n)      /* ST(n) = ST(n) * ST(0), don't pop */             \
        EMITB(0xDC) EMITB(0xC8+(n))

#define mules_np(n)      /* ST(n) = ST(n) * ST(0), pop stack */             \
        EMITB(0xDE) EMITB(0xC8+(n))


#define dives_xn(n)      /* ST(0) = ST(0) / ST(n), don't pop */             \
        EMITB(0xD8) EMITB(0xF0+(n))

#define dives_nx(n)      /* ST(n) = ST(n) / ST(0), don't pop */             \
        EMITB(0xDC) EMITB(0xF8+(n))

#define dives_np(n)      /* ST(n) = ST(n) / ST(0), pop stack */             \
        EMITB(0xDE) EMITB(0xF8+(n))


#define dvres_xn(n)      /* ST(0) = ST(n) / ST(0), don't pop */             \
        EMITB(0xD8) EMITB(0xF8+(n))

#define dvres_nx(n)      /* ST(n) = ST(0) / ST(n), don't pop */             \
        EMITB(0xDC) EMITB(0xF0+(n))

#define dvres_np(n)      /* ST(n) = ST(0) / ST(n), pop stack */             \
        EMITB(0xDE) EMITB(0xF0+(n))


#define sqres_xx()       /* ST(0) = sqr ST(0), don't pop */                 \
        EMITB(0xD9) EMITB(0xFA)

#define neges_xx()       /* ST(0) = neg ST(0), don't pop */                 \
        EMITB(0xD9) EMITB(0xE0)

#define abses_xx()       /* ST(0) = abs ST(0), don't pop */                 \
        EMITB(0xD9) EMITB(0xE1)


#define xm2es_xx()       /* ST(0) = 2^ST(0)-1, don't pop, [-1.0 : +1.0] */  \
        EMITB(0xD9) EMITB(0xF0)

#define lg2es_xx()       /* ST(1) = ST(1)*lg2 ST(0), pop stack */           \
        EMITB(0xD9) EMITB(0xF1)

#define lp2es_xx()       /* ST(1) = ST(1)*lg2 ST(0)+1.0, pop stack */       \
        EMITB(0xD9) EMITB(0xF9)


#define sines_xx()       /* ST(0) = sin ST(0), don't pop, [-2^63:+2^63] */  \
        EMITB(0xD9) EMITB(0xFE)

#define coses_xx()       /* ST(0) = cos ST(0), don't pop, [-2^63:+2^63] */  \
        EMITB(0xD9) EMITB(0xFF)

#define scses_xx()       /* ST(0) = sin ST(0), push cos ST(0), original */  \
        EMITB(0xD9) EMITB(0xFB)

#define tanes_xx()       /* ST(0) = tan ST(0), push +1.0, [-2^63:+2^63] */  \
        EMITB(0xD9) EMITB(0xF2)

#define atnes_xx()       /* ST(1) = atn ST(1)/ST(0), pop stack */           \
        EMITB(0xD9) EMITB(0xF3)


#define remes_xx()       /* ST(0) = ST(0)-Q*ST(1), Q = rnd ST(0)/ST(1) */   \
        EMITB(0xD9) EMITB(0xF5)

#define rexes_xx()       /* ST(0) = ST(0)-Q*ST(1), Q = trn ST(0)/ST(1) */   \
        EMITB(0xD9) EMITB(0xF8)

#define rndes_xx()       /* ST(0) = rnd ST(0), round to integral value */   \
        EMITB(0xD9) EMITB(0xFC)

#define extes_xx()       /* ST(0) = exp ST(0), push mts ST(0) */            \
        EMITB(0xD9) EMITB(0xF4)

#define scles_xx()       /* exp ST(0) = exp ST(0) + rnd ST(1) */            \
        EMITB(0xD9) EMITB(0xFD)


#define cmpes_xn(n)      /* flags = ST(0) ? ST(n), don't pop */             \
        EMITB(0xDB) EMITB(0xF0+(n))

#define cmpes_pn(n)      /* flags = ST(0) ? ST(n), pop stack */             \
        EMITB(0xDF) EMITB(0xF0+(n))

#define moves_nx(n)      /* ST(n) = ST(0), don't pop */                     \
        EMITB(0xDD) EMITB(0xD0+(n))

#define moves_np(n)      /* ST(n) = ST(0), pop stack */                     \
        EMITB(0xDD) EMITB(0xD8+(n))

#define popes_xx()       /* ST(0) = ST(0), pop stack */                     \
        EMITB(0xDD) EMITB(0xD8)


#define fpucw_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xD9)                                   \
        MRM(0x05,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define fpucw_st(MD, DD) /* not portable, do not use outside */             \
FWT ADR REX(0,       RXB(MD)) EMITB(0xD9)                                   \
        MRM(0x07,    MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)


#define fpurxZxx()       /* not portable, do not use outside */             \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x0C7F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fpurp_xx()       /* not portable, do not use outside */             \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x087F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fpurm_xx()       /* not portable, do not use outside */             \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x047F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fpurn_xx()       /* not portable, do not use outside */             \
        fpucw_ld(Mebp,  inf_SCR02(4))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/* internal definitions for combined-arithmetic-jump (arj) */

#define AL0(sz, sg) and##sz##x##sg
#define AL1(sz, sg) ann##sz##x##sg
#define AL2(sz, sg) orr##sz##x##sg
#define AL3(sz, sg) orn##sz##x##sg
#define AL4(sz, sg) xor##sz##x##sg
#define AM0(sz, sg) neg##sz##x##sg
#define AM1(sz, sg) add##sz##x##sg
#define AM2(sz, sg) sub##sz##x##sg
#define AM3(sz, sg) add##sz##n##sg
#define AM4(sz, sg) sub##sz##n##sg
#define AM5(sz, sg) add##sz##z##sg
#define AM6(sz, sg) sub##sz##z##sg
#define AN0(sz, sg) shl##sz##x##sg
#define AN1(sz, sg) shr##sz##x##sg
#define AN2(sz, sg) shr##sz##n##sg
#define AN3(sz, sg) ror##sz##x##sg

#define AR1(P1, op, sz, sg)                                                 \
        op(sz,sg)(W(P1))

#define AR2(P1, P2, op, sz, sg)                                             \
        op(sz,sg)(W(P1), W(P2))

#define AR3(P1, P2, P3, op, sz, sg)                                         \
        op(sz,sg)(W(P1), W(P2), W(P3))

#define CMJ(cc, lb)                                                         \
        cc(lb)

#endif /* RT_RTARCH_X32_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
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
 * rtarch_x32.h: Implementation of x86_64:x32 BASE instructions.
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
 * DD - displacement value (DP, DF, DG, DH, DV) (memory-dest)
 * DG - displacement value (DP, DF, DG, DH, DV) (memory-dsrc)
 * DS - displacement value (DP, DF, DG, DH, DV) (memory-src2)
 * DT - displacement value (DP, DF, DG, DH, DV) (memory-src3)
 *
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

/* structural */

#define REX(rxg, rxm) /* W0 */                                              \
        EMITB(0x40 | (rxg) << 2 | (rxm))

#define REW(rxg, rxm) /* W1 */                                              \
        EMITB(0x48 | (rxg) << 2 | (rxm))

#define MRM(reg, mod, rem)                                                  \
        EMITB((mod) << 6 | (reg) << 3 | (rem))

#define AUX(sib, cdp, cim)  sib  cdp  cim

#if   defined (RT_X32)

#define ADR                                                                 \
        EMITB(0x67)

#elif defined (RT_X64)

#define ADR

#endif /* defined (RT_X32, RT_X64) */

/* selectors  */

#define RXB(reg, mod, sib)  ((reg) >> 3 & 0x01) /* register-extension-bit */
#define REG(reg, mod, sib)  ((reg) >> 0 & 0x07) /* register, lower 3-bits */
#define MOD(reg, mod, sib)  mod
#define SIB(reg, mod, sib)  sib

#define VAL(val, typ, cmd)  val
#define TYP(val, typ, cmd)  typ
#define CMD(val, typ, cmd)  cmd

/* selector for full register (3rd operand, 4-bits-wide) */
#define REN(reg, mod, sib)  reg

/* 3-byte VEX prefix with full customization (W0) */
#define VEX(rxg, rxm, ren, len, pfx, aux)                                   \
        EMITB(0xC4)                                                         \
        EMITB((1 - (rxg)) << 7 | 1 << 6 | (1 - (rxm)) << 5 | (aux))         \
        EMITB((len) << 2 | (0x0F - (ren)) << 3 | (pfx))

/* 3-byte VEX prefix with full customization (W1) */
#define VEW(rxg, rxm, ren, len, pfx, aux)                                   \
        EMITB(0xC4)                                                         \
        EMITB((1 - (rxg)) << 7 | 1 << 6 | (1 - (rxm)) << 5 | (aux))         \
        EMITB((len) << 2 | (0x0F - (ren)) << 3 | (pfx) | 0x80)

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

#define Iecx    0x04, 0x02, EMITB(0x01) /* [ecx + eax + DP] */
#define Iedx    0x04, 0x02, EMITB(0x02) /* [edx + eax + DP] */
#define Iebx    0x04, 0x02, EMITB(0x03) /* [ebx + eax + DP] */
#define Iebp    0x04, 0x02, EMITB(0x05) /* [ebp + eax + DP] */
#define Iesi    0x04, 0x02, EMITB(0x06) /* [esi + eax + DP] */
#define Iedi    0x04, 0x02, EMITB(0x07) /* [edi + eax + DP] */
#define Ieg8    0x0C, 0x02, EMITB(0x00) /* [r8d + eax + DP] */
#define Ieg9    0x0C, 0x02, EMITB(0x01) /* [r9d + eax + DP] */
#define IegA    0x0C, 0x02, EMITB(0x02) /* [r10d + eax + DP] */
#define IegB    0x0C, 0x02, EMITB(0x03) /* [r11d + eax + DP] */
#define IegC    0x0C, 0x02, EMITB(0x04) /* [r12d + eax + DP] */
#define IegD    0x0C, 0x02, EMITB(0x05) /* [r13d + eax + DP] */
#define IegE    0x0C, 0x02, EMITB(0x06) /* [r14d + eax + DP] */

/* immediate    VAL,  TYP,  CMD            (all immediate types are unsigned) */

#define IC(im)  (im), 0x02, EMITB((im) & 0x7F) /* drop sign-ext (zero in ARM) */
#define IB(im)  (im), 0x00, EMITW((im) & 0xFF) /* drop sign-ext (32-bit word) */
#define IM(im)  (im), 0x00, EMITW((im) & 0xFFF) /* native AArch64 add/sub/cmp */
#define IG(im)  (im), 0x00, EMITW((im) & 0x7FFF) /* native MIPS32 add/sub/cmp */
#define IH(im)  (im), 0x00, EMITW((im) & 0xFFFF) /* second native on all ARMs */
#define IV(im)  (im), 0x00, EMITW((im) & 0x7FFFFFFF)  /* native x64 long mode */
#define IW(im)  (im), 0x00, EMITW((im) & 0xFFFFFFFF) /* only for cmdw*_** set */

/* displacement VAL,  TYP,  CMD         (all displacement types are unsigned) */

#define DP(dp)  (dp), 0x00, EMITW((dp) & ((0xFFC  * Q) | 0xC)) /* ext Q=1,2,4 */
#define DF(dp)  (dp), 0x00, EMITW((dp) & ((0x3FFC * Q) | 0xC)) /* ext Q=1,2,4 */
#define DG(dp)  (dp), 0x00, EMITW((dp) & ((0x7FFC * Q) | 0xC)) /* ext Q=1,2,4 */
#define DH(dp)  (dp), 0x00, EMITW((dp) & ((0xFFFC * Q) | 0xC)) /* ext Q=1,2,4 */
#define DV(dp)  (dp), 0x00, EMITW((dp) & 0x7FFFFFFC)  /* native x64 long mode */
#define PLAIN   0x00, 0x00, EMPTY    /* special type for Oeax addressing mode */

/* triplet pass-through wrapper */

#define W(p1, p2, p3)       p1,  p2,  p3

/******************************************************************************/
/**********************************   X32   ***********************************/
/******************************************************************************/

/* mov
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

#define movwx_ld(RD, MS, DS)                                                \
    ADR REX(RXB(RD), RXB(MS)) EMITB(0x8B)                                   \
        MRM(REG(RD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movwx_st(RS, MD, DD)                                                \
    ADR REX(RXB(RS), RXB(MD)) EMITB(0x89)                                   \
        MRM(REG(RS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)


#define adrxx_ld(RD, MS, DS)                                                \
    ADR REW(RXB(RD), RXB(MS)) EMITB(0x8D)                                   \
        MRM(REG(RD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* and
 * set-flags: undefined (*x), yes (*z) */

#define andwx_ri(RG, IS)                                                    \
        andwz_ri(W(RG), W(IS))

#define andwx_mi(MG, DG, IS)                                                \
        andwz_mi(W(MG), W(DG), W(IS))

#define andwx_rr(RG, RS)                                                    \
        andwz_rr(W(RG), W(RS))

#define andwx_ld(RG, MS, DS)                                                \
        andwz_ld(W(RG), W(MS), W(DS))

#define andwx_st(RS, MG, DG)                                                \
        andwz_st(W(RS), W(MG), W(DG))


#define andwz_ri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x04,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IS))

#define andwz_mi(MG, DG, IS)                                                \
    ADR REX(0,       RXB(MG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x04,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), CMD(IS))

#define andwz_rr(RG, RS)                                                    \
        REX(RXB(RG), RXB(RS)) EMITB(0x23)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define andwz_ld(RG, MS, DS)                                                \
    ADR REX(RXB(RG), RXB(MS)) EMITB(0x23)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define andwz_st(RS, MG, DG)                                                \
    ADR REX(RXB(RS), RXB(MG)) EMITB(0x21)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

/* ann (G = ~G & S)
 * set-flags: undefined (*x), yes (*z) */

#define annwx_ri(RG, IS)                                                    \
        annwz_ri(W(RG), W(IS))

#define annwx_mi(MG, DG, IS)                                                \
        annwz_mi(W(MG), W(DG), W(IS))

#define annwx_rr(RG, RS)                                                    \
        annwz_rr(W(RG), W(RS))

#define annwx_ld(RG, MS, DS)                                                \
        annwz_ld(W(RG), W(MS), W(DS))

#define annwx_st(RS, MG, DG)                                                \
        annwz_st(W(RS), W(MG), W(DG))

#define annwx_mr(MG, DG, RS)                                                \
        annwx_st(W(RS), W(MG), W(DG))


#define annwz_ri(RG, IS)                                                    \
        notwx_rx(W(RG))                                                     \
        andwz_ri(W(RG), W(IS))

#define annwz_mi(MG, DG, IS)                                                \
        notwx_mx(W(MG), W(DG))                                              \
        andwz_mi(W(MG), W(DG), W(IS))

/* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */
#if (defined (RT_X32) && RT_X32 < 2) || (defined (RT_X64) && RT_X64 < 2)

#define annwz_rr(RG, RS)                                                    \
        notwx_rx(W(RG))                                                     \
        andwz_rr(W(RG), W(RS))

#define annwz_ld(RG, MS, DS)                                                \
        notwx_rx(W(RG))                                                     \
        andwz_ld(W(RG), W(MS), W(DS))

#else /* RT_X32/RT_X64 >= 2 */

#define annwz_rr(RG, RS)                                                    \
        VEX(RXB(RG), RXB(RS), REN(RG), 0, 0, 2) EMITB(0xF2)                 \
        MRM(REG(RG), MOD(RS), REG(RS))

#define annwz_ld(RG, MS, DS)                                                \
        VEX(RXB(RG), RXB(MS), REN(RG), 0, 0, 2) EMITB(0xF2)                 \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#endif /* RT_X32/RT_X64 >= 2 */

#define annwz_st(RS, MG, DG)                                                \
        notwx_mx(W(MG), W(DG))                                              \
        andwz_st(W(RS), W(MG), W(DG))

#define annwz_mr(MG, DG, RS)                                                \
        annwz_st(W(RS), W(MG), W(DG))

/* orr
 * set-flags: undefined (*x), yes (*z) */

#define orrwx_ri(RG, IS)                                                    \
        orrwz_ri(W(RG), W(IS))

#define orrwx_mi(MG, DG, IS)                                                \
        orrwz_mi(W(MG), W(DG), W(IS))

#define orrwx_rr(RG, RS)                                                    \
        orrwz_rr(W(RG), W(RS))

#define orrwx_ld(RG, MS, DS)                                                \
        orrwz_ld(W(RG), W(MS), W(DS))

#define orrwx_st(RS, MG, DG)                                                \
        orrwz_st(W(RS), W(MG), W(DG))


#define orrwz_ri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x01,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IS))

#define orrwz_mi(MG, DG, IS)                                                \
    ADR REX(0,       RXB(MG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x01,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), CMD(IS))

#define orrwz_rr(RG, RS)                                                    \
        REX(RXB(RG), RXB(RS)) EMITB(0x0B)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define orrwz_ld(RG, MS, DS)                                                \
    ADR REX(RXB(RG), RXB(MS)) EMITB(0x0B)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define orrwz_st(RS, MG, DG)                                                \
    ADR REX(RXB(RS), RXB(MG)) EMITB(0x09)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

/* orn (G = ~G | S)
 * set-flags: undefined (*x), yes (*z) */

#define ornwx_ri(RG, IS)                                                    \
        ornwz_ri(W(RG), W(IS))

#define ornwx_mi(MG, DG, IS)                                                \
        ornwz_mi(W(MG), W(DG), W(IS))

#define ornwx_rr(RG, RS)                                                    \
        ornwz_rr(W(RG), W(RS))

#define ornwx_ld(RG, MS, DS)                                                \
        ornwz_ld(W(RG), W(MS), W(DS))

#define ornwx_st(RS, MG, DG)                                                \
        ornwz_st(W(RS), W(MG), W(DG))

#define ornwx_mr(MG, DG, RS)                                                \
        ornwx_st(W(RS), W(MG), W(DG))


#define ornwz_ri(RG, IS)                                                    \
        notwx_rx(W(RG))                                                     \
        orrwz_ri(W(RG), W(IS))

#define ornwz_mi(MG, DG, IS)                                                \
        notwx_mx(W(MG), W(DG))                                              \
        orrwz_mi(W(MG), W(DG), W(IS))

#define ornwz_rr(RG, RS)                                                    \
        notwx_rx(W(RG))                                                     \
        orrwz_rr(W(RG), W(RS))

#define ornwz_ld(RG, MS, DS)                                                \
        notwx_rx(W(RG))                                                     \
        orrwz_ld(W(RG), W(MS), W(DS))

#define ornwz_st(RS, MG, DG)                                                \
        notwx_mx(W(MG), W(DG))                                              \
        orrwz_st(W(RS), W(MG), W(DG))

#define ornwz_mr(MG, DG, RS)                                                \
        ornwz_st(W(RS), W(MG), W(DG))

/* xor
 * set-flags: undefined (*x), yes (*z) */

#define xorwx_ri(RG, IS)                                                    \
        xorwz_ri(W(RG), W(IS))

#define xorwx_mi(MG, DG, IS)                                                \
        xorwz_mi(W(MG), W(DG), W(IS))

#define xorwx_rr(RG, RS)                                                    \
        xorwz_rr(W(RG), W(RS))

#define xorwx_ld(RG, MS, DS)                                                \
        xorwz_ld(W(RG), W(MS), W(DS))

#define xorwx_st(RS, MG, DG)                                                \
        xorwz_st(W(RS), W(MG), W(DG))


#define xorwz_ri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x06,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IS))

#define xorwz_mi(MG, DG, IS)                                                \
    ADR REX(0,       RXB(MG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x06,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), CMD(IS))

#define xorwz_rr(RG, RS)                                                    \
        REX(RXB(RG), RXB(RS)) EMITB(0x33)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define xorwz_ld(RG, MS, DS)                                                \
    ADR REX(RXB(RG), RXB(MS)) EMITB(0x33)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define xorwz_st(RS, MG, DG)                                                \
    ADR REX(RXB(RS), RXB(MG)) EMITB(0x31)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

/* not
 * set-flags: no */

#define notwx_rx(RG)                                                        \
        REX(0,       RXB(RG)) EMITB(0xF7)                                   \
        MRM(0x02,    MOD(RG), REG(RG))

#define notwx_mx(MG, DG)                                                    \
    ADR REX(0,       RXB(MG)) EMITB(0xF7)                                   \
        MRM(0x02,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

/* neg
 * set-flags: undefined (*x), yes (*z) */

#define negwx_rx(RG)                                                        \
        negwz_rx(W(RG))

#define negwx_mx(MG, DG)                                                    \
        negwz_mx(W(MG), W(DG))


#define negwz_rx(RG)                                                        \
        REX(0,       RXB(RG)) EMITB(0xF7)                                   \
        MRM(0x03,    MOD(RG), REG(RG))

#define negwz_mx(MG, DG)                                                    \
    ADR REX(0,       RXB(MG)) EMITB(0xF7)                                   \
        MRM(0x03,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

/* add
 * set-flags: undefined (*x), yes (*z) */

#define addwx_ri(RG, IS)                                                    \
        addwz_ri(W(RG), W(IS))

#define addwx_mi(MG, DG, IS)                                                \
        addwz_mi(W(MG), W(DG), W(IS))

#define addwx_rr(RG, RS)                                                    \
        addwz_rr(W(RG), W(RS))

#define addwx_ld(RG, MS, DS)                                                \
        addwz_ld(W(RG), W(MS), W(DS))

#define addwx_st(RS, MG, DG)                                                \
        addwz_st(W(RS), W(MG), W(DG))


#define addwz_ri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x00,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IS))

#define addwz_mi(MG, DG, IS)                                                \
    ADR REX(0,       RXB(MG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x00,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), CMD(IS))

#define addwz_rr(RG, RS)                                                    \
        REX(RXB(RG), RXB(RS)) EMITB(0x03)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define addwz_ld(RG, MS, DS)                                                \
    ADR REX(RXB(RG), RXB(MS)) EMITB(0x03)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define addwz_st(RS, MG, DG)                                                \
    ADR REX(RXB(RS), RXB(MG)) EMITB(0x01)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

/* sub
 * set-flags: undefined (*x), yes (*z) */

#define subwx_ri(RG, IS)                                                    \
        subwz_ri(W(RG), W(IS))

#define subwx_mi(MG, DG, IS)                                                \
        subwz_mi(W(MG), W(DG), W(IS))

#define subwx_rr(RG, RS)                                                    \
        subwz_rr(W(RG), W(RS))

#define subwx_ld(RG, MS, DS)                                                \
        subwz_ld(W(RG), W(MS), W(DS))

#define subwx_st(RS, MG, DG)                                                \
        subwz_st(W(RS), W(MG), W(DG))

#define subwx_mr(MG, DG, RS)                                                \
        subwx_st(W(RS), W(MG), W(DG))


#define subwz_ri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x05,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IS))

#define subwz_mi(MG, DG, IS)                                                \
    ADR REX(0,       RXB(MG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x05,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), CMD(IS))

#define subwz_rr(RG, RS)                                                    \
        REX(RXB(RG), RXB(RS)) EMITB(0x2B)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define subwz_ld(RG, MS, DS)                                                \
    ADR REX(RXB(RG), RXB(MS)) EMITB(0x2B)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define subwz_st(RS, MG, DG)                                                \
    ADR REX(RXB(RS), RXB(MG)) EMITB(0x29)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define subwz_mr(MG, DG, RS)                                                \
        subwz_st(W(RS), W(MG), W(DG))

/* shl
 * set-flags: undefined (*x), yes (*z) */

#if RT_X86 < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shlwx_rx(RG)                     /* reads Recx for shift count */   \
        shlwz_rx(W(RG))

#else /* RT_X86 >= 2 */

#define shlwx_rx(RG)                     /* reads Recx for shift count */   \
        VEX(RXB(RG), RXB(RG),    0x01, 0, 1, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#endif /* RT_X86 >= 2 */

#define shlwx_mx(MG, DG)                 /* reads Recx for shift count */   \
        shlwz_mx(W(MG), W(DG))

#define shlwx_ri(RG, IS)                                                    \
        shlwz_ri(W(RG), W(IS))

#define shlwx_mi(MG, DG, IS)                                                \
        shlwz_mi(W(MG), W(DG), W(IS))

#if RT_X86 < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shlwx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shlwz_rr(W(RG), W(RS))

#define shlwx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shlwz_ld(W(RG), W(MS), W(DS))

#define shlwx_st(RS, MG, DG)                                                \
        shlwz_st(W(RS), W(MG), W(DG))

#else /* RT_X86 >= 2 */

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

#endif /* RT_X86 >= 2 */

#define shlwx_mr(MG, DG, RS)                                                \
        shlwx_st(W(RS), W(MG), W(DG))


#define shlwz_rx(RG)                     /* reads Recx for shift count */   \
        REX(0,       RXB(RG)) EMITB(0xD3)                                   \
        MRM(0x04,    MOD(RG), REG(RG))                                      \

#define shlwz_mx(MG, DG)                 /* reads Recx for shift count */   \
    ADR REX(0,       RXB(MG)) EMITB(0xD3)                                   \
        MRM(0x04,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define shlwz_ri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0xC1)                                   \
        MRM(0x04,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))

#define shlwz_mi(MG, DG, IS)                                                \
    ADR REX(0,       RXB(MG)) EMITB(0xC1)                                   \
        MRM(0x04,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMITB(VAL(IS) & 0x1F))

#define shlwz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movwx_rr(Recx, W(RS))                                               \
        shlwz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shlwz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movwx_ld(Recx, W(MS), W(DS))                                        \
        shlwz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shlwz_st(RS, MG, DG)                                                \
        stack_st(Recx)                                                      \
        movwx_rr(Recx, W(RS))                                               \
        shlwz_mx(W(MG), W(DG))                                              \
        stack_ld(Recx)

#define shlwz_mr(MG, DG, RS)                                                \
        shlwz_st(W(RS), W(MG), W(DG))

/* shr
 * set-flags: undefined (*x), yes (*z) */

#if RT_X86 < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shrwx_rx(RG)                     /* reads Recx for shift count */   \
        shrwz_rx(W(RG))

#else /* RT_X86 >= 2 */

#define shrwx_rx(RG)                     /* reads Recx for shift count */   \
        VEX(RXB(RG), RXB(RG),    0x01, 0, 3, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#endif /* RT_X86 >= 2 */

#define shrwx_mx(MG, DG)                 /* reads Recx for shift count */   \
        shrwz_mx(W(MG), W(DG))

#define shrwx_ri(RG, IS)                                                    \
        shrwz_ri(W(RG), W(IS))

#define shrwx_mi(MG, DG, IS)                                                \
        shrwz_mi(W(MG), W(DG), W(IS))

#if RT_X86 < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shrwx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrwz_rr(W(RG), W(RS))

#define shrwx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shrwz_ld(W(RG), W(MS), W(DS))

#define shrwx_st(RS, MG, DG)                                                \
        shrwz_st(W(RS), W(MG), W(DG))

#else /* RT_X86 >= 2 */

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

#endif /* RT_X86 >= 2 */

#define shrwx_mr(MG, DG, RS)                                                \
        shrwx_st(W(RS), W(MG), W(DG))


#define shrwz_rx(RG)                     /* reads Recx for shift count */   \
        REX(0,       RXB(RG)) EMITB(0xD3)                                   \
        MRM(0x05,    MOD(RG), REG(RG))                                      \

#define shrwz_mx(MG, DG)                 /* reads Recx for shift count */   \
    ADR REX(0,       RXB(MG)) EMITB(0xD3)                                   \
        MRM(0x05,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define shrwz_ri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0xC1)                                   \
        MRM(0x05,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))

#define shrwz_mi(MG, DG, IS)                                                \
    ADR REX(0,       RXB(MG)) EMITB(0xC1)                                   \
        MRM(0x05,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMITB(VAL(IS) & 0x1F))

#define shrwz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movwx_rr(Recx, W(RS))                                               \
        shrwz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrwz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movwx_ld(Recx, W(MS), W(DS))                                        \
        shrwz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrwz_st(RS, MG, DG)                                                \
        stack_st(Recx)                                                      \
        movwx_rr(Recx, W(RS))                                               \
        shrwz_mx(W(MG), W(DG))                                              \
        stack_ld(Recx)

#define shrwz_mr(MG, DG, RS)                                                \
        shrwz_st(W(RS), W(MG), W(DG))


#if RT_X86 < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shrwn_rx(RG)                     /* reads Recx for shift count */   \
        REX(0,       RXB(RG)) EMITB(0xD3)                                   \
        MRM(0x07,    MOD(RG), REG(RG))                                      \

#else /* RT_X86 >= 2 */

#define shrwn_rx(RG)                     /* reads Recx for shift count */   \
        VEX(RXB(RG), RXB(RG),    0x01, 0, 2, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#endif /* RT_X86 >= 2 */

#define shrwn_mx(MG, DG)                 /* reads Recx for shift count */   \
    ADR REX(0,       RXB(MG)) EMITB(0xD3)                                   \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define shrwn_ri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0xC1)                                   \
        MRM(0x07,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))

#define shrwn_mi(MG, DG, IS)                                                \
    ADR REX(0,       RXB(MG)) EMITB(0xC1)                                   \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMITB(VAL(IS) & 0x1F))

#if RT_X86 < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

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

#else /* RT_X86 >= 2 */

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

#endif /* RT_X86 >= 2 */

#define shrwn_mr(MG, DG, RS)                                                \
        shrwn_st(W(RS), W(MG), W(DG))

/* ror
 * set-flags: undefined (*x), yes (*z) */

#define rorwx_rx(RG)                     /* reads Recx for shift count */   \
        rorwz_rx(W(RG))

#define rorwx_mx(MG, DG)                 /* reads Recx for shift count */   \
        rorwz_mx(W(MG), W(DG))

#if RT_X86 < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define rorwx_ri(RG, IS)                                                    \
        rorwz_ri(W(RG), W(IS))

#else /* RT_X86 >= 2 */

#define rorwx_ri(RG, IS)                                                    \
        VEX(RXB(RG), RXB(RG),    0x00, 0, 3, 3) EMITB(0xF0)                 \
        MRM(REG(RG), MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))

#endif /* RT_X86 >= 2 */

#define rorwx_mi(MG, DG, IS)                                                \
        rorwz_mi(W(MG), W(DG), W(IS))

#define rorwx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        rorwz_rr(W(RG), W(RS))

#define rorwx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        rorwz_ld(W(RG), W(MS), W(DS))

#define rorwx_st(RS, MG, DG)                                                \
        rorwz_st(W(RS), W(MG), W(DG))

#define rorwx_mr(MG, DG, RS)                                                \
        rorwx_st(W(RS), W(MG), W(DG))


#define rorwz_rx(RG)                     /* reads Recx for shift count */   \
        REX(0,       RXB(RG)) EMITB(0xD3)                                   \
        MRM(0x01,    MOD(RG), REG(RG))                                      \

#define rorwz_mx(MG, DG)                 /* reads Recx for shift count */   \
    ADR REX(0,       RXB(MG)) EMITB(0xD3)                                   \
        MRM(0x01,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define rorwz_ri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0xC1)                                   \
        MRM(0x01,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))

#define rorwz_mi(MG, DG, IS)                                                \
    ADR REX(0,       RXB(MG)) EMITB(0xC1)                                   \
        MRM(0x01,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMITB(VAL(IS) & 0x1F))

#define rorwz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movwx_rr(Recx, W(RS))                                               \
        rorwz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define rorwz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movwx_ld(Recx, W(MS), W(DS))                                        \
        rorwz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define rorwz_st(RS, MG, DG)                                                \
        stack_st(Recx)                                                      \
        movwx_rr(Recx, W(RS))                                               \
        rorwz_mx(W(MG), W(DG))                                              \
        stack_ld(Recx)

#define rorwz_mr(MG, DG, RS)                                                \
        rorwz_st(W(RS), W(MG), W(DG))

/* mul
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


#define mulwx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
    ADR REX(0,       RXB(RS)) EMITB(0xF7)                                   \
        MRM(0x04,    MOD(RS), REG(RS))                                      \

#define mulwx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
    ADR REX(0,       RXB(MS)) EMITB(0xF7)                                   \
        MRM(0x04,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define mulwn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
    ADR REX(0,       RXB(RS)) EMITB(0xF7)                                   \
        MRM(0x05,    MOD(RS), REG(RS))                                      \

#define mulwn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
    ADR REX(0,       RXB(MS)) EMITB(0xF7)                                   \
        MRM(0x05,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define mulwp_xr(RS)     /* Reax is in/out, prepares Redx for divwn_x* */   \
        mulwn_xr(W(RS))       /* product must not exceed operands size */

#define mulwp_xm(MS, DS) /* Reax is in/out, prepares Redx for divwn_x* */   \
        mulwn_xm(W(MS), W(DS))/* product must not exceed operands size */

/* div
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


#define prewx_xx()          /* to be placed immediately prior divwx_x* */   \
        movwx_ri(Redx, IC(0))        /* to prepare Redx for int-divide */

#define prewn_xx()          /* to be placed immediately prior divwn_x* */   \
        movwx_rr(Redx, Reax)         /* to prepare Redx for int-divide */   \
        shrwn_ri(Redx, IC(31))


#define divwx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
    ADR REX(0,       RXB(RS)) EMITB(0xF7)                                   \
        MRM(0x06,    MOD(RS), REG(RS))                                      \
        AUX(EMPTY,   EMPTY,   EMPTY)

#define divwx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
    ADR REX(0,       RXB(MS)) EMITB(0xF7)                                   \
        MRM(0x06,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define divwn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
    ADR REX(0,       RXB(RS)) EMITB(0xF7)                                   \
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

/* rem
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


#define remwx_xx()          /* to be placed immediately prior divwx_x* */   \
                                     /* to prepare for rem calculation */

#define remwx_xr(RS)        /* to be placed immediately after divwx_xr */   \
                                     /* to produce remainder Redx<-rem */

#define remwx_xm(MS, DS)    /* to be placed immediately after divwx_xm */   \
                                     /* to produce remainder Redx<-rem */


#define remwn_xx()          /* to be placed immediately prior divwn_x* */   \
                                     /* to prepare for rem calculation */

#define remwn_xr(RS)        /* to be placed immediately after divwn_xr */   \
                                     /* to produce remainder Redx<-rem */

#define remwn_xm(MS, DS)    /* to be placed immediately after divwn_xm */   \
                                     /* to produce remainder Redx<-rem */

/* arj
 * set-flags: undefined
 * refer to individual instruction descriptions
 * to stay within special register limitations */

#define and_x   and
#define ann_x   ann
#define orr_x   orr
#define orn_x   orn
#define xor_x   xor
#define neg_x   neg
#define add_x   add
#define sub_x   sub
#define shl_x   shl
#define shr_x   shr
#define ror_x   ror

#define EZ_x    jezxx_lb
#define NZ_x    jnzxx_lb


#define arjwx_rx(RG, op, cc, lb)                                            \
        AR1(W(RG), op, wz_rx)                                               \
        CMJ(cc, lb)

#define arjwx_mx(MG, DG, op, cc, lb)                                        \
        AR2(W(MG), W(DG), op, wz_mx)                                        \
        CMJ(cc, lb)

#define arjwx_ri(RG, IS, op, cc, lb)                                        \
        AR2(W(RG), W(IS), op, wz_ri)                                        \
        CMJ(cc, lb)

#define arjwx_mi(MG, DG, IS, op, cc, lb)                                    \
        AR3(W(MG), W(DG), W(IS), op, wz_mi)                                 \
        CMJ(cc, lb)

#define arjwx_rr(RG, RS, op, cc, lb)                                        \
        AR2(W(RG), W(RS), op, wz_rr)                                        \
        CMJ(cc, lb)

#define arjwx_ld(RG, MS, DS, op, cc, lb)                                    \
        AR3(W(RG), W(MS), W(DS), op, wz_ld)                                 \
        CMJ(cc, lb)

#define arjwx_st(RS, MG, DG, op, cc, lb)                                    \
        AR3(W(RS), W(MG), W(DG), op, wz_st)                                 \
        CMJ(cc, lb)

#define arjwx_mr(MG, DG, RS, op, cc, lb)                                    \
        arjwx_st(W(RS), W(MG), W(DG), op, cc, lb)

/* internal definitions for combined-arithmetic-jump (arj) */

#define AR1(P1, op, sg)                                                     \
        op##sg(W(P1))

#define AR2(P1, P2, op, sg)                                                 \
        op##sg(W(P1), W(P2))

#define AR3(P1, P2, P3, op, sg)                                             \
        op##sg(W(P1), W(P2), W(P3))

#define CMJ(cc, lb)                                                         \
        cc(lb)

/* cmj
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

#define cmjwx_mr(MS, DS, RT, cc, lb)                                        \
        cmpwx_mr(W(MS), W(DS), W(RT))                                       \
        CMJ(cc, lb)

/* cmp
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

#define cmpwx_mr(MS, DS, RT)                                                \
    ADR REX(RXB(RT), RXB(MS)) EMITB(0x39)                                   \
        MRM(REG(RT), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/***************** pointer-sized instructions for hybrid mode *****************/

/* jmp
 * set-flags: no
 * maximum byte-address-range for un/conditional jumps is signed 18/16-bit
 * based on minimum natively-encoded offset across supported targets (u/c)
 * MIPS:18-bit, Power:26-bit, AArch32:26-bit, AArch64:28-bit, x86:32-bit /
 * MIPS:18-bit, Power:16-bit, AArch32:26-bit, AArch64:21-bit, x86:32-bit */

#define jmpxx_xr(RS)           /* register-targeted unconditional jump */   \
        REX(0,       RXB(RS)) EMITB(0xFF)   /* <- jump to address in reg */ \
        MRM(0x04,    MOD(RS), REG(RS))

#if   defined (RT_X32)

#define jmpxx_xm(MS, DS)         /* memory-targeted unconditional jump */   \
    ADR REX(1,       RXB(MS)) EMITB(0x8B)   /* <- load r15d from MS/DS */   \
        MRM(0x07,    MOD(MS), REG(MS))      /*    upper half is zeroed */   \
        AUX(SIB(MS), CMD(DS), EMPTY)        /*    as a part of 32-bit op */ \
        REX(0,             1) EMITB(0xFF)   /* <- jump to address in r15 */ \
        MRM(0x04,       0x03, 0x07)

#elif defined (RT_X64)

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

/* stack
 * set-flags: no (sequence cmp/stack_la/jmp is not allowed on MIPS & Power)
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

/* ver
 * set-flags: no */

#define cpuid_xx() /* destroys Reax, Recx, Rebx, Redx, reads Reax, Recx */  \
        EMITB(0x0F) EMITB(0xA2)     /* not portable, do not use outside */

#define verxx_xx() /* destroys Reax, Recx, Rebx, Redx, Resi, Redi */        \
        /* request cpuid:eax=1 */                                           \
        movwx_ri(Reax, IB(1))                                               \
        cpuid_xx()                                                          \
        shrwx_ri(Redx, IB(25))  /* <- SSE1, SSE2 to bit0, bit1 */           \
        andwx_ri(Redx, IB(0x03))                                            \
        movwx_rr(Resi, Redx)                                                \
        movwx_rr(Redx, Recx)                                                \
        shrwx_ri(Redx, IB(17))  /* <- SSE4 to bit2 */                       \
        andwx_ri(Redx, IB(0x04))                                            \
        shrwx_ri(Recx, IB(20))  /* <- AVX1 to bit8 */                       \
        andwx_ri(Recx, IH(0x0100))                                          \
        orrwx_rr(Resi, Redx)                                                \
        orrwx_rr(Resi, Recx)                                                \
        /* request cpuid:eax=0 to test input value eax=7 */                 \
        movwx_ri(Reax, IB(0))                                               \
        cpuid_xx()                                                          \
        subwx_ri(Reax, IB(7))                                               \
        shrwn_ri(Reax, IB(31))                                              \
        movwx_rr(Redi, Reax)                                                \
        notwx_rx(Redi)                                                      \
        /* request cpuid:eax=7:ecx=0 */                                     \
        movwx_ri(Reax, IB(7))                                               \
        movwx_ri(Recx, IB(0))                                               \
        cpuid_xx()                                                          \
        shlwx_ri(Rebx, IB(4))   /* <- AVX2 to bit9 */                       \
        andwx_ri(Rebx, IH(0x0200))                                          \
        andwx_rr(Rebx, Redi)                                                \
        orrwx_rr(Resi, Rebx)                                                \
        movwx_st(Resi, Mebp, inf_VER)

#endif /* RT_RTARCH_X32_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

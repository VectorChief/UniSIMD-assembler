/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X86_H
#define RT_RTARCH_X86_H

#define RT_BASE_REGS        8

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_x86.h: Implementation of x86:i386 BASE instructions.
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

#define MRM(reg, mod, rem)                                                  \
        EMITB((mod) << 6 | (reg) << 3 | (rem))

#define AUX(sib, cdp, cim)  sib  cdp  cim

/* selectors  */

#define REG(reg, mod, sib)  reg
#define MOD(reg, mod, sib)  mod
#define SIB(reg, mod, sib)  sib

#define VAL(val, typ, cmd)  val
#define TYP(val, typ, cmd)  typ
#define CMD(val, typ, cmd)  cmd

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

/* addressing   REG,  MOD,  SIB */

#define Oeax    0x00, 0x00, EMPTY       /* [eax] */

#define Mecx    0x01, 0x02, EMPTY       /* [ecx + DP] */
#define Medx    0x02, 0x02, EMPTY       /* [edx + DP] */
#define Mebx    0x03, 0x02, EMPTY       /* [ebx + DP] */
#define Mebp    0x05, 0x02, EMPTY       /* [ebp + DP] */
#define Mesi    0x06, 0x02, EMPTY       /* [esi + DP] */
#define Medi    0x07, 0x02, EMPTY       /* [edi + DP] */

#define Iecx    0x04, 0x02, EMITB(0x01) /* [ecx + eax + DP] */
#define Iedx    0x04, 0x02, EMITB(0x02) /* [edx + eax + DP] */
#define Iebx    0x04, 0x02, EMITB(0x03) /* [ebx + eax + DP] */
#define Iebp    0x04, 0x02, EMITB(0x05) /* [ebp + eax + DP] */
#define Iesi    0x04, 0x02, EMITB(0x06) /* [esi + eax + DP] */
#define Iedi    0x04, 0x02, EMITB(0x07) /* [edi + eax + DP] */

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
/**********************************   X86   ***********************************/
/******************************************************************************/

/* mov
 * set-flags: no */

#define movwx_ri(RM, IM)                                                    \
        EMITB(0xC7)                                                         \
        MRM(0x00,    MOD(RM), REG(RM))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITW(VAL(IM) & ((TYP(IM) << 6) - 1)))

#define movwx_mi(RM, DP, IM)                                                \
        EMITB(0xC7)                                                         \
        MRM(0x00,    MOD(RM), REG(RM))   /* truncate IC with TYP below */   \
        AUX(SIB(RM), CMD(DP), EMITW(VAL(IM) & ((TYP(IM) << 6) - 1)))

#define movwx_rr(RG, RM)                                                    \
        EMITB(0x8B)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))

#define movwx_ld(RG, RM, DP)                                                \
        EMITB(0x8B)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define movwx_st(RG, RM, DP)                                                \
        EMITB(0x89)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)


#define adrxx_ld(RG, RM, DP)                                                \
        EMITB(0x8D)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* and
 * set-flags: undefined (*x), yes (*z) */

#define andwx_ri(RM, IM)                                                    \
        andwz_ri(W(RM), W(IM))

#define andwx_mi(RM, DP, IM)                                                \
        andwz_mi(W(RM), W(DP), W(IM))

#define andwx_rr(RG, RM)                                                    \
        andwz_rr(W(RG), W(RM))

#define andwx_ld(RG, RM, DP)                                                \
        andwz_ld(W(RG), W(RM), W(DP))

#define andwx_st(RG, RM, DP)                                                \
        andwz_st(W(RG), W(RM), W(DP))


#define andwz_ri(RM, IM)                                                    \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define andwz_mi(RM, DP, IM)                                                \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define andwz_rr(RG, RM)                                                    \
        EMITB(0x23)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))

#define andwz_ld(RG, RM, DP)                                                \
        EMITB(0x23)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define andwz_st(RG, RM, DP)                                                \
        EMITB(0x21)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* orr
 * set-flags: undefined (*x), yes (*z) */

#define orrwx_ri(RM, IM)                                                    \
        orrwz_ri(W(RM), W(IM))

#define orrwx_mi(RM, DP, IM)                                                \
        orrwz_mi(W(RM), W(DP), W(IM))

#define orrwx_rr(RG, RM)                                                    \
        orrwz_rr(W(RG), W(RM))

#define orrwx_ld(RG, RM, DP)                                                \
        orrwz_ld(W(RG), W(RM), W(DP))

#define orrwx_st(RG, RM, DP)                                                \
        orrwz_st(W(RG), W(RM), W(DP))


#define orrwz_ri(RM, IM)                                                    \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(0x01,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define orrwz_mi(RM, DP, IM)                                                \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(0x01,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define orrwz_rr(RG, RM)                                                    \
        EMITB(0x0B)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))

#define orrwz_ld(RG, RM, DP)                                                \
        EMITB(0x0B)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define orrwz_st(RG, RM, DP)                                                \
        EMITB(0x09)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* xor
 * set-flags: undefined (*x), yes (*z) */

#define xorwx_ri(RM, IM)                                                    \
        xorwz_ri(W(RM), W(IM))

#define xorwx_mi(RM, DP, IM)                                                \
        xorwz_mi(W(RM), W(DP), W(IM))

#define xorwx_rr(RG, RM)                                                    \
        xorwz_rr(W(RG), W(RM))

#define xorwx_ld(RG, RM, DP)                                                \
        xorwz_ld(W(RG), W(RM), W(DP))

#define xorwx_st(RG, RM, DP)                                                \
        xorwz_st(W(RG), W(RM), W(DP))


#define xorwz_ri(RM, IM)                                                    \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(0x06,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define xorwz_mi(RM, DP, IM)                                                \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(0x06,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define xorwz_rr(RG, RM)                                                    \
        EMITB(0x33)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))

#define xorwz_ld(RG, RM, DP)                                                \
        EMITB(0x33)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define xorwz_st(RG, RM, DP)                                                \
        EMITB(0x31)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* not
 * set-flags: no */

#define notwx_rx(RM)                                                        \
        EMITB(0xF7)                                                         \
        MRM(0x02,    MOD(RM), REG(RM))

#define notwx_mx(RM, DP)                                                    \
        EMITB(0xF7)                                                         \
        MRM(0x02,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* neg
 * set-flags: undefined (*x), yes (*z) */

#define negwx_rx(RM)                                                        \
        negwz_rx(W(RM))

#define negwx_mx(RM, DP)                                                    \
        negwz_mx(W(RM), W(DP))


#define negwz_rx(RM)                                                        \
        EMITB(0xF7)                                                         \
        MRM(0x03,    MOD(RM), REG(RM))

#define negwz_mx(RM, DP)                                                    \
        EMITB(0xF7)                                                         \
        MRM(0x03,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* add
 * set-flags: undefined (*x), yes (*z) */

#define addwx_ri(RM, IM)                                                    \
        addwz_ri(W(RM), W(IM))

#define addwx_mi(RM, DP, IM)                                                \
        addwz_mi(W(RM), W(DP), W(IM))

#define addwx_rr(RG, RM)                                                    \
        addwz_rr(W(RG), W(RM))

#define addwx_ld(RG, RM, DP)                                                \
        addwz_ld(W(RG), W(RM), W(DP))

#define addwx_st(RG, RM, DP)                                                \
        addwz_st(W(RG), W(RM), W(DP))


#define addwz_ri(RM, IM)                                                    \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(0x00,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define addwz_mi(RM, DP, IM)                                                \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(0x00,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define addwz_rr(RG, RM)                                                    \
        EMITB(0x03)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))

#define addwz_ld(RG, RM, DP)                                                \
        EMITB(0x03)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define addwz_st(RG, RM, DP)                                                \
        EMITB(0x01)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sub
 * set-flags: undefined (*x), yes (*z) */

#define subwx_ri(RM, IM)                                                    \
        subwz_ri(W(RM), W(IM))

#define subwx_mi(RM, DP, IM)                                                \
        subwz_mi(W(RM), W(DP), W(IM))

#define subwx_rr(RG, RM)                                                    \
        subwz_rr(W(RG), W(RM))

#define subwx_ld(RG, RM, DP)                                                \
        subwz_ld(W(RG), W(RM), W(DP))

#define subwx_st(RG, RM, DP)                                                \
        subwz_st(W(RG), W(RM), W(DP))

#define subwx_mr(RM, DP, RG)                                                \
        subwx_st(W(RG), W(RM), W(DP))


#define subwz_ri(RM, IM)                                                    \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define subwz_mi(RM, DP, IM)                                                \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define subwz_rr(RG, RM)                                                    \
        EMITB(0x2B)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))

#define subwz_ld(RG, RM, DP)                                                \
        EMITB(0x2B)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define subwz_st(RG, RM, DP)                                                \
        EMITB(0x29)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define subwz_mr(RM, DP, RG)                                                \
        subwz_st(W(RG), W(RM), W(DP))

/* shl
 * set-flags: undefined (*x), yes (*z) */

#define shlwx_rx(RM)                     /* reads Recx for shift value */   \
        shlwz_rx(W(RM))

#define shlwx_mx(RM, DP)                 /* reads Recx for shift value */   \
        shlwz_mx(W(RM), W(DP))

#define shlwx_ri(RM, IM)                                                    \
        shlwz_ri(W(RM), W(IM))

#define shlwx_mi(RM, DP, IM)                                                \
        shlwz_mi(W(RM), W(DP), W(IM))

#define shlwx_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        shlwz_rr(W(RG), W(RM))

#define shlwx_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        shlwz_ld(W(RG), W(RM), W(DP))

#define shlwx_st(RG, RM, DP)                                                \
        shlwz_st(W(RG), W(RM), W(DP))

#define shlwx_mr(RM, DP, RG)                                                \
        shlwx_st(W(RG), W(RM), W(DP))


#define shlwz_rx(RM)                     /* reads Recx for shift value */   \
        EMITB(0xD3)                                                         \
        MRM(0x04,    MOD(RM), REG(RM))                                      \

#define shlwz_mx(RM, DP)                 /* reads Recx for shift value */   \
        EMITB(0xD3)                                                         \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define shlwz_ri(RM, IM)                                                    \
        EMITB(0xC1)                                                         \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shlwz_mi(RM, DP, IM)                                                \
        EMITB(0xC1)                                                         \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(VAL(IM) & 0x1F))

#define shlwz_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movwx_rr(Recx, W(RM))                                               \
        shlwz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shlwz_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movwx_ld(Recx, W(RM), W(DP))                                        \
        shlwz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shlwz_st(RG, RM, DP)                                                \
        stack_st(Recx)                                                      \
        movwx_rr(Recx, W(RG))                                               \
        shlwz_mx(W(RM), W(DP))                                              \
        stack_ld(Recx)

#define shlwz_mr(RM, DP, RG)                                                \
        shlwz_st(W(RG), W(RM), W(DP))

/* shr
 * set-flags: undefined (*x), yes (*z) */

#define shrwx_rx(RM)                     /* reads Recx for shift value */   \
        shrwz_rx(W(RM))

#define shrwx_mx(RM, DP)                 /* reads Recx for shift value */   \
        shrwz_mx(W(RM), W(DP))

#define shrwx_ri(RM, IM)                                                    \
        shrwz_ri(W(RM), W(IM))

#define shrwx_mi(RM, DP, IM)                                                \
        shrwz_mi(W(RM), W(DP), W(IM))

#define shrwx_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        shrwz_rr(W(RG), W(RM))

#define shrwx_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        shrwz_ld(W(RG), W(RM), W(DP))

#define shrwx_st(RG, RM, DP)                                                \
        shrwz_st(W(RG), W(RM), W(DP))

#define shrwx_mr(RM, DP, RG)                                                \
        shrwx_st(W(RG), W(RM), W(DP))


#define shrwz_rx(RM)                     /* reads Recx for shift value */   \
        EMITB(0xD3)                                                         \
        MRM(0x05,    MOD(RM), REG(RM))                                      \

#define shrwz_mx(RM, DP)                 /* reads Recx for shift value */   \
        EMITB(0xD3)                                                         \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define shrwz_ri(RM, IM)                                                    \
        EMITB(0xC1)                                                         \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shrwz_mi(RM, DP, IM)                                                \
        EMITB(0xC1)                                                         \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(VAL(IM) & 0x1F))

#define shrwz_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movwx_rr(Recx, W(RM))                                               \
        shrwz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrwz_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movwx_ld(Recx, W(RM), W(DP))                                        \
        shrwz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrwz_st(RG, RM, DP)                                                \
        stack_st(Recx)                                                      \
        movwx_rr(Recx, W(RG))                                               \
        shrwz_mx(W(RM), W(DP))                                              \
        stack_ld(Recx)

#define shrwz_mr(RM, DP, RG)                                                \
        shrwz_st(W(RG), W(RM), W(DP))


#define shrwn_rx(RM)                     /* reads Recx for shift value */   \
        EMITB(0xD3)                                                         \
        MRM(0x07,    MOD(RM), REG(RM))                                      \

#define shrwn_mx(RM, DP)                 /* reads Recx for shift value */   \
        EMITB(0xD3)                                                         \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define shrwn_ri(RM, IM)                                                    \
        EMITB(0xC1)                                                         \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shrwn_mi(RM, DP, IM)                                                \
        EMITB(0xC1)                                                         \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(VAL(IM) & 0x1F))

#define shrwn_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movwx_rr(Recx, W(RM))                                               \
        shrwn_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrwn_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movwx_ld(Recx, W(RM), W(DP))                                        \
        shrwn_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrwn_st(RG, RM, DP)                                                \
        stack_st(Recx)                                                      \
        movwx_rr(Recx, W(RG))                                               \
        shrwn_mx(W(RM), W(DP))                                              \
        stack_ld(Recx)

#define shrwn_mr(RM, DP, RG)                                                \
        shrwn_st(W(RG), W(RM), W(DP))

/* mul
 * set-flags: undefined */

#define mulwx_ri(RM, IM)                                                    \
        EMITB(0x69 | TYP(IM))                                               \
        MRM(REG(RM), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define mulwx_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xAF)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define mulwx_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0xAF)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)


#define mulwx_xr(RM)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITB(0xF7)                                                         \
        MRM(0x04,    MOD(RM), REG(RM))                                      \

#define mulwx_xm(RM, DP) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITB(0xF7)                                                         \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)


#define mulwn_xr(RM)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITB(0xF7)                                                         \
        MRM(0x05,    MOD(RM), REG(RM))                                      \

#define mulwn_xm(RM, DP) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITB(0xF7)                                                         \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)


#define mulwp_xr(RM)     /* Reax is in/out, prepares Redx for divwn_x* */   \
        mulwn_xr(W(RM))       /* product must not exceed operands size */

#define mulwp_xm(RM, DP) /* Reax is in/out, prepares Redx for divwn_x* */   \
        mulwn_xm(W(RM), W(DP))/* product must not exceed operands size */

/* div
 * set-flags: undefined */

#define divwx_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movwx_mi(Mebp, inf_SCR01(0), W(IM))                                 \
        movwx_rr(Reax, W(RM))                                               \
        prewx_rr()                                                          \
        divwx_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Redx)                                                      \
        movwx_rr(W(RM), Reax)                                               \
        stack_ld(Reax)

#define divwx_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movwx_rr(Reax, W(RG))                                               \
        prewx_rr()                                                          \
        divwx_xr(W(RM))                                                     \
        stack_ld(Redx)                                                      \
        movwx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divwx_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movwx_rr(Reax, W(RG))                                               \
        prewx_rr()                                                          \
        divwx_xm(W(RM), W(DP))                                              \
        stack_ld(Redx)                                                      \
        movwx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)


#define divwn_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movwx_mi(Mebp, inf_SCR01(0), W(IM))                                 \
        movwx_rr(Reax, W(RM))                                               \
        prewn_rr()                                                          \
        divwn_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Redx)                                                      \
        movwx_rr(W(RM), Reax)                                               \
        stack_ld(Reax)

#define divwn_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movwx_rr(Reax, W(RG))                                               \
        prewn_rr()                                                          \
        divwn_xr(W(RM))                                                     \
        stack_ld(Redx)                                                      \
        movwx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divwn_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movwx_rr(Reax, W(RG))                                               \
        prewn_rr()                                                          \
        divwn_xm(W(RM), W(DP))                                              \
        stack_ld(Redx)                                                      \
        movwx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)


#define prewx_xx()          /* to be placed immediately prior divwx_x* */   \
        movwx_ri(Redx, IC(0))        /* to prepare Redx for int-divide */

#define prewn_xx()          /* to be placed immediately prior divwn_x* */   \
        movwx_rr(Redx, Reax)         /* to prepare Redx for int-divide */   \
        shrwn_ri(Redx, IC(31))


#define divwx_xr(RM)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITB(0xF7)                                                         \
        MRM(0x06,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMPTY)

#define divwx_xm(RM, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITB(0xF7)                                                         \
        MRM(0x06,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)


#define divwn_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITB(0xF7)                                                         \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMPTY)

#define divwn_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITB(0xF7)                                                         \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)


#define divwp_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xr(W(RM))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divwp_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xm(W(RM), W(DP))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem
 * set-flags: undefined */

#define remwx_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movwx_mi(Mebp, inf_SCR01(0), W(IM))                                 \
        movwx_rr(Reax, W(RM))                                               \
        prewx_rr()                                                          \
        divwx_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Reax)                                                      \
        movwx_rr(W(RM), Redx)                                               \
        stack_ld(Redx)

#define remwx_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movwx_rr(Reax, W(RG))                                               \
        prewx_rr()                                                          \
        divwx_xr(W(RM))                                                     \
        stack_ld(Reax)                                                      \
        movwx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remwx_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movwx_rr(Reax, W(RG))                                               \
        prewx_rr()                                                          \
        divwx_xm(W(RM), W(DP))                                              \
        stack_ld(Reax)                                                      \
        movwx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)


#define remwn_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movwx_mi(Mebp, inf_SCR01(0), W(IM))                                 \
        movwx_rr(Reax, W(RM))                                               \
        prewn_rr()                                                          \
        divwn_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Reax)                                                      \
        movwx_rr(W(RM), Redx)                                               \
        stack_ld(Redx)

#define remwn_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movwx_rr(Reax, W(RG))                                               \
        prewn_rr()                                                          \
        divwn_xr(W(RM))                                                     \
        stack_ld(Reax)                                                      \
        movwx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remwn_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movwx_rr(Reax, W(RG))                                               \
        prewn_rr()                                                          \
        divwn_xm(W(RM), W(DP))                                              \
        stack_ld(Reax)                                                      \
        movwx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)


#define remwx_xx()          /* to be placed immediately prior divwx_x* */   \
                                     /* to prepare for rem calculation */

#define remwx_xr(RM)        /* to be placed immediately after divwx_xr */   \
                                     /* to produce remainder Redx<-rem */

#define remwx_xm(RM, DP)    /* to be placed immediately after divwx_xm */   \
                                     /* to produce remainder Redx<-rem */


#define remwn_xx()          /* to be placed immediately prior divwn_x* */   \
                                     /* to prepare for rem calculation */

#define remwn_xr(RM)        /* to be placed immediately after divwn_xr */   \
                                     /* to produce remainder Redx<-rem */

#define remwn_xm(RM, DP)    /* to be placed immediately after divwn_xm */   \
                                     /* to produce remainder Redx<-rem */

/* arj
 * set-flags: undefined
 * refer to individual instructions' description
 * to stay within special register limitations */

#define and_x   and
#define orr_x   orr
#define xor_x   xor

#define neg_x   neg
#define add_x   add
#define sub_x   sub

#define shl_x   shl
#define shr_x   shr

#define EZ_x    jezxx_lb
#define NZ_x    jnzxx_lb


#define arjwx_rx(RM, op, cc, lb)                                            \
        AR1(W(RM), op, wz_rx)                                               \
        CMJ(cc, lb)

#define arjwx_mx(RM, DP, op, cc, lb)                                        \
        AR2(W(RM), W(DP), op, wz_mx)                                        \
        CMJ(cc, lb)

#define arjwx_ri(RM, IM, op, cc, lb)                                        \
        AR2(W(RM), W(IM), op, wz_ri)                                        \
        CMJ(cc, lb)

#define arjwx_mi(RM, DP, IM, op, cc, lb)                                    \
        AR3(W(RM), W(DP), W(IM), op, wz_mi)                                 \
        CMJ(cc, lb)

#define arjwx_rr(RG, RM, op, cc, lb)                                        \
        AR2(W(RG), W(RM), op, wz_rr)                                        \
        CMJ(cc, lb)

#define arjwx_ld(RG, RM, DP, op, cc, lb)                                    \
        AR3(W(RG), W(RM), W(DP), op, wz_ld)                                 \
        CMJ(cc, lb)

#define arjwx_st(RG, RM, DP, op, cc, lb)                                    \
        AR3(W(RG), W(RM), W(DP), op, wz_st)                                 \
        CMJ(cc, lb)

#define arjwx_mr(RM, DP, RG, op, cc, lb)                                    \
        arjwx_st(W(RG), W(RM), W(DP), op, cc, lb)

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


#define cmjwx_rz(RM, cc, lb)                                                \
        cmjwx_ri(W(RM), IC(0), cc, lb)

#define cmjwx_mz(RM, DP, cc, lb)                                            \
        cmjwx_mi(W(RM), W(DP), IC(0), cc, lb)

#define cmjwx_ri(RM, IM, cc, lb)                                            \
        cmpwx_ri(W(RM), W(IM))                                              \
        CMJ(cc, lb)

#define cmjwx_mi(RM, DP, IM, cc, lb)                                        \
        cmpwx_mi(W(RM), W(DP), W(IM))                                       \
        CMJ(cc, lb)

#define cmjwx_rr(RG, RM, cc, lb)                                            \
        cmpwx_rr(W(RG), W(RM))                                              \
        CMJ(cc, lb)

#define cmjwx_rm(RG, RM, DP, cc, lb)                                        \
        cmpwx_rm(W(RG), W(RM), W(DP))                                       \
        CMJ(cc, lb)

#define cmjwx_mr(RM, DP, RG, cc, lb)                                        \
        cmpwx_mr(W(RM), W(DP), W(RG))                                       \
        CMJ(cc, lb)

/* cmp
 * set-flags: yes */

#define cmpwx_ri(RM, IM)                                                    \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define cmpwx_mi(RM, DP, IM)                                                \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define cmpwx_rr(RG, RM)                                                    \
        EMITB(0x3B)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))

#define cmpwx_rm(RG, RM, DP)                                                \
        EMITB(0x3B)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define cmpwx_mr(RM, DP, RG)                                                \
        EMITB(0x39)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/***************** pointer-sized instructions for 32-bit mode *****************/

/* jmp
 * set-flags: no
 * maximum byte-address-range for un/conditional jumps is signed 18/16-bit
 * based on minimum natively-encoded offset across supported targets (u/c)
 * MIPS:18-bit, Power:26-bit, AArch32:26-bit, AArch64:28-bit, x86:32-bit /
 * MIPS:18-bit, Power:16-bit, AArch32:26-bit, AArch64:21-bit, x86:32-bit */

#define jmpxx_xr(RM)           /* register-targeted unconditional jump */   \
        EMITB(0xFF)                         /* <- jump to address in reg */ \
        MRM(0x04,    MOD(RM), REG(RM))

#define jmpxx_xm(RM, DP)         /* memory-targeted unconditional jump */   \
        EMITB(0xFF)                         /* <- jump to address in mem */ \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

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
 * adjust stack pointer with 4-byte (32-bit) steps on legacy 32-bit targets */

#define stack_st(RM)                                                        \
        EMITB(0xFF)                                                         \
        MRM(0x06,    MOD(RM), REG(RM))

#define stack_ld(RM)                                                        \
        EMITB(0x8F)                                                         \
        MRM(0x00,    MOD(RM), REG(RM))

#define stack_sa()   /* save all [Reax - Redi], 8 regs in total */          \
        EMITB(0x60)

#define stack_la()   /* load all [Redi - Reax], 8 regs in total */          \
        EMITB(0x61)

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

#endif /* RT_RTARCH_X86_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

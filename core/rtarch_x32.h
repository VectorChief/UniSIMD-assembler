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
 * cmdxx_rr - applies [cmd] to [r]egister (one operand cmd)
 * cmdxx_mm - applies [cmd] to [m]emory   (one operand cmd)
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
 * cmdx*_** - applies [cmd] to BASE register/memory/immediate args
 * cmd*x_** - applies [cmd] to unsigned integer args, [x] - default
 * cmd*n_** - applies [cmd] to   signed integer args, [n] - negatable
 *
 * cmdz*_** - applies [cmd] while setting condition flags, [z] - zero flag.
 * Regular cmdxx_** instructions may or may not set flags depending
 * on the target architecture, thus no assumptions can be made for jezxx/jnzxx.
 *
 * Argument x-register (implied) is fixed by the implementation.
 * Some formal definitions are not given below to encourage
 * use of friendly aliases for better code readability.
 */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/* structural */

#if defined (RT_X32)

#define ADR                                                                 \
        EMITB(0x67)

#elif defined (RT_X64)

#define ADR

#endif /* defined (RT_X32, RT_X64) */

#define REX(rxg, rxm)                                                       \
        EMITB(0x40 | (rxg) << 2 | (rxm))

#define MRM(reg, mod, rem)                                                  \
        EMITB((mod) << 6 | (reg) << 3 | (rem))

#define AUX(sib, cdp, cim)  sib  cdp  cim

/* selectors  */

#define RXB(reg, mod, sib)  (reg >> 3 & 0x01) /* register-extension-bit */
#define REG(reg, mod, sib)  (reg >> 0 & 0x07) /* register, lower 3-bits */
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
#define IW(im)  (im), 0x00, EMITW((im) & 0xFFFFFFFF)  /* extra load op on x64 */

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

#define movxx_ri(RM, IM)                                                    \
        REX(0,       RXB(RM)) EMITB(0xC7)                                   \
        MRM(0x00,    MOD(RM), REG(RM))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITW(VAL(IM) & ((TYP(IM) << 6) - 1)))

#define movxx_mi(RM, DP, IM)                                                \
    ADR REX(0,       RXB(RM)) EMITB(0xC7)                                   \
        MRM(0x00,    MOD(RM), REG(RM))   /* truncate IC with TYP below */   \
        AUX(SIB(RM), CMD(DP), EMITW(VAL(IM) & ((TYP(IM) << 6) - 1)))

#define movxx_rr(RG, RM)                                                    \
        REX(RXB(RG), RXB(RM)) EMITB(0x8B)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))

#define movxx_ld(RG, RM, DP)                                                \
    ADR REX(RXB(RG), RXB(RM)) EMITB(0x8B)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define movxx_st(RG, RM, DP)                                                \
    ADR REX(RXB(RG), RXB(RM)) EMITB(0x89)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define adrxx_ld(RG, RM, DP)                                                \
    ADR REX(RXB(RG), RXB(RM)) EMITB(0x8D)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define adrxx_lb(lb) /* load label to Reax */                               \
        label_ld(lb)

#define stack_st(RM)                                                        \
        REX(0,       RXB(RM)) EMITB(0xFF)                                   \
        MRM(0x06,    MOD(RM), REG(RM))

#define stack_ld(RM)                                                        \
        REX(0,       RXB(RM)) EMITB(0x8F)                                   \
        MRM(0x00,    MOD(RM), REG(RM))

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

/* and
 * set-flags: undefined (xx), yes (zx) */

#define andxx_ri(RM, IM)                                                    \
        REX(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define andxx_mi(RM, DP, IM)                                                \
    ADR REX(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define andxx_rr(RG, RM)                                                    \
        REX(RXB(RG), RXB(RM)) EMITB(0x23)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))

#define andxx_ld(RG, RM, DP)                                                \
    ADR REX(RXB(RG), RXB(RM)) EMITB(0x23)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define andxx_st(RG, RM, DP)                                                \
    ADR REX(RXB(RG), RXB(RM)) EMITB(0x21)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define andzx_ri(RM, IM)                                                    \
        REX(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define andzx_mi(RM, DP, IM)                                                \
    ADR REX(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define andzx_rr(RG, RM)                                                    \
        REX(RXB(RG), RXB(RM)) EMITB(0x23)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))

#define andzx_ld(RG, RM, DP)                                                \
    ADR REX(RXB(RG), RXB(RM)) EMITB(0x23)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define andzx_st(RG, RM, DP)                                                \
    ADR REX(RXB(RG), RXB(RM)) EMITB(0x21)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* orr
 * set-flags: undefined */

#define orrxx_ri(RM, IM)                                                    \
        REX(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x01,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define orrxx_mi(RM, DP, IM)                                                \
    ADR REX(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x01,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define orrxx_rr(RG, RM)                                                    \
        REX(RXB(RG), RXB(RM)) EMITB(0x0B)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))

#define orrxx_ld(RG, RM, DP)                                                \
    ADR REX(RXB(RG), RXB(RM)) EMITB(0x0B)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define orrxx_st(RG, RM, DP)                                                \
    ADR REX(RXB(RG), RXB(RM)) EMITB(0x09)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* xor
 * set-flags: undefined */

#define xorxx_ri(RM, IM)                                                    \
        REX(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x06,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define xorxx_mi(RM, DP, IM)                                                \
    ADR REX(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x06,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define xorxx_rr(RG, RM)                                                    \
        REX(RXB(RG), RXB(RM)) EMITB(0x33)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))

#define xorxx_ld(RG, RM, DP)                                                \
    ADR REX(RXB(RG), RXB(RM)) EMITB(0x33)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define xorxx_st(RG, RM, DP)                                                \
    ADR REX(RXB(RG), RXB(RM)) EMITB(0x31)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* not
 * set-flags: no */

#define notxx_rr(RM)                                                        \
        REX(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x02,    MOD(RM), REG(RM))

#define notxx_mm(RM, DP)                                                    \
    ADR REX(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x02,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* neg
 * set-flags: undefined (xx), yes (zx) */

#define negxx_rr(RM)                                                        \
        REX(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x03,    MOD(RM), REG(RM))

#define negxx_mm(RM, DP)                                                    \
    ADR REX(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x03,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define negzx_rr(RM)                                                        \
        REX(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x03,    MOD(RM), REG(RM))

#define negzx_mm(RM, DP)                                                    \
    ADR REX(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x03,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* add
 * set-flags: undefined (xx), yes (zx) */

#define addxx_ri(RM, IM)                                                    \
        REX(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x00,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define addxx_mi(RM, DP, IM)                                                \
    ADR REX(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x00,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define addxx_rr(RG, RM)                                                    \
        REX(RXB(RG), RXB(RM)) EMITB(0x03)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))

#define addxx_ld(RG, RM, DP)                                                \
    ADR REX(RXB(RG), RXB(RM)) EMITB(0x03)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define addxx_st(RG, RM, DP)                                                \
    ADR REX(RXB(RG), RXB(RM)) EMITB(0x01)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define addzx_ri(RM, IM)                                                    \
        REX(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x00,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define addzx_mi(RM, DP, IM)                                                \
    ADR REX(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x00,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define addzx_rr(RG, RM)                                                    \
        REX(RXB(RG), RXB(RM)) EMITB(0x03)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))

#define addzx_ld(RG, RM, DP)                                                \
    ADR REX(RXB(RG), RXB(RM)) EMITB(0x03)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define addzx_st(RG, RM, DP)                                                \
    ADR REX(RXB(RG), RXB(RM)) EMITB(0x01)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sub
 * set-flags: undefined (xx), yes (zx) */

#define subxx_ri(RM, IM)                                                    \
        REX(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define subxx_mi(RM, DP, IM)                                                \
    ADR REX(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define subxx_rr(RG, RM)                                                    \
        REX(RXB(RG), RXB(RM)) EMITB(0x2B)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))

#define subxx_ld(RG, RM, DP)                                                \
    ADR REX(RXB(RG), RXB(RM)) EMITB(0x2B)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define subxx_st(RG, RM, DP)                                                \
    ADR REX(RXB(RG), RXB(RM)) EMITB(0x29)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define subxx_mr(RM, DP, RG)                                                \
        subxx_st(W(RG), W(RM), W(DP))

#define subzx_ri(RM, IM)                                                    \
        REX(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define subzx_mi(RM, DP, IM)                                                \
    ADR REX(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define subzx_rr(RG, RM)                                                    \
        REX(RXB(RG), RXB(RM)) EMITB(0x2B)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))

#define subzx_ld(RG, RM, DP)                                                \
    ADR REX(RXB(RG), RXB(RM)) EMITB(0x2B)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define subzx_st(RG, RM, DP)                                                \
    ADR REX(RXB(RG), RXB(RM)) EMITB(0x29)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define subzx_mr(RM, DP, RG)                                                \
        subzx_st(W(RG), W(RM), W(DP))

/* shl
 * set-flags: undefined */

#define shlxx_ri(RM, IM)                                                    \
        REX(0,       RXB(RM)) EMITB(0xC1)                                   \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shlxx_mi(RM, DP, IM)                                                \
    ADR REX(0,       RXB(RM)) EMITB(0xC1)                                   \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(VAL(IM) & 0x1F))

#define shlxx_rx(RM)                     /* reads Recx for shift value */   \
        REX(0,       RXB(RM)) EMITB(0xD3)                                   \
        MRM(0x04,    MOD(RM), REG(RM))                                      \

#define shlxx_mx(RM, DP)                 /* reads Recx for shift value */   \
    ADR REX(0,       RXB(RM)) EMITB(0xD3)                                   \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* shr
 * set-flags: undefined */

#define shrxx_ri(RM, IM)                                                    \
        REX(0,       RXB(RM)) EMITB(0xC1)                                   \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shrxx_mi(RM, DP, IM)                                                \
    ADR REX(0,       RXB(RM)) EMITB(0xC1)                                   \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(VAL(IM) & 0x1F))

#define shrxx_rx(RM)                     /* reads Recx for shift value */   \
        REX(0,       RXB(RM)) EMITB(0xD3)                                   \
        MRM(0x05,    MOD(RM), REG(RM))                                      \

#define shrxx_mx(RM, DP)                 /* reads Recx for shift value */   \
    ADR REX(0,       RXB(RM)) EMITB(0xD3)                                   \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define shrxn_ri(RM, IM)                                                    \
        REX(0,       RXB(RM)) EMITB(0xC1)                                   \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shrxn_mi(RM, DP, IM)                                                \
    ADR REX(0,       RXB(RM)) EMITB(0xC1)                                   \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(VAL(IM) & 0x1F))

#define shrxn_rx(RM)                     /* reads Recx for shift value */   \
        REX(0,       RXB(RM)) EMITB(0xD3)                                   \
        MRM(0x07,    MOD(RM), REG(RM))                                      \

#define shrxn_mx(RM, DP)                 /* reads Recx for shift value */   \
    ADR REX(0,       RXB(RM)) EMITB(0xD3)                                   \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* mul
 * set-flags: undefined */

#define mulxx_ri(RM, IM)                 /* part-range 32-bit multiply */   \
        REX(RXB(RM), RXB(RM)) EMITB(0x69 | TYP(IM))                         \
        MRM(REG(RM), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define mulxx_rr(RG, RM)                 /* part-range 32-bit multiply */   \
        REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0xAF)                       \
        MRM(REG(RG), MOD(RM), REG(RM))

#define mulxx_ld(RG, RM, DP)             /* part-range 32-bit multiply */   \
    ADR REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0xAF)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define mulxn_ri(RM, IM)                 /* part-range 32-bit multiply */   \
        mulxx_ri(W(RM), W(IM))

#define mulxn_rr(RG, RM)                 /* part-range 32-bit multiply */   \
        mulxx_rr(W(RG), W(RM))

#define mulxn_ld(RG, RM, DP)             /* part-range 32-bit multiply */   \
        mulxx_ld(W(RG), W(RM), W(DP))

#define mulxx_xr(RM)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
    ADR REX(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x04,    MOD(RM), REG(RM))                                      \

#define mulxx_xm(RM, DP) /* Reax is in/out, Redx is out(high)-zero-ext */   \
    ADR REX(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define mulxn_xr(RM)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
    ADR REX(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x05,    MOD(RM), REG(RM))                                      \

#define mulxn_xm(RM, DP) /* Reax is in/out, Redx is out(high)-sign-ext */   \
    ADR REX(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define mulxp_xr(RM)     /* Reax is in/out, prepares Redx for divxn/xp */   \
        mulxn_xr(W(RM))         /* (in ARM) part-range 32-bit multiply */

#define mulxp_xm(RM, DP) /* Reax is in/out, prepares Redx for divxn/xp */   \
        mulxn_xm(W(RM), W(DP))  /* (in ARM) part-range 32-bit multiply */

/* div
 * set-flags: undefined */

#define divxx_xr(RM)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
    ADR REX(0,       RXB(RM)) EMITB(0xF7)  /* destroys Redx (out:junk) */   \
        MRM(0x06,    MOD(RM), REG(RM))              /* Xmm0 (in ARMv7) */   \
        AUX(EMPTY,   EMPTY,   EMPTY) /* 32-bit int (fp64 div in ARMv7) */

#define divxx_xm(RM, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
    ADR REX(0,       RXB(RM)) EMITB(0xF7)  /* destroys Redx (out:junk) */   \
        MRM(0x06,    MOD(RM), REG(RM))              /* Xmm0 (in ARMv7) */   \
        AUX(SIB(RM), CMD(DP), EMPTY) /* 32-bit int (fp64 div in ARMv7) */

#define divxn_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
    ADR REX(0,       RXB(RM)) EMITB(0xF7)  /* destroys Redx (out:junk) */   \
        MRM(0x07,    MOD(RM), REG(RM))              /* Xmm0 (in ARMv7) */   \
        AUX(EMPTY,   EMPTY,   EMPTY) /* 32-bit int (fp64 div in ARMv7) */

#define divxn_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
    ADR REX(0,       RXB(RM)) EMITB(0xF7)  /* destroys Redx (out:junk) */   \
        MRM(0x07,    MOD(RM), REG(RM))              /* Xmm0 (in ARMv7) */   \
        AUX(SIB(RM), CMD(DP), EMPTY) /* 32-bit int (fp64 div in ARMv7) */

#define divxp_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divxn_xr(W(RM))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divxp_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divxn_xm(W(RM), W(DP))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem
 * set-flags: undefined */

#define remxx_xx()          /* to be placed immediately prior divxx_x* */   \
                            /* (in ARM) to prepare for rem calculation */

#define remxx_xr(RM)        /* to be placed immediately after divxx_xr */   \
                            /* (in ARM) to produce remainder Redx<-rem */

#define remxx_xm(RM, DP)    /* to be placed immediately after divxx_xm */   \
                            /* (in ARM) to produce remainder Redx<-rem */

#define remxn_xx()          /* to be placed immediately prior divxn_x* */   \
                            /* (in ARM) to prepare for rem calculation */

#define remxn_xr(RM)        /* to be placed immediately after divxn_xr */   \
                            /* (in ARM) to produce remainder Redx<-rem */

#define remxn_xm(RM, DP)    /* to be placed immediately after divxn_xm */   \
                            /* (in ARM) to produce remainder Redx<-rem */

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

#define CMJ(cc, lb) /* internal macro for combined-compare-jump (cmj) */    \
        cc(lb)

/* cmp
 * set-flags: yes */

#define cmpxx_ri(RM, IM)                                                    \
        REX(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define cmpxx_mi(RM, DP, IM)                                                \
    ADR REX(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define cmpxx_rr(RG, RM)                                                    \
        REX(RXB(RG), RXB(RM)) EMITB(0x3B)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))

#define cmpxx_rm(RG, RM, DP)                                                \
    ADR REX(RXB(RG), RXB(RM)) EMITB(0x3B)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define cmpxx_mr(RM, DP, RG)                                                \
    ADR REX(RXB(RG), RXB(RM)) EMITB(0x39)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* jmp
 * set-flags: no
 * maximum byte-address-range for un/conditional jumps is signed 18/16-bit
 * based on minimum natively-encoded offset across supported targets (u/c)
 * MIPS:18-bit, Power:26-bit, AArch32:26-bit, AArch64:28-bit, x86:32-bit /
 * MIPS:18-bit, Power:16-bit, AArch32:26-bit, AArch64:21-bit, x86:32-bit */

#if defined (RT_X32)

#define jmpxx_mm(RM, DP)         /* memory-targeted unconditional jump */   \
    ADR REX(1,       RXB(RM)) EMITB(0x8B)   /* <- load r15d from RM/DP */   \
        MRM(0x07,    MOD(RM), REG(RM))      /*    upper half is zeroed */   \
        AUX(SIB(RM), CMD(DP), EMPTY)        /*    as a part of 32-bit op */ \
        REX(0,             1) EMITB(0xFF)   /* <- jump to address in r15 */ \
        MRM(0x04,       0x03, 0x07)

#elif defined (RT_X64)

#define jmpxx_mm(RM, DP)         /* memory-targeted unconditional jump */   \
    ADR REX(0,       RXB(RM)) EMITB(0xFF)   /* <- jump to address in mem */ \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#endif /* defined (RT_X32, RT_X64) */

#define jmpxx_lb(lb)              /* label-targeted unconditional jump */   \
        ASM_BEG ASM_OP1(jmp, lb) ASM_END

#define jezxx_lb(lb)               /* setting-flags-arithmetic -> jump */   \
        ASM_BEG ASM_OP1(je,  lb) ASM_END

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

/* ver
 * set-flags: no */

#define cpuid_xx() /* destroys Reax, Recx, Rebx, Redx, reads Reax, Recx */  \
        EMITB(0x0F) EMITB(0xA2)     /* not portable, do not use outside */

#define verxx_xx() /* destroys Reax, Recx, Rebx, Redx, Resi, Redi */        \
        /* request cpuid:eax=1 */                                           \
        movxx_ri(Reax, IB(1))                                               \
        cpuid_xx()                                                          \
        shrxx_ri(Redx, IB(25))  /* <- SSE1, SSE2 to bit0, bit1 */           \
        andxx_ri(Redx, IB(0x03))                                            \
        movxx_rr(Resi, Redx)                                                \
        movxx_rr(Redx, Recx)                                                \
        shrxx_ri(Redx, IB(17))  /* <- SSE4 to bit2 */                       \
        andxx_ri(Redx, IB(0x04))                                            \
        shrxx_ri(Recx, IB(20))  /* <- AVX1 to bit8 */                       \
        andxx_ri(Recx, IH(0x0100))                                          \
        orrxx_rr(Resi, Redx)                                                \
        orrxx_rr(Resi, Recx)                                                \
        /* request cpuid:eax=0 to test input value eax=7 */                 \
        movxx_ri(Reax, IB(0))                                               \
        cpuid_xx()                                                          \
        subxx_ri(Reax, IB(7))                                               \
        shrxn_ri(Reax, IB(31))                                              \
        movxx_rr(Redi, Reax)                                                \
        notxx_rr(Redi)                                                      \
        /* request cpuid:eax=7:ecx=0 */                                     \
        movxx_ri(Reax, IB(7))                                               \
        movxx_ri(Recx, IB(0))                                               \
        cpuid_xx()                                                          \
        shlxx_ri(Rebx, IB(4))   /* <- AVX2 to bit9 */                       \
        andxx_ri(Rebx, IH(0x0200))                                          \
        andxx_rr(Rebx, Redi)                                                \
        orrxx_rr(Resi, Rebx)                                                \
        movxx_st(Resi, Mebp, inf_VER)

#endif /* RT_RTARCH_X32_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

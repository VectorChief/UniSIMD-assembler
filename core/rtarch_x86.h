/******************************************************************************/
/* Copyright (c) 2013-2015 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X86_H
#define RT_RTARCH_X86_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_x86.h: Implementation of x86 core instructions.
 *
 * This file is a part of the unified SIMD assembler framework (rtarch.h)
 * designed to be compatible with different processor architectures,
 * while maintaining strictly defined common API.
 *
 * Recommended naming scheme for instructions:
 *
 * cmdxx_ri - applies [cmd] to [r]egister from [i]mmediate
 * cmdxx_mi - applies [cmd] to [m]emory   from [i]mmediate
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
 * cmdx*_** - applies [cmd] to core register/memory/immediate args
 * cmd*x_** - applies [cmd] to unsigned integer args, [x] - default
 * cmd*n_** - applies [cmd] to   signed integer args, [n] - negatable
 *
 * Argument x-register is fixed by the implementation.
 * Some formal definitions are not given below to encourage
 * use of friendly aliases for better code readability.
 */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/* emitters */

#define EMPTY   ASM_BEG ASM_END

#define EMITW(w) /* little endian */                                        \
        EMITB((w) >> 0x00 & 0xFF)                                           \
        EMITB((w) >> 0x08 & 0xFF)                                           \
        EMITB((w) >> 0x10 & 0xFF)                                           \
        EMITB((w) >> 0x18 & 0xFF)

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

/* immediate    VAL,  TYP,  CMD */

#define IB(im)  (im), 0x02, EMITB((im) & 0x7F) /* drop sign-ext (zero in ARM) */
#define IM(im)  (im), 0x00, EMITW((im) & 0xFFF)
#define IH(im)  (im), 0x00, EMITW((im) & 0xFFFF)
#define IW(im)  (im), 0x00, EMITW((im) & 0xFFFFFFFF)

/* displacement VAL,  TYP,  CMD */

#define DP(im)  (im), 0x00, EMITW((im) & 0xFFC)
#define DH(im)  (im), 0x00, EMITW((im) & 0xFFF0)        /* SIMD-only (in ARM) */
#define PLAIN   0x00, 0x00, EMPTY

/* triplet pass-through wrapper */

#define W(p1, p2, p3)       p1,  p2,  p3

/******************************************************************************/
/**********************************   X86   ***********************************/
/******************************************************************************/

/* mov
 * set-flags: no */

#define movxx_ri(RM, IM)                                                    \
        EMITB(0xC7)                                                         \
        MRM(0x00,    MOD(RM), REG(RM)) /* truncate IB with TYP below */     \
        AUX(EMPTY,   EMPTY,   EMITW(VAL(IM) & ((TYP(IM) << 6) - 1)))

#define movxx_mi(RM, DP, IM)                                                \
        EMITB(0xC7)                                                         \
        MRM(0x00,    MOD(RM), REG(RM)) /* truncate IB with TYP below */     \
        AUX(SIB(RM), CMD(DP), EMITW(VAL(IM) & ((TYP(IM) << 6) - 1)))

#define movxx_rr(RG, RM)                                                    \
        EMITB(0x8B)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))

#define movxx_ld(RG, RM, DP)                                                \
        EMITB(0x8B)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define movxx_st(RG, RM, DP)                                                \
        EMITB(0x89)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define adrxx_ld(RG, RM, DP)                                                \
        EMITB(0x8D)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define adrxx_lb(lb) /* load label to Reax */                               \
        label_ld(lb)

#define stack_st(RM)                                                        \
        EMITB(0xFF)                                                         \
        MRM(0x06,    MOD(RM), REG(RM))

#define stack_ld(RM)                                                        \
        EMITB(0x8F)                                                         \
        MRM(0x00,    MOD(RM), REG(RM))

#define stack_sa() /* save all [eax - edi], 8 regs in total */              \
        EMITB(0x60)

#define stack_la() /* load all [edi - eax], 8 regs in total */              \
        EMITB(0x61)

/* and
 * set-flags: yes */

#define andxx_ri(RM, IM)                                                    \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define andxx_mi(RM, DP, IM)                                                \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define andxx_rr(RG, RM)                                                    \
        EMITB(0x23)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))

#define andxx_ld(RG, RM, DP)                                                \
        EMITB(0x23)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define andxx_st(RG, RM, DP)                                                \
        EMITB(0x21)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* orr
 * set-flags: no (in ARM) */

#define orrxx_ri(RM, IM)                                                    \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(0x01,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define orrxx_mi(RM, DP, IM)                                                \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(0x01,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define orrxx_rr(RG, RM)                                                    \
        EMITB(0x0B)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))

#define orrxx_ld(RG, RM, DP)                                                \
        EMITB(0x0B)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define orrxx_st(RG, RM, DP)                                                \
        EMITB(0x09)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* xor
 * set-flags: no (in ARM) */

#define xorxx_ri(RM, IM)                                                    \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(0x06,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define xorxx_mi(RM, DP, IM)                                                \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(0x06,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define xorxx_rr(RG, RM)                                                    \
        EMITB(0x33)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))

#define xorxx_ld(RG, RM, DP)                                                \
        EMITB(0x33)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define xorxx_st(RG, RM, DP)                                                \
        EMITB(0x31)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* not
 * set-flags: no */

#define notxx_rr(RM)                                                        \
        EMITB(0xF7)                                                         \
        MRM(0x02,    MOD(RM), REG(RM))

#define notxx_mm(RM, DP)                                                    \
        EMITB(0xF7)                                                         \
        MRM(0x02,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* neg
 * set-flags: yes */

#define negxx_rr(RM)                                                        \
        EMITB(0xF7)                                                         \
        MRM(0x03,    MOD(RM), REG(RM))

#define negxx_mm(RM, DP)                                                    \
        EMITB(0xF7)                                                         \
        MRM(0x03,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* add
 * set-flags: yes */

#define addxx_ri(RM, IM)                                                    \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(0x00,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define addxx_mi(RM, DP, IM)                                                \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(0x00,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define addxx_rr(RG, RM)                                                    \
        EMITB(0x03)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))

#define addxx_ld(RG, RM, DP)                                                \
        EMITB(0x03)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define addxx_st(RG, RM, DP)                                                \
        EMITB(0x01)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sub
 * set-flags: yes */

#define subxx_ri(RM, IM)                                                    \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define subxx_mi(RM, DP, IM)                                                \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define subxx_rr(RG, RM)                                                    \
        EMITB(0x2B)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))

#define subxx_ld(RG, RM, DP)                                                \
        EMITB(0x2B)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define subxx_st(RG, RM, DP)                                                \
        EMITB(0x29)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define subxx_mr(RM, DP, RG)                                                \
        subxx_st(W(RG), W(RM), W(DP))

/* shl
 * set-flags: no (in ARM) */

#define shlxx_ri(RM, IM)                                                    \
        EMITB(0xC1)                                                         \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shlxx_mi(RM, DP, IM)                                                \
        EMITB(0xC1)                                                         \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(VAL(IM) & 0x1F))

#define shlxx_rx(RM)     /* reads Recx for shift value */                   \
        EMITB(0xD3)                                                         \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMPTY)

#define shlxx_mx(RM, DP) /* reads Recx for shift value */                   \
        EMITB(0xD3)                                                         \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* shr
 * set-flags: no (in ARM) */

#define shrxx_ri(RM, IM)                                                    \
        EMITB(0xC1)                                                         \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shrxx_mi(RM, DP, IM)                                                \
        EMITB(0xC1)                                                         \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(VAL(IM) & 0x1F))

#define shrxx_rx(RM)     /* reads Recx for shift value */                   \
        EMITB(0xD3)                                                         \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMPTY)

#define shrxx_mx(RM, DP) /* reads Recx for shift value */                   \
        EMITB(0xD3)                                                         \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define shrxn_ri(RM, IM)                                                    \
        EMITB(0xC1)                                                         \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shrxn_mi(RM, DP, IM)                                                \
        EMITB(0xC1)                                                         \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(VAL(IM) & 0x1F))

#define shrxn_rx(RM)     /* reads Recx for shift value */                   \
        EMITB(0xD3)                                                         \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMPTY)

#define shrxn_mx(RM, DP) /* reads Recx for shift value */                   \
        EMITB(0xD3)                                                         \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* mul
 * set-flags: no (in ARM) */

#define mulxn_ri(RM, IM)                                                    \
        EMITB(0x69 | TYP(IM))                                               \
        MRM(REG(RM), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define mulxn_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xAF)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define mulxn_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0xAF)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define mulxn_xm(RM, DP) /* Reax is in/out, destroys Redx */                \
        EMITB(0xF7)                                                         \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* div
 * set-flags: no (in ARM) */

#define divxn_xm(RM, DP) /* Reax is in/out, Redx is Reax-sign-extended */   \
        EMITB(0xF7)                        /* destroys Xmm0 (in ARMv7) */   \
        MRM(0x07,    MOD(RM), REG(RM))     /* limited precision */          \
        AUX(SIB(RM), CMD(DP), EMPTY)       /* fp div (in ARMv7) */

/* cmp
 * set-flags: yes */

#define cmpxx_ri(RM, IM)                                                    \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define cmpxx_mi(RM, DP, IM)                                                \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define cmpxx_rr(RG, RM)                                                    \
        EMITB(0x3B)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))

#define cmpxx_rm(RG, RM, DP)                                                \
        EMITB(0x3B)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define cmpxx_mr(RM, DP, RG)                                                \
        EMITB(0x39)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* jmp
 * set-flags: no */

#define jmpxx_mm(RM, DP)                                                    \
        EMITB(0xFF)                                                         \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define jmpxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(jmp, lb) ASM_END

#define jeqxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(je,  lb) ASM_END

#define jezxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(je,  lb) ASM_END

#define jnexx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(jne, lb) ASM_END

#define jnzxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(jnz, lb) ASM_END

#define jltxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(jb,  lb) ASM_END

#define jlexx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(jbe, lb) ASM_END

#define jgtxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(ja,  lb) ASM_END

#define jgexx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(jae, lb) ASM_END

#define jltxn_lb(lb)                                                        \
        ASM_BEG ASM_OP1(jl,  lb) ASM_END

#define jlexn_lb(lb)                                                        \
        ASM_BEG ASM_OP1(jle, lb) ASM_END

#define jgtxn_lb(lb)                                                        \
        ASM_BEG ASM_OP1(jg,  lb) ASM_END

#define jgexn_lb(lb)                                                        \
        ASM_BEG ASM_OP1(jge, lb) ASM_END

#define LBL(lb)                                                             \
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
        shrxx_ri(Recx, IB(20))  /* <- AVX1 to bit8 */                       \
        andxx_ri(Recx, IH(0x0100))                                          \
        movxx_rr(Resi, Redx)                                                \
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

#endif /* RT_RTARCH_X86_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

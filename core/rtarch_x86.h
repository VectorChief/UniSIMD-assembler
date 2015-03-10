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
 * cmdx*_ri - applies [cmd] to [r]egister from [i]mmediate
 * cmdx*_mi - applies [cmd] to [m]emory   from [i]mmediate
 *
 * cmdx*_rm - applies [cmd] to [r]egister from [m]emory
 * cmdx*_ld - applies [cmd] as above
 * cmdx*_mr - applies [cmd] to [m]emory   from [r]egister
 * cmdx*_st - applies [cmd] as above (arg list as cmdx*_ld)
 *
 * cmdx*_rr - applies [cmd] to [r]egister from [r]egister
 * cmdx*_mm - applies [cmd] to [m]emory   from [m]emory
 * cmdx*_rr - applies [cmd] to [r]egister (one operand cmd)
 * cmdx*_mm - applies [cmd] to [m]emory   (one operand cmd)
 *
 * cmdx*_rx - applies [cmd] to [r]egister from x-register
 * cmdx*_mx - applies [cmd] to [m]emory   from x-register
 * cmdx*_xr - applies [cmd] to x-register from [r]egister
 * cmdx*_xm - applies [cmd] to x-register from [m]emory
 *
 * Default [m]emory args for core registers only accept DB, DP offsets,
 * use [w]ide-offset memory args below for DH, DW. There are no alignment
 * restrictions on any offset values in conjunction with core registers.
 * Argument x-register is fixed by the implementation.
 *
 * cmdx*_*w - applies [cmd] to [*] from [w]ide-offset memory
 * cmdx*_lw - applies [cmd] as above
 * cmdx*_w* - applies [cmd] to [w]ide-offset memory from [*]
 * cmdx*_sw - applies [cmd] as above (arg list as cmdx*_lw)
 *
 * cmdx*_rl - applies [cmd] to [r]egister from [l]abel
 * cmdx*_xl - applies [cmd] to x-register from [l]abel
 * cmdx*_lb - applies [cmd] as above
 * label_ld - applies [adr] as above
 *
 * stack_st - applies [mov] to stack from register (push)
 * stack_ld - applies [mov] to register from stack (pop)
 * stack_sa - applies [mov] to stack from all registers
 * stack_la - applies [mov] to all registers from stack
 *
 * cmdx*_** - applies [cmd] in  default mode (args size adjustable per target)
 * cmd*x_** - applies [cmd] to unsigned integer args, [x] - default
 * cmd*n_** - applies [cmd] to   signed integer args, [n] - negatable
 *
 * The cmdx*_** instructions together with SIMD cmdp*_** instructions
 * are intended for SPMD programming model and can be configured per target
 * to work with 32-bit/64-bit data-elements (integers/pointers, fp).
 * In this model data paths are fixed, core and SIMD data-elements are
 * interchangeable, code path divergence is handled via CHECK_MASK macro.
 *
 * The following size-explicit partial arithmetic/load/store instructions
 * allow to operate with sub-word register/memory arguments. The first [*]
 * controls the size [b - byte, h - half, w - word, f - full], the second [*]
 * controls the expansion [x - zero. n - sign]. In order to distinguish from
 * packed size-explicit instructions, [o] is used for core register arguments.
 *
 * cmd**_oi - applies [cmd] to c[o]re-reg from [i]mmediate
 * cmd**_mi - applies [cmd] to [m]emory   from [i]mmediate
 * cmd**_oo - applies [cmd] to c[o]re-reg from c[o]re-reg
 *
 * cmd**_om - applies [cmd] to c[o]re-reg from [m]emory
 * cmd**_lo - applies [cmd] as above
 * cmd**_mo - applies [cmd] to [m]emory from c[o]re-reg
 * cmd**_so - applies [cmd] to as above (arg list as cmd**_lo)
 *
 * cmd**_ow - applies [cmd] to c[o]re-reg from [w]ide-offset memory
 * cmd**_lw - applies [cmd] as above, [w] is only used with core regs
 * cmd**_wo - applies [cmd] to [w]ide-offset memory from c[o]re-reg
 * cmd**_sw - applies [cmd] as above (arg list as cmd**_lw)
 *
 * The sub-word arithmetic/load/store instructions are provided to complement
 * general purpose programming outside of more strict SPMD programming model
 * and might be generally limited in scope due to differences in architectures.
 *
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
#define Resp    0x04, 0x03, EMPTY
#define Rebp    0x05, 0x03, EMPTY
#define Resi    0x06, 0x03, EMPTY
#define Redi    0x07, 0x03, EMPTY

/* addressing   REG,  MOD,  SIB */

#define Oeax    0x00, 0x00, EMPTY       /* [eax] */

#define Mecx    0x01, 0x00, EMPTY       /* [ecx + DP] */
#define Medx    0x02, 0x00, EMPTY       /* [edx + DP] */
#define Mebx    0x03, 0x00, EMPTY       /* [ebx + DP] */
#define Mebp    0x05, 0x00, EMPTY       /* [ebp + DP] */
#define Mesi    0x06, 0x00, EMPTY       /* [esi + DP] */
#define Medi    0x07, 0x00, EMPTY       /* [edi + DP] */

#define Iecx    0x04, 0x00, EMITB(0x01) /* [ecx + eax*1 + DP] */
#define Iedx    0x04, 0x00, EMITB(0x02) /* [edx + eax*1 + DP] */
#define Iebx    0x04, 0x00, EMITB(0x03) /* [ebx + eax*1 + DP] */
#define Iebp    0x04, 0x00, EMITB(0x05) /* [ebp + eax*1 + DP] */
#define Iesi    0x04, 0x00, EMITB(0x06) /* [esi + eax*1 + DP] */
#define Iedi    0x04, 0x00, EMITB(0x07) /* [edi + eax*1 + DP] */

#define Decx    0x04, 0x00, EMITB(0x41) /* [ecx + eax*2 + DP] */
#define Dedx    0x04, 0x00, EMITB(0x42) /* [edx + eax*2 + DP] */
#define Debx    0x04, 0x00, EMITB(0x43) /* [ebx + eax*2 + DP] */
#define Debp    0x04, 0x00, EMITB(0x45) /* [ebp + eax*2 + DP] */
#define Desi    0x04, 0x00, EMITB(0x46) /* [esi + eax*2 + DP] */
#define Dedi    0x04, 0x00, EMITB(0x47) /* [edi + eax*2 + DP] */

#define Qecx    0x04, 0x00, EMITB(0x81) /* [ecx + eax*4 + DP] */
#define Qedx    0x04, 0x00, EMITB(0x82) /* [edx + eax*4 + DP] */
#define Qebx    0x04, 0x00, EMITB(0x83) /* [ebx + eax*4 + DP] */
#define Qebp    0x04, 0x00, EMITB(0x85) /* [ebp + eax*4 + DP] */
#define Qesi    0x04, 0x00, EMITB(0x86) /* [esi + eax*4 + DP] */
#define Qedi    0x04, 0x00, EMITB(0x87) /* [edi + eax*4 + DP] */

#define Secx    0x04, 0x00, EMITB(0xC1) /* [ecx + eax*8 + DP] */
#define Sedx    0x04, 0x00, EMITB(0xC2) /* [edx + eax*8 + DP] */
#define Sebx    0x04, 0x00, EMITB(0xC3) /* [ebx + eax*8 + DP] */
#define Sebp    0x04, 0x00, EMITB(0xC5) /* [ebp + eax*8 + DP] */
#define Sesi    0x04, 0x00, EMITB(0xC6) /* [esi + eax*8 + DP] */
#define Sedi    0x04, 0x00, EMITB(0xC7) /* [edi + eax*8 + DP] */

/* immediate    VAL,  TYP,  CMD */

#define IB(im)  ((im) & 0x7F),   0x02, EMITB((im) & 0x7F)  /* erase sign-bit */
#define IP(im)  ((im) & 0xFF),   0x00, EMITW((im) & 0xFF)  /* P: padded-byte */
#define IH(im)  ((im) & 0xFFFF), 0x00, EMITW((im) & 0xFFFF)
#define IW(im)  ((im)),          0x00, EMITW((im))

/* displacement VAL,  TYP,  CMD */

#define DB(im)  ((im) & 0x7F),   0x01, EMITB((im) & 0x7F)  /* erase sign-bit */
#define DP(im)  ((im) & 0xFFF),  0x02, EMITW((im) & 0xFFF) /* P: partial-half */
#define DH(im)  ((im) & 0xFFFF), 0x02, EMITW((im) & 0xFFFF)/* core/wide, SIMD */
#define DW(im)  ((im)),          0x02, EMITW((im))         /* core/wide, SIMD */

#define PLAIN   0x00,            0x00, EMPTY

/* triplet pass-through wrapper */

#define W(p1, p2, p3)       p1,  p2,  p3

/******************************************************************************/
/**********************************   X86   ***********************************/
/******************************************************************************/

/* mov */

#define movxx_ri(RM, IM)                                                    \
        EMITB(0xC7)                                                         \
            MRM(0x00,    MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   EMITW(VAL(IM)))

#define movxx_mi(RM, DP, IM)                                                \
        EMITB(0xC7)                                                         \
            MRM(0x00,    MOD(RM) | TYP(DP), REG(RM))                        \
            AUX(SIB(RM), CMD(DP), EMITW(VAL(IM)))

#define movxx_rr(RG, RM)                                                    \
        EMITB(0x8B)                                                         \
            MRM(REG(RG), MOD(RM), REG(RM))

#define movxx_ld(RG, RM, DP)                                                \
        EMITB(0x8B)                                                         \
            MRM(REG(RG), MOD(RM) | TYP(DP), REG(RM))                        \
            AUX(SIB(RM), CMD(DP), EMPTY)

#define movxx_st(RG, RM, DP)                                                \
        EMITB(0x89)                                                         \
            MRM(REG(RG), MOD(RM) | TYP(DP), REG(RM))                        \
            AUX(SIB(RM), CMD(DP), EMPTY)

#define adrxx_ld(RG, RM, DP) /* only 10-bit offsets and 4-byte alignment */ \
        EMITB(0x8D)                                                         \
            MRM(REG(RG), MOD(RM) | TYP(DP), REG(RM))                        \
            AUX(SIB(RM), EMITW(VAL(DP) & 0x3FC), EMPTY)

#define adrxx_lb(lb) /* load label to Reax */                               \
        label_ld(lb)

#define stack_st(RM)                                                        \
        EMITB(0xFF)                                                         \
            MRM(0x06,    MOD(RM), REG(RM))

#define stack_ld(RM)                                                        \
        EMITB(0x8F)                                                         \
            MRM(0x00,    MOD(RM), REG(RM))

#define stack_sa() /* save all [EAX - EDI], 8 regs in total */              \
        EMITB(0x60)

#define stack_la() /* load all [EAX - EDI], 8 regs in total */              \
        EMITB(0x61)

/* and */

#define andxx_ri(RM, IM)                                                    \
        EMITB(0x81 | TYP(IM))                                               \
            MRM(0x04,    MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   CMD(IM))

#define andxx_mi(RM, DP, IM)                                                \
        EMITB(0x81 | TYP(IM))                                               \
            MRM(0x04,    MOD(RM) | TYP(DP), REG(RM))                        \
            AUX(SIB(RM), CMD(DP), CMD(IM))

#define andxx_rr(RG, RM)                                                    \
        EMITB(0x23)                                                         \
            MRM(REG(RG), MOD(RM), REG(RM))

#define andxx_ld(RG, RM, DP)                                                \
        EMITB(0x23)                                                         \
            MRM(REG(RG), MOD(RM) | TYP(DP), REG(RM))                        \
            AUX(SIB(RM), CMD(DP), EMPTY)

#define andxx_st(RG, RM, DP)                                                \
        EMITB(0x21)                                                         \
            MRM(REG(RG), MOD(RM) | TYP(DP), REG(RM))                        \
            AUX(SIB(RM), CMD(DP), EMPTY)

/* orr */

#define orrxx_ri(RM, IM)                                                    \
        EMITB(0x81 | TYP(IM))                                               \
            MRM(0x01,    MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   CMD(IM))

#define orrxx_mi(RM, DP, IM)                                                \
        EMITB(0x81 | TYP(IM))                                               \
            MRM(0x01,    MOD(RM) | TYP(DP), REG(RM))                        \
            AUX(SIB(RM), CMD(DP), CMD(IM))

#define orrxx_rr(RG, RM)                                                    \
        EMITB(0x0B)                                                         \
            MRM(REG(RG), MOD(RM), REG(RM))

#define orrxx_ld(RG, RM, DP)                                                \
        EMITB(0x0B)                                                         \
            MRM(REG(RG), MOD(RM) | TYP(DP), REG(RM))                        \
            AUX(SIB(RM), CMD(DP), EMPTY)

#define orrxx_st(RG, RM, DP)                                                \
        EMITB(0x09)                                                         \
            MRM(REG(RG), MOD(RM) | TYP(DP), REG(RM))                        \
            AUX(SIB(RM), CMD(DP), EMPTY)

/* not */

#define notxx_rr(RM)                                                        \
        EMITB(0xF7)                                                         \
            MRM(0x02,    MOD(RM), REG(RM))

#define notxx_mm(RM, DP)                                                    \
        EMITB(0xF7)                                                         \
            MRM(0x02,    MOD(RM) | TYP(DP), REG(RM))                        \
            AUX(SIB(RM), CMD(DP), EMPTY)

/* add */

#define addxx_ri(RM, IM)                                                    \
        EMITB(0x81 | TYP(IM))                                               \
            MRM(0x00,    MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   CMD(IM))

#define addxx_mi(RM, DP, IM)                                                \
        EMITB(0x81 | TYP(IM))                                               \
            MRM(0x00,    MOD(RM) | TYP(DP), REG(RM))                        \
            AUX(SIB(RM), CMD(DP), CMD(IM))

#define addxx_rr(RG, RM)                                                    \
        EMITB(0x03)                                                         \
            MRM(REG(RG), MOD(RM), REG(RM))

#define addxx_ld(RG, RM, DP)                                                \
        EMITB(0x03)                                                         \
            MRM(REG(RG), MOD(RM) | TYP(DP), REG(RM))                        \
            AUX(SIB(RM), CMD(DP), EMPTY)

#define addxx_st(RG, RM, DP)                                                \
        EMITB(0x01)                                                         \
            MRM(REG(RG), MOD(RM) | TYP(DP), REG(RM))                        \
            AUX(SIB(RM), CMD(DP), EMPTY)

/* sub */

#define subxx_ri(RM, IM)                                                    \
        EMITB(0x81 | TYP(IM))                                               \
            MRM(0x05,    MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   CMD(IM))

#define subxx_mi(RM, DP, IM)                                                \
        EMITB(0x81 | TYP(IM))                                               \
            MRM(0x05,    MOD(RM) | TYP(DP), REG(RM))                        \
            AUX(SIB(RM), CMD(DP), CMD(IM))

#define subxx_rr(RG, RM)                                                    \
        EMITB(0x2B)                                                         \
            MRM(REG(RG), MOD(RM), REG(RM))

#define subxx_ld(RG, RM, DP)                                                \
        EMITB(0x2B)                                                         \
            MRM(REG(RG), MOD(RM) | TYP(DP), REG(RM))                        \
            AUX(SIB(RM), CMD(DP), EMPTY)

#define subxx_st(RG, RM, DP)                                                \
        EMITB(0x29)                                                         \
            MRM(REG(RG), MOD(RM) | TYP(DP), REG(RM))                        \
            AUX(SIB(RM), CMD(DP), EMPTY)

#define subxx_mr(RM, DP, RG)                                                \
        subxx_st(W(RG), W(RM), W(DP))

/* shl */

#define shlxx_ri(RM, IM)                                                    \
        EMITB(0xC1)                                                         \
            MRM(0x04,    MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shlxx_mi(RM, DP, IM)                                                \
        EMITB(0xC1)                                                         \
            MRM(0x04,    MOD(RM) | TYP(DP), REG(RM))                        \
            AUX(SIB(RM), CMD(DP), EMITB(VAL(IM) & 0x1F))

/* shr */

#define shrxx_ri(RM, IM)                                                    \
        EMITB(0xC1)                                                         \
            MRM(0x05,    MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shrxx_mi(RM, DP, IM)                                                \
        EMITB(0xC1)                                                         \
            MRM(0x05,    MOD(RM) | TYP(DP), REG(RM))                        \
            AUX(SIB(RM), CMD(DP), EMITB(VAL(IM) & 0x1F))

#define shrxn_ri(RM, IM)                                                    \
        EMITB(0xC1)                                                         \
            MRM(0x07,    MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shrxn_mi(RM, DP, IM)                                                \
        EMITB(0xC1)                                                         \
            MRM(0x07,    MOD(RM) | TYP(DP), REG(RM))                        \
            AUX(SIB(RM), CMD(DP), EMITB(VAL(IM) & 0x1F))

/* mul */

#define mulxn_ri(RM, IM)                                                    \
        EMITB(0x69 | TYP(IM))                                               \
            MRM(0x00,    MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   CMD(IM))

#define mulxn_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xAF)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define mulxn_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0xAF)                                             \
            MRM(REG(RG), MOD(RM) | TYP(DP), REG(RM))                        \
            AUX(SIB(RM), CMD(DP), EMPTY)

#define mulxn_xm(RM, DP) /* Reax is in/out, destroys Redx */                \
        EMITB(0xF7)                                                         \
            MRM(0x05,    MOD(RM) | TYP(DP), REG(RM))                        \
            AUX(SIB(RM), CMD(DP), EMPTY)

/* div */

#define divxn_xm(RM, DP) /* Reax is in/out, Redx is Reax-sign-extended */   \
        EMITB(0xF7)      /* destroys Xmm0 (in ARM) */                       \
            MRM(0x07,    MOD(RM) | TYP(DP), REG(RM))/* limited precision */ \
            AUX(SIB(RM), CMD(DP), EMPTY)            /* fp div (in ARM) */

/* cmp */

#define cmpxx_ri(RM, IM)                                                    \
        EMITB(0x81 | TYP(IM))                                               \
            MRM(0x07,    MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   CMD(IM))

#define cmpxx_mi(RM, DP, IM)                                                \
        EMITB(0x81 | TYP(IM))                                               \
            MRM(0x07,    MOD(RM) | TYP(DP), REG(RM))                        \
            AUX(SIB(RM), CMD(DP), CMD(IM))

#define cmpxx_rr(RG, RM)                                                    \
        EMITB(0x3B)                                                         \
            MRM(REG(RG), MOD(RM), REG(RM))

#define cmpxx_rm(RG, RM, DP)                                                \
        EMITB(0x3B)                                                         \
            MRM(REG(RG), MOD(RM) | TYP(DP), REG(RM))                        \
            AUX(SIB(RM), CMD(DP), EMPTY)

#define cmpxx_mr(RM, DP, RG)                                                \
        EMITB(0x39)                                                         \
            MRM(REG(RG), MOD(RM) | TYP(DP), REG(RM))                        \
            AUX(SIB(RM), CMD(DP), EMPTY)

/* jmp */

#define jmpxx_mm(RM, DP)                                                    \
        EMITB(0xFF)                                                         \
            MRM(0x04,    MOD(RM) | TYP(DP), REG(RM))                        \
            AUX(SIB(RM), CMD(DP), EMPTY)

#define jmpxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(jmp, lb) ASM_END

#define jeqxx_lb(lb)                                                        \
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

#endif /* RT_RTARCH_X86_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

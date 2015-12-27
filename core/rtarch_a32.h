/******************************************************************************/
/* Copyright (c) 2013-2015 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_A32_H
#define RT_RTARCH_A32_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_a32.h: Implementation of AArch64:ILP32 core instructions.
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
 * Argument x-register (implied) is fixed by the implementation.
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

#define MRM(reg, ren, rem)                                                  \
        ((rem) << 16 | (ren) << 5 | (reg))

#define AUX(sib, cdp, cim)  sib  cdp  cim

/* selectors  */

#define REG(reg, mod, sib)  reg
#define MOD(reg, mod, sib)  mod
#define SIB(reg, mod, sib)  sib

#define VAL(val, typ, cmd)  val
#define TYP(val, typ, cmd)  typ
#define CMD(val, typ, cmd)  cmd

/* registers    REG */

#define TMxx    0x1A                    /* w26 */
#define TIxx    0x1B                    /* w27 */
#define TPxx    0x1C                    /* w28 */

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/* registers    REG,  MOD,  SIB */

#define Reax    0x00, 0x00, EMPTY       /* w0 */
#define Recx    0x01, 0x00, EMPTY       /* w1 */
#define Redx    0x02, 0x00, EMPTY       /* w2 */
#define Rebx    0x03, 0x00, EMPTY       /* w3 */
#define Rebp    0x05, 0x00, EMPTY       /* w5 */
#define Resi    0x06, 0x00, EMPTY       /* w6 */
#define Redi    0x07, 0x00, EMPTY       /* w7 */
#define Reg8    0x08, 0x00, EMPTY       /* w8 */
#define Reg9    0x09, 0x00, EMPTY       /* w9 */
#define RegA    0x0A, 0x00, EMPTY       /* w10 */
#define RegB    0x0B, 0x00, EMPTY       /* w11 */
#define RegC    0x0C, 0x00, EMPTY       /* w12 */
#define RegD    0x0D, 0x00, EMPTY       /* w13 */
#define RegE    0x0E, 0x00, EMPTY       /* w14 */

/* addressing   REG,  MOD,  SIB */

#define Oeax    0x00, 0x00, EMPTY       /* [w0] */

#define Mecx    0x01, 0x01, EMPTY       /* [w1, DP] */
#define Medx    0x02, 0x02, EMPTY       /* [w2, DP] */
#define Mebx    0x03, 0x03, EMPTY       /* [w3, DP] */
#define Mebp    0x05, 0x05, EMPTY       /* [w5, DP] */
#define Mesi    0x06, 0x06, EMPTY       /* [w6, DP] */
#define Medi    0x07, 0x07, EMPTY       /* [w7, DP] */
#define Meg8    0x08, 0x08, EMPTY       /* [w8, DP] */
#define Meg9    0x09, 0x09, EMPTY       /* [w9, DP] */
#define MegA    0x0A, 0x0A, EMPTY       /* [w10, DP] */
#define MegB    0x0B, 0x0B, EMPTY       /* [w11, DP] */
#define MegC    0x0C, 0x0C, EMPTY       /* [w12, DP] */
#define MegD    0x0D, 0x0D, EMPTY       /* [w13, DP] */
#define MegE    0x0E, 0x0E, EMPTY       /* [w14, DP] */

#define Iecx    0x01, TPxx, EMITW(0x0B000000 | MRM(TPxx,    0x01,    0x00))
#define Iedx    0x02, TPxx, EMITW(0x0B000000 | MRM(TPxx,    0x02,    0x00))
#define Iebx    0x03, TPxx, EMITW(0x0B000000 | MRM(TPxx,    0x03,    0x00))
#define Iebp    0x05, TPxx, EMITW(0x0B000000 | MRM(TPxx,    0x05,    0x00))
#define Iesi    0x06, TPxx, EMITW(0x0B000000 | MRM(TPxx,    0x06,    0x00))
#define Iedi    0x07, TPxx, EMITW(0x0B000000 | MRM(TPxx,    0x07,    0x00))
#define Ieg8    0x08, TPxx, EMITW(0x0B000000 | MRM(TPxx,    0x08,    0x00))
#define Ieg9    0x09, TPxx, EMITW(0x0B000000 | MRM(TPxx,    0x09,    0x00))
#define IegA    0x0A, TPxx, EMITW(0x0B000000 | MRM(TPxx,    0x0A,    0x00))
#define IegB    0x0B, TPxx, EMITW(0x0B000000 | MRM(TPxx,    0x0B,    0x00))
#define IegC    0x0C, TPxx, EMITW(0x0B000000 | MRM(TPxx,    0x0C,    0x00))
#define IegD    0x0D, TPxx, EMITW(0x0B000000 | MRM(TPxx,    0x0D,    0x00))
#define IegE    0x0E, TPxx, EMITW(0x0B000000 | MRM(TPxx,    0x0E,    0x00))

/* immediate    VAL,  TYP,  CMD */

#define IB(im)  (im),                            /* drop sign-ext in x86 */ \
                EMPTY,         /* for compatibility with zero-ext in ARM */ \
                EMITW(0x52800000 | MRM(TIxx,    0x00,    0x00) |            \
                           ((im) & 0x007F) << 5)

#define IM(im)  (im),                                                       \
                EMPTY,                                                      \
                EMITW(0x52800000 | MRM(TIxx,    0x00,    0x00) |            \
                           ((im) & 0x0FFF) << 5)

#define IH(im)  (im),                                                       \
                EMITW(0x52800000 | MRM(TIxx,    0x00,    0x00) |            \
                           ((im) & 0xFFFF) << 5),                           \
                EMITW(0x52800000 | MRM(TIxx,    0x00,    0x00) |            \
                           ((im) & 0xFFFF) << 5)

#define IW(im)  (im),                                                       \
                EMITW(0x52800000 | MRM(TIxx,    0x00,    0x00) |            \
                           ((im) & 0xFFFF) << 5)                            \
                EMITW(0x72A00000 | MRM(TIxx,    0x00,    0x00) |            \
                           ((im) & 0xFFFF) << 5),                           \
                EMITW(0x52800000 | MRM(TIxx,    0x00,    0x00) |            \
                           ((im) & 0xFFFF) << 5)                            \
                EMITW(0x72A00000 | MRM(TIxx,    0x00,    0x00) |            \
                           ((im) & 0xFFFF) << 5)

/* displacement VAL,  TYP,  CMD */

#define DP(im)  (im),                                                       \
                (((im) & 0x0FF0) << 6),                                     \
                EMPTY

#define DH(im)  (im),                      /* only for SIMD instructions */ \
                (((im) & 0xFFF0) << 6),                                     \
                EMPTY

#define PLAIN   DP(0)

/* triplet pass-through wrapper */

#define W(p1, p2, p3)       p1,  p2,  p3

/******************************************************************************/
/**********************************   A32   ***********************************/
/******************************************************************************/

/* mov
 * set-flags: no */

#define movxx_ri(RM, IM)     /* one unnecessary op for IH, IW */            \
        AUX(EMPTY,   EMPTY,   TYP(IM))                                      \
        EMITW(0x02000000 | MRM(REG(RM), 0x00,    0x00) |                    \
        (+((VAL(IM) & 0xFFFFF000) == 0) & (0x50800000  | VAL(IM) << 5)) |   \
        (+((VAL(IM) & 0xFFFFF000) != 0) & (0x280003E0  | TIxx << 16)))
        /* equals to -1 (not 1) if ^ true */

#define movxx_mi(RM, DP, IM)                                                \
        AUX(SIB(RM), EMPTY,   CMD(IM))                                      \
        EMITW(0xB9000000 | MRM(TIxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8)

#define movxx_rr(RG, RM)                                                    \
        EMITW(0x2A000000 | MRM(REG(RG), 0x1F,    REG(RM)))

#define movxx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xB9400000 | MRM(REG(RG), MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8)

#define movxx_st(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xB9000000 | MRM(REG(RG), MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8)

#define adrxx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0x11000000 | MRM(REG(RG), MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<10)

#define adrxx_lb(lb) /* load label to Reax */                               \
        label_ld(lb)

#define stack_st(RM)                                                        \
        EMITW(0xF81F8C00 | MRM(REG(RM), 0x1F,    0x00))

#define stack_ld(RM)                                                        \
        EMITW(0xF8408400 | MRM(REG(RM), 0x1F,    0x00))

#define stack_sa() /* save all [x0 - x14] w/o x4, [x26 - x28], 17 regs */   \
        EMITW(0xA9B80000 | MRM(0x00,    0x1F,    0x00) | 0x01 << 10)        \
        EMITW(0xA9B80000 | MRM(0x02,    0x1F,    0x00) | 0x03 << 10)        \
        EMITW(0xA9B80000 | MRM(0x05,    0x1F,    0x00) | 0x06 << 10)        \
        EMITW(0xA9B80000 | MRM(0x07,    0x1F,    0x00) | 0x08 << 10)        \
        EMITW(0xA9B80000 | MRM(0x09,    0x1F,    0x00) | 0x0A << 10)        \
        EMITW(0xA9B80000 | MRM(0x0B,    0x1F,    0x00) | 0x0C << 10)        \
        EMITW(0xA9B80000 | MRM(0x0D,    0x1F,    0x00) | 0x0E << 10)        \
        EMITW(0xA9B80000 | MRM(0x1A,    0x1F,    0x00) | 0x1B << 10)        \
        EMITW(0xF81F8C00 | MRM(0x1C,    0x1F,    0x00))

#define stack_la() /* load all [x28 - x26], [x14 - x0] w/o x4, 17 regs */   \
        EMITW(0xF8408400 | MRM(0x1C,    0x1F,    0x00))                     \
        EMITW(0xA8C80000 | MRM(0x1A,    0x1F,    0x00) | 0x1B << 10)        \
        EMITW(0xA8C80000 | MRM(0x0D,    0x1F,    0x00) | 0x0E << 10)        \
        EMITW(0xA8C80000 | MRM(0x0B,    0x1F,    0x00) | 0x0C << 10)        \
        EMITW(0xA8C80000 | MRM(0x09,    0x1F,    0x00) | 0x0A << 10)        \
        EMITW(0xA8C80000 | MRM(0x07,    0x1F,    0x00) | 0x08 << 10)        \
        EMITW(0xA8C80000 | MRM(0x05,    0x1F,    0x00) | 0x06 << 10)        \
        EMITW(0xA8C80000 | MRM(0x02,    0x1F,    0x00) | 0x03 << 10)        \
        EMITW(0xA8C80000 | MRM(0x00,    0x1F,    0x00) | 0x01 << 10)

/* and
 * set-flags: yes */

#define andxx_ri(RM, IM)                                                    \
        AUX(EMPTY,   EMPTY,   CMD(IM))                                      \
        EMITW(0x6A000000 | MRM(REG(RM), REG(RM), TIxx))

#define andxx_mi(RM, DP, IM)                                                \
        AUX(SIB(RM), EMPTY,   CMD(IM))                                      \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x6A000000 | MRM(TMxx,    TMxx,    TIxx))                     \
        EMITW(0xB9000000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8)

#define andxx_rr(RG, RM)                                                    \
        EMITW(0x6A000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define andxx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x6A000000 | MRM(REG(RG), REG(RG), TMxx))

#define andxx_st(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x6A000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xB9000000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8)

/* orr
 * set-flags: no */

#define orrxx_ri(RM, IM)                                                    \
        AUX(EMPTY,   EMPTY,   CMD(IM))                                      \
        EMITW(0x2A000000 | MRM(REG(RM), REG(RM), TIxx))

#define orrxx_mi(RM, DP, IM)                                                \
        AUX(SIB(RM), EMPTY,   CMD(IM))                                      \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x2A000000 | MRM(TMxx,    TMxx,    TIxx))                     \
        EMITW(0xB9000000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8)

#define orrxx_rr(RG, RM)                                                    \
        EMITW(0x2A000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define orrxx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x2A000000 | MRM(REG(RG), REG(RG), TMxx))

#define orrxx_st(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x2A000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xB9000000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8)

/* xor
 * set-flags: no */

#define xorxx_ri(RM, IM)                                                    \
        AUX(EMPTY,   EMPTY,   CMD(IM))                                      \
        EMITW(0x4A000000 | MRM(REG(RM), REG(RM), TIxx))

#define xorxx_mi(RM, DP, IM)                                                \
        AUX(SIB(RM), EMPTY,   CMD(IM))                                      \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x4A000000 | MRM(TMxx,    TMxx,    TIxx))                     \
        EMITW(0xB9000000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8)

#define xorxx_rr(RG, RM)                                                    \
        EMITW(0x4A000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define xorxx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x4A000000 | MRM(REG(RG), REG(RG), TMxx))

#define xorxx_st(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x4A000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xB9000000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8)

/* not
 * set-flags: no */

#define notxx_rr(RM)                                                        \
        EMITW(0x2A200000 | MRM(REG(RM), 0x1F,    REG(RM)))

#define notxx_mm(RM, DP)                                                    \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x2A200000 | MRM(TMxx,    0x1F,    TMxx))                     \
        EMITW(0xB9000000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8)

/* neg
 * set-flags: yes */

#define negxx_rr(RM)                                                        \
        EMITW(0x6B000000 | MRM(REG(RM), 0x1F,    REG(RM)))

#define negxx_mm(RM, DP)                                                    \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x6B000000 | MRM(TMxx,    0x1F,    TMxx))                     \
        EMITW(0xB9000000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8)

/* add
 * set-flags: yes */

#define addxx_ri(RM, IM)                                                    \
        AUX(EMPTY,   EMPTY,   TYP(IM))                                      \
        EMITW(0x21000000 | MRM(REG(RM), REG(RM), 0x00) |                    \
        (+((VAL(IM) & 0xFFFFF000) == 0) & (0x10000000  | VAL(IM) << 10)) |  \
        (+((VAL(IM) & 0xFFFFF000) != 0) & (0x0A000000  | TIxx << 16)))
        /* equals to -1 (not 1) if ^ true */

#define addxx_mi(RM, DP, IM)                                                \
        AUX(SIB(RM), EMPTY,   TYP(IM))                                      \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x21000000 | MRM(TMxx,    TMxx,    0x00) |                    \
        (+((VAL(IM) & 0xFFFFF000) == 0) & (0x10000000  | VAL(IM) << 10)) |  \
        (+((VAL(IM) & 0xFFFFF000) != 0) & (0x0A000000  | TIxx << 16)))      \
        /* equals to -1 (not 1) if ^ true */                                \
        EMITW(0xB9000000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8)

#define addxx_rr(RG, RM)                                                    \
        EMITW(0x2B000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define addxx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x2B000000 | MRM(REG(RG), REG(RG), TMxx))

#define addxx_st(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x2B000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xB9000000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8)

/* sub
 * set-flags: yes */

#define subxx_ri(RM, IM)                                                    \
        AUX(EMPTY,   EMPTY,   TYP(IM))                                      \
        EMITW(0x61000000 | MRM(REG(RM), REG(RM), 0x00) |                    \
        (+((VAL(IM) & 0xFFFFF000) == 0) & (0x10000000  | VAL(IM) << 10)) |  \
        (+((VAL(IM) & 0xFFFFF000) != 0) & (0x0A000000  | TIxx << 16)))
        /* equals to -1 (not 1) if ^ true */

#define subxx_mi(RM, DP, IM)                                                \
        AUX(SIB(RM), EMPTY,   TYP(IM))                                      \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x61000000 | MRM(TMxx,    TMxx,    0x00) |                    \
        (+((VAL(IM) & 0xFFFFF000) == 0) & (0x10000000  | VAL(IM) << 10)) |  \
        (+((VAL(IM) & 0xFFFFF000) != 0) & (0x0A000000  | TIxx << 16)))      \
        /* equals to -1 (not 1) if ^ true */                                \
        EMITW(0xB9000000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8)

#define subxx_rr(RG, RM)                                                    \
        EMITW(0x6B000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define subxx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x6B000000 | MRM(REG(RG), REG(RG), TMxx))

#define subxx_st(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x6B000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xB9000000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8)

#define subxx_mr(RM, DP, RG)                                                \
        subxx_st(W(RG), W(RM), W(DP))

/* shl
 * set-flags: no */

#define shlxx_ri(RM, IM)                                                    \
        EMITW(0x53000000 | MRM(REG(RM), REG(RM), 0x00) |                    \
        (-VAL(IM)&0x1F)<<16 | (31-(VAL(IM)&0x1F))<<10)

#define shlxx_mi(RM, DP, IM)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x53000000 | MRM(TMxx,    TMxx,    0x00) |                    \
        (-VAL(IM)&0x1F)<<16 | (31-(VAL(IM)&0x1F))<<10)                      \
        EMITW(0xB9000000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8)

#define shlxx_rx(RM)     /* reads Recx for shift value */                   \
        EMITW(0x1AC02000 | MRM(REG(RM), REG(RM), 0x01))

#define shlxx_mx(RM, DP) /* reads Recx for shift value */                   \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x1AC02000 | MRM(TMxx,    TMxx,    0x01))                     \
        EMITW(0xB9000000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8)

/* shr
 * set-flags: no */

#define shrxx_ri(RM, IM)                                                    \
        EMITW(0x53007C00 | MRM(REG(RM), REG(RM), 0x00) |(VAL(IM)&0x1F)<<16)

#define shrxx_mi(RM, DP, IM)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x53007C00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IM)&0x1F)<<16) \
        EMITW(0xB9000000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8)

#define shrxx_rx(RM)     /* reads Recx for shift value */                   \
        EMITW(0x1AC02400 | MRM(REG(RM), REG(RM), 0x01))

#define shrxx_mx(RM, DP) /* reads Recx for shift value */                   \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x1AC02400 | MRM(TMxx,    TMxx,    0x01))                     \
        EMITW(0xB9000000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8)

#define shrxn_ri(RM, IM)                                                    \
        EMITW(0x13007C00 | MRM(REG(RM), REG(RM), 0x00) |(VAL(IM)&0x1F)<<16)

#define shrxn_mi(RM, DP, IM)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x13007C00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IM)&0x1F)<<16) \
        EMITW(0xB9000000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8)

#define shrxn_rx(RM)     /* reads Recx for shift value */                   \
        EMITW(0x1AC02800 | MRM(REG(RM), REG(RM), 0x01))

#define shrxn_mx(RM, DP) /* reads Recx for shift value */                   \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x1AC02800 | MRM(TMxx,    TMxx,    0x01))                     \
        EMITW(0xB9000000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8)

/* mul
 * set-flags: no */

#define mulxx_ri(RM, IM)                                                    \
        AUX(EMPTY,   EMPTY,   CMD(IM))                                      \
        EMITW(0x1B007C00 | MRM(REG(RM), REG(RM), TIxx))

#define mulxx_rr(RG, RM)                                                    \
        EMITW(0x1B007C00 | MRM(REG(RG), REG(RG), REG(RM)))

#define mulxx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x1B007C00 | MRM(REG(RG), REG(RG), TMxx))

#define mulxn_ri(RM, IM)                                                    \
        mulxx_ri(W(RM), W(IM))

#define mulxn_rr(RG, RM)                                                    \
        mulxx_rr(W(RG), W(RM))

#define mulxn_ld(RG, RM, DP)                                                \
        mulxx_ld(W(RG), W(RM), W(DP))

#define mulxx_xr(RM)     /* Reax is in/out, Redx is zero-ext-out(high) */   \
        EMITW(0x9BA07C00 | MRM(0x00,    0x00,    REG(RM)))                  \
        EMITW(0xD360FC00 | MRM(0x02,    0x00,    0x00))

#define mulxx_xm(RM, DP) /* Reax is in/out, Redx is zero-ext-out(high) */   \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x9BA07C00 | MRM(0x00,    0x00,    TMxx))                     \
        EMITW(0xD360FC00 | MRM(0x02,    0x00,    0x00))

#define mulxn_xr(RM)     /* Reax is in/out, Redx is sign-ext-out(high) */   \
        EMITW(0x9B207C00 | MRM(0x00,    0x00,    REG(RM)))                  \
        EMITW(0xD360FC00 | MRM(0x02,    0x00,    0x00))

#define mulxn_xm(RM, DP) /* Reax is in/out, Redx is sign-ext-out(high) */   \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x9B207C00 | MRM(0x00,    0x00,    TMxx))                     \
        EMITW(0xD360FC00 | MRM(0x02,    0x00,    0x00))

/* div
 * set-flags: no */

#define divxx_xr(RM)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(0x1AC00800 | MRM(0x00,    0x00,    REG(RM)))

#define divxx_xm(RM, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUX(SIB(RM), EMPTY,   EMPTY) /* destroys Redx, Xmm0 (in ARMv7) */   \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x1AC00800 | MRM(0x00,    0x00,    TMxx))

#define divxn_xr(RM)     /* Reax is in/out, Redx is sign-ext-(Reax)-in */   \
        EMITW(0x1AC00C00 | MRM(0x00,    0x00,    REG(RM)))

#define divxn_xm(RM, DP) /* Reax is in/out, Redx is sign-ext-(Reax)-in */   \
        AUX(SIB(RM), EMPTY,   EMPTY) /* destroys Redx, Xmm0 (in ARMv7) */   \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x1AC00C00 | MRM(0x00,    0x00,    TMxx))

#define divxp_xr(RM)     /* Reax is in/out, Redx is sign-ext-(Reax)-in */   \
        divxn_xr(W(RM))              /* part-range fp32 div (in ARMv7) */

#define divxp_xm(RM, DP) /* Reax is in/out, Redx is sign-ext-(Reax)-in */   \
        divxn_xm(W(RM), W(DP))       /* part-range fp32 div (in ARMv7) */

/* cmp
 * set-flags: yes */

#define cmpxx_ri(RM, IM)                                                    \
        AUX(EMPTY,   EMPTY,   TYP(IM))                                      \
        EMITW(0x61000000 | MRM(0x1F,    REG(RM), 0x00) |                    \
        (+((VAL(IM) & 0xFFFFF000) == 0) & (0x10000000  | VAL(IM) << 10)) |  \
        (+((VAL(IM) & 0xFFFFF000) != 0) & (0x0A000000  | TIxx << 16)))
        /* equals to -1 (not 1) if ^ true */

#define cmpxx_mi(RM, DP, IM)                                                \
        AUX(SIB(RM), EMPTY,   TYP(IM))                                      \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x61000000 | MRM(0x1F,    TMxx,    0x00) |                    \
        (+((VAL(IM) & 0xFFFFF000) == 0) & (0x10000000  | VAL(IM) << 10)) |  \
        (+((VAL(IM) & 0xFFFFF000) != 0) & (0x0A000000  | TIxx << 16)))
        /* equals to -1 (not 1) if ^ true */

#define cmpxx_rr(RG, RM)                                                    \
        EMITW(0x6B000000 | MRM(0x1F,    REG(RG), REG(RM)))

#define cmpxx_rm(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x6B000000 | MRM(0x1F,    REG(RG), TMxx))

#define cmpxx_mr(RM, DP, RG)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0x6B000000 | MRM(0x1F,    TMxx,    REG(RG)))

/* jmp
 * set-flags: no */

#define jmpxx_mm(RM, DP)                                                    \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xB9400000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP)&0xFFC)<<8) \
        EMITW(0xD61F0000 | MRM(0x00,    TMxx,    0x00))

#define jmpxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(b,    lb) ASM_END

#define jeqxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(b.eq, lb) ASM_END

#define jezxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(b.eq, lb) ASM_END

#define jnexx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(b.ne, lb) ASM_END

#define jnzxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(b.ne, lb) ASM_END

#define jltxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(b.lo, lb) ASM_END

#define jlexx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(b.ls, lb) ASM_END

#define jgtxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(b.hi, lb) ASM_END

#define jgexx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(b.hs, lb) ASM_END

#define jltxn_lb(lb)                                                        \
        ASM_BEG ASM_OP1(b.lt, lb) ASM_END

#define jlexn_lb(lb)                                                        \
        ASM_BEG ASM_OP1(b.le, lb) ASM_END

#define jgtxn_lb(lb)                                                        \
        ASM_BEG ASM_OP1(b.gt, lb) ASM_END

#define jgexn_lb(lb)                                                        \
        ASM_BEG ASM_OP1(b.ge, lb) ASM_END

#define LBL(lb)                                                             \
        ASM_BEG ASM_OP0(lb:) ASM_END

/* ver
 * set-flags: no */

#define verxx_xx() /* destroys Reax, Recx, Rebx, Redx, Resi, Redi (in x86)*/\
        movxx_mi(Mebp, inf_VER, IB(1)) /* <- NEON to bit0, without checks */

#endif /* RT_RTARCH_A32_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

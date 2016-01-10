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

#define MZM(reg, brm, vdp, bxx, pxx)                                        \
        (pxx(vdp) | bxx(brm) << 5 | (reg))

#define MIM(reg, ren, vim, mxx)                                             \
        (mxx(vim) |    (ren) << 5 | (reg))

#define AUZ(sib, vim, reg, brm, vdp, cdp, cim)                              \
            sib  cdp(brm, vdp)  cim(reg, vim)

#define EMPTY2(em1, em2)

/* selectors  */

#define REG(reg, mod, sib)  reg
#define MOD(reg, mod, sib)  mod
#define SIB(reg, mod, sib)  sib

#define VAL(val, typ, cmd)  val

#define  M1(val, tp1, tp2)  M1##tp1
#define  G1(val, tp1, tp2)  G1##tp1
#define  M2(val, tp1, tp2)  M2##tp2
#define  G2(val, tp1, tp2)  G2##tp2
#define  G3(val, tp1, tp2)  G3##tp2 /* <- "G3##tp2" not a bug */

#define  B1(val, tp1, tp2)  B1##tp1
#define  P1(val, tp1, tp2)  P1##tp1
#define  C1(val, tp1, tp2)  C1##tp1
#define  C3(val, tp1, tp2)  C3##tp2 /* <- "C3##tp2" not a bug */

/* registers    REG */

#define TMxx    0x1A                    /* w26 */
#define TIxx    0x1B                    /* w27, not used together with TDxx */
#define TDxx    0x1B                    /* w27, not used together with TIxx */
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

/* immediate    VAL,  TP1,  TP2 */

#define IC(im)  ((im) & 0x7F),       0, 1      /* drop sign-ext (in x86) */
#define IB(im)  ((im) & 0xFF),       0, 1        /* 32-bit word (in x86) */
#define IM(im)  ((im) & 0xFFF),      0, 1  /* native AArch64 add/sub/cmp */
#define IG(im)  ((im) & 0x7FFF),     1, 1  /* native on MIPS add/sub/cmp */
#define IH(im)  ((im) & 0xFFFF),     1, 1  /* second native on ARMs/MIPS */
#define IV(im)  ((im) & 0x7FFFFFFF), 2, 2        /* native x64 long mode */
#define IW(im)  ((im) & 0xFFFFFFFF), 2, 2        /* extra load op on x64 */

/* displacement VAL,  TP1,  TP2 */

#define DP(dp)  ((dp) & 0x7FC),      0, 0      /* native MIPS SIMD ld/st */
#define DM(dp)  ((dp) & 0xFFC),      0, 0          /* native on all ARMs */
#define DF(dp)  ((dp) & 0x3FFC),     0, 0   /* native AArch64 core ld/st */
#define DG(dp)  ((dp) & 0x7FFC),     1, 0      /* native MIPS core ld/st */
#define DH(dp)  ((dp) & 0xFFFC),     1, 0   /* second native on all ARMs */
#define DV(dp)  ((dp) & 0x7FFFFFFC), 2, 2        /* native x64 long mode */
#define PLAIN   DP(0)           /* special type for Oeax addressing mode */

/* immediate encoding add/sub/cmp(TP1), and/orr/xor(TP2), mov/mul(TP3) */

#define M10(im) (0x10000000 | (im) << 10)
#define G10(rg, im) EMPTY

#define M11(im) (0x0A000000 | TIxx << 16)
#define G11(rg, im) G31(rg, im)
#define M21(im) (0x0A000000 | TIxx << 16)
#define G21(rg, im) G31(rg, im)
#define G31(rg, im) EMITW(0x52800000 | MRM((rg),    0x00,    0x00) |        \
                               ((im) & 0xFFFF) << 5)

#define M12(im) (0x0A000000 | TIxx << 16)
#define G12(rg, im) G32(rg, im)
#define M22(im) (0x0A000000 | TIxx << 16)
#define G22(rg, im) G32(rg, im)
#define G32(rg, im) EMITW(0x52800000 | MRM((rg),    0x00,    0x00) |        \
                               ((im) & 0xFFFF) << 5)                        \
                    EMITW(0x72A00000 | MRM((rg),    0x00,    0x00) |        \
                               ((im) & 0xFFFF) << 5)

/* displacement encoding core(TP1), adr(TP3) */

#define B10(br) (br)
#define P10(dp) (0x00000000 | (dp) << 8)
#define C10(br, dp) EMPTY
#define C30(br, dp) EMITW(0x52800000 | MRM(TDxx,    0x00,    0x00) |        \
                               ((dp) & 0xFFFC) << 5)

#define B11(br) TPxx
#define P11(dp) (0x00000000)
#define C11(br, dp) C30(br, dp)                                             \
                    EMITW(0x0B000000 | MRM(TPxx,    (br),    TDxx))

#define B12(br) TPxx
#define P12(dp) (0x00000000)
#define C12(br, dp) C32(br, dp)                                             \
                    EMITW(0x0B000000 | MRM(TPxx,    (br),    TDxx))
#define C32(br, dp) EMITW(0x52800000 | MRM(TDxx,    0x00,    0x00) |        \
                               ((dp) & 0xFFFC) << 5)                        \
                    EMITW(0x72A00000 | MRM(TDxx,    0x00,    0x00) |        \
                               ((dp) & 0x7FFF) << 5)

/* triplet pass-through wrapper */

#define W(p1, p2, p3)       p1,  p2,  p3

/******************************************************************************/
/**********************************   A32   ***********************************/
/******************************************************************************/

/* mov
 * set-flags: no */

#define movxx_ri(RM, IM)                                                    \
        AUZ(EMPTY,    VAL(IM), REG(RM), EMPTY,   EMPTY,   EMPTY2, G3(IM))

#define movxx_mi(RM, DP, IM)                                                \
        AUZ(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G3(IM))   \
        EMITW(0xB9000000 | MZM(TIxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define movxx_rr(RG, RM)                                                    \
        EMITW(0x2A000000 | MRM(REG(RG), 0x1F,    REG(RM)))

#define movxx_ld(RG, RM, DP)                                                \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MZM(REG(RG), MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define movxx_st(RG, RM, DP)                                                \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9000000 | MZM(REG(RG), MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define adrxx_ld(RG, RM, DP)                                                \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C3(DP), EMPTY2)   \
        EMITW(0x0B000000 | MRM(REG(RG), MOD(RM), TDxx))

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
        AUZ(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x6A000000 | MIM(REG(RM), REG(RM), VAL(IM), M2(IM)))

#define andxx_mi(RM, DP, IM)                                                \
        AUZ(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x6A000000 | MIM(TMxx,    TMxx,    VAL(IM), M2(IM)))          \
        EMITW(0xB9000000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define andxx_rr(RG, RM)                                                    \
        EMITW(0x6A000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define andxx_ld(RG, RM, DP)                                                \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x6A000000 | MRM(REG(RG), REG(RG), TMxx))

#define andxx_st(RG, RM, DP)                                                \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x6A000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xB9000000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* orr
 * set-flags: no */

#define orrxx_ri(RM, IM)                                                    \
        AUZ(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x2A000000 | MIM(REG(RM), REG(RM), VAL(IM), M2(IM)))

#define orrxx_mi(RM, DP, IM)                                                \
        AUZ(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x2A000000 | MIM(TMxx,    TMxx,    VAL(IM), M2(IM)))          \
        EMITW(0xB9000000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define orrxx_rr(RG, RM)                                                    \
        EMITW(0x2A000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define orrxx_ld(RG, RM, DP)                                                \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x2A000000 | MRM(REG(RG), REG(RG), TMxx))

#define orrxx_st(RG, RM, DP)                                                \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x2A000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xB9000000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* xor
 * set-flags: no */

#define xorxx_ri(RM, IM)                                                    \
        AUZ(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x4A000000 | MIM(REG(RM), REG(RM), VAL(IM), M2(IM)))

#define xorxx_mi(RM, DP, IM)                                                \
        AUZ(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x4A000000 | MIM(TMxx,    TMxx,    VAL(IM), M2(IM)))          \
        EMITW(0xB9000000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define xorxx_rr(RG, RM)                                                    \
        EMITW(0x4A000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define xorxx_ld(RG, RM, DP)                                                \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x4A000000 | MRM(REG(RG), REG(RG), TMxx))

#define xorxx_st(RG, RM, DP)                                                \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x4A000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xB9000000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* not
 * set-flags: no */

#define notxx_rr(RM)                                                        \
        EMITW(0x2A200000 | MRM(REG(RM), 0x1F,    REG(RM)))

#define notxx_mm(RM, DP)                                                    \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x2A200000 | MRM(TMxx,    0x1F,    TMxx))                     \
        EMITW(0xB9000000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* neg
 * set-flags: yes */

#define negxx_rr(RM)                                                        \
        EMITW(0x6B000000 | MRM(REG(RM), 0x1F,    REG(RM)))

#define negxx_mm(RM, DP)                                                    \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x6B000000 | MRM(TMxx,    0x1F,    TMxx))                     \
        EMITW(0xB9000000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* add
 * set-flags: yes */

#define addxx_ri(RM, IM)                                                    \
        AUZ(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x21000000 | MIM(REG(RM), REG(RM), VAL(IM), M1(IM)))

#define addxx_mi(RM, DP, IM)                                                \
        AUZ(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x21000000 | MIM(TMxx,    TMxx,    VAL(IM), M1(IM)))          \
        EMITW(0xB9000000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define addxx_rr(RG, RM)                                                    \
        EMITW(0x2B000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define addxx_ld(RG, RM, DP)                                                \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x2B000000 | MRM(REG(RG), REG(RG), TMxx))

#define addxx_st(RG, RM, DP)                                                \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x2B000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xB9000000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* sub
 * set-flags: yes */

#define subxx_ri(RM, IM)                                                    \
        AUZ(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x61000000 | MIM(REG(RM), REG(RM), VAL(IM), M1(IM)))

#define subxx_mi(RM, DP, IM)                                                \
        AUZ(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x61000000 | MIM(TMxx,    TMxx,    VAL(IM), M1(IM)))          \
        EMITW(0xB9000000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define subxx_rr(RG, RM)                                                    \
        EMITW(0x6B000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define subxx_ld(RG, RM, DP)                                                \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x6B000000 | MRM(REG(RG), REG(RG), TMxx))

#define subxx_st(RG, RM, DP)                                                \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x6B000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xB9000000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define subxx_mr(RM, DP, RG)                                                \
        subxx_st(W(RG), W(RM), W(DP))

/* shl
 * set-flags: no */

#define shlxx_ri(RM, IM)                                                    \
        EMITW(0x53000000 | MRM(REG(RM), REG(RM), 0x00) |                    \
        (-VAL(IM)&0x1F)<<16 | (31-(VAL(IM)&0x1F))<<10)

#define shlxx_mi(RM, DP, IM)                                                \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x53000000 | MRM(TMxx,    TMxx,    0x00) |                    \
        (-VAL(IM)&0x1F)<<16 | (31-(VAL(IM)&0x1F))<<10)                      \
        EMITW(0xB9000000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shlxx_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0x1AC02000 | MRM(REG(RM), REG(RM), 0x01))

#define shlxx_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x1AC02000 | MRM(TMxx,    TMxx,    0x01))                     \
        EMITW(0xB9000000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* shr
 * set-flags: no */

#define shrxx_ri(RM, IM)                                                    \
        EMITW(0x53007C00 | MRM(REG(RM), REG(RM), 0x00) |(VAL(IM)&0x1F)<<16)

#define shrxx_mi(RM, DP, IM)                                                \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x53007C00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IM)&0x1F)<<16) \
        EMITW(0xB9000000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrxx_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0x1AC02400 | MRM(REG(RM), REG(RM), 0x01))

#define shrxx_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x1AC02400 | MRM(TMxx,    TMxx,    0x01))                     \
        EMITW(0xB9000000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrxn_ri(RM, IM)                                                    \
        EMITW(0x13007C00 | MRM(REG(RM), REG(RM), 0x00) |(VAL(IM)&0x1F)<<16)

#define shrxn_mi(RM, DP, IM)                                                \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x13007C00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IM)&0x1F)<<16) \
        EMITW(0xB9000000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrxn_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0x1AC02800 | MRM(REG(RM), REG(RM), 0x01))

#define shrxn_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x1AC02800 | MRM(TMxx,    TMxx,    0x01))                     \
        EMITW(0xB9000000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* mul
 * set-flags: no */

#define mulxx_ri(RM, IM)                 /* part-range 32-bit multiply */   \
        AUZ(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x1B007C00 | MRM(REG(RM), REG(RM), TIxx))

#define mulxx_rr(RG, RM)                 /* part-range 32-bit multiply */   \
        EMITW(0x1B007C00 | MRM(REG(RG), REG(RG), REG(RM)))

#define mulxx_ld(RG, RM, DP)             /* part-range 32-bit multiply */   \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x1B007C00 | MRM(REG(RG), REG(RG), TMxx))

#define mulxn_ri(RM, IM)                 /* part-range 32-bit multiply */   \
        mulxx_ri(W(RM), W(IM))

#define mulxn_rr(RG, RM)                 /* part-range 32-bit multiply */   \
        mulxx_rr(W(RG), W(RM))

#define mulxn_ld(RG, RM, DP)             /* part-range 32-bit multiply */   \
        mulxx_ld(W(RG), W(RM), W(DP))

#define mulxx_xr(RM)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0x9BA07C00 | MRM(0x00,    0x00,    REG(RM)))                  \
        EMITW(0xD360FC00 | MRM(0x02,    0x00,    0x00))

#define mulxx_xm(RM, DP) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9BA07C00 | MRM(0x00,    0x00,    TMxx))                     \
        EMITW(0xD360FC00 | MRM(0x02,    0x00,    0x00))

#define mulxn_xr(RM)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0x9B207C00 | MRM(0x00,    0x00,    REG(RM)))                  \
        EMITW(0xD360FC00 | MRM(0x02,    0x00,    0x00))

#define mulxn_xm(RM, DP) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9B207C00 | MRM(0x00,    0x00,    TMxx))                     \
        EMITW(0xD360FC00 | MRM(0x02,    0x00,    0x00))

#define mulxp_xr(RM)     /* Reax is in/out, prepares Redx for divxn/xp */   \
                                         /* part-range 32-bit multiply */   \
        EMITW(0x1B007C00 | MRM(0x00,    0x00,    REG(RM)))

#define mulxp_xm(RM, DP) /* Reax is in/out, prepares Redx for divxn/xp */   \
                                         /* part-range 32-bit multiply */   \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x1B007C00 | MRM(0x00,    0x00,    TMxx))

/* div
 * set-flags: no */

#define divxx_xr(RM)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
                                     /* destroys Redx, Xmm0 (in ARMv7) */   \
        EMITW(0x1AC00800 | MRM(0x00,    0x00,    REG(RM)))                  \
                                     /* 32-bit int (fp64 div in ARMv7) */

#define divxx_xm(RM, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
                                     /* destroys Redx, Xmm0 (in ARMv7) */   \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x1AC00800 | MRM(0x00,    0x00,    TMxx))                     \
                                     /* 32-bit int (fp64 div in ARMv7) */

#define divxn_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
                                     /* destroys Redx, Xmm0 (in ARMv7) */   \
        EMITW(0x1AC00C00 | MRM(0x00,    0x00,    REG(RM)))                  \
                                     /* 32-bit int (fp64 div in ARMv7) */

#define divxn_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
                                     /* destroys Redx, Xmm0 (in ARMv7) */   \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x1AC00C00 | MRM(0x00,    0x00,    TMxx))                     \
                                     /* 32-bit int (fp64 div in ARMv7) */

#define divxp_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divxn_xr(W(RM))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divxp_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divxn_xm(W(RM), W(DP))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem
 * set-flags: no */

#define remxx_xx()          /* to be placed immediately prior divx*_x* */   \
        movxx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remxx_xr(RM)        /* to be placed immediately after divx*_xr */   \
        EMITW(0x1B008800 | MRM(0x02,    0x00,    REG(RM)))/* Redx<-rem */

#define remxx_xm(RM, DP)    /* to be placed immediately after divx*_xm */   \
        EMITW(0x1B008800 | MRM(0x02,    0x00,    TMxx))   /* Redx<-rem */

/* cmp
 * set-flags: yes */

#define cmpxx_ri(RM, IM)                                                    \
        AUZ(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x61000000 | MIM(0x1F,    REG(RM), VAL(IM), M1(IM)))

#define cmpxx_mi(RM, DP, IM)                                                \
        AUZ(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x61000000 | MIM(0x1F,    TMxx,    VAL(IM), M1(IM)))

#define cmpxx_rr(RG, RM)                                                    \
        EMITW(0x6B000000 | MRM(0x1F,    REG(RG), REG(RM)))

#define cmpxx_rm(RG, RM, DP)                                                \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x6B000000 | MRM(0x1F,    REG(RG), TMxx))

#define cmpxx_mr(RM, DP, RG)                                                \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x6B000000 | MRM(0x1F,    TMxx,    REG(RG)))

/* jmp
 * set-flags: no */

#define jmpxx_mm(RM, DP)                                                    \
        AUZ(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MZM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
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

/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_M32_H
#define RT_RTARCH_M32_H

#define RT_BASE_REGS        16

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_m32.h: Implementation of MIPS32 r5/r6 BASE instructions.
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
 * cmdx*_** - applies [cmd] to BASE register/memory/immediate args
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

/* structural */

#define MRM(reg, ren, rem)                                                  \
        ((rem) << 16 | (ren) << 21 | (reg) << 11)

#define MDM(reg, brm, vdp, bxx, pxx)                                        \
        (pxx(vdp) | bxx(brm) << 21 | (reg) << 16)

#define MIM(reg, ren, vim, txx, mxx)                                        \
        (mxx(vim) |    (ren) << 21 | txx(reg))

#define AUW(sib, vim, reg, brm, vdp, cdp, cim)                              \
            sib  cdp(brm, vdp)  cim(reg, vim)

#define EMPTY1(em1) em1
#define EMPTY2(em1, em2) em1 em2

/* selectors  */

#define REG(reg, mod, sib)  reg
#define MOD(reg, mod, sib)  mod
#define SIB(reg, mod, sib)  sib

#define VAL(val, tp1, tp2)  val
#define TP1(val, tp1, tp2)  tp1
#define TP2(val, tp1, tp2)  tp2

#define  T1(val, tp1, tp2)  T1##tp1
#define  M1(val, tp1, tp2)  M1##tp1
#define  G1(val, tp1, tp2)  G1##tp1
#define  T2(val, tp1, tp2)  T2##tp2
#define  M2(val, tp1, tp2)  M2##tp2
#define  G2(val, tp1, tp2)  G2##tp2
#define  G3(val, tp1, tp2)  G3##tp2 /* <- "G3##tp2" not a bug */

#define  B1(val, tp1, tp2)  B1##tp1
#define  P1(val, tp1, tp2)  P1##tp1
#define  C1(val, tp1, tp2)  C1##tp1
#define  C3(val, tp1, tp2)  C3##tp2 /* <- "C3##tp2" not a bug */

/* registers    REG   (check mapping with ASM_ENTER/ASM_LEAVE in rtarch.h) */

#define TNxx    0x14  /* s4 (r20), default rounding mode */
#define TLxx    0x18  /* t8 (r24), left  arg for compare */
#define TRxx    0x19  /* t9 (r25), right arg for compare */
#define TMxx    0x18  /* t8 */
#define TIxx    0x19  /* t9, not used together with TDxx */
#define TDxx    0x19  /* t9, not used together with TIxx */
#define TPxx    0x01  /* at (r1) */
#define TZxx    0x00  /* zero (r0) */
#define SPxx    0x1D  /* sp (r29) */

#define Teax    0x04  /* a0 (r4) */
#define Tecx    0x0F  /* t7 (r15) */
#define Tedx    0x02  /* v0 (r2) */
#define Tebx    0x03  /* v1 (r3) */
#define Tebp    0x05  /* a1 (r5) */
#define Tesi    0x06  /* a2 (r6) */
#define Tedi    0x07  /* a3 (r7) */
#define Teg8    0x08  /* t0 (r8) */
#define Teg9    0x09  /* t1 (r9) */
#define TegA    0x0A  /* t2 (r10) */
#define TegB    0x0B  /* t3 (r11) */
#define TegC    0x0C  /* t4 (r12) */
#define TegD    0x0D  /* t5 (r13) */
#define TegE    0x0E  /* t6 (r14) */

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/* registers    REG,  MOD,  SIB */

#define Reax    Teax, 0x00, EMPTY
#define Recx    Tecx, 0x00, EMPTY
#define Redx    Tedx, 0x00, EMPTY
#define Rebx    Tebx, 0x00, EMPTY
#define Rebp    Tebp, 0x00, EMPTY
#define Resi    Tesi, 0x00, EMPTY
#define Redi    Tedi, 0x00, EMPTY
#define Reg8    Teg8, 0x00, EMPTY
#define Reg9    Teg9, 0x00, EMPTY
#define RegA    TegA, 0x00, EMPTY
#define RegB    TegB, 0x00, EMPTY
#define RegC    TegC, 0x00, EMPTY
#define RegD    TegD, 0x00, EMPTY
#define RegE    TegE, 0x00, EMPTY

/* addressing   REG,  MOD,  SIB */

#define Oeax    Teax, Teax, EMPTY

#define Mecx    Tecx, Tecx, EMPTY
#define Medx    Tedx, Tedx, EMPTY
#define Mebx    Tebx, Tebx, EMPTY
#define Mebp    Tebp, Tebp, EMPTY
#define Mesi    Tesi, Tesi, EMPTY
#define Medi    Tedi, Tedi, EMPTY
#define Meg8    Teg8, Teg8, EMPTY
#define Meg9    Teg9, Teg9, EMPTY
#define MegA    TegA, TegA, EMPTY
#define MegB    TegB, TegB, EMPTY
#define MegC    TegC, TegC, EMPTY
#define MegD    TegD, TegD, EMPTY
#define MegE    TegE, TegE, EMPTY

#define Iecx    Tecx, TPxx, EMITW(0x00000021 | MRM(TPxx,    Tecx,    Teax))
#define Iedx    Tedx, TPxx, EMITW(0x00000021 | MRM(TPxx,    Tedx,    Teax))
#define Iebx    Tebx, TPxx, EMITW(0x00000021 | MRM(TPxx,    Tebx,    Teax))
#define Iebp    Tebp, TPxx, EMITW(0x00000021 | MRM(TPxx,    Tebp,    Teax))
#define Iesi    Tesi, TPxx, EMITW(0x00000021 | MRM(TPxx,    Tesi,    Teax))
#define Iedi    Tedi, TPxx, EMITW(0x00000021 | MRM(TPxx,    Tedi,    Teax))
#define Ieg8    Teg8, TPxx, EMITW(0x00000021 | MRM(TPxx,    Teg8,    Teax))
#define Ieg9    Teg9, TPxx, EMITW(0x00000021 | MRM(TPxx,    Teg9,    Teax))
#define IegA    TegA, TPxx, EMITW(0x00000021 | MRM(TPxx,    TegA,    Teax))
#define IegB    TegB, TPxx, EMITW(0x00000021 | MRM(TPxx,    TegB,    Teax))
#define IegC    TegC, TPxx, EMITW(0x00000021 | MRM(TPxx,    TegC,    Teax))
#define IegD    TegD, TPxx, EMITW(0x00000021 | MRM(TPxx,    TegD,    Teax))
#define IegE    TegE, TPxx, EMITW(0x00000021 | MRM(TPxx,    TegE,    Teax))

/* immediate    VAL,  TP1,  TP2 */

#define IC(im)  ((im) & 0x7F),       0, 0      /* drop sign-ext (in x86) */
#define IB(im)  ((im) & 0xFF),       0, 0        /* 32-bit word (in x86) */
#define IM(im)  ((im) & 0xFFF),      0, 0  /* native AArch64 add/sub/cmp */
#define IG(im)  ((im) & 0x7FFF),     0, 0  /* native on MIPS add/sub/cmp */
#define IH(im)  ((im) & 0xFFFF),     1, 0  /* second native on ARMs/MIPS */
#define IV(im)  ((im) & 0x7FFFFFFF), 2, 2        /* native x64 long mode */
#define IW(im)  ((im) & 0xFFFFFFFF), 2, 2        /* extra load op on x64 */

/* displacement VAL,  TP1,  TP2 */

#define DP(dp)  ((dp) & 0xFFC),      0, 0    /* native on all ARMs, MIPS */
#define DF(dp)  ((dp) & 0x3FFC),     0, 1   /* native AArch64 BASE ld/st */
#define DG(dp)  ((dp) & 0x7FFC),     0, 1      /* native MIPS BASE ld/st */
#define DH(dp)  ((dp) & 0xFFFC),     1, 1   /* second native on all ARMs */
#define DV(dp)  ((dp) & 0x7FFFFFFC), 2, 2        /* native x64 long mode */
#define PLAIN   DP(0)           /* special type for Oeax addressing mode */

/* immediate encoding add/sub/cmp(TP1), and/orr/xor(TP2), mov/mul(TP3) */

#define T10(tr) ((tr) << 16)
#define M10(im) (0x00000000 | (im))
#define G10(rg, im) EMPTY
#define T20(tr) ((tr) << 16)
#define M20(im) (0x00000000 | (im))
#define G20(rg, im) EMPTY
#define G30(rg, im) EMITW(0x34000000 | (rg) << 16 | (0xFFFF & (im)))

#define T11(tr) ((tr) << 11)
#define M11(im) (0x00000000 | TIxx << 16)
#define G11(rg, im) G30(rg, im)

#define T12(tr) ((tr) << 11)
#define M12(im) (0x00000000 | TIxx << 16)
#define G12(rg, im) G32(rg, im)
#define T22(tr) ((tr) << 11)
#define M22(im) (0x00000000 | TIxx << 16)
#define G22(rg, im) G32(rg, im)
#define G32(rg, im) EMITW(0x3C000000 | (rg) << 16 | (0xFFFF & (im) >> 16))  \
                    EMITW(0x34000000 | (rg) << 16 | (rg) << 21 |            \
                                                    (0xFFFF & (im)))

/* displacement encoding BASE(TP1), adr(TP3) */

#define B10(br) (br)
#define P10(dp) (0x00000000 | (dp))
#define C10(br, dp) EMPTY
#define C30(br, dp) EMITW(0x34000000 | TDxx << 16 | (0xFFFC & (dp)))

#define B11(br) TPxx
#define P11(dp) (0x00000000)
#define C11(br, dp) C30(br, dp)                                             \
                    EMITW(0x00000021 | MRM(TPxx,    (br),    TDxx))
#define C31(br, dp) EMITW(0x34000000 | TDxx << 16 | (0xFFFC & (dp)))

#define B12(br) TPxx
#define P12(dp) (0x00000000)
#define C12(br, dp) C32(br, dp)                                             \
                    EMITW(0x00000021 | MRM(TPxx,    (br),    TDxx))
#define C32(br, dp) EMITW(0x3C000000 | TDxx << 16 | (0x7FFF & (dp) >> 16))  \
                    EMITW(0x34000000 | TDxx << 16 | TDxx << 21 |            \
                                                    (0xFFFC & (dp)))

/* triplet pass-through wrapper */

#define W(p1, p2, p3)       p1,  p2,  p3

/******************************************************************************/
/**********************************   M32   ***********************************/
/******************************************************************************/

/* mov
 * set-flags: no */

#define movxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), REG(RM), EMPTY,   EMPTY,   EMPTY2, G3(IM))

#define movxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G3(IM))   \
        EMITW(0xAC000000 | MDM(TIxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define movxx_rr(RG, RM)                                                    \
        EMITW(0x00000025 | MRM(REG(RG), REG(RM), TZxx))

#define movxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(REG(RG), MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define movxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xAC000000 | MDM(REG(RG), MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define adrxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C3(DP), EMPTY2)   \
        EMITW(0x00000021 | MRM(REG(RG), MOD(RM), TDxx))

#define adrxx_lb(lb) /* load label to Reax */                               \
        label_ld(lb)

#define stack_st(RM)                                                        \
        EMITW(0x24000000 | MRM(0x00,    SPxx,    SPxx) | (-0x04 & 0xFFFF))  \
        EMITW(0xAC000000 | MRM(0x00,    SPxx,    REG(RM)))

#define stack_ld(RM)                                                        \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,    REG(RM)))                  \
        EMITW(0x24000000 | MRM(0x00,    SPxx,    SPxx) | (+0x04 & 0xFFFF))

#define stack_sa()   /* save all, [Reax - RegE] + 4 temps, 18 regs total */ \
        EMITW(0x24000000 | MRM(0x00,    SPxx,    SPxx) | (-0x48 & 0xFFFF))  \
        EMITW(0xAC000000 | MRM(0x00,    SPxx,    Teax) | (+0x00 & 0xFFFF))  \
        EMITW(0xAC000000 | MRM(0x00,    SPxx,    Tecx) | (+0x04 & 0xFFFF))  \
        EMITW(0xAC000000 | MRM(0x00,    SPxx,    Tedx) | (+0x08 & 0xFFFF))  \
        EMITW(0xAC000000 | MRM(0x00,    SPxx,    Tebx) | (+0x0C & 0xFFFF))  \
        EMITW(0xAC000000 | MRM(0x00,    SPxx,    Tebp) | (+0x10 & 0xFFFF))  \
        EMITW(0xAC000000 | MRM(0x00,    SPxx,    Tesi) | (+0x14 & 0xFFFF))  \
        EMITW(0xAC000000 | MRM(0x00,    SPxx,    Tedi) | (+0x18 & 0xFFFF))  \
        EMITW(0xAC000000 | MRM(0x00,    SPxx,    Teg8) | (+0x1C & 0xFFFF))  \
        EMITW(0xAC000000 | MRM(0x00,    SPxx,    Teg9) | (+0x20 & 0xFFFF))  \
        EMITW(0xAC000000 | MRM(0x00,    SPxx,    TegA) | (+0x24 & 0xFFFF))  \
        EMITW(0xAC000000 | MRM(0x00,    SPxx,    TegB) | (+0x28 & 0xFFFF))  \
        EMITW(0xAC000000 | MRM(0x00,    SPxx,    TegC) | (+0x2C & 0xFFFF))  \
        EMITW(0xAC000000 | MRM(0x00,    SPxx,    TegD) | (+0x30 & 0xFFFF))  \
        EMITW(0xAC000000 | MRM(0x00,    SPxx,    TegE) | (+0x34 & 0xFFFF))  \
        EMITW(0xAC000000 | MRM(0x00,    SPxx,    TMxx) | (+0x38 & 0xFFFF))  \
        EMITW(0xAC000000 | MRM(0x00,    SPxx,    TIxx) | (+0x3C & 0xFFFF))  \
        EMITW(0xAC000000 | MRM(0x00,    SPxx,    TPxx) | (+0x40 & 0xFFFF))  \
        EMITW(0xAC000000 | MRM(0x00,    SPxx,    TNxx) | (+0x44 & 0xFFFF))

#define stack_la()   /* load all, 4 temps + [RegE - Reax], 18 regs total */ \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,    TNxx) | (+0x44 & 0xFFFF))  \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,    TPxx) | (+0x40 & 0xFFFF))  \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,    TIxx) | (+0x3C & 0xFFFF))  \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,    TMxx) | (+0x38 & 0xFFFF))  \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,    TegE) | (+0x34 & 0xFFFF))  \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,    TegD) | (+0x30 & 0xFFFF))  \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,    TegC) | (+0x2C & 0xFFFF))  \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,    TegB) | (+0x28 & 0xFFFF))  \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,    TegA) | (+0x24 & 0xFFFF))  \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,    Teg9) | (+0x20 & 0xFFFF))  \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,    Teg8) | (+0x1C & 0xFFFF))  \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,    Tedi) | (+0x18 & 0xFFFF))  \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,    Tesi) | (+0x14 & 0xFFFF))  \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,    Tebp) | (+0x10 & 0xFFFF))  \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,    Tebx) | (+0x0C & 0xFFFF))  \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,    Tedx) | (+0x08 & 0xFFFF))  \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,    Tecx) | (+0x04 & 0xFFFF))  \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,    Teax) | (+0x00 & 0xFFFF))  \
        EMITW(0x24000000 | MRM(0x00,    SPxx,    SPxx) | (+0x48 & 0xFFFF))

/* and
 * set-flags: yes */

#define andxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x30000000) | (+(TP2(IM) != 0) & 0x00000024))    \
        /* if true ^ equals to -1 (not 1) */                                \
        EMITW(0x00000025 | MRM(TLxx,    REG(RM), TZxx))/* <- set flags (Z) */

#define andxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x30000000) | (+(TP2(IM) != 0) & 0x00000024))    \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define andxx_rr(RG, RM)                                                    \
        EMITW(0x00000024 | MRM(REG(RG), REG(RG), REG(RM)))                  \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define andxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000024 | MRM(REG(RG), REG(RG), TMxx))

#define andxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000024 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* orr
 * set-flags: no */

#define orrxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x34000000) | (+(TP2(IM) != 0) & 0x00000025))    \
        /* if true ^ equals to -1 (not 1) */

#define orrxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x34000000) | (+(TP2(IM) != 0) & 0x00000025))    \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define orrxx_rr(RG, RM)                                                    \
        EMITW(0x00000025 | MRM(REG(RG), REG(RG), REG(RM)))

#define orrxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000025 | MRM(REG(RG), REG(RG), TMxx))

#define orrxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000025 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* xor
 * set-flags: no */

#define xorxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x38000000) | (+(TP2(IM) != 0) & 0x00000026))    \
        /* if true ^ equals to -1 (not 1) */

#define xorxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x38000000) | (+(TP2(IM) != 0) & 0x00000026))    \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define xorxx_rr(RG, RM)                                                    \
        EMITW(0x00000026 | MRM(REG(RG), REG(RG), REG(RM)))

#define xorxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000026 | MRM(REG(RG), REG(RG), TMxx))

#define xorxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000026 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* not
 * set-flags: no */

#define notxx_rr(RM)                                                        \
        EMITW(0x00000027 | MRM(REG(RM), TZxx,    REG(RM)))

#define notxx_mm(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000027 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* neg
 * set-flags: yes */

#define negxx_rr(RM)                                                        \
        EMITW(0x00000023 | MRM(REG(RM), TZxx,    REG(RM)))                  \
        EMITW(0x00000025 | MRM(TLxx,    REG(RM), TZxx))/* <- set flags (Z) */

#define negxx_mm(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000023 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* add
 * set-flags: yes */

#define addxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T1(IM), M1(IM)) | \
        (+(TP1(IM) == 0) & 0x24000000) | (+(TP1(IM) != 0) & 0x00000021))    \
        /* if true ^ equals to -1 (not 1) */                                \
        EMITW(0x00000025 | MRM(TLxx,    REG(RM), TZxx))/* <- set flags (Z) */

#define addxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T1(IM), M1(IM)) | \
        (+(TP1(IM) == 0) & 0x24000000) | (+(TP1(IM) != 0) & 0x00000021))    \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define addxx_rr(RG, RM)                                                    \
        EMITW(0x00000021 | MRM(REG(RG), REG(RG), REG(RM)))                  \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define addxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000021 | MRM(REG(RG), REG(RG), TMxx))

#define addxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000021 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* sub
 * set-flags: yes */

#define subxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), 0x00,    T1(IM), EMPTY1) | \
        (+(TP1(IM) == 0) & (0x24000000 | (0xFFFF & -VAL(IM)))) |            \
        (+(TP1(IM) != 0) & (0x00000023 | TIxx << 16)))                      \
        /* if true ^ equals to -1 (not 1) */                                \
        EMITW(0x00000025 | MRM(TLxx,    REG(RM), TZxx))/* <- set flags (Z) */

#define subxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    0x00,    T1(IM), EMPTY1) | \
        (+(TP1(IM) == 0) & (0x24000000 | (0xFFFF & -VAL(IM)))) |            \
        (+(TP1(IM) != 0) & (0x00000023 | TIxx << 16)))                      \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define subxx_rr(RG, RM)                                                    \
        EMITW(0x00000023 | MRM(REG(RG), REG(RG), REG(RM)))                  \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define subxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000023 | MRM(REG(RG), REG(RG), TMxx))

#define subxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000023 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define subxx_mr(RM, DP, RG)                                                \
        subxx_st(W(RG), W(RM), W(DP))

/* shl
 * set-flags: no */

#define shlxx_ri(RM, IM)                                                    \
        EMITW(0x00000000 | MRM(REG(RM), 0x00,    REG(RM)) |                 \
                                                 (0x1F & VAL(IM)) << 6)

#define shlxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IM)) << 6)     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shlxx_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0x00000004 | MRM(REG(RM), Tecx,    REG(RM)))

#define shlxx_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000004 | MRM(TMxx,    Tecx,    TMxx))                     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* shr
 * set-flags: no */

#define shrxx_ri(RM, IM)                                                    \
        EMITW(0x00000002 | MRM(REG(RM), 0x00,    REG(RM)) |                 \
                                                 (0x1F & VAL(IM)) << 6)

#define shrxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000002 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IM)) << 6)     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrxx_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0x00000006 | MRM(REG(RM), Tecx,    REG(RM)))

#define shrxx_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000006 | MRM(TMxx,    Tecx,    TMxx))                     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrxn_ri(RM, IM)                                                    \
        EMITW(0x00000003 | MRM(REG(RM), 0x00,    REG(RM)) |                 \
                                                 (0x1F & VAL(IM)) << 6)

#define shrxn_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000003 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IM)) << 6)     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrxn_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0x00000007 | MRM(REG(RM), Tecx,    REG(RM)))

#define shrxn_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000007 | MRM(TMxx,    Tecx,    TMxx))                     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#if RT_M32 < 6 /* pre-r6 */

/* mul
 * set-flags: no */

#define mulxx_ri(RM, IM)                 /* part-range 32-bit multiply */   \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x70000002 | MRM(REG(RM), REG(RM), TIxx))

#define mulxx_rr(RG, RM)                 /* part-range 32-bit multiply */   \
        EMITW(0x70000002 | MRM(REG(RG), REG(RG), REG(RM)))

#define mulxx_ld(RG, RM, DP)             /* part-range 32-bit multiply */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x70000002 | MRM(REG(RG), REG(RG), TMxx))

#define mulxn_ri(RM, IM)                 /* part-range 32-bit multiply */   \
        mulxx_ri(W(RM), W(IM))

#define mulxn_rr(RG, RM)                 /* part-range 32-bit multiply */   \
        mulxx_rr(W(RG), W(RM))

#define mulxn_ld(RG, RM, DP)             /* part-range 32-bit multiply */   \
        mulxx_ld(W(RG), W(RM), W(DP))

#define mulxx_xr(RM)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0x00000019 | MRM(0x00,    Teax,    REG(RM)))                  \
        EMITW(0x00000012 | MRM(Teax,    0x00,    0x00))                     \
        EMITW(0x00000010 | MRM(Tedx,    0x00,    0x00))

#define mulxx_xm(RM, DP) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000019 | MRM(0x00,    Teax,    TMxx))                     \
        EMITW(0x00000012 | MRM(Teax,    0x00,    0x00))                     \
        EMITW(0x00000010 | MRM(Tedx,    0x00,    0x00))

#define mulxn_xr(RM)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0x00000018 | MRM(0x00,    Teax,    REG(RM)))                  \
        EMITW(0x00000012 | MRM(Teax,    0x00,    0x00))                     \
        EMITW(0x00000010 | MRM(Tedx,    0x00,    0x00))

#define mulxn_xm(RM, DP) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000018 | MRM(0x00,    Teax,    TMxx))                     \
        EMITW(0x00000012 | MRM(Teax,    0x00,    0x00))                     \
        EMITW(0x00000010 | MRM(Tedx,    0x00,    0x00))

#define mulxp_xr(RM)     /* Reax is in/out, prepares Redx for divxn/xp */   \
                                         /* part-range 32-bit multiply */   \
        EMITW(0x70000002 | MRM(Teax,    Teax,    REG(RM)))

#define mulxp_xm(RM, DP) /* Reax is in/out, prepares Redx for divxn/xp */   \
                                         /* part-range 32-bit multiply */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x70000002 | MRM(Teax,    Teax,    TMxx))

/* div
 * set-flags: no */

#define divxx_xr(RM)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
                                     /* destroys Redx, Xmm0 (in ARMv7) */   \
        EMITW(0x0000001B | MRM(0x00,    Teax,    REG(RM)))                  \
        EMITW(0x00000012 | MRM(Teax,    0x00,    0x00))                     \
                                     /* 32-bit int (fp64 div in ARMv7) */

#define divxx_xm(RM, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
                                     /* destroys Redx, Xmm0 (in ARMv7) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x0000001B | MRM(0x00,    Teax,    TMxx))                     \
        EMITW(0x00000012 | MRM(Teax,    0x00,    0x00))                     \
                                     /* 32-bit int (fp64 div in ARMv7) */

#define divxn_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
                                     /* destroys Redx, Xmm0 (in ARMv7) */   \
        EMITW(0x0000001A | MRM(0x00,    Teax,    REG(RM)))                  \
        EMITW(0x00000012 | MRM(Teax,    0x00,    0x00))                     \
                                     /* 32-bit int (fp64 div in ARMv7) */

#define divxn_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
                                     /* destroys Redx, Xmm0 (in ARMv7) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x0000001A | MRM(0x00,    Teax,    TMxx))                     \
        EMITW(0x00000012 | MRM(Teax,    0x00,    0x00))                     \
                                     /* 32-bit int (fp64 div in ARMv7) */

#define divxp_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divxn_xr(W(RM))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divxp_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divxn_xm(W(RM), W(DP))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem
 * set-flags: no */

#define remxx_xx()          /* to be placed immediately prior divxx_x* */   \
                                     /* to prepare for rem calculation */

#define remxx_xr(RM)        /* to be placed immediately after divxx_xr */   \
        EMITW(0x00000010 | MRM(Tedx,    0x00,    0x00))   /* Redx<-rem */

#define remxx_xm(RM, DP)    /* to be placed immediately after divxx_xm */   \
        EMITW(0x00000010 | MRM(Tedx,    0x00,    0x00))   /* Redx<-rem */

#define remxn_xx()          /* to be placed immediately prior divxn_x* */   \
                                     /* to prepare for rem calculation */

#define remxn_xr(RM)        /* to be placed immediately after divxn_xr */   \
        EMITW(0x00000010 | MRM(Tedx,    0x00,    0x00))   /* Redx<-rem */

#define remxn_xm(RM, DP)    /* to be placed immediately after divxn_xm */   \
        EMITW(0x00000010 | MRM(Tedx,    0x00,    0x00))   /* Redx<-rem */

#else  /* r6 */

/* mul
 * set-flags: no */

#define mulxx_ri(RM, IM)                 /* part-range 32-bit multiply */   \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x00000099 | MRM(REG(RM), REG(RM), TIxx))

#define mulxx_rr(RG, RM)                 /* part-range 32-bit multiply */   \
        EMITW(0x00000099 | MRM(REG(RG), REG(RG), REG(RM)))

#define mulxx_ld(RG, RM, DP)             /* part-range 32-bit multiply */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000099 | MRM(REG(RG), REG(RG), TMxx))

#define mulxn_ri(RM, IM)                 /* part-range 32-bit multiply */   \
        mulxx_ri(W(RM), W(IM))

#define mulxn_rr(RG, RM)                 /* part-range 32-bit multiply */   \
        mulxx_rr(W(RG), W(RM))

#define mulxn_ld(RG, RM, DP)             /* part-range 32-bit multiply */   \
        mulxx_ld(W(RG), W(RM), W(DP))

#define mulxx_xr(RM)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0x000000D9 | MRM(Tedx,    Teax,    REG(RM)))                  \
        EMITW(0x00000099 | MRM(Teax,    Teax,    REG(RM)))

#define mulxx_xm(RM, DP) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x000000D9 | MRM(Tedx,    Teax,    TMxx))                     \
        EMITW(0x00000099 | MRM(Teax,    Teax,    TMxx))

#define mulxn_xr(RM)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0x000000D8 | MRM(Tedx,    Teax,    REG(RM)))                  \
        EMITW(0x00000098 | MRM(Teax,    Teax,    REG(RM)))

#define mulxn_xm(RM, DP) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x000000D8 | MRM(Tedx,    Teax,    TMxx))                     \
        EMITW(0x00000098 | MRM(Teax,    Teax,    TMxx))

#define mulxp_xr(RM)     /* Reax is in/out, prepares Redx for divxn/xp */   \
                                         /* part-range 32-bit multiply */   \
        EMITW(0x00000099 | MRM(Teax,    Teax,    REG(RM)))

#define mulxp_xm(RM, DP) /* Reax is in/out, prepares Redx for divxn/xp */   \
                                         /* part-range 32-bit multiply */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000099 | MRM(Teax,    Teax,    TMxx))

/* div
 * set-flags: no */

#define divxx_xr(RM)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
                                     /* destroys Redx, Xmm0 (in ARMv7) */   \
        EMITW(0x0000009B | MRM(Teax,    Teax,    REG(RM)))                  \
                                     /* 32-bit int (fp64 div in ARMv7) */

#define divxx_xm(RM, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
                                     /* destroys Redx, Xmm0 (in ARMv7) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x0000009B | MRM(Teax,    Teax,    TMxx))                     \
                                     /* 32-bit int (fp64 div in ARMv7) */

#define divxn_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
                                     /* destroys Redx, Xmm0 (in ARMv7) */   \
        EMITW(0x0000009A | MRM(Teax,    Teax,    REG(RM)))                  \
                                     /* 32-bit int (fp64 div in ARMv7) */

#define divxn_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
                                     /* destroys Redx, Xmm0 (in ARMv7) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x0000009A | MRM(Teax,    Teax,    TMxx))                     \
                                     /* 32-bit int (fp64 div in ARMv7) */

#define divxp_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divxn_xr(W(RM))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divxp_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divxn_xm(W(RM), W(DP))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem
 * set-flags: no */

#define remxx_xx()          /* to be placed immediately prior divxx_x* */   \
                                     /* to prepare for rem calculation */

#define remxx_xr(RM)        /* to be placed immediately after divxx_xr */   \
        EMITW(0x000000DB | MRM(Tedx,    Teax,    REG(RM)))/* Redx<-rem */

#define remxx_xm(RM, DP)    /* to be placed immediately after divxx_xm */   \
        EMITW(0x000000DB | MRM(Tedx,    Teax,    TMxx))   /* Redx<-rem */

#define remxn_xx()          /* to be placed immediately prior divxn_x* */   \
                                     /* to prepare for rem calculation */

#define remxn_xr(RM)        /* to be placed immediately after divxn_xr */   \
        EMITW(0x000000DA | MRM(Tedx,    Teax,    REG(RM)))/* Redx<-rem */

#define remxn_xm(RM, DP)    /* to be placed immediately after divxn_xm */   \
        EMITW(0x000000DA | MRM(Tedx,    Teax,    TMxx))   /* Redx<-rem */

#endif /* r6 */

/* cmp
 * set-flags: yes */

#define cmpxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x00000025 | MRM(TLxx,    REG(RM), TZxx))

#define cmpxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TRxx,    MOD(RM), VAL(DP), C1(DP), G3(IM))   \
        EMITW(0x8C000000 | MDM(TLxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define cmpxx_rr(RG, RM)                                                    \
        EMITW(0x00000025 | MRM(TRxx,    REG(RM), TZxx))                     \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))

#define cmpxx_rm(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TRxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))

#define cmpxx_mr(RM, DP, RG)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TLxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000025 | MRM(TRxx,    REG(RG), TZxx))

#if RT_M32 < 6 /* pre-r6 */

/* jmp
 * set-flags: no */

#define jmpxx_mm(RM, DP)         /* memory-targeted unconditional jump */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000008 | MRM(0x00,    TMxx,    0x00))                     \
        EMITW(0x00000025 | MRM(TPxx,    TPxx,    TZxx)) /* <- branch delay */

#define jmpxx_lb(lb)              /* label-targeted unconditional jump */   \
        ASM_BEG ASM_OP1(b, lb) ASM_END

#define jezxx_lb(lb)               /* setting-flags-arithmetic -> jump */   \
        ASM_BEG ASM_OP2(beqz, $t8, lb) ASM_END

#define jnzxx_lb(lb)               /* setting-flags-arithmetic -> jump */   \
        ASM_BEG ASM_OP2(bnez, $t8, lb) ASM_END

#define jeqxx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(beq,  $t8, $t9, lb) ASM_END

#define jnexx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(bne,  $t8, $t9, lb) ASM_END

#define jltxx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(sltu, $t8, $t8, $t9) ASM_END                        \
        ASM_BEG ASM_OP2(bnez, $t8, lb) ASM_END

#define jlexx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(sltu, $t8, $t9, $t8) ASM_END                        \
        ASM_BEG ASM_OP2(beqz, $t8, lb) ASM_END

#define jgtxx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(sltu, $t8, $t9, $t8) ASM_END                        \
        ASM_BEG ASM_OP2(bnez, $t8, lb) ASM_END

#define jgexx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(sltu, $t8, $t8, $t9) ASM_END                        \
        ASM_BEG ASM_OP2(beqz, $t8, lb) ASM_END

#define jltxn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(slt,  $t8, $t8, $t9) ASM_END                        \
        ASM_BEG ASM_OP2(bnez, $t8, lb) ASM_END

#define jlexn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(slt,  $t8, $t9, $t8) ASM_END                        \
        ASM_BEG ASM_OP2(beqz, $t8, lb) ASM_END

#define jgtxn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(slt,  $t8, $t9, $t8) ASM_END                        \
        ASM_BEG ASM_OP2(bnez, $t8, lb) ASM_END

#define jgexn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(slt,  $t8, $t8, $t9) ASM_END                        \
        ASM_BEG ASM_OP2(beqz, $t8, lb) ASM_END

#define LBL(lb)                                          /* code label */   \
        ASM_BEG ASM_OP0(lb:) ASM_END

#else  /* r6 */

/* jmp
 * set-flags: no */

#define jmpxx_mm(RM, DP)         /* memory-targeted unconditional jump */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000009 | MRM(0x00,    TMxx,    0x00))                     \
        EMITW(0x00000025 | MRM(TPxx,    TPxx,    TZxx)) /* <- branch delay */

#define jmpxx_lb(lb)              /* label-targeted unconditional jump */   \
        ASM_BEG ASM_OP1(b, lb) ASM_END

#define jezxx_lb(lb)               /* setting-flags-arithmetic -> jump */   \
        ASM_BEG ASM_OP2(beqzc, $t8, lb) ASM_END

#define jnzxx_lb(lb)               /* setting-flags-arithmetic -> jump */   \
        ASM_BEG ASM_OP2(bnezc, $t8, lb) ASM_END

#define jeqxx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(beqc,  $t8, $t9, lb) ASM_END

#define jnexx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(bnec,  $t8, $t9, lb) ASM_END

#define jltxx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(bltuc, $t8, $t9, lb) ASM_END

#define jlexx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(bgeuc, $t9, $t8, lb) ASM_END

#define jgtxx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(bltuc, $t9, $t8, lb) ASM_END

#define jgexx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(bgeuc, $t8, $t9, lb) ASM_END

#define jltxn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(bltc,  $t8, $t9, lb) ASM_END

#define jlexn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(bgec,  $t9, $t8, lb) ASM_END

#define jgtxn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(bltc,  $t9, $t8, lb) ASM_END

#define jgexn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(bgec,  $t8, $t9, lb) ASM_END

#define LBL(lb)                                          /* code label */   \
        ASM_BEG ASM_OP0(lb:) ASM_END

#endif /* r6 */

/* ver
 * set-flags: no */

#define verxx_xx() /* destroys Reax, Recx, Rebx, Redx, Resi, Redi (in x86)*/\
        movxx_mi(Mebp, inf_VER, IB(1)) /* <- SIMD to bit0, without checks */

#endif /* RT_RTARCH_M32_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

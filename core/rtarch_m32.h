/******************************************************************************/
/* Copyright (c) 2013-2015 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_M32_H
#define RT_RTARCH_M32_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_m32.h: Implementation of MIPS32 r5/r6 core instructions.
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

/* registers    REG */

#define TLxx    0x18                    /* t8(r24) */
#define TRxx    0x19                    /* t9(r25) */
#define TMxx    0x18                    /* t8 */
#define TIxx    0x19                    /* t9, not used together with TDxx */
#define TDxx    0x19                    /* t9, not used together with TIxx */
#define TPxx    0x0F                    /* t7(r15) */

#define Teax    0x04                    /* a0(r4) */
#define Tecx    0x01                    /* at(r1) */
#define Tedx    0x02                    /* v0(r2) */

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/* registers    REG,  MOD,  SIB */

#define Reax    Teax, 0x00, EMPTY       /* a0(r4) */
#define Recx    Tecx, 0x00, EMPTY       /* at(r1) */
#define Redx    Tedx, 0x00, EMPTY       /* v0(r2) */
#define Rebx    0x03, 0x00, EMPTY       /* v1(r3) */
#define Rebp    0x05, 0x00, EMPTY       /* a1(r5) */
#define Resi    0x06, 0x00, EMPTY       /* a2(r6) */
#define Redi    0x07, 0x00, EMPTY       /* a3(r7) */
#define Reg8    0x08, 0x00, EMPTY       /* t0(r8) */
#define Reg9    0x09, 0x00, EMPTY       /* t1(r9) */
#define RegA    0x0A, 0x00, EMPTY       /* t2(r10) */
#define RegB    0x0B, 0x00, EMPTY       /* t3(r11) */
#define RegC    0x0C, 0x00, EMPTY       /* t4(r12) */
#define RegD    0x0D, 0x00, EMPTY       /* t5(r13) */
#define RegE    0x0E, 0x00, EMPTY       /* t6(r14) */

/* addressing   REG,  MOD,  SIB */

#define Oeax    Teax, Teax, EMPTY       /* [r4] */

#define Mecx    Tecx, Tecx, EMPTY       /* [r1, DP] */
#define Medx    Tedx, Tedx, EMPTY       /* [r2, DP] */
#define Mebx    0x03, 0x03, EMPTY       /* [r3, DP] */
#define Mebp    0x05, 0x05, EMPTY       /* [r5, DP] */
#define Mesi    0x06, 0x06, EMPTY       /* [r6, DP] */
#define Medi    0x07, 0x07, EMPTY       /* [r7, DP] */
#define Meg8    0x08, 0x08, EMPTY       /* [r8, DP] */
#define Meg9    0x09, 0x09, EMPTY       /* [r9, DP] */
#define MegA    0x0A, 0x0A, EMPTY       /* [r10, DP] */
#define MegB    0x0B, 0x0B, EMPTY       /* [r11, DP] */
#define MegC    0x0C, 0x0C, EMPTY       /* [r12, DP] */
#define MegD    0x0D, 0x0D, EMPTY       /* [r13, DP] */
#define MegE    0x0E, 0x0E, EMPTY       /* [r14, DP] */

#define Iecx    Tecx, TPxx, EMITW(0x00000021 | MRM(TPxx,    Tecx,    Teax))
#define Iedx    Tedx, TPxx, EMITW(0x00000021 | MRM(TPxx,    Tedx,    Teax))
#define Iebx    0x03, TPxx, EMITW(0x00000021 | MRM(TPxx,    0x03,    Teax))
#define Iebp    0x05, TPxx, EMITW(0x00000021 | MRM(TPxx,    0x05,    Teax))
#define Iesi    0x06, TPxx, EMITW(0x00000021 | MRM(TPxx,    0x06,    Teax))
#define Iedi    0x07, TPxx, EMITW(0x00000021 | MRM(TPxx,    0x07,    Teax))
#define Ieg8    0x08, TPxx, EMITW(0x00000021 | MRM(TPxx,    0x08,    Teax))
#define Ieg9    0x09, TPxx, EMITW(0x00000021 | MRM(TPxx,    0x09,    Teax))
#define IegA    0x0A, TPxx, EMITW(0x00000021 | MRM(TPxx,    0x0A,    Teax))
#define IegB    0x0B, TPxx, EMITW(0x00000021 | MRM(TPxx,    0x0B,    Teax))
#define IegC    0x0C, TPxx, EMITW(0x00000021 | MRM(TPxx,    0x0C,    Teax))
#define IegD    0x0D, TPxx, EMITW(0x00000021 | MRM(TPxx,    0x0D,    Teax))
#define IegE    0x0E, TPxx, EMITW(0x00000021 | MRM(TPxx,    0x0E,    Teax))

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
#define DF(dp)  ((dp) & 0x3FFC),     0, 1   /* native AArch64 core ld/st */
#define DG(dp)  ((dp) & 0x7FFC),     0, 1      /* native MIPS core ld/st */
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

/* displacement encoding core(TP1), adr(TP3) */

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
        EMITW(0x00000025 | MRM(REG(RG), 0x00,    REG(RM)))

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

#define stack_st(RM)

#define stack_ld(RM)

#define stack_sa()

#define stack_la()

/* and
 * set-flags: yes */

#define andxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x30000000) | (+(TP2(IM) != 0) & 0x00000024))    \
        /* if true ^ equals to -1 (not 1) */                                \
        EMITW(0x00000025 | MRM(TLxx,    0x00,    REG(RM))) /* <- set flags */

#define andxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x30000000) | (+(TP2(IM) != 0) & 0x00000024))    \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define andxx_rr(RG, RM)                                                    \
        EMITW(0x00000024 | MRM(REG(RG), REG(RG), REG(RM)))                  \
        EMITW(0x00000025 | MRM(TLxx,    0x00,    REG(RG))) /* <- set flags */

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
        EMITW(0x00000027 | MRM(REG(RM), 0x00,    REG(RM)))

#define notxx_mm(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000027 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* neg
 * set-flags: yes */

#define negxx_rr(RM)                                                        \
        EMITW(0x00000023 | MRM(REG(RM), 0x00,    REG(RM)))                  \
        EMITW(0x00000025 | MRM(TLxx,    0x00,    REG(RM))) /* <- set flags */

#define negxx_mm(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000023 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* add
 * set-flags: yes */

#define addxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T1(IM), M1(IM)) | \
        (+(TP1(IM) == 0) & 0x24000000) | (+(TP1(IM) != 0) & 0x00000021))    \
        /* if true ^ equals to -1 (not 1) */                                \
        EMITW(0x00000025 | MRM(TLxx,    0x00,    REG(RM))) /* <- set flags */

#define addxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T1(IM), M1(IM)) | \
        (+(TP1(IM) == 0) & 0x24000000) | (+(TP1(IM) != 0) & 0x00000021))    \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define addxx_rr(RG, RM)                                                    \
        EMITW(0x00000021 | MRM(REG(RG), REG(RG), REG(RM)))                  \
        EMITW(0x00000025 | MRM(TLxx,    0x00,    REG(RG))) /* <- set flags */

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
        (+(TP1(IM) == 0) & 0x24000000) | (+(TP1(IM) != 0) & 0x00000023)   | \
        (+(TP1(IM) == 0) & 0x0000FFFF  &   -VAL(IM)))                       \
        /* if true ^ equals to -1 (not 1) */                                \
        EMITW(0x00000025 | MRM(TLxx,    0x00,    REG(RM))) /* <- set flags */

#define subxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    0x00,    T1(IM), EMPTY1) | \
        (+(TP1(IM) == 0) & 0x24000000) | (+(TP1(IM) != 0) & 0x00000023)   | \
        (+(TP1(IM) == 0) & 0x0000FFFF  &   -VAL(IM)))                       \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define subxx_rr(RG, RM)                                                    \
        EMITW(0x00000023 | MRM(REG(RG), REG(RG), REG(RM)))                  \
        EMITW(0x00000025 | MRM(TLxx,    0x00,    REG(RG))) /* <- set flags */

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

#define remxx_xx()          /* to be placed immediately prior divx*_x* */   \
                                     /* to prepare for rem calculation */

#define remxx_xr(RM)        /* to be placed immediately after divx*_xr */   \
        EMITW(0x00000010 | MRM(Tedx,    0x00,    0x00))   /* Redx<-rem */

#define remxx_xm(RM, DP)    /* to be placed immediately after divx*_xm */   \
        EMITW(0x00000010 | MRM(Tedx,    0x00,    0x00))   /* Redx<-rem */

/* cmp
 * set-flags: yes */

#define cmpxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x00000025 | MRM(TLxx,    0x00,    REG(RM)))

#define cmpxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TRxx,    MOD(RM), VAL(DP), C1(DP), G3(IM))   \
        EMITW(0x8C000000 | MDM(TLxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define cmpxx_rr(RG, RM)                                                    \
        EMITW(0x00000025 | MRM(TRxx,    0x00,    REG(RM)))                  \
        EMITW(0x00000025 | MRM(TLxx,    0x00,    REG(RG)))

#define cmpxx_rm(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TRxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000025 | MRM(TLxx,    0x00,    REG(RG)))

#define cmpxx_mr(RM, DP, RG)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TLxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000025 | MRM(TRxx,    0x00,    REG(RG)))

/* jmp
 * set-flags: no */

#define jmpxx_mm(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000008 | MRM(0x00,    TMxx,    0x00))                     \
        EMITW(0x00000000 | MRM(0x00,    0x00,    0x00)) /* <- branch delay */

#define jmpxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(b, lb) ASM_END

#define jeqxx_lb(lb)                                                        \
        ASM_BEG ASM_OP3(beq,  $t8, $t9, lb) ASM_END

#define jezxx_lb(lb)                                                        \
        ASM_BEG ASM_OP2(beqz, $t8, lb) ASM_END

#define jnexx_lb(lb)                                                        \
        ASM_BEG ASM_OP3(bne,  $t8, $t9, lb) ASM_END

#define jnzxx_lb(lb)                                                        \
        ASM_BEG ASM_OP2(bnez, $t8, lb) ASM_END

#define jltxx_lb(lb)                                                        \
        ASM_BEG ASM_OP3(sltu, $t8, $t8, $t9) ASM_END                        \
        ASM_BEG ASM_OP2(bnez, $t8, lb) ASM_END

#define jlexx_lb(lb)                                                        \
        ASM_BEG ASM_OP3(sltu, $t8, $t9, $t8) ASM_END                        \
        ASM_BEG ASM_OP2(beqz, $t8, lb) ASM_END

#define jgtxx_lb(lb)                                                        \
        ASM_BEG ASM_OP3(sltu, $t8, $t9, $t8) ASM_END                        \
        ASM_BEG ASM_OP2(bnez, $t8, lb) ASM_END

#define jgexx_lb(lb)                                                        \
        ASM_BEG ASM_OP3(sltu, $t8, $t8, $t9) ASM_END                        \
        ASM_BEG ASM_OP2(beqz, $t8, lb) ASM_END

#define jltxn_lb(lb)                                                        \
        ASM_BEG ASM_OP3(slt,  $t8, $t8, $t9) ASM_END                        \
        ASM_BEG ASM_OP2(bnez, $t8, lb) ASM_END

#define jlexn_lb(lb)                                                        \
        ASM_BEG ASM_OP3(slt,  $t8, $t9, $t8) ASM_END                        \
        ASM_BEG ASM_OP2(beqz, $t8, lb) ASM_END

#define jgtxn_lb(lb)                                                        \
        ASM_BEG ASM_OP3(slt,  $t8, $t9, $t8) ASM_END                        \
        ASM_BEG ASM_OP2(bnez, $t8, lb) ASM_END

#define jgexn_lb(lb)                                                        \
        ASM_BEG ASM_OP3(slt,  $t8, $t8, $t9) ASM_END                        \
        ASM_BEG ASM_OP2(beqz, $t8, lb) ASM_END

#define LBL(lb)                                                             \
        ASM_BEG ASM_OP0(lb:) ASM_END

/* ver
 * set-flags: no */

#define verxx_xx() /* destroys Reax, Recx, Rebx, Redx, Resi, Redi (in x86)*/\
        movxx_mi(Mebp, inf_VER, IB(1)) /* <- SIMD to bit0, without checks */

#endif /* RT_RTARCH_M32_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

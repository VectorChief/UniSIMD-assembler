/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_P64_H
#define RT_RTARCH_P64_H

#define RT_BASE_REGS        16

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_p64.h: Implementation of PowerPC 64-bit BASE instructions.
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
 * cmdw*_** - applies [cmd] to 32-bit BASE register/memory/immediate args
 * cmdx*_** - applies [cmd] to full-size BASE register/memory/immediate args
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

#define MRM(reg, ren, rem) /* arithmetic */                                 \
        ((reg) << 21 | (ren) << 11 | (rem) << 16)

#define MSM(reg, ren, rem) /* logic, shifts */                              \
        ((reg) << 16 | (ren) << 11 | (rem) << 21)

#define MTM(reg, ren, rem) /* divide, stack */                              \
        ((reg) << 21 | (ren) << 16 | (rem) << 11)

#define MDM(reg, brm, vdp, bxx, pxx)                                        \
        (pxx(vdp) | bxx(brm) << 16 | (reg) << 21)

#define MIM(reg, ren, vim, txx, mxx)                                        \
        (mxx(vim) | txx(reg, ren))

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
#define  T3(val, tp1, tp2)  T3##tp1
#define  M3(val, tp1, tp2)  M3##tp1
#define  G3(val, tp1, tp2)  G3##tp2 /* <- "G3##tp2" not a bug */

#define  B1(val, tp1, tp2)  B1##tp1
#define  P1(val, tp1, tp2)  P1##tp1
#define  C1(val, tp1, tp2)  C1##tp1
#define  C3(val, tp1, tp2)  C3##tp2 /* <- "C3##tp2" not a bug */

/* immediate encoding add/sub/cmp(TP1), and/orr/xor(TP2), mov/mul(TP3) */

#define T10(tr, sr) ((tr) << 21 | (sr) << 16)
#define M10(im) (0x00000000 | (im))
#define G10(rg, im) EMPTY
#define T20(tr, sr) ((tr) << 16 | (sr) << 21)
#define M20(im) (0x00000000 | (im))
#define G20(rg, im) EMPTY
#define T30(tr, sr) ((tr) << 16 | (sr) << 21)
#define M30(im) (0x00000000 | (im))
#define G30(rg, im) EMITW(0x60000000 | (rg) << 16 | (0xFFFF & (im)))

#define T11(tr, sr) ((tr) << 21 | (sr) << 11)
#define M11(im) (0x00000000 | TIxx << 16)
#define G11(rg, im) G30(rg, im)
#define T31(tr, sr) ((tr) << 16 | (sr) << 21)
#define M31(im) (0x00000000 | TIxx << 11)

#define T12(tr, sr) ((tr) << 21 | (sr) << 11)
#define M12(im) (0x00000000 | TIxx << 16)
#define G12(rg, im) G32(rg, im)
#define T22(tr, sr) ((tr) << 16 | (sr) << 21)
#define M22(im) (0x00000000 | TIxx << 11)
#define G22(rg, im) G32(rg, im)
#define T32(tr, sr) ((tr) << 16 | (sr) << 21)
#define M32(im) (0x00000000 | TIxx << 11)
#define G32(rg, im) EMITW(0x64000000 | (rg) << 16 | (0xFFFF & (im) >> 16))  \
                    EMITW(0x60000000 | (rg) << 16 | (rg) << 21 |            \
                                                    (0xFFFF & (im)))

/* displacement encoding BASE(TP1), adr(TP3) */

#define B10(br) (br)
#define P10(dp) (0x00000000 | (dp))
#define C10(br, dp) EMPTY
#define C30(br, dp) EMITW(0x60000000 | TDxx << 16 | (0xFFFC & (dp)))

#define B11(br) TPxx
#define P11(dp) (0x00000000)
#define C11(br, dp) C30(br, dp)                                             \
                    EMITW(0x7C000214 | MRM(TPxx,    (br),    TDxx))
#define C31(br, dp) EMITW(0x60000000 | TDxx << 16 | (0xFFFC & (dp)))

#define B12(br) TPxx
#define P12(dp) (0x00000000)
#define C12(br, dp) C32(br, dp)                                             \
                    EMITW(0x7C000214 | MRM(TPxx,    (br),    TDxx))
#define C32(br, dp) EMITW(0x64000000 | TDxx << 16 | (0x7FFF & (dp) >> 16))  \
                    EMITW(0x60000000 | TDxx << 16 | TDxx << 21 |            \
                                                    (0xFFFC & (dp)))

/* registers    REG   (check mapping with ASM_ENTER/ASM_LEAVE in rtarch.h) */

#define Tff1    0x11  /* f17 */
#define Tff2    0x12  /* f18 */

#define TLxx    0x18  /* r24, left  arg for compare */
#define TRxx    0x19  /* r25, right arg for compare */
#define TMxx    0x18  /* r24 */
#define TIxx    0x19  /* r25, not used together with TDxx */
#define TDxx    0x19  /* r25, not used together with TIxx */
#define TPxx    0x1A  /* r26 */
#define TCxx    0x1B  /* r27 */
#define TVxx    0x1C  /* r28 */
#define TZxx    0x00  /* r0 */
#define SPxx    0x01  /* r1 */

#define Teax    0x04  /* r4, must be larger reg-num than zero (r0) */
#define Tecx    0x0F  /* r15 */
#define Tedx    0x02  /* r2 */
#define Tebx    0x03  /* r3 */
#define Tebp    0x05  /* r5 */
#define Tesi    0x06  /* r6 */
#define Tedi    0x07  /* r7 */
#define Teg8    0x08  /* r8 */
#define Teg9    0x09  /* r9 */
#define TegA    0x0A  /* r10 */
#define TegB    0x0B  /* r11 */
#define TegC    0x0C  /* r12 */
#define TegD    0x0D  /* r13 */
#define TegE    0x0E  /* r14 */

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/* registers    REG,  MOD,  SIB */

#define Reax    Teax, %%r4,  EMPTY
#define Recx    Tecx, %%r15, EMPTY
#define Redx    Tedx, %%r2,  EMPTY
#define Rebx    Tebx, %%r3,  EMPTY
#define Rebp    Tebp, %%r5,  EMPTY
#define Resi    Tesi, %%r6,  EMPTY
#define Redi    Tedi, %%r7,  EMPTY
#define Reg8    Teg8, %%r8,  EMPTY
#define Reg9    Teg9, %%r9,  EMPTY
#define RegA    TegA, %%r10, EMPTY
#define RegB    TegB, %%r11, EMPTY
#define RegC    TegC, %%r12, EMPTY
#define RegD    TegD, %%r13, EMPTY
#define RegE    TegE, %%r14, EMPTY

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

#define Iecx    Tecx, TPxx, EMITW(0x7C000214 | MRM(TPxx,    Tecx,    Teax))
#define Iedx    Tedx, TPxx, EMITW(0x7C000214 | MRM(TPxx,    Tedx,    Teax))
#define Iebx    Tebx, TPxx, EMITW(0x7C000214 | MRM(TPxx,    Tebx,    Teax))
#define Iebp    Tebp, TPxx, EMITW(0x7C000214 | MRM(TPxx,    Tebp,    Teax))
#define Iesi    Tesi, TPxx, EMITW(0x7C000214 | MRM(TPxx,    Tesi,    Teax))
#define Iedi    Tedi, TPxx, EMITW(0x7C000214 | MRM(TPxx,    Tedi,    Teax))
#define Ieg8    Teg8, TPxx, EMITW(0x7C000214 | MRM(TPxx,    Teg8,    Teax))
#define Ieg9    Teg9, TPxx, EMITW(0x7C000214 | MRM(TPxx,    Teg9,    Teax))
#define IegA    TegA, TPxx, EMITW(0x7C000214 | MRM(TPxx,    TegA,    Teax))
#define IegB    TegB, TPxx, EMITW(0x7C000214 | MRM(TPxx,    TegB,    Teax))
#define IegC    TegC, TPxx, EMITW(0x7C000214 | MRM(TPxx,    TegC,    Teax))
#define IegD    TegD, TPxx, EMITW(0x7C000214 | MRM(TPxx,    TegD,    Teax))
#define IegE    TegE, TPxx, EMITW(0x7C000214 | MRM(TPxx,    TegE,    Teax))

/* immediate    VAL,  TP1,  TP2       (all immediate types are unsigned) */

#define IC(im)  ((im) & 0x7F),       0, 0      /* drop sign-ext (in x86) */
#define IB(im)  ((im) & 0xFF),       0, 0        /* 32-bit word (in x86) */
#define IM(im)  ((im) & 0xFFF),      0, 0  /* native AArch64 add/sub/cmp */
#define IG(im)  ((im) & 0x7FFF),     0, 0  /* native on MIPS add/sub/cmp */
#define IH(im)  ((im) & 0xFFFF),     1, 0  /* second native on ARMs/MIPS */
#define IV(im)  ((im) & 0x7FFFFFFF), 2, 2        /* native x64 long mode */
#define IW(im)  ((im) & 0xFFFFFFFF), 2, 2        /* extra load op on x64 */

/* displacement VAL,  TP1,  TP2    (all displacement types are unsigned) */

#define DP(dp)  ((dp) & 0xFFC),      0, 0    /* native on all ARMs, MIPS */
#define DF(dp)  ((dp) & 0x3FFC),     0, 0   /* native AArch64 BASE ld/st */
#define DG(dp)  ((dp) & 0x7FFC),     0, 0      /* native MIPS BASE ld/st */
#define DH(dp)  ((dp) & 0xFFFC),     1, 1   /* second native on all ARMs */
#define DV(dp)  ((dp) & 0x7FFFFFFC), 2, 2        /* native x64 long mode */
#define PLAIN   DP(0)           /* special type for Oeax addressing mode */

/* triplet pass-through wrapper */

#define W(p1, p2, p3)       p1,  p2,  p3

/******************************************************************************/
/**********************************   P64   ***********************************/
/******************************************************************************/

/* mov
 * set-flags: no */

#define movwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), REG(RM), EMPTY,   EMPTY,   EMPTY2, G3(IM))

#define movwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G3(IM))   \
        EMITW(0x90000000 | MDM(TIxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define movwx_rr(RG, RM)                                                    \
        EMITW(0x7C000378 | MSM(REG(RG), REG(RM), REG(RM)))

#define movwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(REG(RG), MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define movwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x90000000 | MDM(REG(RG), MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define movxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), REG(RM), EMPTY,   EMPTY,   EMPTY2, G3(IM))

#define movxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G3(IM))   \
        EMITW(0xF8000000 | MDM(TIxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define movxx_rr(RG, RM)                                                    \
        EMITW(0x7C000378 | MSM(REG(RG), REG(RM), REG(RM)))

#define movxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(REG(RG), MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define movxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF8000000 | MDM(REG(RG), MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define adrxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C3(DP), EMPTY2)   \
        EMITW(0x7C000214 | MRM(REG(RG), MOD(RM), TDxx))

#define adrxx_lb(lb) /* load label to Reax */                               \
        label_ld(lb)

#define stack_st(RM)                                                        \
        EMITW(0x38000000 | MTM(SPxx,    SPxx,    0x00) | (-0x08 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(REG(RM), SPxx,    0x00))

#define stack_ld(RM)                                                        \
        EMITW(0xE8000000 | MTM(REG(RM), SPxx,    0x00))                     \
        EMITW(0x38000000 | MTM(SPxx,    SPxx,    0x00) | (+0x08 & 0xFFFF))

#if RT_SIMD_COMPAT_DIV != 0 || RT_SIMD_COMPAT_SQR != 0

#define stack_sa()   /* save all, [Reax - RegE] + 8 temps, 22 regs total */ \
        EMITW(0x38000000 | MTM(SPxx,    SPxx,    0x00) | (-0xB0 & 0xFFFF))  \
        EMITW(0xD8000000 | MTM(Tff1,    SPxx,    0x00) | (+0x00 & 0xFFFF))  \
        EMITW(0xD8000000 | MTM(Tff2,    SPxx,    0x00) | (+0x08 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(Teax,    SPxx,    0x00) | (+0x10 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(Tecx,    SPxx,    0x00) | (+0x18 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(Tedx,    SPxx,    0x00) | (+0x20 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(Tebx,    SPxx,    0x00) | (+0x28 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(Tebp,    SPxx,    0x00) | (+0x30 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(Tesi,    SPxx,    0x00) | (+0x38 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(Tedi,    SPxx,    0x00) | (+0x40 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(Teg8,    SPxx,    0x00) | (+0x48 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(Teg9,    SPxx,    0x00) | (+0x50 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TegA,    SPxx,    0x00) | (+0x58 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TegB,    SPxx,    0x00) | (+0x60 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TegC,    SPxx,    0x00) | (+0x68 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TegD,    SPxx,    0x00) | (+0x70 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TegE,    SPxx,    0x00) | (+0x78 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TMxx,    SPxx,    0x00) | (+0x80 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TIxx,    SPxx,    0x00) | (+0x88 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TPxx,    SPxx,    0x00) | (+0x90 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TCxx,    SPxx,    0x00) | (+0x98 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TVxx,    SPxx,    0x00) | (+0xA0 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TZxx,    SPxx,    0x00) | (+0xA8 & 0xFFFF))

#define stack_la()   /* load all, 8 temps + [RegE - Reax], 22 regs total */ \
        EMITW(0xE8000000 | MTM(TZxx,    SPxx,    0x00) | (+0xA8 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TVxx,    SPxx,    0x00) | (+0xA0 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TCxx,    SPxx,    0x00) | (+0x98 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TPxx,    SPxx,    0x00) | (+0x90 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TIxx,    SPxx,    0x00) | (+0x88 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TMxx,    SPxx,    0x00) | (+0x80 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TegE,    SPxx,    0x00) | (+0x78 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TegD,    SPxx,    0x00) | (+0x70 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TegC,    SPxx,    0x00) | (+0x68 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TegB,    SPxx,    0x00) | (+0x60 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TegA,    SPxx,    0x00) | (+0x58 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(Teg9,    SPxx,    0x00) | (+0x50 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(Teg8,    SPxx,    0x00) | (+0x48 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(Tedi,    SPxx,    0x00) | (+0x40 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(Tesi,    SPxx,    0x00) | (+0x38 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(Tebp,    SPxx,    0x00) | (+0x30 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(Tebx,    SPxx,    0x00) | (+0x28 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(Tedx,    SPxx,    0x00) | (+0x20 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(Tecx,    SPxx,    0x00) | (+0x18 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(Teax,    SPxx,    0x00) | (+0x10 & 0xFFFF))  \
        EMITW(0xC8000000 | MTM(Tff2,    SPxx,    0x00) | (+0x08 & 0xFFFF))  \
        EMITW(0xC8000000 | MTM(Tff1,    SPxx,    0x00) | (+0x00 & 0xFFFF))  \
        EMITW(0x38000000 | MTM(SPxx,    SPxx,    0x00) | (+0xB0 & 0xFFFF))

#else /* RT_SIMD_COMPAT_DIV != 0 || RT_SIMD_COMPAT_SQR != 0 */

#define stack_sa()   /* save all, [Reax - RegE] + 6 temps, 20 regs total */ \
        EMITW(0x38000000 | MTM(SPxx,    SPxx,    0x00) | (-0xA0 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(Teax,    SPxx,    0x00) | (+0x00 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(Tecx,    SPxx,    0x00) | (+0x08 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(Tedx,    SPxx,    0x00) | (+0x10 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(Tebx,    SPxx,    0x00) | (+0x18 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(Tebp,    SPxx,    0x00) | (+0x20 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(Tesi,    SPxx,    0x00) | (+0x28 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(Tedi,    SPxx,    0x00) | (+0x30 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(Teg8,    SPxx,    0x00) | (+0x38 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(Teg9,    SPxx,    0x00) | (+0x40 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TegA,    SPxx,    0x00) | (+0x48 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TegB,    SPxx,    0x00) | (+0x50 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TegC,    SPxx,    0x00) | (+0x58 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TegD,    SPxx,    0x00) | (+0x60 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TegE,    SPxx,    0x00) | (+0x68 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TMxx,    SPxx,    0x00) | (+0x70 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TIxx,    SPxx,    0x00) | (+0x78 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TPxx,    SPxx,    0x00) | (+0x80 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TCxx,    SPxx,    0x00) | (+0x88 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TVxx,    SPxx,    0x00) | (+0x90 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TZxx,    SPxx,    0x00) | (+0x98 & 0xFFFF))

#define stack_la()   /* load all, 6 temps + [RegE - Reax], 20 regs total */ \
        EMITW(0xE8000000 | MTM(TZxx,    SPxx,    0x00) | (+0x98 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TVxx,    SPxx,    0x00) | (+0x90 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TCxx,    SPxx,    0x00) | (+0x88 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TPxx,    SPxx,    0x00) | (+0x80 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TIxx,    SPxx,    0x00) | (+0x78 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TMxx,    SPxx,    0x00) | (+0x70 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TegE,    SPxx,    0x00) | (+0x68 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TegD,    SPxx,    0x00) | (+0x60 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TegC,    SPxx,    0x00) | (+0x58 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TegB,    SPxx,    0x00) | (+0x50 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TegA,    SPxx,    0x00) | (+0x48 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(Teg9,    SPxx,    0x00) | (+0x40 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(Teg8,    SPxx,    0x00) | (+0x38 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(Tedi,    SPxx,    0x00) | (+0x30 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(Tesi,    SPxx,    0x00) | (+0x28 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(Tebp,    SPxx,    0x00) | (+0x20 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(Tebx,    SPxx,    0x00) | (+0x18 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(Tedx,    SPxx,    0x00) | (+0x10 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(Tecx,    SPxx,    0x00) | (+0x08 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(Teax,    SPxx,    0x00) | (+0x00 & 0xFFFF))  \
        EMITW(0x38000000 | MTM(SPxx,    SPxx,    0x00) | (+0xA0 & 0xFFFF))

#endif /* RT_SIMD_COMPAT_DIV != 0 || RT_SIMD_COMPAT_SQR != 0 */

/* and
 * set-flags: undefined (xx), yes (zx) */

#define andwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x70000000) | (+(TP2(IM) != 0) & 0x7C000038))    \
        /* if true ^ equals to -1 (not 1) */

#define andwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x70000000) | (+(TP2(IM) != 0) & 0x7C000038))    \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define andwx_rr(RG, RM)                                                    \
        EMITW(0x7C000038 | MSM(REG(RG), REG(RG), REG(RM)))

#define andwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000038 | MSM(REG(RG), REG(RG), TMxx))

#define andwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000038 | MSM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define andxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x70000000) | (+(TP2(IM) != 0) & 0x7C000038))    \
        /* if true ^ equals to -1 (not 1) */

#define andxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x70000000) | (+(TP2(IM) != 0) & 0x7C000038))    \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define andxx_rr(RG, RM)                                                    \
        EMITW(0x7C000038 | MSM(REG(RG), REG(RG), REG(RM)))

#define andxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000038 | MSM(REG(RG), REG(RG), TMxx))

#define andxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000038 | MSM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define andzx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x70000000) | (+(TP2(IM) != 0) & 0x7C000039))    \
        /* if true ^ equals to -1 (not 1) */

#define andzx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x70000000) | (+(TP2(IM) != 0) & 0x7C000039))    \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define andzx_rr(RG, RM)                                                    \
        EMITW(0x7C000039 | MSM(REG(RG), REG(RG), REG(RM)))

#define andzx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000039 | MSM(REG(RG), REG(RG), TMxx))

#define andzx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000039 | MSM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* orr
 * set-flags: undefined */

#define orrwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x60000000) | (+(TP2(IM) != 0) & 0x7C000378))    \
        /* if true ^ equals to -1 (not 1) */

#define orrwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x60000000) | (+(TP2(IM) != 0) & 0x7C000378))    \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define orrwx_rr(RG, RM)                                                    \
        EMITW(0x7C000378 | MSM(REG(RG), REG(RG), REG(RM)))

#define orrwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000378 | MSM(REG(RG), REG(RG), TMxx))

#define orrwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000378 | MSM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define orrxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x60000000) | (+(TP2(IM) != 0) & 0x7C000378))    \
        /* if true ^ equals to -1 (not 1) */

#define orrxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x60000000) | (+(TP2(IM) != 0) & 0x7C000378))    \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define orrxx_rr(RG, RM)                                                    \
        EMITW(0x7C000378 | MSM(REG(RG), REG(RG), REG(RM)))

#define orrxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000378 | MSM(REG(RG), REG(RG), TMxx))

#define orrxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000378 | MSM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* xor
 * set-flags: undefined */

#define xorwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x68000000) | (+(TP2(IM) != 0) & 0x7C000278))    \
        /* if true ^ equals to -1 (not 1) */

#define xorwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x68000000) | (+(TP2(IM) != 0) & 0x7C000278))    \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define xorwx_rr(RG, RM)                                                    \
        EMITW(0x7C000278 | MSM(REG(RG), REG(RG), REG(RM)))

#define xorwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000278 | MSM(REG(RG), REG(RG), TMxx))

#define xorwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000278 | MSM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define xorxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x68000000) | (+(TP2(IM) != 0) & 0x7C000278))    \
        /* if true ^ equals to -1 (not 1) */

#define xorxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x68000000) | (+(TP2(IM) != 0) & 0x7C000278))    \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define xorxx_rr(RG, RM)                                                    \
        EMITW(0x7C000278 | MSM(REG(RG), REG(RG), REG(RM)))

#define xorxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000278 | MSM(REG(RG), REG(RG), TMxx))

#define xorxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000278 | MSM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* not
 * set-flags: no */

#define notwx_rr(RM)                                                        \
        EMITW(0x7C0000F8 | MSM(REG(RM), REG(RM), REG(RM)))

#define notwx_mm(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0000F8 | MSM(TMxx,    TMxx,    TMxx))                     \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define notxx_rr(RM)                                                        \
        EMITW(0x7C0000F8 | MSM(REG(RM), REG(RM), REG(RM)))

#define notxx_mm(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0000F8 | MSM(TMxx,    TMxx,    TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* neg
 * set-flags: undefined (xx), yes (zx) */

#define negwx_rr(RM)                                                        \
        EMITW(0x7C0000D0 | MRM(REG(RM), 0x00,    REG(RM)))

#define negwx_mm(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0000D0 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define negxx_rr(RM)                                                        \
        EMITW(0x7C0000D0 | MRM(REG(RM), 0x00,    REG(RM)))

#define negxx_mm(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0000D0 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define negzx_rr(RM)                                                        \
        EMITW(0x7C0000D1 | MRM(REG(RM), 0x00,    REG(RM)))

#define negzx_mm(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0000D1 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* add
 * set-flags: undefined (xx), yes (zx) */

#define addwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T1(IM), M1(IM)) | \
        (+(TP1(IM) == 0) & 0x38000000) | (+(TP1(IM) != 0) & 0x7C000214))    \
        /* if true ^ equals to -1 (not 1) */

#define addwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T1(IM), M1(IM)) | \
        (+(TP1(IM) == 0) & 0x38000000) | (+(TP1(IM) != 0) & 0x7C000214))    \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define addwx_rr(RG, RM)                                                    \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), REG(RM)))

#define addwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), TMxx))

#define addwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000214 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define addxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T1(IM), M1(IM)) | \
        (+(TP1(IM) == 0) & 0x38000000) | (+(TP1(IM) != 0) & 0x7C000214))    \
        /* if true ^ equals to -1 (not 1) */

#define addxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T1(IM), M1(IM)) | \
        (+(TP1(IM) == 0) & 0x38000000) | (+(TP1(IM) != 0) & 0x7C000214))    \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define addxx_rr(RG, RM)                                                    \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), REG(RM)))

#define addxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), TMxx))

#define addxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000214 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define addzx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T1(IM), M1(IM)) | \
        (+(TP1(IM) == 0) & 0x34000000) | (+(TP1(IM) != 0) & 0x7C000215))    \
        /* if true ^ equals to -1 (not 1) */

#define addzx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T1(IM), M1(IM)) | \
        (+(TP1(IM) == 0) & 0x34000000) | (+(TP1(IM) != 0) & 0x7C000215))    \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define addzx_rr(RG, RM)                                                    \
        EMITW(0x7C000215 | MRM(REG(RG), REG(RG), REG(RM)))

#define addzx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000215 | MRM(REG(RG), REG(RG), TMxx))

#define addzx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000215 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* sub
 * set-flags: undefined (xx), yes (zx) */

#define subwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), 0x00,    T1(IM), EMPTY1) | \
        (+(TP1(IM) == 0) & (0x38000000 | (0xFFFF & -VAL(IM)))) |            \
        (+(TP1(IM) != 0) & (0x7C000050 | TIxx << 16)))                      \
        /* if true ^ equals to -1 (not 1) */

#define subwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    0x00,    T1(IM), EMPTY1) | \
        (+(TP1(IM) == 0) & (0x38000000 | (0xFFFF & -VAL(IM)))) |            \
        (+(TP1(IM) != 0) & (0x7C000050 | TIxx << 16)))                      \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define subwx_rr(RG, RM)                                                    \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), REG(RM)))

#define subwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), TMxx))

#define subwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000050 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define subwx_mr(RM, DP, RG)                                                \
        subwx_st(W(RG), W(RM), W(DP))


#define subxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), 0x00,    T1(IM), EMPTY1) | \
        (+(TP1(IM) == 0) & (0x38000000 | (0xFFFF & -VAL(IM)))) |            \
        (+(TP1(IM) != 0) & (0x7C000050 | TIxx << 16)))                      \
        /* if true ^ equals to -1 (not 1) */

#define subxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    0x00,    T1(IM), EMPTY1) | \
        (+(TP1(IM) == 0) & (0x38000000 | (0xFFFF & -VAL(IM)))) |            \
        (+(TP1(IM) != 0) & (0x7C000050 | TIxx << 16)))                      \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define subxx_rr(RG, RM)                                                    \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), REG(RM)))

#define subxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), TMxx))

#define subxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000050 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define subxx_mr(RM, DP, RG)                                                \
        subxx_st(W(RG), W(RM), W(DP))


#define subzx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), 0x00,    T1(IM), EMPTY1) | \
        (+(TP1(IM) == 0) & (0x34000000 | (0xFFFF & -VAL(IM)))) |            \
        (+(TP1(IM) != 0) & (0x7C000051 | TIxx << 16)))                      \
        /* if true ^ equals to -1 (not 1) */

#define subzx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    0x00,    T1(IM), EMPTY1) | \
        (+(TP1(IM) == 0) & (0x34000000 | (0xFFFF & -VAL(IM)))) |            \
        (+(TP1(IM) != 0) & (0x7C000051 | TIxx << 16)))                      \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define subzx_rr(RG, RM)                                                    \
        EMITW(0x7C000051 | MRM(REG(RG), REG(RG), REG(RM)))

#define subzx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000051 | MRM(REG(RG), REG(RG), TMxx))

#define subzx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000051 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define subzx_mr(RM, DP, RG)                                                \
        subzx_st(W(RG), W(RM), W(DP))

/* shl
 * set-flags: undefined */

#define shlwx_ri(RM, IM)                                                    \
        EMITW(0x60000000 | TIxx << 16 | (0x1F & VAL(IM)))                   \
        EMITW(0x7C000030 | MSM(REG(RM), TIxx,    REG(RM)))

#define shlwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x60000000 | TIxx << 16 | (0x1F & VAL(IM)))                   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000030 | MSM(TMxx,    TIxx,    TMxx))                     \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shlwx_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000030 | MSM(REG(RG), REG(RM), REG(RG)))

#define shlwx_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000030 | MSM(REG(RG), TMxx,    REG(RG)))

#define shlwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000030 | MSM(TMxx,    REG(RG), TMxx))                     \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shlwx_mr(RM, DP, RG)                                                \
        shlwx_st(W(RG), W(RM), W(DP))

#define shlwx_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0x7C000030 | MSM(REG(RM), Tecx,    REG(RM)))

#define shlwx_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000030 | MSM(TMxx,    Tecx,    TMxx))                     \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define shlxx_ri(RM, IM)                                                    \
        EMITW(0x60000000 | TIxx << 16 | (0x3F & VAL(IM)))                   \
        EMITW(0x7C000030 | MSM(REG(RM), TIxx,    REG(RM)))

#define shlxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x60000000 | TIxx << 16 | (0x3F & VAL(IM)))                   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000030 | MSM(TMxx,    TIxx,    TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shlxx_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000030 | MSM(REG(RG), REG(RM), REG(RG)))

#define shlxx_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000030 | MSM(REG(RG), TMxx,    REG(RG)))

#define shlxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000030 | MSM(TMxx,    REG(RG), TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shlxx_mr(RM, DP, RG)                                                \
        shlxx_st(W(RG), W(RM), W(DP))

#define shlxx_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0x7C000030 | MSM(REG(RM), Tecx,    REG(RM)))

#define shlxx_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000030 | MSM(TMxx,    Tecx,    TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* shr
 * set-flags: undefined */

#define shrwx_ri(RM, IM)                                                    \
        EMITW(0x60000000 | TIxx << 16 | (0x1F & VAL(IM)))                   \
        EMITW(0x7C000430 | MSM(REG(RM), TIxx,    REG(RM)))

#define shrwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x60000000 | TIxx << 16 | (0x1F & VAL(IM)))                   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000430 | MSM(TMxx,    TIxx,    TMxx))                     \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrwx_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000430 | MSM(REG(RG), REG(RM), REG(RG)))

#define shrwx_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000430 | MSM(REG(RG), TMxx,    REG(RG)))

#define shrwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000430 | MSM(TMxx,    REG(RG), TMxx))                     \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrwx_mr(RM, DP, RG)                                                \
        shrwx_st(W(RG), W(RM), W(DP))

#define shrwx_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0x7C000430 | MSM(REG(RM), Tecx,    REG(RM)))

#define shrwx_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000430 | MSM(TMxx,    Tecx,    TMxx))                     \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define shrxx_ri(RM, IM)                                                    \
        EMITW(0x60000000 | TIxx << 16 | (0x3F & VAL(IM)))                   \
        EMITW(0x7C000430 | MSM(REG(RM), TIxx,    REG(RM)))

#define shrxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x60000000 | TIxx << 16 | (0x3F & VAL(IM)))                   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000430 | MSM(TMxx,    TIxx,    TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrxx_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000430 | MSM(REG(RG), REG(RM), REG(RG)))

#define shrxx_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000430 | MSM(REG(RG), TMxx,    REG(RG)))

#define shrxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000430 | MSM(TMxx,    REG(RG), TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrxx_mr(RM, DP, RG)                                                \
        shrxx_st(W(RG), W(RM), W(DP))

#define shrxx_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0x7C000430 | MSM(REG(RM), Tecx,    REG(RM)))

#define shrxx_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000430 | MSM(TMxx,    Tecx,    TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define shrwn_ri(RM, IM)                                                    \
        EMITW(0x7C000670 | MSM(REG(RM), (0x1F & VAL(IM)), REG(RM)))

#define shrwn_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000670 | MSM(TMxx,    (0x1F & VAL(IM)), TMxx))            \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrwn_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000630 | MSM(REG(RG), REG(RM), REG(RG)))

#define shrwn_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000630 | MSM(REG(RG), TMxx,    REG(RG)))

#define shrwn_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000630 | MSM(TMxx,    REG(RG), TMxx))                     \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrwn_mr(RM, DP, RG)                                                \
        shrwn_st(W(RG), W(RM), W(DP))

#define shrwn_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0x7C000630 | MSM(REG(RM), Tecx,    REG(RM)))

#define shrwn_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000630 | MSM(TMxx,    Tecx,    TMxx))                     \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define shrxn_ri(RM, IM)                                                    \
        EMITW(0x7C000670 | MSM(REG(RM), (0x3F & VAL(IM)), REG(RM)))

#define shrxn_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000670 | MSM(TMxx,    (0x3F & VAL(IM)), TMxx))            \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrxn_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000630 | MSM(REG(RG), REG(RM), REG(RG)))

#define shrxn_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000630 | MSM(REG(RG), TMxx,    REG(RG)))

#define shrxn_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000630 | MSM(TMxx,    REG(RG), TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrxn_mr(RM, DP, RG)                                                \
        shrxn_st(W(RG), W(RM), W(DP))

#define shrxn_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0x7C000630 | MSM(REG(RM), Tecx,    REG(RM)))

#define shrxn_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000630 | MSM(TMxx,    Tecx,    TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* mul
 * set-flags: undefined */

#define mulwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x7C0001D6 | MRM(REG(RM), REG(RM), TIxx))

#define mulwx_rr(RG, RM)                                                    \
        EMITW(0x7C0001D6 | MRM(REG(RG), REG(RG), REG(RM)))

#define mulwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0001D6 | MRM(REG(RG), REG(RG), TMxx))


#define mulxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x7C0001D2 | MRM(REG(RM), REG(RM), TIxx))

#define mulxx_rr(RG, RM)                                                    \
        EMITW(0x7C0001D2 | MRM(REG(RG), REG(RG), REG(RM)))

#define mulxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0001D2 | MRM(REG(RG), REG(RG), TMxx))


#define mulwx_xr(RM)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0x7C000016 | MRM(Tedx,    Teax,    REG(RM)))                  \
        EMITW(0x7C0001D6 | MRM(Teax,    Teax,    REG(RM)))

#define mulwx_xm(RM, DP) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000016 | MRM(Tedx,    Teax,    TMxx))                     \
        EMITW(0x7C0001D6 | MRM(Teax,    Teax,    TMxx))


#define mulxx_xr(RM)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0x7C000012 | MRM(Tedx,    Teax,    REG(RM)))                  \
        EMITW(0x7C0001D2 | MRM(Teax,    Teax,    REG(RM)))

#define mulxx_xm(RM, DP) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000012 | MRM(Tedx,    Teax,    TMxx))                     \
        EMITW(0x7C0001D2 | MRM(Teax,    Teax,    TMxx))


#define mulwn_xr(RM)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0x7C000096 | MRM(Tedx,    Teax,    REG(RM)))                  \
        EMITW(0x7C0001D6 | MRM(Teax,    Teax,    REG(RM)))

#define mulwn_xm(RM, DP) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000096 | MRM(Tedx,    Teax,    TMxx))                     \
        EMITW(0x7C0001D6 | MRM(Teax,    Teax,    TMxx))


#define mulxn_xr(RM)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0x7C000092 | MRM(Tedx,    Teax,    REG(RM)))                  \
        EMITW(0x7C0001D2 | MRM(Teax,    Teax,    REG(RM)))

#define mulxn_xm(RM, DP) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000092 | MRM(Tedx,    Teax,    TMxx))                     \
        EMITW(0x7C0001D2 | MRM(Teax,    Teax,    TMxx))

/* div
 * set-flags: undefined */

#define divwx_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x7C000396 | MTM(REG(RM), REG(RM), TIxx))

#define divwx_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        EMITW(0x7C000396 | MTM(REG(RG), REG(RG), REG(RM)))

#define divwx_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000396 | MTM(REG(RG), REG(RG), TMxx))


#define divxx_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x7C000392 | MTM(REG(RM), REG(RM), TIxx))

#define divxx_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        EMITW(0x7C000392 | MTM(REG(RG), REG(RG), REG(RM)))

#define divxx_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000392 | MTM(REG(RG), REG(RG), TMxx))


#define divwn_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x7C0003D6 | MTM(REG(RM), REG(RM), TIxx))

#define divwn_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        EMITW(0x7C0003D6 | MTM(REG(RG), REG(RG), REG(RM)))

#define divwn_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0003D6 | MTM(REG(RG), REG(RG), TMxx))


#define divxn_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x7C0003D2 | MTM(REG(RM), REG(RM), TIxx))

#define divxn_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        EMITW(0x7C0003D2 | MTM(REG(RG), REG(RG), REG(RM)))

#define divxn_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0003D2 | MTM(REG(RG), REG(RG), TMxx))


#define prewx_xx()          /* to be placed immediately prior divwx_x* */   \
                                     /* to prepare Redx for int-divide */

#define prewn_xx()          /* to be placed immediately prior divwn_x* */   \
                                     /* to prepare Redx for int-divide */


#define prexx_xx()          /* to be placed immediately prior divxx_x* */   \
        prewx_xx()                   /* to prepare Redx for int-divide */

#define prexn_xx()          /* to be placed immediately prior divxn_x* */   \
        prewn_xx()                   /* to prepare Redx for int-divide */


#define divwx_xr(RM)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(0x7C000396 | MTM(Teax,    Teax,    REG(RM)))

#define divwx_xm(RM, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000396 | MTM(Teax,    Teax,    TMxx))


#define divxx_xr(RM)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(0x7C000392 | MTM(Teax,    Teax,    REG(RM)))

#define divxx_xm(RM, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000392 | MTM(Teax,    Teax,    TMxx))


#define divwn_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(0x7C0003D6 | MTM(Teax,    Teax,    REG(RM)))

#define divwn_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0003D6 | MTM(Teax,    Teax,    TMxx))


#define divxn_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(0x7C0003D2 | MTM(Teax,    Teax,    REG(RM)))

#define divxn_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0003D2 | MTM(Teax,    Teax,    TMxx))

/* rem
 * set-flags: undefined */

#define remwx_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movwx_rr(Redx, W(RM))                                               \
        divwx_ri(W(RM), W(IM))                                              \
        EMITW(0x7C0001D6 | MRM(TMxx,    REG(RM), TIxx))                     \
        EMITW(0x7C000050 | MRM(REG(RM), Tedx,    TMxx))                     \
        stack_ld(Redx)

#define remwx_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        stack_st(Redx)                                                      \
        movwx_rr(Redx, W(RG))                                               \
        divwx_rr(W(RG), W(RM))                                              \
        EMITW(0x7C0001D6 | MRM(TMxx,    REG(RG), REG(RM)))                  \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)

#define remwx_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movwx_rr(Redx, W(RG))                                               \
        divwx_ld(W(RG), W(RM), W(DP))                                       \
        EMITW(0x7C0001D6 | MRM(TMxx,    REG(RG), TMxx))                     \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)


#define remxx_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movxx_rr(Redx, W(RM))                                               \
        divxx_ri(W(RM), W(IM))                                              \
        EMITW(0x7C0001D2 | MRM(TMxx,    REG(RM), TIxx))                     \
        EMITW(0x7C000050 | MRM(REG(RM), Tedx,    TMxx))                     \
        stack_ld(Redx)

#define remxx_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        stack_st(Redx)                                                      \
        movxx_rr(Redx, W(RG))                                               \
        divxx_rr(W(RG), W(RM))                                              \
        EMITW(0x7C0001D2 | MRM(TMxx,    REG(RG), REG(RM)))                  \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)

#define remxx_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movxx_rr(Redx, W(RG))                                               \
        divxx_ld(W(RG), W(RM), W(DP))                                       \
        EMITW(0x7C0001D2 | MRM(TMxx,    REG(RG), TMxx))                     \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)


#define remwn_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movwx_rr(Redx, W(RM))                                               \
        divwn_ri(W(RM), W(IM))                                              \
        EMITW(0x7C0001D6 | MRM(TMxx,    REG(RM), TIxx))                     \
        EMITW(0x7C000050 | MRM(REG(RM), Tedx,    TMxx))                     \
        stack_ld(Redx)

#define remwn_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        stack_st(Redx)                                                      \
        movwx_rr(Redx, W(RG))                                               \
        divwn_rr(W(RG), W(RM))                                              \
        EMITW(0x7C0001D6 | MRM(TMxx,    REG(RG), REG(RM)))                  \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)

#define remwn_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movwx_rr(Redx, W(RG))                                               \
        divwn_ld(W(RG), W(RM), W(DP))                                       \
        EMITW(0x7C0001D6 | MRM(TMxx,    REG(RG), TMxx))                     \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)


#define remxn_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movxx_rr(Redx, W(RM))                                               \
        divxn_ri(W(RM), W(IM))                                              \
        EMITW(0x7C0001D2 | MRM(TMxx,    REG(RM), TIxx))                     \
        EMITW(0x7C000050 | MRM(REG(RM), Tedx,    TMxx))                     \
        stack_ld(Redx)

#define remxn_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        stack_st(Redx)                                                      \
        movxx_rr(Redx, W(RG))                                               \
        divxn_rr(W(RG), W(RM))                                              \
        EMITW(0x7C0001D2 | MRM(TMxx,    REG(RG), REG(RM)))                  \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)

#define remxn_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movxx_rr(Redx, W(RG))                                               \
        divxn_ld(W(RG), W(RM), W(DP))                                       \
        EMITW(0x7C0001D2 | MRM(TMxx,    REG(RG), TMxx))                     \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)


#define remwx_xx()          /* to be placed immediately prior divwx_x* */   \
        movwx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remwx_xr(RM)        /* to be placed immediately after divwx_xr */   \
        EMITW(0x7C0001D6 | MRM(TMxx,    Teax,    REG(RM)))                  \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */

#define remwx_xm(RM, DP)    /* to be placed immediately after divwx_xm */   \
        EMITW(0x7C0001D6 | MRM(TMxx,    Teax,    TMxx))                     \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */


#define remxx_xx()          /* to be placed immediately prior divxx_x* */   \
        movxx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remxx_xr(RM)        /* to be placed immediately after divxx_xr */   \
        EMITW(0x7C0001D2 | MRM(TMxx,    Teax,    REG(RM)))                  \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */

#define remxx_xm(RM, DP)    /* to be placed immediately after divxx_xm */   \
        EMITW(0x7C0001D2 | MRM(TMxx,    Teax,    TMxx))                     \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */


#define remwn_xx()          /* to be placed immediately prior divwn_x* */   \
        movwx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remwn_xr(RM)        /* to be placed immediately after divwn_xr */   \
        EMITW(0x7C0001D6 | MRM(TMxx,    Teax,    REG(RM)))                  \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */

#define remwn_xm(RM, DP)    /* to be placed immediately after divwn_xm */   \
        EMITW(0x7C0001D6 | MRM(TMxx,    Teax,    TMxx))                     \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */


#define remxn_xx()          /* to be placed immediately prior divxn_x* */   \
        movxx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remxn_xr(RM)        /* to be placed immediately after divxn_xr */   \
        EMITW(0x7C0001D2 | MRM(TMxx,    Teax,    REG(RM)))                  \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */

#define remxn_xm(RM, DP)    /* to be placed immediately after divxn_xm */   \
        EMITW(0x7C0001D2 | MRM(TMxx,    Teax,    TMxx))                     \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */

/* cmj
 * set-flags: undefined */

#define EQ_x    A0
#define NE_x    A1

#define LT_x    A2
#define LE_x    A3
#define GT_x    A4
#define GE_x    A5

#define LT_n    A6
#define LE_n    A7
#define GT_n    A8
#define GE_n    A9


#define cmjwx_rz(RM, CC, lb)                                                \
        cmjwx_ri(W(RM), IC(0), CC, lb)

#define cmjwx_mz(RM, DP, CC, lb)                                            \
        cmjwx_mi(W(RM), W(DP), IC(0), CC, lb)

#define cmjwx_ri(RM, IM, CC, lb)                                            \
        CWI(CC, MOD(RM), REG(RM), W(IM), lb)

#define cmjwx_mi(RM, DP, IM, CC, lb)                                        \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        CWI(CC, %%r24,   TMxx,    W(IM), lb)

#define cmjwx_rr(RG, RM, CC, lb)                                            \
        CWR(CC, MOD(RG), MOD(RM), lb)

#define cmjwx_rm(RG, RM, DP, CC, lb)                                        \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        CWR(CC, MOD(RG), %%r24,   lb)

#define cmjwx_mr(RM, DP, RG, CC, lb)                                        \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        CWR(CC, %%r24,   MOD(RG), lb)


#define cmjxx_rz(RM, CC, lb)                                                \
        cmjxx_ri(W(RM), IC(0), CC, lb)

#define cmjxx_mz(RM, DP, CC, lb)                                            \
        cmjxx_mi(W(RM), W(DP), IC(0), CC, lb)

#define cmjxx_ri(RM, IM, CC, lb)                                            \
        CXI(CC, MOD(RM), REG(RM), W(IM), lb)

#define cmjxx_mi(RM, DP, IM, CC, lb)                                        \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        CXI(CC, %%r24,   TMxx,    W(IM), lb)

#define cmjxx_rr(RG, RM, CC, lb)                                            \
        CXR(CC, MOD(RG), MOD(RM), lb)

#define cmjxx_rm(RG, RM, DP, CC, lb)                                        \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        CXR(CC, MOD(RG), %%r24,   lb)

#define cmjxx_mr(RM, DP, RG, CC, lb)                                        \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        CXR(CC, %%r24,   MOD(RG), lb)

/* cmp
 * set-flags: yes */

#define cmpwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x7C0007B4 | MSM(TLxx,    0x00,    REG(RM)))                  \
        EMITW(0x7C0007B4 | MSM(TRxx,    0x00,    TRxx))

#define cmpwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TRxx,    MOD(RM), VAL(DP), C1(DP), G3(IM))   \
        EMITW(0xE8000002 | MDM(TLxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0007B4 | MSM(TRxx,    0x00,    TRxx))

#define cmpwx_rr(RG, RM)                                                    \
        EMITW(0x7C0007B4 | MSM(TRxx,    0x00,    REG(RM)))                  \
        EMITW(0x7C0007B4 | MSM(TLxx,    0x00,    REG(RG)))

#define cmpwx_rm(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000002 | MDM(TRxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0007B4 | MSM(TLxx,    0x00,    REG(RG)))

#define cmpwx_mr(RM, DP, RG)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000002 | MDM(TLxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0007B4 | MSM(TRxx,    0x00,    REG(RG)))


#define cmpxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x7C000378 | MSM(TLxx,    REG(RM), REG(RM)))

#define cmpxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TRxx,    MOD(RM), VAL(DP), C1(DP), G3(IM))   \
        EMITW(0xE8000000 | MDM(TLxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define cmpxx_rr(RG, RM)                                                    \
        EMITW(0x7C000378 | MSM(TRxx,    REG(RM), REG(RM)))                  \
        EMITW(0x7C000378 | MSM(TLxx,    REG(RG), REG(RG)))

#define cmpxx_rm(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TRxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000378 | MSM(TLxx,    REG(RG), REG(RG)))

#define cmpxx_mr(RM, DP, RG)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TLxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000378 | MSM(TRxx,    REG(RG), REG(RG)))

/* jmp
 * set-flags: no
 * maximum byte-address-range for un/conditional jumps is signed 18/16-bit
 * based on minimum natively-encoded offset across supported targets (u/c)
 * MIPS:18-bit, Power:26-bit, AArch32:26-bit, AArch64:28-bit, x86:32-bit /
 * MIPS:18-bit, Power:16-bit, AArch32:26-bit, AArch64:21-bit, x86:32-bit */

#define jmpxx_rr(RM)           /* register-targeted unconditional jump */   \
        EMITW(0x7C0003A6 | MRM(REG(RM), 0x00,    0x09)) /* ctr <- reg */    \
        EMITW(0x4C000420 | MTM(0x0C,    0x0A,    0x00)) /* beqctr cr2 */

#define jmpxx_mm(RM, DP)         /* memory-targeted unconditional jump */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0003A6 | MRM(TMxx,    0x00,    0x09)) /* ctr <- r24 */    \
        EMITW(0x4C000420 | MTM(0x0C,    0x0A,    0x00)) /* beqctr cr2 */

#define jmpxx_lb(lb)              /* label-targeted unconditional jump */   \
        ASM_BEG ASM_OP1(b, lb) ASM_END

#define jezxx_lb(lb)               /* setting-flags-arithmetic -> jump */   \
        ASM_BEG ASM_OP1(beq,   lb) ASM_END

#define jnzxx_lb(lb)               /* setting-flags-arithmetic -> jump */   \
        ASM_BEG ASM_OP1(bne,   lb) ASM_END

#define jeqxx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmpld, %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(beq,   lb) ASM_END

#define jnexx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmpld, %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(bne,   lb) ASM_END

#define jltxx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmpld, %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(blt,   lb) ASM_END

#define jlexx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmpld, %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(ble,   lb) ASM_END

#define jgtxx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmpld, %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(bgt,   lb) ASM_END

#define jgexx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmpld, %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(bge,   lb) ASM_END

#define jltxn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmpd,  %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(blt,   lb) ASM_END

#define jlexn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmpd,  %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(ble,   lb) ASM_END

#define jgtxn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmpd,  %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(bgt,   lb) ASM_END

#define jgexn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmpd,  %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(bge,   lb) ASM_END

#define LBL(lb)                                          /* code label */   \
        ASM_BEG ASM_OP0(lb:) ASM_END

/* internal definitions for combined-compare-jump (cmj) */

#define IWA0(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x28000000) | (+(TP2(IM) != 0) & 0x7C000040))    \
        ASM_BEG ASM_OP1(beq,   lb) ASM_END

#define IWA1(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x28000000) | (+(TP2(IM) != 0) & 0x7C000040))    \
        ASM_BEG ASM_OP1(bne,   lb) ASM_END

#define IWA2(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x28000000) | (+(TP2(IM) != 0) & 0x7C000040))    \
        ASM_BEG ASM_OP1(blt,   lb) ASM_END

#define IWA3(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x28000000) | (+(TP2(IM) != 0) & 0x7C000040))    \
        ASM_BEG ASM_OP1(ble,   lb) ASM_END

#define IWA4(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x28000000) | (+(TP2(IM) != 0) & 0x7C000040))    \
        ASM_BEG ASM_OP1(bgt,   lb) ASM_END

#define IWA5(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x28000000) | (+(TP2(IM) != 0) & 0x7C000040))    \
        ASM_BEG ASM_OP1(bge,   lb) ASM_END

#define IWA6(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T3(IM), M3(IM)) | \
        (+(TP2(IM) == 0) & 0x2C000000) | (+(TP2(IM) != 0) & 0x7C000000))    \
        ASM_BEG ASM_OP1(blt,   lb) ASM_END

#define IWA7(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T3(IM), M3(IM)) | \
        (+(TP2(IM) == 0) & 0x2C000000) | (+(TP2(IM) != 0) & 0x7C000000))    \
        ASM_BEG ASM_OP1(ble,   lb) ASM_END

#define IWA8(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T3(IM), M3(IM)) | \
        (+(TP2(IM) == 0) & 0x2C000000) | (+(TP2(IM) != 0) & 0x7C000000))    \
        ASM_BEG ASM_OP1(bgt,   lb) ASM_END

#define IWA9(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T3(IM), M3(IM)) | \
        (+(TP2(IM) == 0) & 0x2C000000) | (+(TP2(IM) != 0) & 0x7C000000))    \
        ASM_BEG ASM_OP1(bge,   lb) ASM_END

#define CWI(CC, r1, p1, IM, lb)                                             \
        IW##CC(r1, p1, W(IM), lb)


#define RWA0(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmplw, r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(beq,   lb) ASM_END

#define RWA1(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmplw, r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(bne,   lb) ASM_END

#define RWA2(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmplw, r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(blt,   lb) ASM_END

#define RWA3(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmplw, r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(ble,   lb) ASM_END

#define RWA4(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmplw, r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(bgt,   lb) ASM_END

#define RWA5(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmplw, r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(bge,   lb) ASM_END

#define RWA6(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmpw,  r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(blt,   lb) ASM_END

#define RWA7(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmpw,  r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(ble,   lb) ASM_END

#define RWA8(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmpw,  r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(bgt,   lb) ASM_END

#define RWA9(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmpw,  r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(bge,   lb) ASM_END

#define CWR(CC, r1, r2, lb)                                                 \
        RW##CC(r1, r2, lb)


#define IXA0(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x28200000) | (+(TP2(IM) != 0) & 0x7C200040))    \
        ASM_BEG ASM_OP1(beq,   lb) ASM_END

#define IXA1(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x28200000) | (+(TP2(IM) != 0) & 0x7C200040))    \
        ASM_BEG ASM_OP1(bne,   lb) ASM_END

#define IXA2(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x28200000) | (+(TP2(IM) != 0) & 0x7C200040))    \
        ASM_BEG ASM_OP1(blt,   lb) ASM_END

#define IXA3(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x28200000) | (+(TP2(IM) != 0) & 0x7C200040))    \
        ASM_BEG ASM_OP1(ble,   lb) ASM_END

#define IXA4(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x28200000) | (+(TP2(IM) != 0) & 0x7C200040))    \
        ASM_BEG ASM_OP1(bgt,   lb) ASM_END

#define IXA5(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x28200000) | (+(TP2(IM) != 0) & 0x7C200040))    \
        ASM_BEG ASM_OP1(bge,   lb) ASM_END

#define IXA6(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T3(IM), M3(IM)) | \
        (+(TP2(IM) == 0) & 0x2C200000) | (+(TP2(IM) != 0) & 0x7C200000))    \
        ASM_BEG ASM_OP1(blt,   lb) ASM_END

#define IXA7(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T3(IM), M3(IM)) | \
        (+(TP2(IM) == 0) & 0x2C200000) | (+(TP2(IM) != 0) & 0x7C200000))    \
        ASM_BEG ASM_OP1(ble,   lb) ASM_END

#define IXA8(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T3(IM), M3(IM)) | \
        (+(TP2(IM) == 0) & 0x2C200000) | (+(TP2(IM) != 0) & 0x7C200000))    \
        ASM_BEG ASM_OP1(bgt,   lb) ASM_END

#define IXA9(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T3(IM), M3(IM)) | \
        (+(TP2(IM) == 0) & 0x2C200000) | (+(TP2(IM) != 0) & 0x7C200000))    \
        ASM_BEG ASM_OP1(bge,   lb) ASM_END

#define CXI(CC, r1, p1, IM, lb)                                             \
        IX##CC(r1, p1, W(IM), lb)


#define RXA0(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmpld, r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(beq,   lb) ASM_END

#define RXA1(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmpld, r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(bne,   lb) ASM_END

#define RXA2(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmpld, r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(blt,   lb) ASM_END

#define RXA3(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmpld, r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(ble,   lb) ASM_END

#define RXA4(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmpld, r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(bgt,   lb) ASM_END

#define RXA5(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmpld, r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(bge,   lb) ASM_END

#define RXA6(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmpd,  r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(blt,   lb) ASM_END

#define RXA7(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmpd,  r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(ble,   lb) ASM_END

#define RXA8(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmpd,  r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(bgt,   lb) ASM_END

#define RXA9(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmpd,  r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(bge,   lb) ASM_END

#define CXR(CC, r1, r2, lb)                                                 \
        RX##CC(r1, r2, lb)

/* ver
 * set-flags: no */

#define verxx_xx() /* destroys Reax, Recx, Rebx, Redx, Resi, Redi (in x86)*/\
        movxx_mi(Mebp, inf_VER, IB(3)) /* <- VMX, VSX to bit0, bit1 */

#endif /* RT_RTARCH_P64_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

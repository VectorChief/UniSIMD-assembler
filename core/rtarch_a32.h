/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_A32_H
#define RT_RTARCH_A32_H

#define RT_BASE_REGS        16

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_a32.h: Implementation of AArch64:ILP32 BASE instructions.
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

#define MRM(reg, ren, rem)                                                  \
        ((rem) << 16 | (ren) << 5 | (reg))

#define MDM(reg, brm, vdp, bxx, pxx)                                        \
        (pxx(vdp) | bxx(brm) << 5 | (reg))

#define MIM(reg, ren, vim, txx, mxx)                                        \
        (mxx(vim) | (ren) << 5 | txx(reg))

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

#define TNxx    0x16  /* w22 */
#define TMxx    0x1A  /* w26 */
#define TIxx    0x1B  /* w27, not used together with TDxx */
#define TDxx    0x1B  /* w27, not used together with TIxx */
#define TPxx    0x1C  /* w28 */
#define TZxx    0x1F  /* w31 */
#define SPxx    0x1F  /* w31 */

#define Teax    0x00  /* w0 */
#define Tecx    0x01  /* w1 */
#define Tedx    0x02  /* w2 */
#define Tebx    0x03  /* w3 */
#define Tebp    0x05  /* w5 */
#define Tesi    0x06  /* w6 */
#define Tedi    0x07  /* w7 */
#define Teg8    0x08  /* w8 */
#define Teg9    0x09  /* w9 */
#define TegA    0x0A  /* w10 */
#define TegB    0x0B  /* w11 */
#define TegC    0x0C  /* w12 */
#define TegD    0x0D  /* w13 */
#define TegE    0x0E  /* w14 */

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

#define Iecx    Tecx, TPxx, EMITW(0x0B000000 | MRM(TPxx,    Tecx,    Teax))
#define Iedx    Tedx, TPxx, EMITW(0x0B000000 | MRM(TPxx,    Tedx,    Teax))
#define Iebx    Tebx, TPxx, EMITW(0x0B000000 | MRM(TPxx,    Tebx,    Teax))
#define Iebp    Tebp, TPxx, EMITW(0x0B000000 | MRM(TPxx,    Tebp,    Teax))
#define Iesi    Tesi, TPxx, EMITW(0x0B000000 | MRM(TPxx,    Tesi,    Teax))
#define Iedi    Tedi, TPxx, EMITW(0x0B000000 | MRM(TPxx,    Tedi,    Teax))
#define Ieg8    Teg8, TPxx, EMITW(0x0B000000 | MRM(TPxx,    Teg8,    Teax))
#define Ieg9    Teg9, TPxx, EMITW(0x0B000000 | MRM(TPxx,    Teg9,    Teax))
#define IegA    TegA, TPxx, EMITW(0x0B000000 | MRM(TPxx,    TegA,    Teax))
#define IegB    TegB, TPxx, EMITW(0x0B000000 | MRM(TPxx,    TegB,    Teax))
#define IegC    TegC, TPxx, EMITW(0x0B000000 | MRM(TPxx,    TegC,    Teax))
#define IegD    TegD, TPxx, EMITW(0x0B000000 | MRM(TPxx,    TegD,    Teax))
#define IegE    TegE, TPxx, EMITW(0x0B000000 | MRM(TPxx,    TegE,    Teax))

/* immediate    VAL,  TP1,  TP2 */

#define IC(im)  ((im) & 0x7F),       0, 1      /* drop sign-ext (in x86) */
#define IB(im)  ((im) & 0xFF),       0, 1        /* 32-bit word (in x86) */
#define IM(im)  ((im) & 0xFFF),      0, 1  /* native AArch64 add/sub/cmp */
#define IG(im)  ((im) & 0x7FFF),     1, 1  /* native on MIPS add/sub/cmp */
#define IH(im)  ((im) & 0xFFFF),     1, 1  /* second native on ARMs/MIPS */
#define IV(im)  ((im) & 0x7FFFFFFF), 2, 2        /* native x64 long mode */
#define IW(im)  ((im) & 0xFFFFFFFF), 2, 2        /* extra load op on x64 */

/* displacement VAL,  TP1,  TP2 */

#define DP(dp)  ((dp) & 0xFFC),      0, 0    /* native on all ARMs, MIPS */
#define DF(dp)  ((dp) & 0x3FFC),     0, 0   /* native AArch64 BASE ld/st */
#define DG(dp)  ((dp) & 0x7FFC),     1, 0      /* native MIPS BASE ld/st */
#define DH(dp)  ((dp) & 0xFFFC),     1, 0   /* second native on all ARMs */
#define DV(dp)  ((dp) & 0x7FFFFFFC), 2, 2        /* native x64 long mode */
#define PLAIN   DP(0)           /* special type for Oeax addressing mode */

/* immediate encoding add/sub/cmp(TP1), and/orr/xor(TP2), mov/mul(TP3) */

#define T10(tr) (tr)
#define M10(im) (0x10000000 | (im) << 10)
#define G10(rg, im) EMPTY

#define T11(tr) (tr)
#define M11(im) (0x0A000000 | TIxx << 16)
#define G11(rg, im) G31(rg, im)
#define T21(tr) (tr)
#define M21(im) (0x0A000000 | TIxx << 16)
#define G21(rg, im) G31(rg, im)
#define G31(rg, im) EMITW(0x52800000 | MRM((rg),    0x00,    0x00) |        \
                             (0xFFFF & (im)) << 5)

#define T12(tr) (tr)
#define M12(im) (0x0A000000 | TIxx << 16)
#define G12(rg, im) G32(rg, im)
#define T22(tr) (tr)
#define M22(im) (0x0A000000 | TIxx << 16)
#define G22(rg, im) G32(rg, im)
#define G32(rg, im) EMITW(0x52800000 | MRM((rg),    0x00,    0x00) |        \
                             (0xFFFF & (im)) << 5)                          \
                    EMITW(0x72A00000 | MRM((rg),    0x00,    0x00) |        \
                             (0xFFFF & (im) >> 16) << 5)

/* displacement encoding BASE(TP1), adr(TP3) */

#define B10(br) (br)
#define P10(dp) (0x00000000 | (dp) << 8)
#define C10(br, dp) EMPTY
#define C30(br, dp) EMITW(0x52800000 | MRM(TDxx,    0x00,    0x00) |        \
                             (0xFFFC & (dp)) << 5)

#define B11(br) TPxx
#define P11(dp) (0x00000000)
#define C11(br, dp) C30(br, dp)                                             \
                    EMITW(0x0B000000 | MRM(TPxx,    (br),    TDxx))

#define B12(br) TPxx
#define P12(dp) (0x00000000)
#define C12(br, dp) C32(br, dp)                                             \
                    EMITW(0x0B000000 | MRM(TPxx,    (br),    TDxx))
#define C32(br, dp) EMITW(0x52800000 | MRM(TDxx,    0x00,    0x00) |        \
                             (0xFFFC & (dp)) << 5)                          \
                    EMITW(0x72A00000 | MRM(TDxx,    0x00,    0x00) |        \
                             (0x7FFF & (dp) >> 16) << 5)

/* triplet pass-through wrapper */

#define W(p1, p2, p3)       p1,  p2,  p3

/******************************************************************************/
/**********************************   A32   ***********************************/
/******************************************************************************/

/* mov
 * set-flags: no */

#define movxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), REG(RM), EMPTY,   EMPTY,   EMPTY2, G3(IM))

#define movxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G3(IM))   \
        EMITW(0xB9000000 | MDM(TIxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define movxx_rr(RG, RM)                                                    \
        EMITW(0x2A000000 | MRM(REG(RG), TZxx,    REG(RM)))

#define movxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(REG(RG), MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define movxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9000000 | MDM(REG(RG), MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define adrxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C3(DP), EMPTY2)   \
        EMITW(0x0B000000 | MRM(REG(RG), MOD(RM), TDxx))

#define adrxx_lb(lb) /* load label to Reax */                               \
        label_ld(lb)

#define stack_st(RM)                                                        \
        EMITW(0xF81F8C00 | MRM(REG(RM), SPxx,    0x00))

#define stack_ld(RM)                                                        \
        EMITW(0xF8408400 | MRM(REG(RM), SPxx,    0x00))

#if RT_SIMD_FAST_FCTRL == 0

#define stack_sa()   /* save all, [Reax - RegE] + 4 temps, 18 regs total */ \
        EMITW(0xA9BF0000 | MRM(Teax,    SPxx,    0x00) | Tecx << 10)        \
        EMITW(0xA9BF0000 | MRM(Tedx,    SPxx,    0x00) | Tebx << 10)        \
        EMITW(0xA9BF0000 | MRM(Tebp,    SPxx,    0x00) | Tesi << 10)        \
        EMITW(0xA9BF0000 | MRM(Tedi,    SPxx,    0x00) | Teg8 << 10)        \
        EMITW(0xA9BF0000 | MRM(Teg9,    SPxx,    0x00) | TegA << 10)        \
        EMITW(0xA9BF0000 | MRM(TegB,    SPxx,    0x00) | TegC << 10)        \
        EMITW(0xA9BF0000 | MRM(TegD,    SPxx,    0x00) | TegE << 10)        \
        EMITW(0xA9BF0000 | MRM(TMxx,    SPxx,    0x00) | TIxx << 10)        \
        EMITW(0xA9BF0000 | MRM(TPxx,    SPxx,    0x00) | TNxx << 10)

#define stack_la()   /* load all, 4 temps + [RegE - Reax], 18 regs total */ \
        EMITW(0xA8C10000 | MRM(TPxx,    SPxx,    0x00) | TNxx << 10)        \
        EMITW(0xA8C10000 | MRM(TMxx,    SPxx,    0x00) | TIxx << 10)        \
        EMITW(0xA8C10000 | MRM(TegD,    SPxx,    0x00) | TegE << 10)        \
        EMITW(0xA8C10000 | MRM(TegB,    SPxx,    0x00) | TegC << 10)        \
        EMITW(0xA8C10000 | MRM(Teg9,    SPxx,    0x00) | TegA << 10)        \
        EMITW(0xA8C10000 | MRM(Tedi,    SPxx,    0x00) | Teg8 << 10)        \
        EMITW(0xA8C10000 | MRM(Tebp,    SPxx,    0x00) | Tesi << 10)        \
        EMITW(0xA8C10000 | MRM(Tedx,    SPxx,    0x00) | Tebx << 10)        \
        EMITW(0xA8C10000 | MRM(Teax,    SPxx,    0x00) | Tecx << 10)

#else /* RT_SIMD_FAST_FCTRL */

#define stack_sa()   /* save all, [Reax - RegE] + 7 temps, 21 regs total */ \
        EMITW(0xA9BF0000 | MRM(Teax,    SPxx,    0x00) | Tecx << 10)        \
        EMITW(0xA9BF0000 | MRM(Tedx,    SPxx,    0x00) | Tebx << 10)        \
        EMITW(0xA9BF0000 | MRM(Tebp,    SPxx,    0x00) | Tesi << 10)        \
        EMITW(0xA9BF0000 | MRM(Tedi,    SPxx,    0x00) | Teg8 << 10)        \
        EMITW(0xA9BF0000 | MRM(Teg9,    SPxx,    0x00) | TegA << 10)        \
        EMITW(0xA9BF0000 | MRM(TegB,    SPxx,    0x00) | TegC << 10)        \
        EMITW(0xA9BF0000 | MRM(TegD,    SPxx,    0x00) | TegE << 10)        \
        EMITW(0xA9BF0000 | MRM(TMxx,    SPxx,    0x00) | TIxx << 10)        \
        EMITW(0xA9BF0000 | MRM(TPxx,    SPxx,    0x00) | TNxx << 10)        \
        EMITW(0xA9BF0000 | MRM(TNxx+1,  SPxx,    0x00) |(TNxx+2) << 10)     \
        EMITW(0xF81F8C00 | MRM(TNxx+3,  SPxx,    0x00))

#define stack_la()   /* load all, 7 temps + [RegE - Reax], 21 regs total */ \
        EMITW(0xF8408400 | MRM(TNxx+3,  SPxx,    0x00))                     \
        EMITW(0xA8C10000 | MRM(TNxx+1,  SPxx,    0x00) |(TNxx+2) << 10)     \
        EMITW(0xA8C10000 | MRM(TPxx,    SPxx,    0x00) | TNxx << 10)        \
        EMITW(0xA8C10000 | MRM(TMxx,    SPxx,    0x00) | TIxx << 10)        \
        EMITW(0xA8C10000 | MRM(TegD,    SPxx,    0x00) | TegE << 10)        \
        EMITW(0xA8C10000 | MRM(TegB,    SPxx,    0x00) | TegC << 10)        \
        EMITW(0xA8C10000 | MRM(Teg9,    SPxx,    0x00) | TegA << 10)        \
        EMITW(0xA8C10000 | MRM(Tedi,    SPxx,    0x00) | Teg8 << 10)        \
        EMITW(0xA8C10000 | MRM(Tebp,    SPxx,    0x00) | Tesi << 10)        \
        EMITW(0xA8C10000 | MRM(Tedx,    SPxx,    0x00) | Tebx << 10)        \
        EMITW(0xA8C10000 | MRM(Teax,    SPxx,    0x00) | Tecx << 10)

#endif /* RT_SIMD_FAST_FCTRL */

/* and
 * set-flags: yes (z-version only) */

#define andxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)))

#define andxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)))  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define andxx_rr(RG, RM)                                                    \
        EMITW(0x0A000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define andxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x0A000000 | MRM(REG(RG), REG(RG), TMxx))

#define andxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x0A000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define andzx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x60000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)))

#define andzx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x60000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)))  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define andzx_rr(RG, RM)                                                    \
        EMITW(0x6A000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define andzx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x6A000000 | MRM(REG(RG), REG(RG), TMxx))

#define andzx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x6A000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* orr
 * set-flags: no */

#define orrxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x20000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)))

#define orrxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x20000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)))  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define orrxx_rr(RG, RM)                                                    \
        EMITW(0x2A000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define orrxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x2A000000 | MRM(REG(RG), REG(RG), TMxx))

#define orrxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x2A000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* xor
 * set-flags: no */

#define xorxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x40000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)))

#define xorxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x40000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)))  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define xorxx_rr(RG, RM)                                                    \
        EMITW(0x4A000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define xorxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x4A000000 | MRM(REG(RG), REG(RG), TMxx))

#define xorxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x4A000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* not
 * set-flags: no */

#define notxx_rr(RM)                                                        \
        EMITW(0x2A200000 | MRM(REG(RM), TZxx,    REG(RM)))

#define notxx_mm(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x2A200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* neg
 * set-flags: yes (z-version only) */

#define negxx_rr(RM)                                                        \
        EMITW(0x4B000000 | MRM(REG(RM), TZxx,    REG(RM)))

#define negxx_mm(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x4B000000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define negzx_rr(RM)                                                        \
        EMITW(0x6B000000 | MRM(REG(RM), TZxx,    REG(RM)))

#define negzx_mm(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x6B000000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* add
 * set-flags: yes (z-version only) */

#define addxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x01000000 | MIM(REG(RM), REG(RM), VAL(IM), T1(IM), M1(IM)))

#define addxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x01000000 | MIM(TMxx,    TMxx,    VAL(IM), T1(IM), M1(IM)))  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define addxx_rr(RG, RM)                                                    \
        EMITW(0x0B000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define addxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x0B000000 | MRM(REG(RG), REG(RG), TMxx))

#define addxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x0B000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define addzx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x21000000 | MIM(REG(RM), REG(RM), VAL(IM), T1(IM), M1(IM)))

#define addzx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x21000000 | MIM(TMxx,    TMxx,    VAL(IM), T1(IM), M1(IM)))  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define addzx_rr(RG, RM)                                                    \
        EMITW(0x2B000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define addzx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x2B000000 | MRM(REG(RG), REG(RG), TMxx))

#define addzx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x2B000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* sub
 * set-flags: yes (z-version only) */

#define subxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x41000000 | MIM(REG(RM), REG(RM), VAL(IM), T1(IM), M1(IM)))

#define subxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x41000000 | MIM(TMxx,    TMxx,    VAL(IM), T1(IM), M1(IM)))  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define subxx_rr(RG, RM)                                                    \
        EMITW(0x4B000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define subxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x4B000000 | MRM(REG(RG), REG(RG), TMxx))

#define subxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x4B000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define subxx_mr(RM, DP, RG)                                                \
        subxx_st(W(RG), W(RM), W(DP))

#define subzx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x61000000 | MIM(REG(RM), REG(RM), VAL(IM), T1(IM), M1(IM)))

#define subzx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x61000000 | MIM(TMxx,    TMxx,    VAL(IM), T1(IM), M1(IM)))  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define subzx_rr(RG, RM)                                                    \
        EMITW(0x6B000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define subzx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x6B000000 | MRM(REG(RG), REG(RG), TMxx))

#define subzx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x6B000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define subzx_mr(RM, DP, RG)                                                \
        subzx_st(W(RG), W(RM), W(DP))

/* shl
 * set-flags: no */

#define shlxx_ri(RM, IM)                                                    \
        EMITW(0x53000000 | MRM(REG(RM), REG(RM), 0x00) |                    \
        (-VAL(IM)&0x1F)<<16 | (31-(VAL(IM)&0x1F))<<10)

#define shlxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x53000000 | MRM(TMxx,    TMxx,    0x00) |                    \
        (-VAL(IM)&0x1F)<<16 | (31-(VAL(IM)&0x1F))<<10)                      \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shlxx_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0x1AC02000 | MRM(REG(RM), REG(RM), Tecx))

#define shlxx_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x1AC02000 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* shr
 * set-flags: no */

#define shrxx_ri(RM, IM)                                                    \
        EMITW(0x53007C00 | MRM(REG(RM), REG(RM), 0x00) |(VAL(IM)&0x1F)<<16)

#define shrxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x53007C00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IM)&0x1F)<<16) \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrxx_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0x1AC02400 | MRM(REG(RM), REG(RM), Tecx))

#define shrxx_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x1AC02400 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrxn_ri(RM, IM)                                                    \
        EMITW(0x13007C00 | MRM(REG(RM), REG(RM), 0x00) |(VAL(IM)&0x1F)<<16)

#define shrxn_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x13007C00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IM)&0x1F)<<16) \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrxn_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0x1AC02800 | MRM(REG(RM), REG(RM), Tecx))

#define shrxn_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x1AC02800 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* mul
 * set-flags: no */

#define mulxx_ri(RM, IM)                 /* part-range 32-bit multiply */   \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x1B007C00 | MRM(REG(RM), REG(RM), TIxx))

#define mulxx_rr(RG, RM)                 /* part-range 32-bit multiply */   \
        EMITW(0x1B007C00 | MRM(REG(RG), REG(RG), REG(RM)))

#define mulxx_ld(RG, RM, DP)             /* part-range 32-bit multiply */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x1B007C00 | MRM(REG(RG), REG(RG), TMxx))

#define mulxn_ri(RM, IM)                 /* part-range 32-bit multiply */   \
        mulxx_ri(W(RM), W(IM))

#define mulxn_rr(RG, RM)                 /* part-range 32-bit multiply */   \
        mulxx_rr(W(RG), W(RM))

#define mulxn_ld(RG, RM, DP)             /* part-range 32-bit multiply */   \
        mulxx_ld(W(RG), W(RM), W(DP))

#define mulxx_xr(RM)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0x9BA07C00 | MRM(Teax,    Teax,    REG(RM)))                  \
        EMITW(0xD360FC00 | MRM(Tedx,    Teax,    0x00))

#define mulxx_xm(RM, DP) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9BA07C00 | MRM(Teax,    Teax,    TMxx))                     \
        EMITW(0xD360FC00 | MRM(Tedx,    Teax,    0x00))

#define mulxn_xr(RM)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0x9B207C00 | MRM(Teax,    Teax,    REG(RM)))                  \
        EMITW(0xD360FC00 | MRM(Tedx,    Teax,    0x00))

#define mulxn_xm(RM, DP) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9B207C00 | MRM(Teax,    Teax,    TMxx))                     \
        EMITW(0xD360FC00 | MRM(Tedx,    Teax,    0x00))

#define mulxp_xr(RM)     /* Reax is in/out, prepares Redx for divxn/xp */   \
                                         /* part-range 32-bit multiply */   \
        EMITW(0x1B007C00 | MRM(Teax,    Teax,    REG(RM)))

#define mulxp_xm(RM, DP) /* Reax is in/out, prepares Redx for divxn/xp */   \
                                         /* part-range 32-bit multiply */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x1B007C00 | MRM(Teax,    Teax,    TMxx))

/* div
 * set-flags: no */

#define divxx_xr(RM)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
                                     /* destroys Redx, Xmm0 (in ARMv7) */   \
        EMITW(0x1AC00800 | MRM(Teax,    Teax,    REG(RM)))                  \
                                     /* 32-bit int (fp64 div in ARMv7) */

#define divxx_xm(RM, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
                                     /* destroys Redx, Xmm0 (in ARMv7) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x1AC00800 | MRM(Teax,    Teax,    TMxx))                     \
                                     /* 32-bit int (fp64 div in ARMv7) */

#define divxn_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
                                     /* destroys Redx, Xmm0 (in ARMv7) */   \
        EMITW(0x1AC00C00 | MRM(Teax,    Teax,    REG(RM)))                  \
                                     /* 32-bit int (fp64 div in ARMv7) */

#define divxn_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
                                     /* destroys Redx, Xmm0 (in ARMv7) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x1AC00C00 | MRM(Teax,    Teax,    TMxx))                     \
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
        movxx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remxx_xr(RM)        /* to be placed immediately after divxx_xr */   \
        EMITW(0x1B008000 | MRM(Tedx,    Teax,    REG(RM)) | Tedx << 10)     \
                                                          /* Redx<-rem */

#define remxx_xm(RM, DP)    /* to be placed immediately after divxx_xm */   \
        EMITW(0x1B008000 | MRM(Tedx,    Teax,    TMxx) | Tedx << 10)        \
                                                          /* Redx<-rem */

#define remxn_xx()          /* to be placed immediately prior divxn_x* */   \
        movxx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remxn_xr(RM)        /* to be placed immediately after divxn_xr */   \
        EMITW(0x1B008000 | MRM(Tedx,    Teax,    REG(RM)) | Tedx << 10)     \
                                                          /* Redx<-rem */

#define remxn_xm(RM, DP)    /* to be placed immediately after divxn_xm */   \
        EMITW(0x1B008000 | MRM(Tedx,    Teax,    TMxx) | Tedx << 10)        \
                                                          /* Redx<-rem */

/* cmj
 * set-flags: no */

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

#define cmjxx_ri(RM, IM, CC, lb)                                            \
        cmpxx_ri(W(RM), W(IM))                                              \
        CMJ(CC, lb)

#define cmjxx_mi(RM, DP, IM, CC, lb)                                        \
        cmpxx_mi(W(RM), W(DP), W(IM))                                       \
        CMJ(CC, lb)

#define cmjxx_rr(RG, RM, CC, lb)                                            \
        cmpxx_rr(W(RG), W(RM))                                              \
        CMJ(CC, lb)

#define cmjxx_rm(RG, RM, DP, CC, lb)                                        \
        cmpxx_rm(W(RG), W(RM), W(DP))                                       \
        CMJ(CC, lb)

#define cmjxx_mr(RM, DP, RG, CC, lb)                                        \
        cmpxx_mr(W(RM), W(DP), W(RG))                                       \
        CMJ(CC, lb)

#define CMJ(CC, lb) /* internal macro for combined-compare-jump (cmj) */    \
        CC(lb)

/* cmp
 * set-flags: yes */

#define cmpxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x61000000 | MIM(TZxx,    REG(RM), VAL(IM), T1(IM), M1(IM)))

#define cmpxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x61000000 | MIM(TZxx,    TMxx,    VAL(IM), T1(IM), M1(IM)))

#define cmpxx_rr(RG, RM)                                                    \
        EMITW(0x6B000000 | MRM(TZxx,    REG(RG), REG(RM)))

#define cmpxx_rm(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x6B000000 | MRM(TZxx,    REG(RG), TMxx))

#define cmpxx_mr(RM, DP, RG)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x6B000000 | MRM(TZxx,    TMxx,    REG(RG)))

/* jmp
 * set-flags: no */

#define jmpxx_mm(RM, DP)         /* memory-targeted unconditional jump */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xD61F0000 | MRM(0x00,    TMxx,    0x00))

#define jmpxx_lb(lb)              /* label-targeted unconditional jump */   \
        ASM_BEG ASM_OP1(b,    lb) ASM_END

#define jezxx_lb(lb)               /* setting-flags-arithmetic -> jump */   \
        ASM_BEG ASM_OP1(b.eq, lb) ASM_END

#define jnzxx_lb(lb)               /* setting-flags-arithmetic -> jump */   \
        ASM_BEG ASM_OP1(b.ne, lb) ASM_END

#define jeqxx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(b.eq, lb) ASM_END

#define jnexx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(b.ne, lb) ASM_END

#define jltxx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(b.lo, lb) ASM_END

#define jlexx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(b.ls, lb) ASM_END

#define jgtxx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(b.hi, lb) ASM_END

#define jgexx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(b.hs, lb) ASM_END

#define jltxn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(b.lt, lb) ASM_END

#define jlexn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(b.le, lb) ASM_END

#define jgtxn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(b.gt, lb) ASM_END

#define jgexn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(b.ge, lb) ASM_END

#define LBL(lb)                                          /* code label */   \
        ASM_BEG ASM_OP0(lb:) ASM_END

/* ver
 * set-flags: no */

#define verxx_xx() /* destroys Reax, Recx, Rebx, Redx, Resi, Redi (in x86)*/\
        movxx_mi(Mebp, inf_VER, IB(1)) /* <- NEON to bit0, without checks */

#endif /* RT_RTARCH_A32_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

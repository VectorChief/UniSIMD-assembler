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
 * cmdx*_** - applies [cmd] to A-size BASE register/memory/immediate args
 * cmd*x_** - applies [cmd] to unsigned integer args, [x] - default
 * cmd*n_** - applies [cmd] to   signed integer args, [n] - negatable
 * cmd*p_** - applies [cmd] to   signed integer args, [p] - part-range
 *
 * cmd*z_** - applies [cmd] while setting condition flags, [z] - zero flag.
 * Regular cmd*x_** instructions may or may not set flags depending
 * on the target architecture, thus no assumptions can be made for jezxx/jnzxx.
 *
 * The cmdw*_** and cmdx*_** subsets are not easily compatible on all targets,
 * thus any register affected by cmdw*_** cannot be used in cmdx*_** subset.
 * Alternatively, data flow must not exceed 31-bit range for 32-bit operations
 * to produce consistent results usable in 64-bit subset across all targets.
 *
 * Only a64 and x64 have a complete 32-bit support in 64-bit mode both zeroing
 * the upper half of the result, while m64 sign-extending all 32-bit operations
 * and p64 overflowing 32-bit arithmetic into the upper half. Similar reasons
 * of inconsistency prohibit use of IW immediate type within 64-bit subset,
 * where a64 and p64 zero-extend, while x64 and m64 sign-extend 32-bit value.
 *
 * Note that offset correction for endianness E is only applicable for addresses
 * within pointer fields, when (in-heap) address and pointer sizes don't match.
 * Working with 32-bit data in 64-bit fields in any other circumstances must be
 * done consistently within a subset of one size (cmdw*_**, cmdx*_** or C/C++).
 * Alternatively, data written natively in C/C++ can be worked on from within
 * a given (one) subset if appropriate offset correction is used from rtarch.h.
 * Mixing of cmdw*_** and cmdx*_** without C/C++ is supported via F definition.
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

/* registers    REG   (check mapping with ASM_ENTER/ASM_LEAVE in rtarch.h) */

#define TNxx    0x14  /* s4 (r20), default FCTRL round mode */
#define TAxx    0x15  /* s5 (r21), extra reg for FAST_FCTRL */
#define TCxx    0x16  /* s6 (r22), extra reg for FAST_FCTRL */
#define TExx    0x17  /* s7 (r23), extra reg for FAST_FCTRL */

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

#define Reax    Teax, $a0,  EMPTY
#define Recx    Tecx, $t7,  EMPTY
#define Redx    Tedx, $v0,  EMPTY
#define Rebx    Tebx, $v1,  EMPTY
#define Rebp    Tebp, $a1,  EMPTY
#define Resi    Tesi, $a2,  EMPTY
#define Redi    Tedi, $a3,  EMPTY
#define Reg8    Teg8, $t0,  EMPTY
#define Reg9    Teg9, $t1,  EMPTY
#define RegA    TegA, $t2,  EMPTY
#define RegB    TegB, $t3,  EMPTY
#define RegC    TegC, $t4,  EMPTY
#define RegD    TegD, $t5,  EMPTY
#define RegE    TegE, $t6,  EMPTY

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

/* immediate    VAL,  TP1,  TP2       (all immediate types are unsigned) */

#define IC(im)  ((im) & 0x7F),       0, 0      /* drop sign-ext (in x86) */
#define IB(im)  ((im) & 0xFF),       0, 0        /* 32-bit word (in x86) */
#define IM(im)  ((im) & 0xFFF),      0, 0  /* native AArch64 add/sub/cmp */
#define IG(im)  ((im) & 0x7FFF),     0, 0  /* native on MIPS add/sub/cmp */
#define IH(im)  ((im) & 0xFFFF),     1, 0  /* second native on ARMs/MIPS */
#define IV(im)  ((im) & 0x7FFFFFFF), 2, 2        /* native x64 long mode */
#define IW(im)  ((im) & 0xFFFFFFFF), 2, 2       /* only for cmdw*_** set */

/* displacement VAL,  TP1,  TP2    (all displacement types are unsigned) */

#define DP(dp)  ((dp) & 0xFFC),      0, 0    /* native on all ARMs, MIPS */
#define DF(dp)  ((dp) & 0x3FFC),     0, 1   /* native AArch64 BASE ld/st */
#define DG(dp)  ((dp) & 0x7FFC),     0, 1      /* native MIPS BASE ld/st */
#define DH(dp)  ((dp) & 0xFFFC),     1, 1   /* second native on all ARMs */
#define DV(dp)  ((dp) & 0x7FFFFFFC), 2, 2        /* native x64 long mode */
#define PLAIN   DP(0)           /* special type for Oeax addressing mode */

/* triplet pass-through wrapper */

#define W(p1, p2, p3)       p1,  p2,  p3

/******************************************************************************/
/**********************************   M32   ***********************************/
/******************************************************************************/

/* mov
 * set-flags: no */

#define movwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), REG(RM), EMPTY,   EMPTY,   EMPTY2, G3(IM))

#define movwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G3(IM))   \
        EMITW(0xAC000000 | MDM(TIxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define movwx_rr(RG, RM)                                                    \
        EMITW(0x00000025 | MRM(REG(RG), REG(RM), TZxx))

#define movwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(REG(RG), MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define movwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xAC000000 | MDM(REG(RG), MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define movxx_ri(RM, IM)                                                    \
        movwx_ri(W(RM), W(IM))

#define movxx_mi(RM, DP, IM)                                                \
        movwx_mi(W(RM), W(DP), W(IM))

#define movxx_rr(RG, RM)                                                    \
        movwx_rr(W(RG), W(RM))

#define movxx_ld(RG, RM, DP)                                                \
        movwx_ld(W(RG), W(RM), W(DP))

#define movxx_st(RG, RM, DP)                                                \
        movwx_st(W(RG), W(RM), W(DP))


#define adrxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C3(DP), EMPTY2)   \
        EMITW(0x00000021 | MRM(REG(RG), MOD(RM), TDxx))

#define adrxx_lb(lb) /* load label to Reax */                               \
        label_ld(lb)

#if defined (RT_M32)

#define stack_st(RM)                                                        \
        EMITW(0x24000000 | MRM(0x00,    SPxx,    SPxx) | (-0x08 & 0xFFFF))  \
        EMITW(0xAC000000 | MRM(0x00,    SPxx,    REG(RM)))

#define stack_ld(RM)                                                        \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,    REG(RM)))                  \
        EMITW(0x24000000 | MRM(0x00,    SPxx,    SPxx) | (+0x08 & 0xFFFF))

#if RT_SIMD_FAST_FCTRL == 0

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

#else /* RT_SIMD_FAST_FCTRL */

#define stack_sa()   /* save all, [Reax - RegE] + 7 temps, 21 regs total */ \
        EMITW(0x24000000 | MRM(0x00,    SPxx,    SPxx) | (-0x58 & 0xFFFF))  \
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
        EMITW(0xAC000000 | MRM(0x00,    SPxx,    TNxx) | (+0x44 & 0xFFFF))  \
        EMITW(0xAC000000 | MRM(0x00,    SPxx,  1+TNxx) | (+0x48 & 0xFFFF))  \
        EMITW(0xAC000000 | MRM(0x00,    SPxx,  2+TNxx) | (+0x4C & 0xFFFF))  \
        EMITW(0xAC000000 | MRM(0x00,    SPxx,  3+TNxx) | (+0x50 & 0xFFFF))

#define stack_la()   /* load all, 7 temps + [RegE - Reax], 21 regs total */ \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,  3+TNxx) | (+0x50 & 0xFFFF))  \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,  2+TNxx) | (+0x4C & 0xFFFF))  \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,  1+TNxx) | (+0x48 & 0xFFFF))  \
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
        EMITW(0x24000000 | MRM(0x00,    SPxx,    SPxx) | (+0x58 & 0xFFFF))

#endif /* RT_SIMD_FAST_FCTRL */

#elif defined (RT_M64)

#define stack_st(RM)                                                        \
        EMITW(0x64000000 | MRM(0x00,    SPxx,    SPxx) | (-0x08 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    REG(RM)))

#define stack_ld(RM)                                                        \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    REG(RM)))                  \
        EMITW(0x64000000 | MRM(0x00,    SPxx,    SPxx) | (+0x08 & 0xFFFF))

#if RT_SIMD_FAST_FCTRL == 0

#define stack_sa()   /* save all, [Reax - RegE] + 4 temps, 18 regs total */ \
        EMITW(0x64000000 | MRM(0x00,    SPxx,    SPxx) | (-0x90 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    Teax) | (+0x00 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    Tecx) | (+0x08 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    Tedx) | (+0x10 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    Tebx) | (+0x18 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    Tebp) | (+0x20 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    Tesi) | (+0x28 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    Tedi) | (+0x30 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    Teg8) | (+0x38 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    Teg9) | (+0x40 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    TegA) | (+0x48 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    TegB) | (+0x50 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    TegC) | (+0x58 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    TegD) | (+0x60 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    TegE) | (+0x68 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    TMxx) | (+0x70 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    TIxx) | (+0x78 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    TPxx) | (+0x80 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    TNxx) | (+0x88 & 0xFFFF))

#define stack_la()   /* load all, 4 temps + [RegE - Reax], 18 regs total */ \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    TNxx) | (+0x88 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    TPxx) | (+0x80 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    TIxx) | (+0x78 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    TMxx) | (+0x70 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    TegE) | (+0x68 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    TegD) | (+0x60 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    TegC) | (+0x58 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    TegB) | (+0x50 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    TegA) | (+0x48 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    Teg9) | (+0x40 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    Teg8) | (+0x38 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    Tedi) | (+0x30 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    Tesi) | (+0x28 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    Tebp) | (+0x20 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    Tebx) | (+0x18 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    Tedx) | (+0x10 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    Tecx) | (+0x08 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    Teax) | (+0x00 & 0xFFFF))  \
        EMITW(0x64000000 | MRM(0x00,    SPxx,    SPxx) | (+0x90 & 0xFFFF))

#else /* RT_SIMD_FAST_FCTRL */

#define stack_sa()   /* save all, [Reax - RegE] + 7 temps, 21 regs total */ \
        EMITW(0x64000000 | MRM(0x00,    SPxx,    SPxx) | (-0xA8 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    Teax) | (+0x00 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    Tecx) | (+0x08 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    Tedx) | (+0x10 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    Tebx) | (+0x18 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    Tebp) | (+0x20 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    Tesi) | (+0x28 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    Tedi) | (+0x30 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    Teg8) | (+0x38 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    Teg9) | (+0x40 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    TegA) | (+0x48 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    TegB) | (+0x50 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    TegC) | (+0x58 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    TegD) | (+0x60 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    TegE) | (+0x68 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    TMxx) | (+0x70 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    TIxx) | (+0x78 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    TPxx) | (+0x80 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,    TNxx) | (+0x88 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,  1+TNxx) | (+0x90 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,  2+TNxx) | (+0x98 & 0xFFFF))  \
        EMITW(0xFC000000 | MRM(0x00,    SPxx,  3+TNxx) | (+0xA0 & 0xFFFF))

#define stack_la()   /* load all, 7 temps + [RegE - Reax], 21 regs total */ \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,  3+TNxx) | (+0xA0 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,  2+TNxx) | (+0x98 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,  1+TNxx) | (+0x90 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    TNxx) | (+0x88 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    TPxx) | (+0x80 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    TIxx) | (+0x78 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    TMxx) | (+0x70 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    TegE) | (+0x68 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    TegD) | (+0x60 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    TegC) | (+0x58 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    TegB) | (+0x50 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    TegA) | (+0x48 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    Teg9) | (+0x40 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    Teg8) | (+0x38 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    Tedi) | (+0x30 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    Tesi) | (+0x28 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    Tebp) | (+0x20 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    Tebx) | (+0x18 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    Tedx) | (+0x10 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    Tecx) | (+0x08 & 0xFFFF))  \
        EMITW(0xDC000000 | MRM(0x00,    SPxx,    Teax) | (+0x00 & 0xFFFF))  \
        EMITW(0x64000000 | MRM(0x00,    SPxx,    SPxx) | (+0xA8 & 0xFFFF))

#endif /* RT_SIMD_FAST_FCTRL */

#endif /* defined (RT_M32, RT_M64) */

/* and
 * set-flags: undefined (*x), yes (*z) */

#define andwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x30000000) | (+(TP2(IM) != 0) & 0x00000024))    \
        /* if true ^ equals to -1 (not 1) */

#define andwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x30000000) | (+(TP2(IM) != 0) & 0x00000024))    \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define andwx_rr(RG, RM)                                                    \
        EMITW(0x00000024 | MRM(REG(RG), REG(RG), REG(RM)))

#define andwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000024 | MRM(REG(RG), REG(RG), TMxx))

#define andwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000024 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define andxx_ri(RM, IM)                                                    \
        andwx_ri(W(RM), W(IM))

#define andxx_mi(RM, DP, IM)                                                \
        andwx_mi(W(RM), W(DP), W(IM))

#define andxx_rr(RG, RM)                                                    \
        andwx_rr(W(RG), W(RM))

#define andxx_ld(RG, RM, DP)                                                \
        andwx_ld(W(RG), W(RM), W(DP))

#define andxx_st(RG, RM, DP)                                                \
        andwx_st(W(RG), W(RM), W(DP))


#define andwz_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x30000000) | (+(TP2(IM) != 0) & 0x00000024))    \
        /* if true ^ equals to -1 (not 1) */                                \
        EMITW(0x00000025 | MRM(TLxx,    REG(RM), TZxx))/* <- set flags (Z) */

#define andwz_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x30000000) | (+(TP2(IM) != 0) & 0x00000024))    \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define andwz_rr(RG, RM)                                                    \
        EMITW(0x00000024 | MRM(REG(RG), REG(RG), REG(RM)))                  \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define andwz_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000024 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define andwz_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000024 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define andxz_ri(RM, IM)                                                    \
        andwz_ri(W(RM), W(IM))

#define andxz_mi(RM, DP, IM)                                                \
        andwz_mi(W(RM), W(DP), W(IM))

#define andxz_rr(RG, RM)                                                    \
        andwz_rr(W(RG), W(RM))

#define andxz_ld(RG, RM, DP)                                                \
        andwz_ld(W(RG), W(RM), W(DP))

#define andxz_st(RG, RM, DP)                                                \
        andwz_st(W(RG), W(RM), W(DP))

/* orr
 * set-flags: undefined */

#define orrwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x34000000) | (+(TP2(IM) != 0) & 0x00000025))    \
        /* if true ^ equals to -1 (not 1) */

#define orrwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x34000000) | (+(TP2(IM) != 0) & 0x00000025))    \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define orrwx_rr(RG, RM)                                                    \
        EMITW(0x00000025 | MRM(REG(RG), REG(RG), REG(RM)))

#define orrwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000025 | MRM(REG(RG), REG(RG), TMxx))

#define orrwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000025 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define orrxx_ri(RM, IM)                                                    \
        orrwx_ri(W(RM), W(IM))

#define orrxx_mi(RM, DP, IM)                                                \
        orrwx_mi(W(RM), W(DP), W(IM))

#define orrxx_rr(RG, RM)                                                    \
        orrwx_rr(W(RG), W(RM))

#define orrxx_ld(RG, RM, DP)                                                \
        orrwx_ld(W(RG), W(RM), W(DP))

#define orrxx_st(RG, RM, DP)                                                \
        orrwx_st(W(RG), W(RM), W(DP))

/* xor
 * set-flags: undefined */

#define xorwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x38000000) | (+(TP2(IM) != 0) & 0x00000026))    \
        /* if true ^ equals to -1 (not 1) */

#define xorwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x38000000) | (+(TP2(IM) != 0) & 0x00000026))    \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define xorwx_rr(RG, RM)                                                    \
        EMITW(0x00000026 | MRM(REG(RG), REG(RG), REG(RM)))

#define xorwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000026 | MRM(REG(RG), REG(RG), TMxx))

#define xorwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000026 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define xorxx_ri(RM, IM)                                                    \
        xorwx_ri(W(RM), W(IM))

#define xorxx_mi(RM, DP, IM)                                                \
        xorwx_mi(W(RM), W(DP), W(IM))

#define xorxx_rr(RG, RM)                                                    \
        xorwx_rr(W(RG), W(RM))

#define xorxx_ld(RG, RM, DP)                                                \
        xorwx_ld(W(RG), W(RM), W(DP))

#define xorxx_st(RG, RM, DP)                                                \
        xorwx_st(W(RG), W(RM), W(DP))

/* not
 * set-flags: no */

#define notwx_rr(RM)                                                        \
        EMITW(0x00000027 | MRM(REG(RM), TZxx,    REG(RM)))

#define notwx_mm(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000027 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define notxx_rr(RM)                                                        \
        notwx_rr(W(RM))

#define notxx_mm(RM, DP)                                                    \
        notwx_mm(W(RM), W(DP))

/* neg
 * set-flags: undefined (*x), yes (*z) */

#define negwx_rr(RM)                                                        \
        EMITW(0x00000023 | MRM(REG(RM), TZxx,    REG(RM)))

#define negwx_mm(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000023 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define negxx_rr(RM)                                                        \
        negwx_rr(W(RM))

#define negxx_mm(RM, DP)                                                    \
        negwx_mm(W(RM), W(DP))


#define negwz_rr(RM)                                                        \
        EMITW(0x00000023 | MRM(REG(RM), TZxx,    REG(RM)))                  \
        EMITW(0x00000025 | MRM(TLxx,    REG(RM), TZxx))/* <- set flags (Z) */

#define negwz_mm(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000023 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define negxz_rr(RM)                                                        \
        negwz_rr(W(RM))

#define negxz_mm(RM, DP)                                                    \
        negwz_mm(W(RM), W(DP))

/* add
 * set-flags: undefined (*x), yes (*z) */

#define addwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T1(IM), M1(IM)) | \
        (+(TP1(IM) == 0) & 0x24000000) | (+(TP1(IM) != 0) & 0x00000021))    \
        /* if true ^ equals to -1 (not 1) */

#define addwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T1(IM), M1(IM)) | \
        (+(TP1(IM) == 0) & 0x24000000) | (+(TP1(IM) != 0) & 0x00000021))    \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define addwx_rr(RG, RM)                                                    \
        EMITW(0x00000021 | MRM(REG(RG), REG(RG), REG(RM)))

#define addwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000021 | MRM(REG(RG), REG(RG), TMxx))

#define addwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000021 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define addxx_ri(RM, IM)                                                    \
        addwx_ri(W(RM), W(IM))

#define addxx_mi(RM, DP, IM)                                                \
        addwx_mi(W(RM), W(DP), W(IM))

#define addxx_rr(RG, RM)                                                    \
        addwx_rr(W(RG), W(RM))

#define addxx_ld(RG, RM, DP)                                                \
        addwx_ld(W(RG), W(RM), W(DP))

#define addxx_st(RG, RM, DP)                                                \
        addwx_st(W(RG), W(RM), W(DP))


#define addwz_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T1(IM), M1(IM)) | \
        (+(TP1(IM) == 0) & 0x24000000) | (+(TP1(IM) != 0) & 0x00000021))    \
        /* if true ^ equals to -1 (not 1) */                                \
        EMITW(0x00000025 | MRM(TLxx,    REG(RM), TZxx))/* <- set flags (Z) */

#define addwz_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T1(IM), M1(IM)) | \
        (+(TP1(IM) == 0) & 0x24000000) | (+(TP1(IM) != 0) & 0x00000021))    \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define addwz_rr(RG, RM)                                                    \
        EMITW(0x00000021 | MRM(REG(RG), REG(RG), REG(RM)))                  \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define addwz_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000021 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define addwz_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000021 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define addxz_ri(RM, IM)                                                    \
        addwz_ri(W(RM), W(IM))

#define addxz_mi(RM, DP, IM)                                                \
        addwz_mi(W(RM), W(DP), W(IM))

#define addxz_rr(RG, RM)                                                    \
        addwz_rr(W(RG), W(RM))

#define addxz_ld(RG, RM, DP)                                                \
        addwz_ld(W(RG), W(RM), W(DP))

#define addxz_st(RG, RM, DP)                                                \
        addwz_st(W(RG), W(RM), W(DP))

/* sub
 * set-flags: undefined (*x), yes (*z) */

#define subwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), 0x00,    T1(IM), EMPTY1) | \
        (+(TP1(IM) == 0) & (0x24000000 | (0xFFFF & -VAL(IM)))) |            \
        (+(TP1(IM) != 0) & (0x00000023 | TIxx << 16)))                      \
        /* if true ^ equals to -1 (not 1) */

#define subwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    0x00,    T1(IM), EMPTY1) | \
        (+(TP1(IM) == 0) & (0x24000000 | (0xFFFF & -VAL(IM)))) |            \
        (+(TP1(IM) != 0) & (0x00000023 | TIxx << 16)))                      \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define subwx_rr(RG, RM)                                                    \
        EMITW(0x00000023 | MRM(REG(RG), REG(RG), REG(RM)))

#define subwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000023 | MRM(REG(RG), REG(RG), TMxx))

#define subwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000023 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define subwx_mr(RM, DP, RG)                                                \
        subwx_st(W(RG), W(RM), W(DP))


#define subxx_ri(RM, IM)                                                    \
        subwx_ri(W(RM), W(IM))

#define subxx_mi(RM, DP, IM)                                                \
        subwx_mi(W(RM), W(DP), W(IM))

#define subxx_rr(RG, RM)                                                    \
        subwx_rr(W(RG), W(RM))

#define subxx_ld(RG, RM, DP)                                                \
        subwx_ld(W(RG), W(RM), W(DP))

#define subxx_st(RG, RM, DP)                                                \
        subwx_st(W(RG), W(RM), W(DP))

#define subxx_mr(RM, DP, RG)                                                \
        subxx_st(W(RG), W(RM), W(DP))


#define subwz_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), 0x00,    T1(IM), EMPTY1) | \
        (+(TP1(IM) == 0) & (0x24000000 | (0xFFFF & -VAL(IM)))) |            \
        (+(TP1(IM) != 0) & (0x00000023 | TIxx << 16)))                      \
        /* if true ^ equals to -1 (not 1) */                                \
        EMITW(0x00000025 | MRM(TLxx,    REG(RM), TZxx))/* <- set flags (Z) */

#define subwz_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    0x00,    T1(IM), EMPTY1) | \
        (+(TP1(IM) == 0) & (0x24000000 | (0xFFFF & -VAL(IM)))) |            \
        (+(TP1(IM) != 0) & (0x00000023 | TIxx << 16)))                      \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define subwz_rr(RG, RM)                                                    \
        EMITW(0x00000023 | MRM(REG(RG), REG(RG), REG(RM)))                  \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define subwz_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000023 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define subwz_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000023 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define subwz_mr(RM, DP, RG)                                                \
        subwz_st(W(RG), W(RM), W(DP))


#define subxz_ri(RM, IM)                                                    \
        subwz_ri(W(RM), W(IM))

#define subxz_mi(RM, DP, IM)                                                \
        subwz_mi(W(RM), W(DP), W(IM))

#define subxz_rr(RG, RM)                                                    \
        subwz_rr(W(RG), W(RM))

#define subxz_ld(RG, RM, DP)                                                \
        subwz_ld(W(RG), W(RM), W(DP))

#define subxz_st(RG, RM, DP)                                                \
        subwz_st(W(RG), W(RM), W(DP))

#define subxz_mr(RM, DP, RG)                                                \
        subxz_st(W(RG), W(RM), W(DP))

/* shl
 * set-flags: undefined */

#define shlwx_ri(RM, IM)                                                    \
        EMITW(0x00000000 | MRM(REG(RM), 0x00,    REG(RM)) |                 \
                                                 (0x1F & VAL(IM)) << 6)

#define shlwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IM)) << 6)     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shlwx_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        EMITW(0x00000004 | MRM(REG(RG), REG(RM), REG(RG)))

#define shlwx_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000004 | MRM(REG(RG), TMxx,    REG(RG)))

#define shlwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000004 | MRM(TMxx,    REG(RG), TMxx))                     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shlwx_mr(RM, DP, RG)                                                \
        shlwx_st(W(RG), W(RM), W(DP))

#define shlwx_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0x00000004 | MRM(REG(RM), Tecx,    REG(RM)))

#define shlwx_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000004 | MRM(TMxx,    Tecx,    TMxx))                     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define shlxx_ri(RM, IM)                                                    \
        shlwx_ri(W(RM), W(IM))

#define shlxx_mi(RM, DP, IM)                                                \
        shlwx_mi(W(RM), W(DP), W(IM))

#define shlxx_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        shlwx_rr(W(RG), W(RM))

#define shlxx_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        shlwx_ld(W(RG), W(RM), W(DP))

#define shlxx_st(RG, RM, DP)                                                \
        shlwx_st(W(RG), W(RM), W(DP))

#define shlxx_mr(RM, DP, RG)                                                \
        shlxx_st(W(RG), W(RM), W(DP))

#define shlxx_rx(RM)                     /* reads Recx for shift value */   \
        shlwx_rx(W(RM))

#define shlxx_mx(RM, DP)                 /* reads Recx for shift value */   \
        shlwx_mx(W(RM), W(DP))

/* shr
 * set-flags: undefined */

#define shrwx_ri(RM, IM)                                                    \
        EMITW(0x00000002 | MRM(REG(RM), 0x00,    REG(RM)) |                 \
                                                 (0x1F & VAL(IM)) << 6)

#define shrwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000002 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IM)) << 6)     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrwx_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        EMITW(0x00000006 | MRM(REG(RG), REG(RM), REG(RG)))

#define shrwx_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000006 | MRM(REG(RG), TMxx,    REG(RG)))

#define shrwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000006 | MRM(TMxx,    REG(RG), TMxx))                     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrwx_mr(RM, DP, RG)                                                \
        shrwx_st(W(RG), W(RM), W(DP))

#define shrwx_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0x00000006 | MRM(REG(RM), Tecx,    REG(RM)))

#define shrwx_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000006 | MRM(TMxx,    Tecx,    TMxx))                     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define shrxx_ri(RM, IM)                                                    \
        shrwx_ri(W(RM), W(IM))

#define shrxx_mi(RM, DP, IM)                                                \
        shrwx_mi(W(RM), W(DP), W(IM))

#define shrxx_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        shrwx_rr(W(RG), W(RM))

#define shrxx_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        shrwx_ld(W(RG), W(RM), W(DP))

#define shrxx_st(RG, RM, DP)                                                \
        shrwx_st(W(RG), W(RM), W(DP))

#define shrxx_mr(RM, DP, RG)                                                \
        shrxx_st(W(RG), W(RM), W(DP))

#define shrxx_rx(RM)                     /* reads Recx for shift value */   \
        shrwx_rx(W(RM))

#define shrxx_mx(RM, DP)                 /* reads Recx for shift value */   \
        shrwx_mx(W(RM), W(DP))


#define shrwn_ri(RM, IM)                                                    \
        EMITW(0x00000003 | MRM(REG(RM), 0x00,    REG(RM)) |                 \
                                                 (0x1F & VAL(IM)) << 6)

#define shrwn_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000003 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IM)) << 6)     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrwn_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        EMITW(0x00000007 | MRM(REG(RG), REG(RM), REG(RG)))

#define shrwn_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000007 | MRM(REG(RG), TMxx,    REG(RG)))

#define shrwn_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000007 | MRM(TMxx,    REG(RG), TMxx))                     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrwn_mr(RM, DP, RG)                                                \
        shrwn_st(W(RG), W(RM), W(DP))

#define shrwn_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0x00000007 | MRM(REG(RM), Tecx,    REG(RM)))

#define shrwn_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000007 | MRM(TMxx,    Tecx,    TMxx))                     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define shrxn_ri(RM, IM)                                                    \
        shrwn_ri(W(RM), W(IM))

#define shrxn_mi(RM, DP, IM)                                                \
        shrwn_mi(W(RM), W(DP), W(IM))

#define shrxn_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        shrwn_rr(W(RG), W(RM))

#define shrxn_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        shrwn_ld(W(RG), W(RM), W(DP))

#define shrxn_st(RG, RM, DP)                                                \
        shrwn_st(W(RG), W(RM), W(DP))

#define shrxn_mr(RM, DP, RG)                                                \
        shrxn_st(W(RG), W(RM), W(DP))

#define shrxn_rx(RM)                     /* reads Recx for shift value */   \
        shrwn_rx(W(RM))

#define shrxn_mx(RM, DP)                 /* reads Recx for shift value */   \
        shrwn_mx(W(RM), W(DP))

/* pre-r6 */
#if (defined (RT_M32) && RT_M32 < 6) || (defined (RT_M64) && RT_M64 < 6)

/* mul
 * set-flags: undefined */

#define mulwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x70000002 | MRM(REG(RM), REG(RM), TIxx))

#define mulwx_rr(RG, RM)                                                    \
        EMITW(0x70000002 | MRM(REG(RG), REG(RG), REG(RM)))

#define mulwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x70000002 | MRM(REG(RG), REG(RG), TMxx))


#define mulxx_ri(RM, IM)                                                    \
        mulwx_ri(W(RM), W(IM))

#define mulxx_rr(RG, RM)                                                    \
        mulwx_rr(W(RG), W(RM))

#define mulxx_ld(RG, RM, DP)                                                \
        mulwx_ld(W(RG), W(RM), W(DP))


#define mulwx_xr(RM)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0x00000019 | MRM(0x00,    Teax,    REG(RM)))                  \
        EMITW(0x00000012 | MRM(Teax,    0x00,    0x00))                     \
        EMITW(0x00000010 | MRM(Tedx,    0x00,    0x00))

#define mulwx_xm(RM, DP) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000019 | MRM(0x00,    Teax,    TMxx))                     \
        EMITW(0x00000012 | MRM(Teax,    0x00,    0x00))                     \
        EMITW(0x00000010 | MRM(Tedx,    0x00,    0x00))


#define mulxx_xr(RM)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        mulwx_xr(W(RM))

#define mulxx_xm(RM, DP) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        mulwx_xm(W(RM), W(DP))


#define mulwn_xr(RM)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0x00000018 | MRM(0x00,    Teax,    REG(RM)))                  \
        EMITW(0x00000012 | MRM(Teax,    0x00,    0x00))                     \
        EMITW(0x00000010 | MRM(Tedx,    0x00,    0x00))

#define mulwn_xm(RM, DP) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000018 | MRM(0x00,    Teax,    TMxx))                     \
        EMITW(0x00000012 | MRM(Teax,    0x00,    0x00))                     \
        EMITW(0x00000010 | MRM(Tedx,    0x00,    0x00))


#define mulxn_xr(RM)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        mulwn_xr(W(RM))

#define mulxn_xm(RM, DP) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        mulwn_xm(W(RM), W(DP))


#define mulwp_xr(RM)     /* Reax is in/out, prepares Redx for divwn_x* */   \
        mulwx_rr(Reax, W(RM))         /* must not exceed operands size */

#define mulwp_xm(RM, DP) /* Reax is in/out, prepares Redx for divwn_x* */   \
        mulwx_ld(Reax, W(RM), W(DP))  /* must not exceed operands size */


#define mulxp_xr(RM)     /* Reax is in/out, prepares Redx for divxn_x* */   \
        mulxx_rr(Reax, W(RM))         /* must not exceed operands size */

#define mulxp_xm(RM, DP) /* Reax is in/out, prepares Redx for divxn_x* */   \
        mulxx_ld(Reax, W(RM), W(DP))  /* must not exceed operands size */

/* div
 * set-flags: undefined */

#define divwx_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x0000001B | MRM(0x00,    REG(RM), TIxx))                     \
        EMITW(0x00000012 | MRM(REG(RM), 0x00,    0x00))

#define divwx_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        EMITW(0x0000001B | MRM(0x00,    REG(RG), REG(RM)))                  \
        EMITW(0x00000012 | MRM(REG(RG), 0x00,    0x00))

#define divwx_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x0000001B | MRM(0x00,    REG(RG), TMxx))                     \
        EMITW(0x00000012 | MRM(REG(RG), 0x00,    0x00))


#define divxx_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        divwx_ri(W(RM), W(IM))

#define divxx_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        divwx_rr(W(RG), W(RM))

#define divxx_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        divwx_ld(W(RG), W(RM), W(DP))


#define divwn_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x0000001A | MRM(0x00,    REG(RM), TIxx))                     \
        EMITW(0x00000012 | MRM(REG(RM), 0x00,    0x00))

#define divwn_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        EMITW(0x0000001A | MRM(0x00,    REG(RG), REG(RM)))                  \
        EMITW(0x00000012 | MRM(REG(RG), 0x00,    0x00))

#define divwn_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x0000001A | MRM(0x00,    REG(RG), TMxx))                     \
        EMITW(0x00000012 | MRM(REG(RG), 0x00,    0x00))


#define divxn_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        divwn_ri(W(RM), W(IM))

#define divxn_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        divwn_rr(W(RG), W(RM))

#define divxn_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        divwn_ld(W(RG), W(RM), W(DP))


#define prewx_xx()          /* to be placed immediately prior divwx_x* */   \
                                     /* to prepare Redx for int-divide */

#define prewn_xx()          /* to be placed immediately prior divwn_x* */   \
                                     /* to prepare Redx for int-divide */


#define prexx_xx()          /* to be placed immediately prior divxx_x* */   \
        prewx_xx()                   /* to prepare Redx for int-divide */

#define prexn_xx()          /* to be placed immediately prior divxn_x* */   \
        prewn_xx()                   /* to prepare Redx for int-divide */


#define divwx_xr(RM)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(0x0000001B | MRM(0x00,    Teax,    REG(RM)))                  \
        EMITW(0x00000012 | MRM(Teax,    0x00,    0x00))

#define divwx_xm(RM, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x0000001B | MRM(0x00,    Teax,    TMxx))                     \
        EMITW(0x00000012 | MRM(Teax,    0x00,    0x00))


#define divxx_xr(RM)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        divwx_xr(W(RM))

#define divxx_xm(RM, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        divwx_xm(W(RM), W(DP))


#define divwn_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(0x0000001A | MRM(0x00,    Teax,    REG(RM)))                  \
        EMITW(0x00000012 | MRM(Teax,    0x00,    0x00))

#define divwn_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x0000001A | MRM(0x00,    Teax,    TMxx))                     \
        EMITW(0x00000012 | MRM(Teax,    0x00,    0x00))


#define divxn_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xr(W(RM))

#define divxn_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xm(W(RM), W(DP))


#define divwp_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xr(W(RM))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divwp_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xm(W(RM), W(DP))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */


#define divxp_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divxn_xr(W(RM))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divxp_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divxn_xm(W(RM), W(DP))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem
 * set-flags: undefined */

#define remwx_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x0000001B | MRM(0x00,    REG(RM), TIxx))                     \
        EMITW(0x00000010 | MRM(REG(RM), 0x00,    0x00))

#define remwx_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        EMITW(0x0000001B | MRM(0x00,    REG(RG), REG(RM)))                  \
        EMITW(0x00000010 | MRM(REG(RG), 0x00,    0x00))

#define remwx_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x0000001B | MRM(0x00,    REG(RG), TMxx))                     \
        EMITW(0x00000010 | MRM(REG(RG), 0x00,    0x00))


#define remxx_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        remwx_ri(W(RM), W(IM))

#define remxx_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        remwx_rr(W(RG), W(RM))

#define remxx_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        remwx_ld(W(RG), W(RM), W(DP))


#define remwn_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x0000001A | MRM(0x00,    REG(RM), TIxx))                     \
        EMITW(0x00000010 | MRM(REG(RM), 0x00,    0x00))

#define remwn_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        EMITW(0x0000001A | MRM(0x00,    REG(RG), REG(RM)))                  \
        EMITW(0x00000010 | MRM(REG(RG), 0x00,    0x00))

#define remwn_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x0000001A | MRM(0x00,    REG(RG), TMxx))                     \
        EMITW(0x00000010 | MRM(REG(RG), 0x00,    0x00))


#define remxn_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        remwn_ri(W(RM), W(IM))

#define remxn_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        remwn_rr(W(RG), W(RM))

#define remxn_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        remwn_ld(W(RG), W(RM), W(DP))


#define remwx_xx()          /* to be placed immediately prior divwx_x* */   \
                                     /* to prepare for rem calculation */

#define remwx_xr(RM)        /* to be placed immediately after divwx_xr */   \
        EMITW(0x00000010 | MRM(Tedx,    0x00,    0x00))   /* Redx<-rem */

#define remwx_xm(RM, DP)    /* to be placed immediately after divwx_xm */   \
        EMITW(0x00000010 | MRM(Tedx,    0x00,    0x00))   /* Redx<-rem */


#define remxx_xx()          /* to be placed immediately prior divxx_x* */   \
        remwx_xx()                   /* to prepare for rem calculation */

#define remxx_xr(RM)        /* to be placed immediately after divxx_xr */   \
        remwx_xr(W(RM))                                   /* Redx<-rem */

#define remxx_xm(RM, DP)    /* to be placed immediately after divxx_xm */   \
        remwx_xm(W(RM), W(DP))                            /* Redx<-rem */


#define remwn_xx()          /* to be placed immediately prior divwn_x* */   \
                                     /* to prepare for rem calculation */

#define remwn_xr(RM)        /* to be placed immediately after divwn_xr */   \
        EMITW(0x00000010 | MRM(Tedx,    0x00,    0x00))   /* Redx<-rem */

#define remwn_xm(RM, DP)    /* to be placed immediately after divwn_xm */   \
        EMITW(0x00000010 | MRM(Tedx,    0x00,    0x00))   /* Redx<-rem */


#define remxn_xx()          /* to be placed immediately prior divxn_x* */   \
        remwn_xx()                   /* to prepare for rem calculation */

#define remxn_xr(RM)        /* to be placed immediately after divxn_xr */   \
        remwn_xr(W(RM))                                   /* Redx<-rem */

#define remxn_xm(RM, DP)    /* to be placed immediately after divxn_xm */   \
        remwn_xm(W(RM), W(DP))                            /* Redx<-rem */

#else  /* r6 */

/* mul
 * set-flags: undefined */

#define mulwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x00000099 | MRM(REG(RM), REG(RM), TIxx))

#define mulwx_rr(RG, RM)                                                    \
        EMITW(0x00000099 | MRM(REG(RG), REG(RG), REG(RM)))

#define mulwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000099 | MRM(REG(RG), REG(RG), TMxx))


#define mulxx_ri(RM, IM)                                                    \
        mulwx_ri(W(RM), W(IM))

#define mulxx_rr(RG, RM)                                                    \
        mulwx_rr(W(RG), W(RM))

#define mulxx_ld(RG, RM, DP)                                                \
        mulwx_ld(W(RG), W(RM), W(DP))


#define mulwx_xr(RM)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0x000000D9 | MRM(Tedx,    Teax,    REG(RM)))                  \
        EMITW(0x00000099 | MRM(Teax,    Teax,    REG(RM)))

#define mulwx_xm(RM, DP) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x000000D9 | MRM(Tedx,    Teax,    TMxx))                     \
        EMITW(0x00000099 | MRM(Teax,    Teax,    TMxx))


#define mulxx_xr(RM)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        mulwx_xr(W(RM))

#define mulxx_xm(RM, DP) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        mulwx_xm(W(RM), W(DP))


#define mulwn_xr(RM)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0x000000D8 | MRM(Tedx,    Teax,    REG(RM)))                  \
        EMITW(0x00000098 | MRM(Teax,    Teax,    REG(RM)))

#define mulwn_xm(RM, DP) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x000000D8 | MRM(Tedx,    Teax,    TMxx))                     \
        EMITW(0x00000098 | MRM(Teax,    Teax,    TMxx))


#define mulxn_xr(RM)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        mulwn_xr(W(RM))

#define mulxn_xm(RM, DP) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        mulwn_xm(W(RM), W(DP))


#define mulwp_xr(RM)     /* Reax is in/out, prepares Redx for divwn_x* */   \
        mulwx_rr(Reax, W(RM))         /* must not exceed operands size */

#define mulwp_xm(RM, DP) /* Reax is in/out, prepares Redx for divwn_x* */   \
        mulwx_ld(Reax, W(RM), W(DP))  /* must not exceed operands size */


#define mulxp_xr(RM)     /* Reax is in/out, prepares Redx for divxn_x* */   \
        mulxx_rr(Reax, W(RM))         /* must not exceed operands size */

#define mulxp_xm(RM, DP) /* Reax is in/out, prepares Redx for divxn_x* */   \
        mulxx_ld(Reax, W(RM), W(DP))  /* must not exceed operands size */

/* div
 * set-flags: undefined */

#define divwx_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x0000009B | MRM(REG(RM), REG(RM), TIxx))

#define divwx_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        EMITW(0x0000009B | MRM(REG(RG), REG(RG), REG(RM)))

#define divwx_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x0000009B | MRM(REG(RG), REG(RG), TMxx))


#define divxx_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        divwx_ri(W(RM), W(IM))

#define divxx_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        divwx_rr(W(RG), W(RM))

#define divxx_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        divwx_ld(W(RG), W(RM), W(DP))


#define divwn_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x0000009A | MRM(REG(RM), REG(RM), TIxx))

#define divwn_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        EMITW(0x0000009A | MRM(REG(RG), REG(RG), REG(RM)))

#define divwn_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x0000009A | MRM(REG(RG), REG(RG), TMxx))


#define divxn_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        divwn_ri(W(RM), W(IM))

#define divxn_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        divwn_rr(W(RG), W(RM))

#define divxn_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        divwn_ld(W(RG), W(RM), W(DP))


#define prewx_xx()          /* to be placed immediately prior divwx_x* */   \
                                     /* to prepare Redx for int-divide */

#define prewn_xx()          /* to be placed immediately prior divwn_x* */   \
                                     /* to prepare Redx for int-divide */


#define prexx_xx()          /* to be placed immediately prior divxx_x* */   \
        prewx_xx()                   /* to prepare Redx for int-divide */

#define prexn_xx()          /* to be placed immediately prior divxn_x* */   \
        prewn_xx()                   /* to prepare Redx for int-divide */


#define divwx_xr(RM)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(0x0000009B | MRM(Teax,    Teax,    REG(RM)))

#define divwx_xm(RM, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x0000009B | MRM(Teax,    Teax,    TMxx))


#define divxx_xr(RM)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        divwx_xr(W(RM))

#define divxx_xm(RM, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        divwx_xm(W(RM), W(DP))


#define divwn_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(0x0000009A | MRM(Teax,    Teax,    REG(RM)))

#define divwn_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x0000009A | MRM(Teax,    Teax,    TMxx))


#define divxn_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xr(W(RM))

#define divxn_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xm(W(RM), W(DP))


#define divwp_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xr(W(RM))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divwp_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xm(W(RM), W(DP))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */


#define divxp_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divxn_xr(W(RM))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divxp_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divxn_xm(W(RM), W(DP))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem
 * set-flags: undefined */

#define remwx_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x000000DB | MRM(REG(RM), REG(RM), TIxx))

#define remwx_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        EMITW(0x000000DB | MRM(REG(RG), REG(RG), REG(RM)))

#define remwx_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x000000DB | MRM(REG(RG), REG(RG), TMxx))


#define remxx_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        remwx_ri(W(RM), W(IM))

#define remxx_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        remwx_rr(W(RG), W(RM))

#define remxx_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        remwx_ld(W(RG), W(RM), W(DP))


#define remwn_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x000000DA | MRM(REG(RM), REG(RM), TIxx))

#define remwn_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        EMITW(0x000000DA | MRM(REG(RG), REG(RG), REG(RM)))

#define remwn_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x000000DA | MRM(REG(RG), REG(RG), TMxx))


#define remxn_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        remwn_ri(W(RM), W(IM))

#define remxn_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        remwn_rr(W(RG), W(RM))

#define remxn_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        remwn_ld(W(RG), W(RM), W(DP))


#define remwx_xx()          /* to be placed immediately prior divwx_x* */   \
        movwx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remwx_xr(RM)        /* to be placed immediately after divwx_xr */   \
        EMITW(0x000000DB | MRM(Tedx,    Tedx,    REG(RM)))/* Redx<-rem */

#define remwx_xm(RM, DP)    /* to be placed immediately after divwx_xm */   \
        EMITW(0x000000DB | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */


#define remxx_xx()          /* to be placed immediately prior divxx_x* */   \
        remwx_xx()                   /* to prepare for rem calculation */

#define remxx_xr(RM)        /* to be placed immediately after divxx_xr */   \
        remwx_xr(W(RM))                                   /* Redx<-rem */

#define remxx_xm(RM, DP)    /* to be placed immediately after divxx_xm */   \
        remwx_xm(W(RM), W(DP))                            /* Redx<-rem */


#define remwn_xx()          /* to be placed immediately prior divwn_x* */   \
        movwx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remwn_xr(RM)        /* to be placed immediately after divwn_xr */   \
        EMITW(0x000000DA | MRM(Tedx,    Tedx,    REG(RM)))/* Redx<-rem */

#define remwn_xm(RM, DP)    /* to be placed immediately after divwn_xm */   \
        EMITW(0x000000DA | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */


#define remxn_xx()          /* to be placed immediately prior divxn_x* */   \
        remwn_xx()                   /* to prepare for rem calculation */

#define remxn_xr(RM)        /* to be placed immediately after divxn_xr */   \
        remwn_xr(W(RM))                                   /* Redx<-rem */

#define remxn_xm(RM, DP)    /* to be placed immediately after divxn_xm */   \
        remwn_xm(W(RM), W(DP))                            /* Redx<-rem */

#endif /* r6 */

/* arj
 * set-flags: undefined */

#define and_x   and
#define add_x   add
#define sub_x   sub

#define EZ_x    J0
#define NZ_x    J1


#define arjwx_ri(RM, IM, OP, CC, lb)                                        \
        AR2(W(RM), W(IM), OP, wx_ri)                                        \
        CMZ(CC, MOD(RM), lb)

#define arjwx_mi(RM, DP, IM, OP, CC, lb)                                    \
        AR3(W(RM), W(DP), W(IM), OP, wz_mi)                                 \
        CMZ(CC, $t8,     lb)

#define arjwx_rr(RG, RM, OP, CC, lb)                                        \
        AR2(W(RG), W(RM), OP, wx_rr)                                        \
        CMZ(CC, MOD(RG), lb)

#define arjwx_ld(RG, RM, DP, OP, CC, lb)                                    \
        AR3(W(RG), W(RM), W(DP), OP, wx_ld)                                 \
        CMZ(CC, MOD(RG), lb)

#define arjwx_st(RG, RM, DP, OP, CC, lb)                                    \
        AR3(W(RG), W(RM), W(DP), OP, wz_st)                                 \
        CMZ(CC, $t8,     lb)

#define arjwx_mr(RM, DP, RG, OP, CC, lb)                                    \
        arjwx_st(W(RG), W(RM), W(DP), OP, CC, lb)


#define arjxx_ri(RM, IM, OP, CC, lb)                                        \
        arjwx_ri(W(RM), W(IM), OP, CC, lb)

#define arjxx_mi(RM, DP, IM, OP, CC, lb)                                    \
        arjwx_mi(W(RM), W(DP), W(IM), OP, CC, lb)

#define arjxx_rr(RG, RM, OP, CC, lb)                                        \
        arjwx_rr(W(RG), W(RM), OP, CC, lb)

#define arjxx_ld(RG, RM, DP, OP, CC, lb)                                    \
        arjwx_ld(W(RG), W(RM), W(DP), OP, CC, lb)

#define arjxx_st(RG, RM, DP, OP, CC, lb)                                    \
        arjwx_st(W(RG), W(RM), W(DP), OP, CC, lb)

#define arjxx_mr(RM, DP, RG, OP, CC, lb)                                    \
        arjxx_st(W(RG), W(RM), W(DP), OP, CC, lb)

/* internal definitions for combined-arithmetic-jump (arj) */

#define AR2(P1, P2, OP, SG)                                                 \
        OP##SG(W(P1), W(P2))

#define AR3(P1, P2, P3, OP, SG)                                             \
        OP##SG(W(P1), W(P2), W(P3))

/* cmj
 * set-flags: undefined */

#define EQ_x    J0
#define NE_x    J1

#define LT_x    J2
#define LE_x    J3
#define GT_x    J4
#define GE_x    J5

#define LT_n    J6
#define LE_n    J7
#define GT_n    J8
#define GE_n    J9


#define cmjwx_rz(RM, CC, lb)                                                \
        CMZ(CC, MOD(RM), lb)

#define cmjwx_mz(RM, DP, CC, lb)                                            \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        CMZ(CC, $t8,     lb)

#define cmjwx_ri(RM, IM, CC, lb)                                            \
        CMI(CC, MOD(RM), REG(RM), W(IM), lb)

#define cmjwx_mi(RM, DP, IM, CC, lb)                                        \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        CMI(CC, $t8,     TMxx,    W(IM), lb)

#define cmjwx_rr(RG, RM, CC, lb)                                            \
        CMR(CC, MOD(RG), MOD(RM), lb)

#define cmjwx_rm(RG, RM, DP, CC, lb)                                        \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        CMR(CC, MOD(RG), $t8,     lb)

#define cmjwx_mr(RM, DP, RG, CC, lb)                                        \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        CMR(CC, $t8,     MOD(RG), lb)


#define cmjxx_rz(RM, CC, lb)                                                \
        cmjxx_ri(W(RM), IC(0), CC, lb)

#define cmjxx_mz(RM, DP, CC, lb)                                            \
        cmjxx_mi(W(RM), W(DP), IC(0), CC, lb)

#define cmjxx_ri(RM, IM, CC, lb)                                            \
        cmjwx_ri(W(RM), W(IM), CC, lb)

#define cmjxx_mi(RM, DP, IM, CC, lb)                                        \
        cmjwx_mi(W(RM), W(DP), W(IM), CC, lb)

#define cmjxx_rr(RG, RM, CC, lb)                                            \
        cmjwx_rr(W(RG), W(RM), CC, lb)

#define cmjxx_rm(RG, RM, DP, CC, lb)                                        \
        cmjwx_rm(W(RG), W(RM), W(DP), CC, lb)

#define cmjxx_mr(RM, DP, RG, CC, lb)                                        \
        cmjwx_mr(W(RM), W(DP), W(RG), CC, lb)

/* cmp
 * set-flags: yes */

#define cmpwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x00000025 | MRM(TLxx,    REG(RM), TZxx))

#define cmpwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TRxx,    MOD(RM), VAL(DP), C1(DP), G3(IM))   \
        EMITW(0x8C000000 | MDM(TLxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define cmpwx_rr(RG, RM)                                                    \
        EMITW(0x00000025 | MRM(TRxx,    REG(RM), TZxx))                     \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))

#define cmpwx_rm(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TRxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))

#define cmpwx_mr(RM, DP, RG)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TLxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000025 | MRM(TRxx,    REG(RG), TZxx))


#define cmpxx_ri(RM, IM)                                                    \
        cmpwx_ri(W(RM), W(IM))

#define cmpxx_mi(RM, DP, IM)                                                \
        cmpwx_mi(W(RM), W(DP), W(IM))

#define cmpxx_rr(RG, RM)                                                    \
        cmpwx_rr(W(RG), W(RM))

#define cmpxx_rm(RG, RM, DP)                                                \
        cmpwx_rm(W(RG), W(RM), W(DP))

#define cmpxx_mr(RM, DP, RG)                                                \
        cmpwx_mr(W(RM), W(DP), W(RG))

/* pre-r6 */
#if (defined (RT_M32) && RT_M32 < 6) || (defined (RT_M64) && RT_M64 < 6)

/* jmp
 * set-flags: no
 * maximum byte-address-range for un/conditional jumps is signed 18/16-bit
 * based on minimum natively-encoded offset across supported targets (u/c)
 * MIPS:18-bit, Power:26-bit, AArch32:26-bit, AArch64:28-bit, x86:32-bit /
 * MIPS:18-bit, Power:16-bit, AArch32:26-bit, AArch64:21-bit, x86:32-bit */

#define jmpxx_rr(RM)           /* register-targeted unconditional jump */   \
        EMITW(0x00000008 | MRM(0x00,    REG(RM), 0x00))                     \
        EMITW(0x00000025 | MRM(TPxx,    TPxx,    TZxx)) /* <- branch delay */

#if defined (RT_M32)

#define jmpxx_mm(RM, DP)         /* memory-targeted unconditional jump */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000008 | MRM(0x00,    TMxx,    0x00))                     \
        EMITW(0x00000025 | MRM(TPxx,    TPxx,    TZxx)) /* <- branch delay */

#elif defined (RT_M64)

#define jmpxx_mm(RM, DP)         /* memory-targeted unconditional jump */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xDC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000008 | MRM(0x00,    TMxx,    0x00))                     \
        EMITW(0x00000025 | MRM(TPxx,    TPxx,    TZxx)) /* <- branch delay */

#endif /* defined (RT_M32, RT_M64) */

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

/* internal definitions for combined-compare-jump (cmj) */

#define ZJ0(r1, lb)                                                         \
        ASM_BEG ASM_OP3(beq,  r1, $zero, lb) ASM_END

#define ZJ1(r1, lb)                                                         \
        ASM_BEG ASM_OP3(bne,  r1, $zero, lb) ASM_END

#define ZJ2(r1, lb) /* "never" branch as unsigned is always >= 0 */         \
        EMPTY

#define ZJ3(r1, lb)                                                         \
        ASM_BEG ASM_OP3(beq,  r1, $zero, lb) ASM_END

#define ZJ4(r1, lb)                                                         \
        ASM_BEG ASM_OP3(bne,  r1, $zero, lb) ASM_END

#define ZJ5(r1, lb) /* "always" branch as unsigned is never < 0 */          \
        ASM_BEG ASM_OP1(b, lb) ASM_END

#define ZJ6(r1, lb)                                                         \
        ASM_BEG ASM_OP2(bltz, r1, lb) ASM_END

#define ZJ7(r1, lb)                                                         \
        ASM_BEG ASM_OP2(blez, r1, lb) ASM_END

#define ZJ8(r1, lb)                                                         \
        ASM_BEG ASM_OP2(bgtz, r1, lb) ASM_END

#define ZJ9(r1, lb)                                                         \
        ASM_BEG ASM_OP2(bgez, r1, lb) ASM_END

#define CMZ(CC, r1, lb)                                                     \
        Z##CC(r1, lb)


#define IJ0(r1, p1, IM, lb)                                                 \
        AUW(EMPTY,    VAL(IM), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        ASM_BEG ASM_OP3(beq, r1, $t9, lb) ASM_END

#define IJ1(r1, p1, IM, lb)                                                 \
        AUW(EMPTY,    VAL(IM), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        ASM_BEG ASM_OP3(bne, r1, $t9, lb) ASM_END

#define IJ2(r1, p1, IM, lb)                                                 \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(TLxx,    p1,      VAL(IM), T1(IM), M1(IM)) | \
        (+(TP1(IM) == 0) & 0x2C000000) | (+(TP1(IM) != 0) & 0x0000002B))    \
        ASM_BEG ASM_OP2(bnez, $t8, lb) ASM_END

#define IJ3(r1, p1, IM, lb)                                                 \
        AUW(EMPTY,    VAL(IM), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        ASM_BEG ASM_OP3(sltu, $t8, $t9, r1) ASM_END                         \
        ASM_BEG ASM_OP2(beqz, $t8, lb) ASM_END

#define IJ4(r1, p1, IM, lb)                                                 \
        AUW(EMPTY,    VAL(IM), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        ASM_BEG ASM_OP3(sltu, $t8, $t9, r1) ASM_END                         \
        ASM_BEG ASM_OP2(bnez, $t8, lb) ASM_END

#define IJ5(r1, p1, IM, lb)                                                 \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(TLxx,    p1,      VAL(IM), T1(IM), M1(IM)) | \
        (+(TP1(IM) == 0) & 0x2C000000) | (+(TP1(IM) != 0) & 0x0000002B))    \
        ASM_BEG ASM_OP2(beqz, $t8, lb) ASM_END

#define IJ6(r1, p1, IM, lb)                                                 \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(TLxx,    p1,      VAL(IM), T1(IM), M1(IM)) | \
        (+(TP1(IM) == 0) & 0x28000000) | (+(TP1(IM) != 0) & 0x0000002A))    \
        ASM_BEG ASM_OP2(bnez, $t8, lb) ASM_END

#define IJ7(r1, p1, IM, lb)                                                 \
        AUW(EMPTY,    VAL(IM), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        ASM_BEG ASM_OP3(slt,  $t8, $t9, r1) ASM_END                         \
        ASM_BEG ASM_OP2(beqz, $t8, lb) ASM_END

#define IJ8(r1, p1, IM, lb)                                                 \
        AUW(EMPTY,    VAL(IM), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        ASM_BEG ASM_OP3(slt,  $t8, $t9, r1) ASM_END                         \
        ASM_BEG ASM_OP2(bnez, $t8, lb) ASM_END

#define IJ9(r1, p1, IM, lb)                                                 \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(TLxx,    p1,      VAL(IM), T1(IM), M1(IM)) | \
        (+(TP1(IM) == 0) & 0x28000000) | (+(TP1(IM) != 0) & 0x0000002A))    \
        ASM_BEG ASM_OP2(beqz, $t8, lb) ASM_END

#define CMI(CC, r1, p1, IM, lb)                                             \
        I##CC(r1, p1, W(IM), lb)


#define RJ0(r1, r2, lb)                                                     \
        ASM_BEG ASM_OP3(beq,  r1,  r2, lb) ASM_END

#define RJ1(r1, r2, lb)                                                     \
        ASM_BEG ASM_OP3(bne,  r1,  r2, lb) ASM_END

#define RJ2(r1, r2, lb)                                                     \
        ASM_BEG ASM_OP3(sltu, $t8, r1, r2) ASM_END                          \
        ASM_BEG ASM_OP2(bnez, $t8, lb) ASM_END

#define RJ3(r1, r2, lb)                                                     \
        ASM_BEG ASM_OP3(sltu, $t8, r2, r1) ASM_END                          \
        ASM_BEG ASM_OP2(beqz, $t8, lb) ASM_END

#define RJ4(r1, r2, lb)                                                     \
        ASM_BEG ASM_OP3(sltu, $t8, r2, r1) ASM_END                          \
        ASM_BEG ASM_OP2(bnez, $t8, lb) ASM_END

#define RJ5(r1, r2, lb)                                                     \
        ASM_BEG ASM_OP3(sltu, $t8, r1, r2) ASM_END                          \
        ASM_BEG ASM_OP2(beqz, $t8, lb) ASM_END

#define RJ6(r1, r2, lb)                                                     \
        ASM_BEG ASM_OP3(slt,  $t8, r1, r2) ASM_END                          \
        ASM_BEG ASM_OP2(bnez, $t8, lb) ASM_END

#define RJ7(r1, r2, lb)                                                     \
        ASM_BEG ASM_OP3(slt,  $t8, r2, r1) ASM_END                          \
        ASM_BEG ASM_OP2(beqz, $t8, lb) ASM_END

#define RJ8(r1, r2, lb)                                                     \
        ASM_BEG ASM_OP3(slt,  $t8, r2, r1) ASM_END                          \
        ASM_BEG ASM_OP2(bnez, $t8, lb) ASM_END

#define RJ9(r1, r2, lb)                                                     \
        ASM_BEG ASM_OP3(slt,  $t8, r1, r2) ASM_END                          \
        ASM_BEG ASM_OP2(beqz, $t8, lb) ASM_END

#define CMR(CC, r1, r2, lb)                                                 \
        R##CC(r1, r2, lb)

#else  /* r6 */

/* jmp
 * set-flags: no
 * maximum byte-address-range for un/conditional jumps is signed 18/16-bit
 * based on minimum natively-encoded offset across supported targets (u/c)
 * MIPS:18-bit, Power:26-bit, AArch32:26-bit, AArch64:28-bit, x86:32-bit /
 * MIPS:18-bit, Power:16-bit, AArch32:26-bit, AArch64:21-bit, x86:32-bit */

#define jmpxx_rr(RM)           /* register-targeted unconditional jump */   \
        EMITW(0x00000009 | MRM(0x00,    REG(RM), 0x00))                     \
        EMITW(0x00000025 | MRM(TPxx,    TPxx,    TZxx)) /* <- branch delay */

#if defined (RT_M32)

#define jmpxx_mm(RM, DP)         /* memory-targeted unconditional jump */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000009 | MRM(0x00,    TMxx,    0x00))                     \
        EMITW(0x00000025 | MRM(TPxx,    TPxx,    TZxx)) /* <- branch delay */

#elif defined (RT_M64)

#define jmpxx_mm(RM, DP)         /* memory-targeted unconditional jump */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xDC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000009 | MRM(0x00,    TMxx,    0x00))                     \
        EMITW(0x00000025 | MRM(TPxx,    TPxx,    TZxx)) /* <- branch delay */

#endif /* defined (RT_M32, RT_M64) */

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

/* internal definitions for combined-compare-jump (cmj) */

#define ZJ0(r1, lb)                                                         \
        ASM_BEG ASM_OP2(beqzc, r1, lb) ASM_END

#define ZJ1(r1, lb)                                                         \
        ASM_BEG ASM_OP2(bnezc, r1, lb) ASM_END

#define ZJ2(r1, lb) /* "never" branch as unsigned is always >= 0 */         \
        EMPTY

#define ZJ3(r1, lb)                                                         \
        ASM_BEG ASM_OP2(beqzc, r1, lb) ASM_END

#define ZJ4(r1, lb)                                                         \
        ASM_BEG ASM_OP2(bnezc, r1, lb) ASM_END

#define ZJ5(r1, lb) /* "always" branch as unsigned is never < 0 */          \
        ASM_BEG ASM_OP1(b, lb) ASM_END

#define ZJ6(r1, lb)                                                         \
        ASM_BEG ASM_OP2(bltzc, r1, lb) ASM_END

#define ZJ7(r1, lb)                                                         \
        ASM_BEG ASM_OP2(blezc, r1, lb) ASM_END

#define ZJ8(r1, lb)                                                         \
        ASM_BEG ASM_OP2(bgtzc, r1, lb) ASM_END

#define ZJ9(r1, lb)                                                         \
        ASM_BEG ASM_OP2(bgezc, r1, lb) ASM_END

#define CMZ(CC, r1, lb)                                                     \
        Z##CC(r1, lb)


#define IJ0(r1, p1, IM, lb)                                                 \
        AUW(EMPTY,    VAL(IM), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        RA0(r1, $t9, lb)

#define IJ1(r1, p1, IM, lb)                                                 \
        AUW(EMPTY,    VAL(IM), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        RA1(r1, $t9, lb)

#define IJ2(r1, p1, IM, lb)                                                 \
        AUW(EMPTY,    VAL(IM), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        RA2(r1, $t9, lb)

#define IJ3(r1, p1, IM, lb)                                                 \
        AUW(EMPTY,    VAL(IM), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        RA3(r1, $t9, lb)

#define IJ4(r1, p1, IM, lb)                                                 \
        AUW(EMPTY,    VAL(IM), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        RA4(r1, $t9, lb)

#define IJ5(r1, p1, IM, lb)                                                 \
        AUW(EMPTY,    VAL(IM), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        RA5(r1, $t9, lb)

#define IJ6(r1, p1, IM, lb)                                                 \
        AUW(EMPTY,    VAL(IM), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        RA6(r1, $t9, lb)

#define IJ7(r1, p1, IM, lb)                                                 \
        AUW(EMPTY,    VAL(IM), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        RA7(r1, $t9, lb)

#define IJ8(r1, p1, IM, lb)                                                 \
        AUW(EMPTY,    VAL(IM), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        RA8(r1, $t9, lb)

#define IJ9(r1, p1, IM, lb)                                                 \
        AUW(EMPTY,    VAL(IM), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        RA9(r1, $t9, lb)

#define CMI(CC, r1, p1, IM, lb)                                             \
        I##CC(r1, p1, W(IM), lb)


#define RJ0(r1, r2, lb)                                                     \
        ASM_BEG ASM_OP3(beqc,  r1, r2, lb) ASM_END

#define RJ1(r1, r2, lb)                                                     \
        ASM_BEG ASM_OP3(bnec,  r1, r2, lb) ASM_END

#define RJ2(r1, r2, lb)                                                     \
        ASM_BEG ASM_OP3(bltuc, r1, r2, lb) ASM_END

#define RJ3(r1, r2, lb)                                                     \
        ASM_BEG ASM_OP3(bgeuc, r2, r1, lb) ASM_END

#define RJ4(r1, r2, lb)                                                     \
        ASM_BEG ASM_OP3(bltuc, r2, r1, lb) ASM_END

#define RJ5(r1, r2, lb)                                                     \
        ASM_BEG ASM_OP3(bgeuc, r1, r2, lb) ASM_END

#define RJ6(r1, r2, lb)                                                     \
        ASM_BEG ASM_OP3(bltc,  r1, r2, lb) ASM_END

#define RJ7(r1, r2, lb)                                                     \
        ASM_BEG ASM_OP3(bgec,  r2, r1, lb) ASM_END

#define RJ8(r1, r2, lb)                                                     \
        ASM_BEG ASM_OP3(bltc,  r2, r1, lb) ASM_END

#define RJ9(r1, r2, lb)                                                     \
        ASM_BEG ASM_OP3(bgec,  r1, r2, lb) ASM_END

#define CMR(CC, r1, r2, lb)                                                 \
        R##CC(r1, r2, lb)

#endif /* r6 */

/* ver
 * set-flags: no */

#define verxx_xx() /* destroys Reax, Recx, Rebx, Redx, Resi, Redi (in x86)*/\
        movwx_mi(Mebp, inf_VER, IB(1)) /* <- MSA to bit0 */

#endif /* RT_RTARCH_M32_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

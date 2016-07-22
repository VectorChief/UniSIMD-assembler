/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_A64_H
#define RT_RTARCH_A64_H

#define RT_BASE_REGS        16

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_a64.h: Implementation of AArch64:ARMv8 BASE instructions.
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
 *
 * cmdz*_** - applies [cmd] while setting condition flags, [z] - zero flag.
 * Regular cmdxx_** instructions may or may not set flags depending
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
#define VXL(val, tp1, tp2)  (((val) >> 1) & 0x3FFC)
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
                    EMITW(0x8B000000 | MRM(TPxx,    (br),    TDxx))

#define B12(br) TPxx
#define P12(dp) (0x00000000)
#define C12(br, dp) C32(br, dp)                                             \
                    EMITW(0x8B000000 | MRM(TPxx,    (br),    TDxx))
#define C32(br, dp) EMITW(0x52800000 | MRM(TDxx,    0x00,    0x00) |        \
                             (0xFFFC & (dp)) << 5)                          \
                    EMITW(0x72A00000 | MRM(TDxx,    0x00,    0x00) |        \
                             (0x7FFF & (dp) >> 16) << 5)

/* registers    REG   (check mapping with ASM_ENTER/ASM_LEAVE in rtarch.h) */

#define TNxx    0x16  /* x22, default FCTRL round mode */
#define TAxx    0x17  /* x23, extra reg for FAST_FCTRL */
#define TCxx    0x18  /* x24, extra reg for FAST_FCTRL */
#define TExx    0x19  /* x25, extra reg for FAST_FCTRL */

#define TMxx    0x1A  /* x26 */
#define TIxx    0x1B  /* x27, not used together with TDxx */
#define TDxx    0x1B  /* x27, not used together with TIxx */
#define TPxx    0x1C  /* x28 */
#define TZxx    0x1F  /* x31 */
#define SPxx    0x1F  /* x31 */

#define Teax    0x00  /* x0 */
#define Tecx    0x01  /* x1 */
#define Tedx    0x02  /* x2 */
#define Tebx    0x03  /* x3 */
#define Tebp    0x05  /* x5 */
#define Tesi    0x06  /* w6 */
#define Tedi    0x07  /* x7 */
#define Teg8    0x08  /* x8 */
#define Teg9    0x09  /* x9 */
#define TegA    0x0A  /* x10 */
#define TegB    0x0B  /* x11 */
#define TegC    0x0C  /* x12 */
#define TegD    0x0D  /* x13 */
#define TegE    0x0E  /* x14 */

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

#define Iecx    Tecx, TPxx, EMITW(0x8B000000 | MRM(TPxx,    Tecx,    Teax))
#define Iedx    Tedx, TPxx, EMITW(0x8B000000 | MRM(TPxx,    Tedx,    Teax))
#define Iebx    Tebx, TPxx, EMITW(0x8B000000 | MRM(TPxx,    Tebx,    Teax))
#define Iebp    Tebp, TPxx, EMITW(0x8B000000 | MRM(TPxx,    Tebp,    Teax))
#define Iesi    Tesi, TPxx, EMITW(0x8B000000 | MRM(TPxx,    Tesi,    Teax))
#define Iedi    Tedi, TPxx, EMITW(0x8B000000 | MRM(TPxx,    Tedi,    Teax))
#define Ieg8    Teg8, TPxx, EMITW(0x8B000000 | MRM(TPxx,    Teg8,    Teax))
#define Ieg9    Teg9, TPxx, EMITW(0x8B000000 | MRM(TPxx,    Teg9,    Teax))
#define IegA    TegA, TPxx, EMITW(0x8B000000 | MRM(TPxx,    TegA,    Teax))
#define IegB    TegB, TPxx, EMITW(0x8B000000 | MRM(TPxx,    TegB,    Teax))
#define IegC    TegC, TPxx, EMITW(0x8B000000 | MRM(TPxx,    TegC,    Teax))
#define IegD    TegD, TPxx, EMITW(0x8B000000 | MRM(TPxx,    TegD,    Teax))
#define IegE    TegE, TPxx, EMITW(0x8B000000 | MRM(TPxx,    TegE,    Teax))

/* immediate    VAL,  TP1,  TP2       (all immediate types are unsigned) */

#define IC(im)  ((im) & 0x7F),       0, 1      /* drop sign-ext (in x86) */
#define IB(im)  ((im) & 0xFF),       0, 1        /* 32-bit word (in x86) */
#define IM(im)  ((im) & 0xFFF),      0, 1  /* native AArch64 add/sub/cmp */
#define IG(im)  ((im) & 0x7FFF),     1, 1  /* native on MIPS add/sub/cmp */
#define IH(im)  ((im) & 0xFFFF),     1, 1  /* second native on ARMs/MIPS */
#define IV(im)  ((im) & 0x7FFFFFFF), 2, 2        /* native x64 long mode */
#define IW(im)  ((im) & 0xFFFFFFFF), 2, 2       /* only for cmdw*_** set */

/* displacement VAL,  TP1,  TP2    (all displacement types are unsigned) */

#define DP(dp)  ((dp) & 0xFFC),      0, 0    /* native on all ARMs, MIPS */
#define DF(dp)  ((dp) & 0x3FFC),     0, 0   /* native AArch64 BASE ld/st */
#define DG(dp)  ((dp) & 0x7FFC),     1, 0      /* native MIPS BASE ld/st */
#define DH(dp)  ((dp) & 0xFFFC),     1, 0   /* second native on all ARMs */
#define DV(dp)  ((dp) & 0x7FFFFFFC), 2, 2        /* native x64 long mode */
#define PLAIN   DP(0)           /* special type for Oeax addressing mode */

/* triplet pass-through wrapper */

#define W(p1, p2, p3)       p1,  p2,  p3

/******************************************************************************/
/**********************************   A64   ***********************************/
/******************************************************************************/

/* mov
 * set-flags: no */

#define movwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), REG(RM), EMPTY,   EMPTY,   EMPTY2, G3(IM))

#define movwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G3(IM))   \
        EMITW(0xB9000000 | MDM(TIxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define movwx_rr(RG, RM)                                                    \
        EMITW(0x2A000000 | MRM(REG(RG), TZxx,    REG(RM)))

#define movwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(REG(RG), MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define movwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9000000 | MDM(REG(RG), MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define movxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), REG(RM), EMPTY,   EMPTY,   EMPTY2, G3(IM))

#define movxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G3(IM))   \
        EMITW(0xF9000000 | MDM(TIxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define movxx_rr(RG, RM)                                                    \
        EMITW(0xAA000000 | MRM(REG(RG), TZxx,    REG(RM)))

#define movxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(REG(RG), MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define movxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9000000 | MDM(REG(RG), MOD(RM), VXL(DP), B1(DP), P1(DP)))


#define adrxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C3(DP), EMPTY2)   \
        EMITW(0x8B000000 | MRM(REG(RG), MOD(RM), TDxx))

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
 * set-flags: undefined (xx), yes (zx) */

#define andwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)))

#define andwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)))  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define andwx_rr(RG, RM)                                                    \
        EMITW(0x0A000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define andwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x0A000000 | MRM(REG(RG), REG(RG), TMxx))

#define andwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x0A000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define andxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x80000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)))

#define andxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x80000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define andxx_rr(RG, RM)                                                    \
        EMITW(0x8A000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define andxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x8A000000 | MRM(REG(RG), REG(RG), TMxx))

#define andxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x8A000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))


#define andzx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0xE0000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)))

#define andzx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define andzx_rr(RG, RM)                                                    \
        EMITW(0xEA000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define andzx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xEA000000 | MRM(REG(RG), REG(RG), TMxx))

#define andzx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xEA000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

/* orr
 * set-flags: undefined */

#define orrwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x20000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)))

#define orrwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x20000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)))  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define orrwx_rr(RG, RM)                                                    \
        EMITW(0x2A000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define orrwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x2A000000 | MRM(REG(RG), REG(RG), TMxx))

#define orrwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x2A000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define orrxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0xA0000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)))

#define orrxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xA0000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define orrxx_rr(RG, RM)                                                    \
        EMITW(0xAA000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define orrxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xAA000000 | MRM(REG(RG), REG(RG), TMxx))

#define orrxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xAA000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

/* xor
 * set-flags: undefined */

#define xorwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x40000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)))

#define xorwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x40000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)))  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define xorwx_rr(RG, RM)                                                    \
        EMITW(0x4A000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define xorwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x4A000000 | MRM(REG(RG), REG(RG), TMxx))

#define xorwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x4A000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define xorxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0xC0000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)))

#define xorxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xC0000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define xorxx_rr(RG, RM)                                                    \
        EMITW(0xCA000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define xorxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xCA000000 | MRM(REG(RG), REG(RG), TMxx))

#define xorxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xCA000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

/* not
 * set-flags: no */

#define notwx_rr(RM)                                                        \
        EMITW(0x2A200000 | MRM(REG(RM), TZxx,    REG(RM)))

#define notwx_mm(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x2A200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define notxx_rr(RM)                                                        \
        EMITW(0xAA200000 | MRM(REG(RM), TZxx,    REG(RM)))

#define notxx_mm(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xAA200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

/* neg
 * set-flags: undefined (xx), yes (zx) */

#define negwx_rr(RM)                                                        \
        EMITW(0x4B000000 | MRM(REG(RM), TZxx,    REG(RM)))

#define negwx_mm(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x4B000000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define negxx_rr(RM)                                                        \
        EMITW(0xCB000000 | MRM(REG(RM), TZxx,    REG(RM)))

#define negxx_mm(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xCB000000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))


#define negzx_rr(RM)                                                        \
        EMITW(0xEB000000 | MRM(REG(RM), TZxx,    REG(RM)))

#define negzx_mm(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xEB000000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

/* add
 * set-flags: undefined (xx), yes (zx) */

#define addwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x01000000 | MIM(REG(RM), REG(RM), VAL(IM), T1(IM), M1(IM)))

#define addwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x01000000 | MIM(TMxx,    TMxx,    VAL(IM), T1(IM), M1(IM)))  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define addwx_rr(RG, RM)                                                    \
        EMITW(0x0B000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define addwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x0B000000 | MRM(REG(RG), REG(RG), TMxx))

#define addwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x0B000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define addxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x81000000 | MIM(REG(RM), REG(RM), VAL(IM), T1(IM), M1(IM)))

#define addxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x81000000 | MIM(TMxx,    TMxx,    VAL(IM), T1(IM), M1(IM)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define addxx_rr(RG, RM)                                                    \
        EMITW(0x8B000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define addxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x8B000000 | MRM(REG(RG), REG(RG), TMxx))

#define addxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x8B000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))


#define addzx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0xA1000000 | MIM(REG(RM), REG(RM), VAL(IM), T1(IM), M1(IM)))

#define addzx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xA1000000 | MIM(TMxx,    TMxx,    VAL(IM), T1(IM), M1(IM)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define addzx_rr(RG, RM)                                                    \
        EMITW(0xAB000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define addzx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xAB000000 | MRM(REG(RG), REG(RG), TMxx))

#define addzx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xAB000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

/* sub
 * set-flags: undefined (xx), yes (zx) */

#define subwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x41000000 | MIM(REG(RM), REG(RM), VAL(IM), T1(IM), M1(IM)))

#define subwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x41000000 | MIM(TMxx,    TMxx,    VAL(IM), T1(IM), M1(IM)))  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define subwx_rr(RG, RM)                                                    \
        EMITW(0x4B000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define subwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x4B000000 | MRM(REG(RG), REG(RG), TMxx))

#define subwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x4B000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define subwx_mr(RM, DP, RG)                                                \
        subwx_st(W(RG), W(RM), W(DP))


#define subxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0xD1000000 | MIM(REG(RM), REG(RM), VAL(IM), T1(IM), M1(IM)))

#define subxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xD1000000 | MIM(TMxx,    TMxx,    VAL(IM), T1(IM), M1(IM)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define subxx_rr(RG, RM)                                                    \
        EMITW(0xDB000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define subxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xDB000000 | MRM(REG(RG), REG(RG), TMxx))

#define subxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xDB000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define subxx_mr(RM, DP, RG)                                                \
        subxx_st(W(RG), W(RM), W(DP))


#define subzx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0xE1000000 | MIM(REG(RM), REG(RM), VAL(IM), T1(IM), M1(IM)))

#define subzx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1000000 | MIM(TMxx,    TMxx,    VAL(IM), T1(IM), M1(IM)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define subzx_rr(RG, RM)                                                    \
        EMITW(0xEB000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define subzx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xEB000000 | MRM(REG(RG), REG(RG), TMxx))

#define subzx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xEB000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define subzx_mr(RM, DP, RG)                                                \
        subzx_st(W(RG), W(RM), W(DP))

/* shl
 * set-flags: undefined */

#define shlwx_ri(RM, IM)                                                    \
        EMITW(0x53000000 | MRM(REG(RM), REG(RM), 0x00) |                    \
        (-VAL(IM)&0x1F)<<16 | (31-(VAL(IM)&0x1F))<<10)

#define shlwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x53000000 | MRM(TMxx,    TMxx,    0x00) |                    \
        (-VAL(IM)&0x1F)<<16 | (31-(VAL(IM)&0x1F))<<10)                      \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shlwx_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        EMITW(0x1AC02000 | MRM(REG(RG), REG(RG), REG(RM)))

#define shlwx_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x1AC02000 | MRM(REG(RG), REG(RG), TMxx))

#define shlwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x1AC02000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shlwx_mr(RM, DP, RG)                                                \
        shlwx_st(W(RG), W(RM), W(DP))

#define shlwx_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0x1AC02000 | MRM(REG(RM), REG(RM), Tecx))

#define shlwx_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x1AC02000 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define shlxx_ri(RM, IM)                                                    \
        EMITW(0xD3400000 | MRM(REG(RM), REG(RM), 0x00) |                    \
        (-VAL(IM)&0x3F)<<16 | (63-(VAL(IM)&0x3F))<<10)

#define shlxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xD3400000 | MRM(TMxx,    TMxx,    0x00) |                    \
        (-VAL(IM)&0x3F)<<16 | (63-(VAL(IM)&0x3F))<<10)                      \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define shlxx_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        EMITW(0x9AC02000 | MRM(REG(RG), REG(RG), REG(RM)))

#define shlxx_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02000 | MRM(REG(RG), REG(RG), TMxx))

#define shlxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define shlxx_mr(RM, DP, RG)                                                \
        shlxx_st(W(RG), W(RM), W(DP))

#define shlxx_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0x9AC02000 | MRM(REG(RM), REG(RM), Tecx))

#define shlxx_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02000 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

/* shr
 * set-flags: undefined */

#define shrwx_ri(RM, IM)                                                    \
        EMITW(0x53007C00 | MRM(REG(RM), REG(RM), 0x00) |(VAL(IM)&0x1F)<<16)

#define shrwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x53007C00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IM)&0x1F)<<16) \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrwx_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        EMITW(0x1AC02400 | MRM(REG(RG), REG(RG), REG(RM)))

#define shrwx_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x1AC02400 | MRM(REG(RG), REG(RG), TMxx))

#define shrwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x1AC02400 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrwx_mr(RM, DP, RG)                                                \
        shrwx_st(W(RG), W(RM), W(DP))

#define shrwx_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0x1AC02400 | MRM(REG(RM), REG(RM), Tecx))

#define shrwx_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x1AC02400 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define shrxx_ri(RM, IM)                                                    \
        EMITW(0xD340FC00 | MRM(REG(RM), REG(RM), 0x00) |(VAL(IM)&0x3F)<<16)

#define shrxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xD340FC00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IM)&0x3F)<<16) \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define shrxx_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        EMITW(0x9AC02400 | MRM(REG(RG), REG(RG), REG(RM)))

#define shrxx_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02400 | MRM(REG(RG), REG(RG), TMxx))

#define shrxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02400 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define shrxx_mr(RM, DP, RG)                                                \
        shrxx_st(W(RG), W(RM), W(DP))

#define shrxx_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0x9AC02400 | MRM(REG(RM), REG(RM), Tecx))

#define shrxx_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02400 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))


#define shrwn_ri(RM, IM)                                                    \
        EMITW(0x13007C00 | MRM(REG(RM), REG(RM), 0x00) |(VAL(IM)&0x1F)<<16)

#define shrwn_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x13007C00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IM)&0x1F)<<16) \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrwn_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        EMITW(0x1AC02800 | MRM(REG(RG), REG(RG), REG(RM)))

#define shrwn_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x1AC02800 | MRM(REG(RG), REG(RG), TMxx))

#define shrwn_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x1AC02800 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrwn_mr(RM, DP, RG)                                                \
        shrwn_st(W(RG), W(RM), W(DP))

#define shrwn_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0x1AC02800 | MRM(REG(RM), REG(RM), Tecx))

#define shrwn_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x1AC02800 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define shrxn_ri(RM, IM)                                                    \
        EMITW(0x9340FC00 | MRM(REG(RM), REG(RM), 0x00) |(VAL(IM)&0x3F)<<16)

#define shrxn_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9340FC00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IM)&0x3F)<<16) \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define shrxn_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        EMITW(0x9AC02800 | MRM(REG(RG), REG(RG), REG(RM)))

#define shrxn_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02800 | MRM(REG(RG), REG(RG), TMxx))

#define shrxn_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02800 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define shrxn_mr(RM, DP, RG)                                                \
        shrxn_st(W(RG), W(RM), W(DP))

#define shrxn_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0x9AC02800 | MRM(REG(RM), REG(RM), Tecx))

#define shrxn_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02800 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

/* mul
 * set-flags: undefined */

#define mulwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x1B007C00 | MRM(REG(RM), REG(RM), TIxx))

#define mulwx_rr(RG, RM)                                                    \
        EMITW(0x1B007C00 | MRM(REG(RG), REG(RG), REG(RM)))

#define mulwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x1B007C00 | MRM(REG(RG), REG(RG), TMxx))


#define mulxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x9B007C00 | MRM(REG(RM), REG(RM), TIxx))

#define mulxx_rr(RG, RM)                                                    \
        EMITW(0x9B007C00 | MRM(REG(RG), REG(RG), REG(RM)))

#define mulxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9B007C00 | MRM(REG(RG), REG(RG), TMxx))


#define mulwx_xr(RM)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0x9BA07C00 | MRM(Teax,    Teax,    REG(RM)))                  \
        EMITW(0xD360FC00 | MRM(Tedx,    Teax,    0x00))

#define mulwx_xm(RM, DP) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9BA07C00 | MRM(Teax,    Teax,    TMxx))                     \
        EMITW(0xD360FC00 | MRM(Tedx,    Teax,    0x00))


#define mulxx_xr(RM)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0x9BC07C00 | MRM(Tedx,    Teax,    REG(RM)))                  \
        EMITW(0x9B007C00 | MRM(Teax,    Teax,    REG(RM)))

#define mulxx_xm(RM, DP) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9BC07C00 | MRM(Tedx,    Teax,    TMxx))                     \
        EMITW(0x9B007C00 | MRM(Teax,    Teax,    TMxx))


#define mulwn_xr(RM)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0x9B207C00 | MRM(Teax,    Teax,    REG(RM)))                  \
        EMITW(0xD360FC00 | MRM(Tedx,    Teax,    0x00))

#define mulwn_xm(RM, DP) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9B207C00 | MRM(Teax,    Teax,    TMxx))                     \
        EMITW(0xD360FC00 | MRM(Tedx,    Teax,    0x00))


#define mulxn_xr(RM)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0x9B407C00 | MRM(Tedx,    Teax,    REG(RM)))                  \
        EMITW(0x9B007C00 | MRM(Teax,    Teax,    REG(RM)))

#define mulxn_xm(RM, DP) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9B407C00 | MRM(Tedx,    Teax,    TMxx))                     \
        EMITW(0x9B007C00 | MRM(Teax,    Teax,    TMxx))

/* div
 * set-flags: undefined */

#define divwx_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x1AC00800 | MRM(REG(RM), REG(RM), TIxx))

#define divwx_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        EMITW(0x1AC00800 | MRM(REG(RG), REG(RG), REG(RM)))

#define divwx_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x1AC00800 | MRM(REG(RG), REG(RG), TMxx))


#define divxx_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x9AC00800 | MRM(REG(RM), REG(RM), TIxx))

#define divxx_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        EMITW(0x9AC00800 | MRM(REG(RG), REG(RG), REG(RM)))

#define divxx_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC00800 | MRM(REG(RG), REG(RG), TMxx))


#define divwn_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x1AC00C00 | MRM(REG(RM), REG(RM), TIxx))

#define divwn_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        EMITW(0x1AC00C00 | MRM(REG(RG), REG(RG), REG(RM)))

#define divwn_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x1AC00C00 | MRM(REG(RG), REG(RG), TMxx))


#define divxn_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x9AC00C00 | MRM(REG(RM), REG(RM), TIxx))

#define divxn_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        EMITW(0x9AC00C00 | MRM(REG(RG), REG(RG), REG(RM)))

#define divxn_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC00C00 | MRM(REG(RG), REG(RG), TMxx))


#define prewx_xx()          /* to be placed immediately prior divwx_x* */   \
                                     /* to prepare Redx for int-divide */

#define prewn_xx()          /* to be placed immediately prior divwn_x* */   \
                                     /* to prepare Redx for int-divide */


#define prexx_xx()          /* to be placed immediately prior divxx_x* */   \
        prewx_xx()                   /* to prepare Redx for int-divide */

#define prexn_xx()          /* to be placed immediately prior divxn_x* */   \
        prewn_xx()                   /* to prepare Redx for int-divide */


#define divwx_xr(RM)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(0x1AC00800 | MRM(Teax,    Teax,    REG(RM)))

#define divwx_xm(RM, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x1AC00800 | MRM(Teax,    Teax,    TMxx))


#define divxx_xr(RM)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(0x9AC00800 | MRM(Teax,    Teax,    REG(RM)))

#define divxx_xm(RM, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC00800 | MRM(Teax,    Teax,    TMxx))


#define divwn_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(0x1AC00C00 | MRM(Teax,    Teax,    REG(RM)))

#define divwn_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x1AC00C00 | MRM(Teax,    Teax,    TMxx))


#define divxn_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(0x9AC00C00 | MRM(Teax,    Teax,    REG(RM)))

#define divxn_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC00C00 | MRM(Teax,    Teax,    TMxx))

/* rem
 * set-flags: undefined */

#define remwx_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movwx_rr(Redx, W(RM))                                               \
        divwx_ri(W(RM), W(IM))                                              \
        EMITW(0x1B008000 | MRM(REG(RM), REG(RM), TIxx) | Tedx << 10)        \
        stack_ld(Redx)

#define remwx_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        stack_st(Redx)                                                      \
        movwx_rr(Redx, W(RG))                                               \
        divwx_rr(W(RG), W(RM))                                              \
        EMITW(0x1B008000 | MRM(REG(RG), REG(RG), REG(RM)) | Tedx << 10)     \
        stack_ld(Redx)

#define remwx_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movwx_rr(Redx, W(RG))                                               \
        divwx_ld(W(RG), W(RM), W(DP))                                       \
        EMITW(0x1B008000 | MRM(REG(RG), REG(RG), TMxx) | Tedx << 10)        \
        stack_ld(Redx)


#define remxx_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movxx_rr(Redx, W(RM))                                               \
        divxx_ri(W(RM), W(IM))                                              \
        EMITW(0x9B008000 | MRM(REG(RM), REG(RM), TIxx) | Tedx << 10)        \
        stack_ld(Redx)

#define remxx_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        stack_st(Redx)                                                      \
        movxx_rr(Redx, W(RG))                                               \
        divxx_rr(W(RG), W(RM))                                              \
        EMITW(0x9B008000 | MRM(REG(RG), REG(RG), REG(RM)) | Tedx << 10)     \
        stack_ld(Redx)

#define remxx_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movxx_rr(Redx, W(RG))                                               \
        divxx_ld(W(RG), W(RM), W(DP))                                       \
        EMITW(0x9B008000 | MRM(REG(RG), REG(RG), TMxx) | Tedx << 10)        \
        stack_ld(Redx)


#define remwn_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movwx_rr(Redx, W(RM))                                               \
        divwn_ri(W(RM), W(IM))                                              \
        EMITW(0x1B008000 | MRM(REG(RM), REG(RM), TIxx) | Tedx << 10)        \
        stack_ld(Redx)

#define remwn_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        stack_st(Redx)                                                      \
        movwx_rr(Redx, W(RG))                                               \
        divwn_rr(W(RG), W(RM))                                              \
        EMITW(0x1B008000 | MRM(REG(RG), REG(RG), REG(RM)) | Tedx << 10)     \
        stack_ld(Redx)

#define remwn_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movwx_rr(Redx, W(RG))                                               \
        divwn_ld(W(RG), W(RM), W(DP))                                       \
        EMITW(0x1B008000 | MRM(REG(RG), REG(RG), TMxx) | Tedx << 10)        \
        stack_ld(Redx)


#define remxn_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movxx_rr(Redx, W(RM))                                               \
        divxn_ri(W(RM), W(IM))                                              \
        EMITW(0x9B008000 | MRM(REG(RM), REG(RM), TIxx) | Tedx << 10)        \
        stack_ld(Redx)

#define remxn_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        stack_st(Redx)                                                      \
        movxx_rr(Redx, W(RG))                                               \
        divxn_rr(W(RG), W(RM))                                              \
        EMITW(0x9B008000 | MRM(REG(RG), REG(RG), REG(RM)) | Tedx << 10)     \
        stack_ld(Redx)

#define remxn_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movxx_rr(Redx, W(RG))                                               \
        divxn_ld(W(RG), W(RM), W(DP))                                       \
        EMITW(0x9B008000 | MRM(REG(RG), REG(RG), TMxx) | Tedx << 10)        \
        stack_ld(Redx)


#define remwx_xx()          /* to be placed immediately prior divwx_x* */   \
        movwx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remwx_xr(RM)        /* to be placed immediately after divwx_xr */   \
        EMITW(0x1B008000 | MRM(Tedx,    Teax,    REG(RM)) | Tedx << 10)     \
                                                          /* Redx<-rem */

#define remwx_xm(RM, DP)    /* to be placed immediately after divwx_xm */   \
        EMITW(0x1B008000 | MRM(Tedx,    Teax,    TMxx) | Tedx << 10)        \
                                                          /* Redx<-rem */


#define remxx_xx()          /* to be placed immediately prior divxx_x* */   \
        movxx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remxx_xr(RM)        /* to be placed immediately after divxx_xr */   \
        EMITW(0x9B008000 | MRM(Tedx,    Teax,    REG(RM)) | Tedx << 10)     \
                                                          /* Redx<-rem */

#define remxx_xm(RM, DP)    /* to be placed immediately after divxx_xm */   \
        EMITW(0x9B008000 | MRM(Tedx,    Teax,    TMxx) | Tedx << 10)        \
                                                          /* Redx<-rem */


#define remwn_xx()          /* to be placed immediately prior divwn_x* */   \
        movwx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remwn_xr(RM)        /* to be placed immediately after divwn_xr */   \
        EMITW(0x1B008000 | MRM(Tedx,    Teax,    REG(RM)) | Tedx << 10)     \
                                                          /* Redx<-rem */

#define remwn_xm(RM, DP)    /* to be placed immediately after divwn_xm */   \
        EMITW(0x1B008000 | MRM(Tedx,    Teax,    TMxx) | Tedx << 10)        \
                                                          /* Redx<-rem */


#define remxn_xx()          /* to be placed immediately prior divxn_x* */   \
        movxx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remxn_xr(RM)        /* to be placed immediately after divxn_xr */   \
        EMITW(0x9B008000 | MRM(Tedx,    Teax,    REG(RM)) | Tedx << 10)     \
                                                          /* Redx<-rem */

#define remxn_xm(RM, DP)    /* to be placed immediately after divxn_xm */   \
        EMITW(0x9B008000 | MRM(Tedx,    Teax,    TMxx) | Tedx << 10)        \
                                                          /* Redx<-rem */

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


#define cmjwx_rz(RM, CC, lb)                                                \
        cmjwx_ri(W(RM), IC(0), CC, lb)

#define cmjwx_mz(RM, DP, CC, lb)                                            \
        cmjwx_mi(W(RM), W(DP), IC(0), CC, lb)

#define cmjwx_ri(RM, IM, CC, lb)                                            \
        cmpwx_ri(W(RM), W(IM))                                              \
        CMJ(CC, lb)

#define cmjwx_mi(RM, DP, IM, CC, lb)                                        \
        cmpwx_mi(W(RM), W(DP), W(IM))                                       \
        CMJ(CC, lb)

#define cmjwx_rr(RG, RM, CC, lb)                                            \
        cmpwx_rr(W(RG), W(RM))                                              \
        CMJ(CC, lb)

#define cmjwx_rm(RG, RM, DP, CC, lb)                                        \
        cmpwx_rm(W(RG), W(RM), W(DP))                                       \
        CMJ(CC, lb)

#define cmjwx_mr(RM, DP, RG, CC, lb)                                        \
        cmpwx_mr(W(RM), W(DP), W(RG))                                       \
        CMJ(CC, lb)

#define CMJ(CC, lb) /* internal macro for combined-compare-jump (cmj) */    \
        CC(lb)


#define cmjxx_rz(RM, CC, lb)                                                \
        cmjxx_ri(W(RM), IC(0), CC, lb)

#define cmjxx_mz(RM, DP, CC, lb)                                            \
        cmjxx_mi(W(RM), W(DP), IC(0), CC, lb)

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

/* cmp
 * set-flags: yes */

#define cmpwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x61000000 | MIM(TZxx,    REG(RM), VAL(IM), T1(IM), M1(IM)))

#define cmpwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x61000000 | MIM(TZxx,    TMxx,    VAL(IM), T1(IM), M1(IM)))

#define cmpwx_rr(RG, RM)                                                    \
        EMITW(0x6B000000 | MRM(TZxx,    REG(RG), REG(RM)))

#define cmpwx_rm(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x6B000000 | MRM(TZxx,    REG(RG), TMxx))

#define cmpwx_mr(RM, DP, RG)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x6B000000 | MRM(TZxx,    TMxx,    REG(RG)))


#define cmpxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0xE1000000 | MIM(TZxx,    REG(RM), VAL(IM), T1(IM), M1(IM)))

#define cmpxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1000000 | MIM(TZxx,    TMxx,    VAL(IM), T1(IM), M1(IM)))

#define cmpxx_rr(RG, RM)                                                    \
        EMITW(0xEB000000 | MRM(TZxx,    REG(RG), REG(RM)))

#define cmpxx_rm(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xEB000000 | MRM(TZxx,    REG(RG), TMxx))

#define cmpxx_mr(RM, DP, RG)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xEB000000 | MRM(TZxx,    TMxx,    REG(RG)))

/* jmp
 * set-flags: no
 * maximum byte-address-range for un/conditional jumps is signed 18/16-bit
 * based on minimum natively-encoded offset across supported targets (u/c)
 * MIPS:18-bit, Power:26-bit, AArch32:26-bit, AArch64:28-bit, x86:32-bit /
 * MIPS:18-bit, Power:16-bit, AArch32:26-bit, AArch64:21-bit, x86:32-bit */

#define jmpxx_rr(RM)           /* register-targeted unconditional jump */   \
        EMITW(0xD61F0000 | MRM(0x00,    REG(RM), 0x00))

#define jmpxx_mm(RM, DP)         /* memory-targeted unconditional jump */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
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
        movwx_mi(Mebp, inf_VER, IB(1)) /* <- NEON to bit0 */

#endif /* RT_RTARCH_A64_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

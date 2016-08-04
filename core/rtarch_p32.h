/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_P32_H
#define RT_RTARCH_P32_H

#define RT_BASE_REGS        16

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_p32.h: Implementation of PowerPC 32-bit BASE instructions.
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
 * cmdxx_rx - applies [cmd] to [r]egister (one-operand cmd)
 * cmdxx_mx - applies [cmd] to [m]emory   (one-operand cmd)
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
 * Regular cmd*x_**, cmd*n_** instructions may or may not set flags depending
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
 *
 * Alternatively, data written natively in C/C++ can be worked on from within
 * a given (one) subset if appropriate offset correction is used from rtarch.h.
 * Mixing of cmdw*_** and cmdx*_** without C/C++ is supported via F definition,
 * but requires two offsets for each field, with F for (w*) and plain for (x*).
 *
 * Argument x-register (implied) is fixed by the implementation.
 * Some formal definitions are not given below to encourage
 * use of friendly aliases for better code readability.
 *
 * Not all canonical forms of BASE instructions have efficient implementation.
 * For example, some forms of shifts and division use stack ops on x86 targets,
 * while standalone remainder operations can only be done natively on MIPS.
 * Consider using special fixed-register forms for maximum performance.
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
#define IW(im)  ((im) & 0xFFFFFFFF), 2, 2       /* only for cmdw*_** set */

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
/**********************************   P32   ***********************************/
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


#define adrxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C3(DP), EMPTY2)   \
        EMITW(0x7C000214 | MRM(REG(RG), MOD(RM), TDxx))

/* and
 * set-flags: undefined (*x), yes (*z) */

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


#if RT_BASE_COMPAT_ZFL == 0 

#define andwz_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x70000000) | (+(TP2(IM) != 0) & 0x7C000039))    \
        /* if true ^ equals to -1 (not 1) */

#define andwz_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x70000000) | (+(TP2(IM) != 0) & 0x7C000039))    \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define andwz_rr(RG, RM)                                                    \
        EMITW(0x7C000039 | MSM(REG(RG), REG(RG), REG(RM)))

#define andwz_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000039 | MSM(REG(RG), REG(RG), TMxx))

#define andwz_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000039 | MSM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#else /* RT_BASE_COMPAT_ZFL */

#define andwz_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x70000000) | (+(TP2(IM) != 0) & 0x7C000038))    \
        /* if true ^ equals to -1 (not 1) */                                \
        EMITW(0x28000000 | REG(RM) << 16)              /* <- set flags (Z) */

#define andwz_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x70000000) | (+(TP2(IM) != 0) & 0x7C000038))    \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#define andwz_rr(RG, RM)                                                    \
        EMITW(0x7C000038 | MSM(REG(RG), REG(RG), REG(RM)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define andwz_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000038 | MSM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define andwz_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000038 | MSM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

/* orr
 * set-flags: undefined (*x), yes (*z) */

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


#define orrwz_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x60000000) | (+(TP2(IM) != 0) & 0x7C000378))    \
        /* if true ^ equals to -1 (not 1) */                                \
        EMITW(0x28000000 | REG(RM) << 16)              /* <- set flags (Z) */

#define orrwz_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x60000000) | (+(TP2(IM) != 0) & 0x7C000378))    \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#if RT_BASE_COMPAT_ZFL == 0 

#define orrwx_rr(RG, RM)                                                    \
        EMITW(0x7C000379 | MSM(REG(RG), REG(RG), REG(RM)))

#define orrwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000379 | MSM(REG(RG), REG(RG), TMxx))

#define orrwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000379 | MSM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#else /* RT_BASE_COMPAT_ZFL */

#define orrwz_rr(RG, RM)                                                    \
        EMITW(0x7C000378 | MSM(REG(RG), REG(RG), REG(RM)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define orrwz_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000378 | MSM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define orrwz_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000378 | MSM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

/* xor
 * set-flags: undefined (*x), yes (*z) */

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


#define xorwz_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x68000000) | (+(TP2(IM) != 0) & 0x7C000278))    \
        /* if true ^ equals to -1 (not 1) */                                \
        EMITW(0x28000000 | REG(RM) << 16)              /* <- set flags (Z) */

#define xorwz_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x68000000) | (+(TP2(IM) != 0) & 0x7C000278))    \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#if RT_BASE_COMPAT_ZFL == 0 

#define xorwx_rr(RG, RM)                                                    \
        EMITW(0x7C000279 | MSM(REG(RG), REG(RG), REG(RM)))

#define xorwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000279 | MSM(REG(RG), REG(RG), TMxx))

#define xorwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000279 | MSM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#else /* RT_BASE_COMPAT_ZFL */

#define xorwz_rr(RG, RM)                                                    \
        EMITW(0x7C000278 | MSM(REG(RG), REG(RG), REG(RM)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define xorwz_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000278 | MSM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define xorwz_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000278 | MSM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

/* not
 * set-flags: no */

#define notwx_rx(RM)                                                        \
        EMITW(0x7C0000F8 | MSM(REG(RM), REG(RM), REG(RM)))

#define notwx_mx(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0000F8 | MSM(TMxx,    TMxx,    TMxx))                     \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* neg
 * set-flags: undefined (*x), yes (*z) */

#define negwx_rx(RM)                                                        \
        EMITW(0x7C0000D0 | MRM(REG(RM), 0x00,    REG(RM)))

#define negwx_mx(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0000D0 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#if RT_BASE_COMPAT_ZFL == 0 

#define negwz_rx(RM)                                                        \
        EMITW(0x7C0000D1 | MRM(REG(RM), 0x00,    REG(RM)))

#define negwz_mx(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0000D1 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#else /* RT_BASE_COMPAT_ZFL */

#define negwz_rx(RM)                                                        \
        EMITW(0x7C0000D0 | MRM(REG(RM), 0x00,    REG(RM)))                  \
        EMITW(0x28000000 | REG(RM) << 16)              /* <- set flags (Z) */

#define negwz_mx(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0000D0 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

/* add
 * set-flags: undefined (*x), yes (*z) */

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


#if RT_BASE_COMPAT_ZFL == 0 

#define addwz_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T1(IM), M1(IM)) | \
        (+(TP1(IM) == 0) & 0x34000000) | (+(TP1(IM) != 0) & 0x7C000215))    \
        /* if true ^ equals to -1 (not 1) */

#define addwz_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T1(IM), M1(IM)) | \
        (+(TP1(IM) == 0) & 0x34000000) | (+(TP1(IM) != 0) & 0x7C000215))    \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define addwz_rr(RG, RM)                                                    \
        EMITW(0x7C000215 | MRM(REG(RG), REG(RG), REG(RM)))

#define addwz_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000215 | MRM(REG(RG), REG(RG), TMxx))

#define addwz_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000215 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#else /* RT_BASE_COMPAT_ZFL */

#define addwz_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), VAL(IM), T1(IM), M1(IM)) | \
        (+(TP1(IM) == 0) & 0x38000000) | (+(TP1(IM) != 0) & 0x7C000214))    \
        /* if true ^ equals to -1 (not 1) */                                \
        EMITW(0x28000000 | REG(RM) << 16)              /* <- set flags (Z) */

#define addwz_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T1(IM), M1(IM)) | \
        (+(TP1(IM) == 0) & 0x38000000) | (+(TP1(IM) != 0) & 0x7C000214))    \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#define addwz_rr(RG, RM)                                                    \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), REG(RM)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define addwz_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define addwz_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000214 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

/* sub
 * set-flags: undefined (*x), yes (*z) */

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


#if RT_BASE_COMPAT_ZFL == 0 

#define subwz_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), 0x00,    T1(IM), EMPTY1) | \
        (+(TP1(IM) == 0) & (0x34000000 | (0xFFFF & -VAL(IM)))) |            \
        (+(TP1(IM) != 0) & (0x7C000051 | TIxx << 16)))                      \
        /* if true ^ equals to -1 (not 1) */

#define subwz_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    0x00,    T1(IM), EMPTY1) | \
        (+(TP1(IM) == 0) & (0x34000000 | (0xFFFF & -VAL(IM)))) |            \
        (+(TP1(IM) != 0) & (0x7C000051 | TIxx << 16)))                      \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define subwz_rr(RG, RM)                                                    \
        EMITW(0x7C000051 | MRM(REG(RG), REG(RG), REG(RM)))

#define subwz_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000051 | MRM(REG(RG), REG(RG), TMxx))

#define subwz_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000051 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define subwz_mr(RM, DP, RG)                                                \
        subwz_st(W(RG), W(RM), W(DP))

#else /* RT_BASE_COMPAT_ZFL */

#define subwz_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(REG(RM), REG(RM), 0x00,    T1(IM), EMPTY1) | \
        (+(TP1(IM) == 0) & (0x38000000 | (0xFFFF & -VAL(IM)))) |            \
        (+(TP1(IM) != 0) & (0x7C000050 | TIxx << 16)))                      \
        /* if true ^ equals to -1 (not 1) */                                \
        EMITW(0x28000000 | REG(RM) << 16)              /* <- set flags (Z) */

#define subwz_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    0x00,    T1(IM), EMPTY1) | \
        (+(TP1(IM) == 0) & (0x38000000 | (0xFFFF & -VAL(IM)))) |            \
        (+(TP1(IM) != 0) & (0x7C000050 | TIxx << 16)))                      \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#define subwz_rr(RG, RM)                                                    \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), REG(RM)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define subwz_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define subwz_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000050 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#define subwz_mr(RM, DP, RG)                                                \
        subwz_st(W(RG), W(RM), W(DP))

#endif /* RT_BASE_COMPAT_ZFL */

/* shl
 * set-flags: undefined (*x), yes (*z) */

#define shlwx_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0x7C000030 | MSM(REG(RM), Tecx,    REG(RM)))

#define shlwx_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000030 | MSM(TMxx,    Tecx,    TMxx))                     \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

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


#define shlwz_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0x7C000031 | MSM(REG(RM), Tecx,    REG(RM)))

#define shlwz_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000031 | MSM(TMxx,    Tecx,    TMxx))                     \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shlwz_ri(RM, IM)                                                    \
        EMITW(0x60000000 | TIxx << 16 | (0x1F & VAL(IM)))                   \
        EMITW(0x7C000031 | MSM(REG(RM), TIxx,    REG(RM)))

#define shlwz_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x60000000 | TIxx << 16 | (0x1F & VAL(IM)))                   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000031 | MSM(TMxx,    TIxx,    TMxx))                     \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shlwz_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000031 | MSM(REG(RG), REG(RM), REG(RG)))

#define shlwz_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000031 | MSM(REG(RG), TMxx,    REG(RG)))

#define shlwz_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000031 | MSM(TMxx,    REG(RG), TMxx))                     \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shlwz_mr(RM, DP, RG)                                                \
        shlwz_st(W(RG), W(RM), W(DP))

/* shr
 * set-flags: undefined (*x), yes (*z) */

#define shrwx_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0x7C000430 | MSM(REG(RM), Tecx,    REG(RM)))

#define shrwx_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000430 | MSM(TMxx,    Tecx,    TMxx))                     \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

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


#define shrwz_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0x7C000431 | MSM(REG(RM), Tecx,    REG(RM)))

#define shrwz_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000431 | MSM(TMxx,    Tecx,    TMxx))                     \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrwz_ri(RM, IM)                                                    \
        EMITW(0x60000000 | TIxx << 16 | (0x1F & VAL(IM)))                   \
        EMITW(0x7C000431 | MSM(REG(RM), TIxx,    REG(RM)))

#define shrwz_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x60000000 | TIxx << 16 | (0x1F & VAL(IM)))                   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000431 | MSM(TMxx,    TIxx,    TMxx))                     \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrwz_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000431 | MSM(REG(RG), REG(RM), REG(RG)))

#define shrwz_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000431 | MSM(REG(RG), TMxx,    REG(RG)))

#define shrwz_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000431 | MSM(TMxx,    REG(RG), TMxx))                     \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrwz_mr(RM, DP, RG)                                                \
        shrwz_st(W(RG), W(RM), W(DP))


#define shrwn_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0x7C000630 | MSM(REG(RM), Tecx,    REG(RM)))

#define shrwn_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000630 | MSM(TMxx,    Tecx,    TMxx))                     \
        EMITW(0x90000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

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


#define mulwx_xr(RM)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0x7C000016 | MRM(Tedx,    Teax,    REG(RM)))                  \
        EMITW(0x7C0001D6 | MRM(Teax,    Teax,    REG(RM)))

#define mulwx_xm(RM, DP) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000016 | MRM(Tedx,    Teax,    TMxx))                     \
        EMITW(0x7C0001D6 | MRM(Teax,    Teax,    TMxx))


#define mulwn_xr(RM)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0x7C000096 | MRM(Tedx,    Teax,    REG(RM)))                  \
        EMITW(0x7C0001D6 | MRM(Teax,    Teax,    REG(RM)))

#define mulwn_xm(RM, DP) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000096 | MRM(Tedx,    Teax,    TMxx))                     \
        EMITW(0x7C0001D6 | MRM(Teax,    Teax,    TMxx))


#define mulwp_xr(RM)     /* Reax is in/out, prepares Redx for divwn_x* */   \
        mulwx_rr(Reax, W(RM)) /* product must not exceed operands size */

#define mulwp_xm(RM, DP) /* Reax is in/out, prepares Redx for divwn_x* */   \
        mulwx_ld(Reax, W(RM), W(DP))  /* must not exceed operands size */

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


#define divwn_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x7C0003D6 | MTM(REG(RM), REG(RM), TIxx))

#define divwn_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        EMITW(0x7C0003D6 | MTM(REG(RG), REG(RG), REG(RM)))

#define divwn_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0003D6 | MTM(REG(RG), REG(RG), TMxx))


#define prewx_xx()          /* to be placed immediately prior divwx_x* */   \
                                     /* to prepare Redx for int-divide */

#define prewn_xx()          /* to be placed immediately prior divwn_x* */   \
                                     /* to prepare Redx for int-divide */


#define divwx_xr(RM)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(0x7C000396 | MTM(Teax,    Teax,    REG(RM)))

#define divwx_xm(RM, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000396 | MTM(Teax,    Teax,    TMxx))


#define divwn_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(0x7C0003D6 | MTM(Teax,    Teax,    REG(RM)))

#define divwn_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0003D6 | MTM(Teax,    Teax,    TMxx))


#define divwp_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xr(W(RM))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divwp_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xm(W(RM), W(DP))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

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


#define remwx_xx()          /* to be placed immediately prior divwx_x* */   \
        movwx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remwx_xr(RM)        /* to be placed immediately after divwx_xr */   \
        EMITW(0x7C0001D6 | MRM(TMxx,    Teax,    REG(RM)))                  \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */

#define remwx_xm(RM, DP)    /* to be placed immediately after divwx_xm */   \
        EMITW(0x7C0001D6 | MRM(TMxx,    Teax,    TMxx))                     \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */


#define remwn_xx()          /* to be placed immediately prior divwn_x* */   \
        movwx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remwn_xr(RM)        /* to be placed immediately after divwn_xr */   \
        EMITW(0x7C0001D6 | MRM(TMxx,    Teax,    REG(RM)))                  \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */

#define remwn_xm(RM, DP)    /* to be placed immediately after divwn_xm */   \
        EMITW(0x7C0001D6 | MRM(TMxx,    Teax,    TMxx))                     \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */

/* arj
 * set-flags: undefined
 * refer to individual instructions' description
 * to stay within special register limitations */

#define and_x   and
#define orr_x   orr
#define xor_x   xor

#define neg_x   neg
#define add_x   add
#define sub_x   sub

#define shl_x   shl
#define shr_x   shr

#define EZ_x    jezxx_lb
#define NZ_x    jnzxx_lb


#define arjwx_rx(RM, op, cc, lb)                                            \
        AR1(W(RM), op, wz_rx)                                               \
        CMJ(cc, lb)

#define arjwx_mx(RM, DP, op, cc, lb)                                        \
        AR2(W(RM), W(DP), op, wz_mx)                                        \
        CMJ(cc, lb)

#define arjwx_ri(RM, IM, op, cc, lb)                                        \
        AR2(W(RM), W(IM), op, wz_ri)                                        \
        CMJ(cc, lb)

#define arjwx_mi(RM, DP, IM, op, cc, lb)                                    \
        AR3(W(RM), W(DP), W(IM), op, wz_mi)                                 \
        CMJ(cc, lb)

#define arjwx_rr(RG, RM, op, cc, lb)                                        \
        AR2(W(RG), W(RM), op, wz_rr)                                        \
        CMJ(cc, lb)

#define arjwx_ld(RG, RM, DP, op, cc, lb)                                    \
        AR3(W(RG), W(RM), W(DP), op, wz_ld)                                 \
        CMJ(cc, lb)

#define arjwx_st(RG, RM, DP, op, cc, lb)                                    \
        AR3(W(RG), W(RM), W(DP), op, wz_st)                                 \
        CMJ(cc, lb)

#define arjwx_mr(RM, DP, RG, op, cc, lb)                                    \
        arjwx_st(W(RG), W(RM), W(DP), op, cc, lb)

/* internal definitions for combined-arithmetic-jump (arj) */

#define AR1(P1, op, sg)                                                     \
        op##sg(W(P1))

#define AR2(P1, P2, op, sg)                                                 \
        op##sg(W(P1), W(P2))

#define AR3(P1, P2, P3, op, sg)                                             \
        op##sg(W(P1), W(P2), W(P3))

#define CMJ(cc, lb)                                                         \
        cc(lb)

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


#define cmjwx_rz(RM, cc, lb)                                                \
        cmjwx_ri(W(RM), IC(0), cc, lb)

#define cmjwx_mz(RM, DP, cc, lb)                                            \
        cmjwx_mi(W(RM), W(DP), IC(0), cc, lb)

#define cmjwx_ri(RM, IM, cc, lb)                                            \
        CWI(cc, MOD(RM), REG(RM), W(IM), lb)

#define cmjwx_mi(RM, DP, IM, cc, lb)                                        \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        CWI(cc, %%r24,   TMxx,    W(IM), lb)

#define cmjwx_rr(RG, RM, cc, lb)                                            \
        CWR(cc, MOD(RG), MOD(RM), lb)

#define cmjwx_rm(RG, RM, DP, cc, lb)                                        \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        CWR(cc, MOD(RG), %%r24,   lb)

#define cmjwx_mr(RM, DP, RG, cc, lb)                                        \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        CWR(cc, %%r24,   MOD(RG), lb)

/* internal definitions for combined-compare-jump (cmj) */

#define IWJ0(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x28000000) | (+(TP2(IM) != 0) & 0x7C000040))    \
        ASM_BEG ASM_OP1(beq,   lb) ASM_END

#define IWJ1(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x28000000) | (+(TP2(IM) != 0) & 0x7C000040))    \
        ASM_BEG ASM_OP1(bne,   lb) ASM_END

#define IWJ2(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x28000000) | (+(TP2(IM) != 0) & 0x7C000040))    \
        ASM_BEG ASM_OP1(blt,   lb) ASM_END

#define IWJ3(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x28000000) | (+(TP2(IM) != 0) & 0x7C000040))    \
        ASM_BEG ASM_OP1(ble,   lb) ASM_END

#define IWJ4(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x28000000) | (+(TP2(IM) != 0) & 0x7C000040))    \
        ASM_BEG ASM_OP1(bgt,   lb) ASM_END

#define IWJ5(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x28000000) | (+(TP2(IM) != 0) & 0x7C000040))    \
        ASM_BEG ASM_OP1(bge,   lb) ASM_END

#define IWJ6(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T3(IM), M3(IM)) | \
        (+(TP2(IM) == 0) & 0x2C000000) | (+(TP2(IM) != 0) & 0x7C000000))    \
        ASM_BEG ASM_OP1(blt,   lb) ASM_END

#define IWJ7(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T3(IM), M3(IM)) | \
        (+(TP2(IM) == 0) & 0x2C000000) | (+(TP2(IM) != 0) & 0x7C000000))    \
        ASM_BEG ASM_OP1(ble,   lb) ASM_END

#define IWJ8(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T3(IM), M3(IM)) | \
        (+(TP2(IM) == 0) & 0x2C000000) | (+(TP2(IM) != 0) & 0x7C000000))    \
        ASM_BEG ASM_OP1(bgt,   lb) ASM_END

#define IWJ9(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T3(IM), M3(IM)) | \
        (+(TP2(IM) == 0) & 0x2C000000) | (+(TP2(IM) != 0) & 0x7C000000))    \
        ASM_BEG ASM_OP1(bge,   lb) ASM_END

#define CWI(cc, r1, p1, IM, lb)                                             \
        IW##cc(r1, p1, W(IM), lb)


#define RWJ0(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmplw, r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(beq,   lb) ASM_END

#define RWJ1(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmplw, r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(bne,   lb) ASM_END

#define RWJ2(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmplw, r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(blt,   lb) ASM_END

#define RWJ3(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmplw, r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(ble,   lb) ASM_END

#define RWJ4(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmplw, r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(bgt,   lb) ASM_END

#define RWJ5(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmplw, r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(bge,   lb) ASM_END

#define RWJ6(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmpw,  r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(blt,   lb) ASM_END

#define RWJ7(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmpw,  r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(ble,   lb) ASM_END

#define RWJ8(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmpw,  r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(bgt,   lb) ASM_END

#define RWJ9(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmpw,  r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(bge,   lb) ASM_END

#define CWR(cc, r1, r2, lb)                                                 \
        RW##cc(r1, r2, lb)


#if RT_ADDRESS == 32

/* cmp
 * set-flags: yes */

#define cmpwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x7C000378 | MSM(TLxx,    REG(RM), REG(RM)))

#define cmpwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TRxx,    MOD(RM), VAL(DP), C1(DP), G3(IM))   \
        EMITW(0x80000000 | MDM(TLxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define cmpwx_rr(RG, RM)                                                    \
        EMITW(0x7C000378 | MSM(TRxx,    REG(RM), REG(RM)))                  \
        EMITW(0x7C000378 | MSM(TLxx,    REG(RG), REG(RG)))

#define cmpwx_rm(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TRxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000378 | MSM(TLxx,    REG(RG), REG(RG)))

#define cmpwx_mr(RM, DP, RG)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TLxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000378 | MSM(TRxx,    REG(RG), REG(RG)))

/***************** pointer-sized instructions for hybrid mode *****************/

/* jmp
 * set-flags: no
 * maximum byte-address-range for un/conditional jumps is signed 18/16-bit
 * based on minimum natively-encoded offset across supported targets (u/c)
 * MIPS:18-bit, Power:26-bit, AArch32:26-bit, AArch64:28-bit, x86:32-bit /
 * MIPS:18-bit, Power:16-bit, AArch32:26-bit, AArch64:21-bit, x86:32-bit */

#define jmpxx_xr(RM)           /* register-targeted unconditional jump */   \
        EMITW(0x7C0003A6 | MRM(REG(RM), 0x00,    0x09)) /* ctr <- reg */    \
        EMITW(0x4C000420 | MTM(0x0C,    0x0A,    0x00)) /* beqctr cr2 */

#if   defined (RT_P32)

#define jmpxx_xm(RM, DP)         /* memory-targeted unconditional jump */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x80000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0003A6 | MRM(TMxx,    0x00,    0x09)) /* ctr <- r24 */    \
        EMITW(0x4C000420 | MTM(0x0C,    0x0A,    0x00)) /* beqctr cr2 */

#elif defined (RT_P64)

#define jmpxx_xm(RM, DP)         /* memory-targeted unconditional jump */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0003A6 | MRM(TMxx,    0x00,    0x09)) /* ctr <- r24 */    \
        EMITW(0x4C000420 | MTM(0x0C,    0x0A,    0x00)) /* beqctr cr2 */

#endif /* defined (RT_P32, RT_P64) */

#define jmpxx_lb(lb)              /* label-targeted unconditional jump */   \
        ASM_BEG ASM_OP1(b, lb) ASM_END

#define jezxx_lb(lb)               /* setting-flags-arithmetic -> jump */   \
        ASM_BEG ASM_OP1(beq,   lb) ASM_END

#define jnzxx_lb(lb)               /* setting-flags-arithmetic -> jump */   \
        ASM_BEG ASM_OP1(bne,   lb) ASM_END

#define jeqxx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmplw, %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(beq,   lb) ASM_END

#define jnexx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmplw, %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(bne,   lb) ASM_END

#define jltxx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmplw, %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(blt,   lb) ASM_END

#define jlexx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmplw, %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(ble,   lb) ASM_END

#define jgtxx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmplw, %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(bgt,   lb) ASM_END

#define jgexx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmplw, %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(bge,   lb) ASM_END

#define jltxn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmpw,  %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(blt,   lb) ASM_END

#define jlexn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmpw,  %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(ble,   lb) ASM_END

#define jgtxn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmpw,  %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(bgt,   lb) ASM_END

#define jgexn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmpw,  %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(bge,   lb) ASM_END

#define LBL(lb)                                          /* code label */   \
        ASM_BEG ASM_OP0(lb:) ASM_END

/* stack
 * set-flags: no
 * always adjust stack pointer with 8-byte (64-bit) steps */

#if defined (RT_P32)

#define stack_st(RM)                                                        \
        EMITW(0x38000000 | MTM(SPxx,    SPxx,    0x00) | (-0x08 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(REG(RM), SPxx,    0x00))

#define stack_ld(RM)                                                        \
        EMITW(0x80000000 | MTM(REG(RM), SPxx,    0x00))                     \
        EMITW(0x38000000 | MTM(SPxx,    SPxx,    0x00) | (+0x08 & 0xFFFF))

#if RT_SIMD_COMPAT_DIV != 0 || RT_SIMD_COMPAT_SQR != 0

#define stack_sa()   /* save all, [Reax - RegE] + 8 temps, 22 regs total */ \
        EMITW(0x38000000 | MTM(SPxx,    SPxx,    0x00) | (-0x60 & 0xFFFF))  \
        EMITW(0xD8000000 | MTM(Tff1,    SPxx,    0x00) | (+0x00 & 0xFFFF))  \
        EMITW(0xD8000000 | MTM(Tff2,    SPxx,    0x00) | (+0x08 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(Teax,    SPxx,    0x00) | (+0x10 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(Tecx,    SPxx,    0x00) | (+0x14 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(Tedx,    SPxx,    0x00) | (+0x18 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(Tebx,    SPxx,    0x00) | (+0x1C & 0xFFFF))  \
        EMITW(0x90000000 | MTM(Tebp,    SPxx,    0x00) | (+0x20 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(Tesi,    SPxx,    0x00) | (+0x24 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(Tedi,    SPxx,    0x00) | (+0x28 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(Teg8,    SPxx,    0x00) | (+0x2C & 0xFFFF))  \
        EMITW(0x90000000 | MTM(Teg9,    SPxx,    0x00) | (+0x30 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(TegA,    SPxx,    0x00) | (+0x34 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(TegB,    SPxx,    0x00) | (+0x38 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(TegC,    SPxx,    0x00) | (+0x3C & 0xFFFF))  \
        EMITW(0x90000000 | MTM(TegD,    SPxx,    0x00) | (+0x40 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(TegE,    SPxx,    0x00) | (+0x44 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(TMxx,    SPxx,    0x00) | (+0x48 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(TIxx,    SPxx,    0x00) | (+0x4C & 0xFFFF))  \
        EMITW(0x90000000 | MTM(TPxx,    SPxx,    0x00) | (+0x50 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(TCxx,    SPxx,    0x00) | (+0x54 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(TVxx,    SPxx,    0x00) | (+0x58 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(TZxx,    SPxx,    0x00) | (+0x5C & 0xFFFF))

#define stack_la()   /* load all, 8 temps + [RegE - Reax], 22 regs total */ \
        EMITW(0x80000000 | MTM(TZxx,    SPxx,    0x00) | (+0x5C & 0xFFFF))  \
        EMITW(0x80000000 | MTM(TVxx,    SPxx,    0x00) | (+0x58 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(TCxx,    SPxx,    0x00) | (+0x54 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(TPxx,    SPxx,    0x00) | (+0x50 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(TIxx,    SPxx,    0x00) | (+0x4C & 0xFFFF))  \
        EMITW(0x80000000 | MTM(TMxx,    SPxx,    0x00) | (+0x48 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(TegE,    SPxx,    0x00) | (+0x44 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(TegD,    SPxx,    0x00) | (+0x40 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(TegC,    SPxx,    0x00) | (+0x3C & 0xFFFF))  \
        EMITW(0x80000000 | MTM(TegB,    SPxx,    0x00) | (+0x38 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(TegA,    SPxx,    0x00) | (+0x34 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(Teg9,    SPxx,    0x00) | (+0x30 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(Teg8,    SPxx,    0x00) | (+0x2C & 0xFFFF))  \
        EMITW(0x80000000 | MTM(Tedi,    SPxx,    0x00) | (+0x28 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(Tesi,    SPxx,    0x00) | (+0x24 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(Tebp,    SPxx,    0x00) | (+0x20 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(Tebx,    SPxx,    0x00) | (+0x1C & 0xFFFF))  \
        EMITW(0x80000000 | MTM(Tedx,    SPxx,    0x00) | (+0x18 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(Tecx,    SPxx,    0x00) | (+0x14 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(Teax,    SPxx,    0x00) | (+0x10 & 0xFFFF))  \
        EMITW(0xC8000000 | MTM(Tff2,    SPxx,    0x00) | (+0x08 & 0xFFFF))  \
        EMITW(0xC8000000 | MTM(Tff1,    SPxx,    0x00) | (+0x00 & 0xFFFF))  \
        EMITW(0x38000000 | MTM(SPxx,    SPxx,    0x00) | (+0x60 & 0xFFFF))

#else /* RT_SIMD_COMPAT_DIV != 0 || RT_SIMD_COMPAT_SQR != 0 */

#define stack_sa()   /* save all, [Reax - RegE] + 6 temps, 20 regs total */ \
        EMITW(0x38000000 | MTM(SPxx,    SPxx,    0x00) | (-0x50 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(Teax,    SPxx,    0x00) | (+0x00 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(Tecx,    SPxx,    0x00) | (+0x04 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(Tedx,    SPxx,    0x00) | (+0x08 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(Tebx,    SPxx,    0x00) | (+0x0C & 0xFFFF))  \
        EMITW(0x90000000 | MTM(Tebp,    SPxx,    0x00) | (+0x10 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(Tesi,    SPxx,    0x00) | (+0x14 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(Tedi,    SPxx,    0x00) | (+0x18 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(Teg8,    SPxx,    0x00) | (+0x1C & 0xFFFF))  \
        EMITW(0x90000000 | MTM(Teg9,    SPxx,    0x00) | (+0x20 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(TegA,    SPxx,    0x00) | (+0x24 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(TegB,    SPxx,    0x00) | (+0x28 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(TegC,    SPxx,    0x00) | (+0x2C & 0xFFFF))  \
        EMITW(0x90000000 | MTM(TegD,    SPxx,    0x00) | (+0x30 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(TegE,    SPxx,    0x00) | (+0x34 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(TMxx,    SPxx,    0x00) | (+0x38 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(TIxx,    SPxx,    0x00) | (+0x3C & 0xFFFF))  \
        EMITW(0x90000000 | MTM(TPxx,    SPxx,    0x00) | (+0x40 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(TCxx,    SPxx,    0x00) | (+0x44 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(TVxx,    SPxx,    0x00) | (+0x48 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(TZxx,    SPxx,    0x00) | (+0x4C & 0xFFFF))

#define stack_la()   /* load all, 6 temps + [RegE - Reax], 20 regs total */ \
        EMITW(0x80000000 | MTM(TZxx,    SPxx,    0x00) | (+0x4C & 0xFFFF))  \
        EMITW(0x80000000 | MTM(TVxx,    SPxx,    0x00) | (+0x48 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(TCxx,    SPxx,    0x00) | (+0x44 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(TPxx,    SPxx,    0x00) | (+0x40 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(TIxx,    SPxx,    0x00) | (+0x3C & 0xFFFF))  \
        EMITW(0x80000000 | MTM(TMxx,    SPxx,    0x00) | (+0x38 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(TegE,    SPxx,    0x00) | (+0x34 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(TegD,    SPxx,    0x00) | (+0x30 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(TegC,    SPxx,    0x00) | (+0x2C & 0xFFFF))  \
        EMITW(0x80000000 | MTM(TegB,    SPxx,    0x00) | (+0x28 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(TegA,    SPxx,    0x00) | (+0x24 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(Teg9,    SPxx,    0x00) | (+0x20 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(Teg8,    SPxx,    0x00) | (+0x1C & 0xFFFF))  \
        EMITW(0x80000000 | MTM(Tedi,    SPxx,    0x00) | (+0x18 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(Tesi,    SPxx,    0x00) | (+0x14 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(Tebp,    SPxx,    0x00) | (+0x10 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(Tebx,    SPxx,    0x00) | (+0x0C & 0xFFFF))  \
        EMITW(0x80000000 | MTM(Tedx,    SPxx,    0x00) | (+0x08 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(Tecx,    SPxx,    0x00) | (+0x04 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(Teax,    SPxx,    0x00) | (+0x00 & 0xFFFF))  \
        EMITW(0x38000000 | MTM(SPxx,    SPxx,    0x00) | (+0x50 & 0xFFFF))

#endif /* RT_SIMD_COMPAT_DIV != 0 || RT_SIMD_COMPAT_SQR != 0 */

#endif  /* defined (RT_P32) */

#endif /* RT_ADDRESS */


#if defined (RT_P64)

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

#endif /* defined (RT_P64) */

/* ver
 * set-flags: no */

#define verxx_xx() /* destroys Reax, Recx, Rebx, Redx, Resi, Redi (in x86)*/\
        movwx_mi(Mebp, inf_VER, IB(3)) /* <- VMX, VSX to bit0, bit1 */

#endif /* RT_RTARCH_P32_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

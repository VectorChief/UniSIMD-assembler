/******************************************************************************/
/* Copyright (c) 2013-2017 VectorChief (at github, bitbucket, sourceforge)    */
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
 * cmdy*_** - applies [cmd] to L-size BASE register/memory/immediate args
 * cmdz*_** - applies [cmd] to 64-bit BASE register/memory/immediate args
 *
 * cmd*x_** - applies [cmd] to unsigned integer args, [x] - default
 * cmd*n_** - applies [cmd] to   signed integer args, [n] - negatable
 * cmd*p_** - applies [cmd] to   signed integer args, [p] - part-range
 *
 * cmd*z_** - applies [cmd] while setting condition flags, [z] - zero flag.
 * Regular cmd*x_**, cmd*n_** instructions may or may not set flags depending
 * on the target architecture, thus no assumptions can be made for jezxx/jnzxx.
 *
 * Interpretation of instruction parameters:
 *
 * upper-case params have triplet structure and require W to pass-forward
 * lower-case params are singular and can be used/passed as such directly
 *
 * RD - BASE register serving as destination only, if present
 * RG - BASE register serving as destination and fisrt source
 * RS - BASE register serving as second source (first if any)
 * RT - BASE register serving as third source (second if any)
 *
 * MD - BASE addressing mode (Oeax, M***, I***) (memory-dest)
 * MG - BASE addressing mode (Oeax, M***, I***) (memory-dsrc)
 * MS - BASE addressing mode (Oeax, M***, I***) (memory-src2)
 * MT - BASE addressing mode (Oeax, M***, I***) (memory-src3)
 *
 * DD - displacement value (DP, DF, DG, DH, DV) (memory-dest)
 * DG - displacement value (DP, DF, DG, DH, DV) (memory-dsrc)
 * DS - displacement value (DP, DF, DG, DH, DV) (memory-src2)
 * DT - displacement value (DP, DF, DG, DH, DV) (memory-src3)
 *
 * IS - immediate value (is used as a second or first source)
 * IT - immediate value (is used as a third or second source)
 *
 * Alphabetical view of current/future instruction namespaces is in rtzero.h.
 * Configurable BASE/SIMD subsets (cmdx*, cmdy*, cmdp*) are defined in rtbase.h.
 * Mixing of 64/32-bit fields in backend structures may lead to misalignment
 * of 64-bit fields to 4-byte boundary, which is not supported on some targets.
 * Place fields carefully to ensure natural alignment for all data types.
 * Note that within cmdx*_** subset most of the instructions follow in-heap
 * address size (RT_ADDRESS or A) and only label_ld/st, jmpxx_xr/xm follow
 * pointer size (RT_POINTER or P) as code/data/stack segments are fixed.
 * Stack ops always work with full registers regardless of the mode chosen.
 *
 * 32-bit and 64-bit BASE subsets are not easily compatible on all targets,
 * thus any register modified with 32-bit op cannot be used in 64-bit subset.
 * Alternatively, data flow must not exceed 31-bit range for 32-bit operations
 * to produce consistent results usable in 64-bit subsets across all targets.
 * Registers written with 64-bit op aren't always compatible with 32-bit either,
 * as m64 requires the upper half to be all 0s or all 1s for m32 arithmetic.
 * Only a64 and x64 have a complete 32-bit support in 64-bit mode both zeroing
 * the upper half of the result, while m64 sign-extending all 32-bit operations
 * and p64 overflowing 32-bit arithmetic into the upper half. Similar reasons
 * of inconsistency prohibit use of IW immediate type within 64-bit subsets,
 * where a64 and p64 zero-extend, while x64 and m64 sign-extend 32-bit value.
 *
 * Note that offset correction for endianness E is only applicable for addresses
 * within pointer fields, when (in-heap) address and pointer sizes don't match.
 * Working with 32-bit data in 64-bit fields in any other circumstances must be
 * done consistently within a subset of one size (32-bit, 64-bit or C/C++).
 * Alternatively, data written natively in C/C++ can be worked on from within
 * a given (one) subset if appropriate offset correction is used from rtarch.h.
 *
 * Setting-flags instruction naming scheme may change again in the future for
 * better orthogonality with operand size, type and args-list. It is therefore
 * recommended to use combined-arithmetic-jump (arj) for better API stability
 * and maximum efficiency across all supported targets. For similar reasons
 * of higher performance on MIPS and Power use combined-compare-jump (cmj).
 * Not all canonical forms of BASE instructions have efficient implementation.
 * For example, some forms of shifts and division use stack ops on x86 targets,
 * while standalone remainder operations can only be done natively on MIPS.
 * Consider using special fixed-register forms for maximum performance.
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
        ((rem) << 16 | (ren) << 21 | (reg) << 11)

#define MSM(reg, ren, rem) /* shifts */                                     \
        ((rem) << 21 | (ren) << 16 | (reg) << 11)

#define MDM(reg, brm, vdp, bxx, pxx)                                        \
        (pxx(vdp) | bxx(brm) << 21 | (reg) << 16)

#define MIM(reg, ren, vim, txx, mxx)                                        \
        (mxx(vim) |    (ren) << 21 | txx(reg))

#define AUW(sib, vim, reg, brm, vdp, cdp, cim)                              \
            sib  cdp(brm, vdp)  cim(reg, vim)

#define ADR ((A-1)*0x0000000C)

#define EMPTY1(em1) em1
#define EMPTY2(em1, em2) em1 em2

/* selectors  */

#define REG(reg, mod, sib)  reg
#define RYG(reg, mod, sib)  ((reg) + 16)
#define MOD(reg, mod, sib)  mod
#define SIB(reg, mod, sib)  sib

#define VAL(val, tp1, tp2)  val
#define VYL(val, tp1, tp2)  ((val) | 0x10)
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
                    EMITW(0x00000021 | MRM(TPxx,    (br),    TDxx) | ADR)
#define C31(br, dp) EMITW(0x34000000 | TDxx << 16 | (0xFFFC & (dp)))

#define B12(br) TPxx
#define P12(dp) (0x00000000)
#define C12(br, dp) C32(br, dp)                                             \
                    EMITW(0x00000021 | MRM(TPxx,    (br),    TDxx) | ADR)
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
#define TMxx    0x18  /* t8 (r24) */
#define TIxx    0x19  /* t9 (r25), not used at the same time with TDxx */
#define TDxx    0x12  /* s2 (r18), not used at the same time with TIxx */
#define TPxx    0x13  /* s3 (r19) */
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

#define Iecx    Tecx, TPxx, EMITW(0x00000021 | MRM(TPxx, Tecx, Teax) | ADR)
#define Iedx    Tedx, TPxx, EMITW(0x00000021 | MRM(TPxx, Tedx, Teax) | ADR)
#define Iebx    Tebx, TPxx, EMITW(0x00000021 | MRM(TPxx, Tebx, Teax) | ADR)
#define Iebp    Tebp, TPxx, EMITW(0x00000021 | MRM(TPxx, Tebp, Teax) | ADR)
#define Iesi    Tesi, TPxx, EMITW(0x00000021 | MRM(TPxx, Tesi, Teax) | ADR)
#define Iedi    Tedi, TPxx, EMITW(0x00000021 | MRM(TPxx, Tedi, Teax) | ADR)
#define Ieg8    Teg8, TPxx, EMITW(0x00000021 | MRM(TPxx, Teg8, Teax) | ADR)
#define Ieg9    Teg9, TPxx, EMITW(0x00000021 | MRM(TPxx, Teg9, Teax) | ADR)
#define IegA    TegA, TPxx, EMITW(0x00000021 | MRM(TPxx, TegA, Teax) | ADR)
#define IegB    TegB, TPxx, EMITW(0x00000021 | MRM(TPxx, TegB, Teax) | ADR)
#define IegC    TegC, TPxx, EMITW(0x00000021 | MRM(TPxx, TegC, Teax) | ADR)
#define IegD    TegD, TPxx, EMITW(0x00000021 | MRM(TPxx, TegD, Teax) | ADR)
#define IegE    TegE, TPxx, EMITW(0x00000021 | MRM(TPxx, TegE, Teax) | ADR)

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

/* mov (D = S)
 * set-flags: no */

#define movwx_ri(RD, IS)                                                    \
        AUW(EMPTY,    VAL(IS), REG(RD), EMPTY,   EMPTY,   EMPTY2, G3(IS))

#define movwx_mi(MD, DD, IS)                                                \
        AUW(SIB(MD),  VAL(IS), TDxx,    MOD(MD), VAL(DD), C1(DD), G3(IS))   \
        EMITW(0xAC000000 | MDM(TDxx,    MOD(MD), VAL(DD), B1(DD), P1(DD)))

#define movwx_rr(RD, RS)                                                    \
        EMITW(0x00000025 | MRM(REG(RD), REG(RS), TZxx))

#define movwx_ld(RD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(REG(RD), MOD(MS), VAL(DS), B1(DS), P1(DS)))

#define movwx_st(RS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0xAC000000 | MDM(REG(RS), MOD(MD), VAL(DD), B1(DD), P1(DD)))


#define movwx_rj(RD, IT, IS)     /* IT - upper 32-bit, IS - lower 32-bit */ \
        movwx_ri(W(RD), W(IS))

#define movwx_mj(MD, DD, IT, IS) /* IT - upper 32-bit, IS - lower 32-bit */ \
        movwx_mi(W(MD), W(DD), W(IS))

/* and (G = G & S)
 * set-flags: undefined (*x), yes (*z) */

#define andwx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (+(TP2(IS) == 0) & 0x30000000) | (+(TP2(IS) != 0) & 0x00000024))    \
        /* if true ^ equals to -1 (not 1) */

#define andwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (+(TP2(IS) == 0) & 0x30000000) | (+(TP2(IS) != 0) & 0x00000024))    \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define andwx_rr(RG, RS)                                                    \
        EMITW(0x00000024 | MRM(REG(RG), REG(RG), REG(RS)))

#define andwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x00000024 | MRM(REG(RG), REG(RG), TMxx))

#define andwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000024 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define andwx_mr(MG, DG, RS)                                                \
        andwx_st(W(RS), W(MG), W(DG))


#define andwz_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (+(TP2(IS) == 0) & 0x30000000) | (+(TP2(IS) != 0) & 0x00000024))    \
        /* if true ^ equals to -1 (not 1) */                                \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define andwz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (+(TP2(IS) == 0) & 0x30000000) | (+(TP2(IS) != 0) & 0x00000024))    \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define andwz_rr(RG, RS)                                                    \
        EMITW(0x00000024 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define andwz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x00000024 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define andwz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000024 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define andwz_mr(MG, DG, RS)                                                \
        andwz_st(W(RS), W(MG), W(DG))

/* ann (G = ~G & S)
 * set-flags: undefined (*x), yes (*z) */

#define annwx_ri(RG, IS)                                                    \
        notwx_rx(W(RG))                                                     \
        andwx_ri(W(RG), W(IS))

#define annwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000027 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (+(TP2(IS) == 0) & 0x30000000) | (+(TP2(IS) != 0) & 0x00000024))    \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define annwx_rr(RG, RS)                                                    \
        notwx_rx(W(RG))                                                     \
        andwx_rr(W(RG), W(RS))

#define annwx_ld(RG, MS, DS)                                                \
        notwx_rx(W(RG))                                                     \
        andwx_ld(W(RG), W(MS), W(DS))

#define annwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000027 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x00000024 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define annwx_mr(MG, DG, RS)                                                \
        annwx_st(W(RS), W(MG), W(DG))


#define annwz_ri(RG, IS)                                                    \
        notwx_rx(W(RG))                                                     \
        andwz_ri(W(RG), W(IS))

#define annwz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000027 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (+(TP2(IS) == 0) & 0x30000000) | (+(TP2(IS) != 0) & 0x00000024))    \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define annwz_rr(RG, RS)                                                    \
        notwx_rx(W(RG))                                                     \
        andwz_rr(W(RG), W(RS))

#define annwz_ld(RG, MS, DS)                                                \
        notwx_rx(W(RG))                                                     \
        andwz_ld(W(RG), W(MS), W(DS))

#define annwz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000027 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x00000024 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define annwz_mr(MG, DG, RS)                                                \
        annwz_st(W(RS), W(MG), W(DG))

/* orr (G = G | S)
 * set-flags: undefined (*x), yes (*z) */

#define orrwx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (+(TP2(IS) == 0) & 0x34000000) | (+(TP2(IS) != 0) & 0x00000025))    \
        /* if true ^ equals to -1 (not 1) */

#define orrwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (+(TP2(IS) == 0) & 0x34000000) | (+(TP2(IS) != 0) & 0x00000025))    \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define orrwx_rr(RG, RS)                                                    \
        EMITW(0x00000025 | MRM(REG(RG), REG(RG), REG(RS)))

#define orrwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x00000025 | MRM(REG(RG), REG(RG), TMxx))

#define orrwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000025 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define orrwx_mr(MG, DG, RS)                                                \
        orrwx_st(W(RS), W(MG), W(DG))


#define orrwz_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (+(TP2(IS) == 0) & 0x34000000) | (+(TP2(IS) != 0) & 0x00000025))    \
        /* if true ^ equals to -1 (not 1) */                                \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define orrwz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (+(TP2(IS) == 0) & 0x34000000) | (+(TP2(IS) != 0) & 0x00000025))    \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define orrwz_rr(RG, RS)                                                    \
        EMITW(0x00000025 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define orrwz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x00000025 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define orrwz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000025 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define orrwz_mr(MG, DG, RS)                                                \
        orrwz_st(W(RS), W(MG), W(DG))

/* orn (G = ~G | S)
 * set-flags: undefined (*x), yes (*z) */

#define ornwx_ri(RG, IS)                                                    \
        notwx_rx(W(RG))                                                     \
        orrwx_ri(W(RG), W(IS))

#define ornwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000027 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (+(TP2(IS) == 0) & 0x34000000) | (+(TP2(IS) != 0) & 0x00000025))    \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define ornwx_rr(RG, RS)                                                    \
        notwx_rx(W(RG))                                                     \
        orrwx_rr(W(RG), W(RS))

#define ornwx_ld(RG, MS, DS)                                                \
        notwx_rx(W(RG))                                                     \
        orrwx_ld(W(RG), W(MS), W(DS))

#define ornwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000027 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x00000025 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define ornwx_mr(MG, DG, RS)                                                \
        ornwx_st(W(RS), W(MG), W(DG))


#define ornwz_ri(RG, IS)                                                    \
        notwx_rx(W(RG))                                                     \
        orrwz_ri(W(RG), W(IS))

#define ornwz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000027 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (+(TP2(IS) == 0) & 0x34000000) | (+(TP2(IS) != 0) & 0x00000025))    \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define ornwz_rr(RG, RS)                                                    \
        notwx_rx(W(RG))                                                     \
        orrwz_rr(W(RG), W(RS))

#define ornwz_ld(RG, MS, DS)                                                \
        notwx_rx(W(RG))                                                     \
        orrwz_ld(W(RG), W(MS), W(DS))

#define ornwz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000027 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x00000025 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define ornwz_mr(MG, DG, RS)                                                \
        ornwz_st(W(RS), W(MG), W(DG))

/* xor (G = G ^ S)
 * set-flags: undefined (*x), yes (*z) */

#define xorwx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (+(TP2(IS) == 0) & 0x38000000) | (+(TP2(IS) != 0) & 0x00000026))    \
        /* if true ^ equals to -1 (not 1) */

#define xorwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (+(TP2(IS) == 0) & 0x38000000) | (+(TP2(IS) != 0) & 0x00000026))    \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define xorwx_rr(RG, RS)                                                    \
        EMITW(0x00000026 | MRM(REG(RG), REG(RG), REG(RS)))

#define xorwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x00000026 | MRM(REG(RG), REG(RG), TMxx))

#define xorwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000026 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define xorwx_mr(MG, DG, RS)                                                \
        xorwx_st(W(RS), W(MG), W(DG))


#define xorwz_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (+(TP2(IS) == 0) & 0x38000000) | (+(TP2(IS) != 0) & 0x00000026))    \
        /* if true ^ equals to -1 (not 1) */                                \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define xorwz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (+(TP2(IS) == 0) & 0x38000000) | (+(TP2(IS) != 0) & 0x00000026))    \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define xorwz_rr(RG, RS)                                                    \
        EMITW(0x00000026 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define xorwz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x00000026 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define xorwz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000026 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define xorwz_mr(MG, DG, RS)                                                \
        xorwz_st(W(RS), W(MG), W(DG))

/* not (G = ~G)
 * set-flags: no */

#define notwx_rx(RG)                                                        \
        EMITW(0x00000027 | MRM(REG(RG), TZxx,    REG(RG)))

#define notwx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TDxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000027 | MRM(TDxx,    TZxx,    TDxx))                     \
        EMITW(0xAC000000 | MDM(TDxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

/* neg (G = -G)
 * set-flags: undefined (*x), yes (*z) */

#define negwx_rx(RG)                                                        \
        EMITW(0x00000023 | MRM(REG(RG), TZxx,    REG(RG)))

#define negwx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000023 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))


#define negwz_rx(RG)                                                        \
        EMITW(0x00000023 | MRM(REG(RG), TZxx,    REG(RG)))                  \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define negwz_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000023 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

/* add (G = G + S)
 * set-flags: undefined (*x), yes (*z) */

#define addwx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)) | \
        (+(TP1(IS) == 0) & 0x24000000) | (+(TP1(IS) != 0) & 0x00000021))    \
        /* if true ^ equals to -1 (not 1) */

#define addwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)) | \
        (+(TP1(IS) == 0) & 0x24000000) | (+(TP1(IS) != 0) & 0x00000021))    \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define addwx_rr(RG, RS)                                                    \
        EMITW(0x00000021 | MRM(REG(RG), REG(RG), REG(RS)))

#define addwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x00000021 | MRM(REG(RG), REG(RG), TMxx))

#define addwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000021 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define addwx_mr(MG, DG, RS)                                                \
        addwx_st(W(RS), W(MG), W(DG))


#define addwz_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)) | \
        (+(TP1(IS) == 0) & 0x24000000) | (+(TP1(IS) != 0) & 0x00000021))    \
        /* if true ^ equals to -1 (not 1) */                                \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define addwz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)) | \
        (+(TP1(IS) == 0) & 0x24000000) | (+(TP1(IS) != 0) & 0x00000021))    \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define addwz_rr(RG, RS)                                                    \
        EMITW(0x00000021 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define addwz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x00000021 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define addwz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000021 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define addwz_mr(MG, DG, RS)                                                \
        addwz_st(W(RS), W(MG), W(DG))

/* sub (G = G - S)
 * set-flags: undefined (*x), yes (*z) */

#define subwx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), 0x00,    T1(IS), EMPTY1) | \
        (+(TP1(IS) == 0) & (0x24000000 | (0xFFFF & -VAL(IS)))) |            \
        (+(TP1(IS) != 0) & (0x00000023 | TIxx << 16)))                      \
        /* if true ^ equals to -1 (not 1) */

#define subwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    0x00,    T1(IS), EMPTY1) | \
        (+(TP1(IS) == 0) & (0x24000000 | (0xFFFF & -VAL(IS)))) |            \
        (+(TP1(IS) != 0) & (0x00000023 | TIxx << 16)))                      \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define subwx_rr(RG, RS)                                                    \
        EMITW(0x00000023 | MRM(REG(RG), REG(RG), REG(RS)))

#define subwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x00000023 | MRM(REG(RG), REG(RG), TMxx))

#define subwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000023 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define subwx_mr(MG, DG, RS)                                                \
        subwx_st(W(RS), W(MG), W(DG))


#define subwz_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), 0x00,    T1(IS), EMPTY1) | \
        (+(TP1(IS) == 0) & (0x24000000 | (0xFFFF & -VAL(IS)))) |            \
        (+(TP1(IS) != 0) & (0x00000023 | TIxx << 16)))                      \
        /* if true ^ equals to -1 (not 1) */                                \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define subwz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    0x00,    T1(IS), EMPTY1) | \
        (+(TP1(IS) == 0) & (0x24000000 | (0xFFFF & -VAL(IS)))) |            \
        (+(TP1(IS) != 0) & (0x00000023 | TIxx << 16)))                      \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define subwz_rr(RG, RS)                                                    \
        EMITW(0x00000023 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define subwz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x00000023 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define subwz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000023 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define subwz_mr(MG, DG, RS)                                                \
        subwz_st(W(RS), W(MG), W(DG))

/* shl (G = G << S)
 * set-flags: undefined (*x), yes (*z) */

#define shlwx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x00000004 | MSM(REG(RG), REG(RG), Tecx))

#define shlwx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000004 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define shlwx_ri(RG, IS)                                                    \
        EMITW(0x00000000 | MSM(REG(RG), REG(RG), 0x00) |                    \
                                                 (0x1F & VAL(IS)) << 6)

#define shlwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000000 | MSM(TMxx,    TMxx,    0x00) |                    \
                                                 (0x1F & VAL(IS)) << 6)     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define shlwx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x00000004 | MSM(REG(RG), REG(RG), REG(RS)))

#define shlwx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x00000004 | MSM(REG(RG), REG(RG), TMxx))

#define shlwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000004 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define shlwx_mr(MG, DG, RS)                                                \
        shlwx_st(W(RS), W(MG), W(DG))


#define shlwz_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x00000004 | MSM(REG(RG), REG(RG), Tecx))                     \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define shlwz_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000004 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define shlwz_ri(RG, IS)                                                    \
        EMITW(0x00000000 | MSM(REG(RG), REG(RG), 0x00) |                    \
                                                 (0x1F & VAL(IS)) << 6)     \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define shlwz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000000 | MSM(TMxx,    TMxx,    0x00) |                    \
                                                 (0x1F & VAL(IS)) << 6)     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define shlwz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x00000004 | MSM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define shlwz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x00000004 | MSM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define shlwz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000004 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define shlwz_mr(MG, DG, RS)                                                \
        shlwz_st(W(RS), W(MG), W(DG))

/* shr (G = G >> S)
 * set-flags: undefined (*x), yes (*z) */

#define shrwx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x00000006 | MSM(REG(RG), REG(RG), Tecx))

#define shrwx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000006 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define shrwx_ri(RG, IS)                                                    \
        EMITW(0x00000002 | MSM(REG(RG), REG(RG), 0x00) |                    \
                                                 (0x1F & VAL(IS)) << 6)

#define shrwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000002 | MSM(TMxx,    TMxx,    0x00) |                    \
                                                 (0x1F & VAL(IS)) << 6)     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define shrwx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x00000006 | MSM(REG(RG), REG(RG), REG(RS)))

#define shrwx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x00000006 | MSM(REG(RG), REG(RG), TMxx))

#define shrwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000006 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define shrwx_mr(MG, DG, RS)                                                \
        shrwx_st(W(RS), W(MG), W(DG))


#define shrwz_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x00000006 | MSM(REG(RG), REG(RG), Tecx))                     \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define shrwz_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000006 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define shrwz_ri(RG, IS)                                                    \
        EMITW(0x00000002 | MSM(REG(RG), REG(RG), 0x00) |                    \
                                                 (0x1F & VAL(IS)) << 6)     \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define shrwz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000002 | MSM(TMxx,    TMxx,    0x00) |                    \
                                                 (0x1F & VAL(IS)) << 6)     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define shrwz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x00000006 | MSM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define shrwz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x00000006 | MSM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define shrwz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000006 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define shrwz_mr(MG, DG, RS)                                                \
        shrwz_st(W(RS), W(MG), W(DG))


#define shrwn_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x00000007 | MSM(REG(RG), REG(RG), Tecx))

#define shrwn_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000007 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define shrwn_ri(RG, IS)                                                    \
        EMITW(0x00000003 | MSM(REG(RG), REG(RG), 0x00) |                    \
                                                 (0x1F & VAL(IS)) << 6)

#define shrwn_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000003 | MSM(TMxx,    TMxx,    0x00) |                    \
                                                 (0x1F & VAL(IS)) << 6)     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define shrwn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x00000007 | MSM(REG(RG), REG(RG), REG(RS)))

#define shrwn_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x00000007 | MSM(REG(RG), REG(RG), TMxx))

#define shrwn_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000007 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define shrwn_mr(MG, DG, RS)                                                \
        shrwn_st(W(RS), W(MG), W(DG))

/* ror (G = G >> S | G << 32 - S)
 * set-flags: undefined (*x), yes (*z) */

#define rorwx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x00000046 | MSM(REG(RG), REG(RG), Tecx))

#define rorwx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000046 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define rorwx_ri(RG, IS)                                                    \
        EMITW(0x00200002 | MSM(REG(RG), REG(RG), 0x00) |                    \
                                                 (0x1F & VAL(IS)) << 6)

#define rorwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00200002 | MSM(TMxx,    TMxx,    0x00) |                    \
                                                 (0x1F & VAL(IS)) << 6)     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define rorwx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x00000046 | MSM(REG(RG), REG(RG), REG(RS)))

#define rorwx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x00000046 | MSM(REG(RG), REG(RG), TMxx))

#define rorwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000046 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define rorwx_mr(MG, DG, RS)                                                \
        rorwx_st(W(RS), W(MG), W(DG))


#define rorwz_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x00000046 | MSM(REG(RG), REG(RG), Tecx))                     \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define rorwz_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000046 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define rorwz_ri(RG, IS)                                                    \
        EMITW(0x00200002 | MSM(REG(RG), REG(RG), 0x00) |                    \
                                                 (0x1F & VAL(IS)) << 6)     \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define rorwz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00200002 | MSM(TMxx,    TMxx,    0x00) |                    \
                                                 (0x1F & VAL(IS)) << 6)     \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define rorwz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x00000046 | MSM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define rorwz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x00000046 | MSM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x00000025 | MRM(TLxx,    REG(RG), TZxx))/* <- set flags (Z) */

#define rorwz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000046 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xAC000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define rorwz_mr(MG, DG, RS)                                                \
        rorwz_st(W(RS), W(MG), W(DG))

/* pre-r6 */
#if (defined (RT_M32) && RT_M32 < 6) || (defined (RT_M64) && RT_M64 < 6)

/* mul (G = G * S)
 * set-flags: undefined */

#define mulwx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x70000002 | MRM(REG(RG), REG(RG), TIxx))

#define mulwx_rr(RG, RS)                                                    \
        EMITW(0x70000002 | MRM(REG(RG), REG(RG), REG(RS)))

#define mulwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x70000002 | MRM(REG(RG), REG(RG), TMxx))


#define mulwx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0x00000019 | MRM(0x00,    Teax,    REG(RS)))                  \
        EMITW(0x00000012 | MRM(Teax,    0x00,    0x00))                     \
        EMITW(0x00000010 | MRM(Tedx,    0x00,    0x00))

#define mulwx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x00000019 | MRM(0x00,    Teax,    TMxx))                     \
        EMITW(0x00000012 | MRM(Teax,    0x00,    0x00))                     \
        EMITW(0x00000010 | MRM(Tedx,    0x00,    0x00))


#define mulwn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0x00000018 | MRM(0x00,    Teax,    REG(RS)))                  \
        EMITW(0x00000012 | MRM(Teax,    0x00,    0x00))                     \
        EMITW(0x00000010 | MRM(Tedx,    0x00,    0x00))

#define mulwn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x00000018 | MRM(0x00,    Teax,    TMxx))                     \
        EMITW(0x00000012 | MRM(Teax,    0x00,    0x00))                     \
        EMITW(0x00000010 | MRM(Tedx,    0x00,    0x00))


#define mulwp_xr(RS)     /* Reax is in/out, prepares Redx for divwn_x* */   \
        mulwx_rr(Reax, W(RS)) /* product must not exceed operands size */

#define mulwp_xm(MS, DS) /* Reax is in/out, prepares Redx for divwn_x* */   \
        mulwx_ld(Reax, W(MS), W(DS))  /* must not exceed operands size */

/* div (G = G / S)
 * set-flags: undefined */

#define divwx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x0000001B | MRM(0x00,    REG(RG), TIxx))                     \
        EMITW(0x00000012 | MRM(REG(RG), 0x00,    0x00))

#define divwx_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        EMITW(0x0000001B | MRM(0x00,    REG(RG), REG(RS)))                  \
        EMITW(0x00000012 | MRM(REG(RG), 0x00,    0x00))

#define divwx_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0000001B | MRM(0x00,    REG(RG), TMxx))                     \
        EMITW(0x00000012 | MRM(REG(RG), 0x00,    0x00))


#define divwn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x0000001A | MRM(0x00,    REG(RG), TIxx))                     \
        EMITW(0x00000012 | MRM(REG(RG), 0x00,    0x00))

#define divwn_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        EMITW(0x0000001A | MRM(0x00,    REG(RG), REG(RS)))                  \
        EMITW(0x00000012 | MRM(REG(RG), 0x00,    0x00))

#define divwn_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0000001A | MRM(0x00,    REG(RG), TMxx))                     \
        EMITW(0x00000012 | MRM(REG(RG), 0x00,    0x00))


#define prewx_xx()          /* to be placed immediately prior divwx_x* */   \
                                     /* to prepare Redx for int-divide */

#define prewn_xx()          /* to be placed immediately prior divwn_x* */   \
                                     /* to prepare Redx for int-divide */


#define divwx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(0x0000001B | MRM(0x00,    Teax,    REG(RS)))                  \
        EMITW(0x00000012 | MRM(Teax,    0x00,    0x00))

#define divwx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0000001B | MRM(0x00,    Teax,    TMxx))                     \
        EMITW(0x00000012 | MRM(Teax,    0x00,    0x00))


#define divwn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(0x0000001A | MRM(0x00,    Teax,    REG(RS)))                  \
        EMITW(0x00000012 | MRM(Teax,    0x00,    0x00))

#define divwn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0000001A | MRM(0x00,    Teax,    TMxx))                     \
        EMITW(0x00000012 | MRM(Teax,    0x00,    0x00))


#define divwp_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xr(W(RS))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divwp_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xm(W(MS), W(DS))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem (G = G % S)
 * set-flags: undefined */

#define remwx_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x0000001B | MRM(0x00,    REG(RG), TIxx))                     \
        EMITW(0x00000010 | MRM(REG(RG), 0x00,    0x00))

#define remwx_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        EMITW(0x0000001B | MRM(0x00,    REG(RG), REG(RS)))                  \
        EMITW(0x00000010 | MRM(REG(RG), 0x00,    0x00))

#define remwx_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0000001B | MRM(0x00,    REG(RG), TMxx))                     \
        EMITW(0x00000010 | MRM(REG(RG), 0x00,    0x00))


#define remwn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x0000001A | MRM(0x00,    REG(RG), TIxx))                     \
        EMITW(0x00000010 | MRM(REG(RG), 0x00,    0x00))

#define remwn_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        EMITW(0x0000001A | MRM(0x00,    REG(RG), REG(RS)))                  \
        EMITW(0x00000010 | MRM(REG(RG), 0x00,    0x00))

#define remwn_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0000001A | MRM(0x00,    REG(RG), TMxx))                     \
        EMITW(0x00000010 | MRM(REG(RG), 0x00,    0x00))


#define remwx_xx()          /* to be placed immediately prior divwx_x* */   \
                                     /* to prepare for rem calculation */

#define remwx_xr(RS)        /* to be placed immediately after divwx_xr */   \
        EMITW(0x00000010 | MRM(Tedx,    0x00,    0x00))   /* Redx<-rem */

#define remwx_xm(MS, DS)    /* to be placed immediately after divwx_xm */   \
        EMITW(0x00000010 | MRM(Tedx,    0x00,    0x00))   /* Redx<-rem */


#define remwn_xx()          /* to be placed immediately prior divwn_x* */   \
                                     /* to prepare for rem calculation */

#define remwn_xr(RS)        /* to be placed immediately after divwn_xr */   \
        EMITW(0x00000010 | MRM(Tedx,    0x00,    0x00))   /* Redx<-rem */

#define remwn_xm(MS, DS)    /* to be placed immediately after divwn_xm */   \
        EMITW(0x00000010 | MRM(Tedx,    0x00,    0x00))   /* Redx<-rem */

#else  /* r6 */

/* mul (G = G * S)
 * set-flags: undefined */

#define mulwx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x00000099 | MRM(REG(RG), REG(RG), TIxx))

#define mulwx_rr(RG, RS)                                                    \
        EMITW(0x00000099 | MRM(REG(RG), REG(RG), REG(RS)))

#define mulwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x00000099 | MRM(REG(RG), REG(RG), TMxx))


#define mulwx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0x000000D9 | MRM(Tedx,    Teax,    REG(RS)))                  \
        EMITW(0x00000099 | MRM(Teax,    Teax,    REG(RS)))

#define mulwx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x000000D9 | MRM(Tedx,    Teax,    TMxx))                     \
        EMITW(0x00000099 | MRM(Teax,    Teax,    TMxx))


#define mulwn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0x000000D8 | MRM(Tedx,    Teax,    REG(RS)))                  \
        EMITW(0x00000098 | MRM(Teax,    Teax,    REG(RS)))

#define mulwn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x000000D8 | MRM(Tedx,    Teax,    TMxx))                     \
        EMITW(0x00000098 | MRM(Teax,    Teax,    TMxx))


#define mulwp_xr(RS)     /* Reax is in/out, prepares Redx for divwn_x* */   \
        mulwx_rr(Reax, W(RS)) /* product must not exceed operands size */

#define mulwp_xm(MS, DS) /* Reax is in/out, prepares Redx for divwn_x* */   \
        mulwx_ld(Reax, W(MS), W(DS))  /* must not exceed operands size */

/* div (G = G / S)
 * set-flags: undefined */

#define divwx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x0000009B | MRM(REG(RG), REG(RG), TIxx))

#define divwx_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        EMITW(0x0000009B | MRM(REG(RG), REG(RG), REG(RS)))

#define divwx_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0000009B | MRM(REG(RG), REG(RG), TMxx))


#define divwn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x0000009A | MRM(REG(RG), REG(RG), TIxx))

#define divwn_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        EMITW(0x0000009A | MRM(REG(RG), REG(RG), REG(RS)))

#define divwn_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0000009A | MRM(REG(RG), REG(RG), TMxx))


#define prewx_xx()          /* to be placed immediately prior divwx_x* */   \
                                     /* to prepare Redx for int-divide */

#define prewn_xx()          /* to be placed immediately prior divwn_x* */   \
                                     /* to prepare Redx for int-divide */


#define divwx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(0x0000009B | MRM(Teax,    Teax,    REG(RS)))

#define divwx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0000009B | MRM(Teax,    Teax,    TMxx))


#define divwn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(0x0000009A | MRM(Teax,    Teax,    REG(RS)))

#define divwn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0000009A | MRM(Teax,    Teax,    TMxx))


#define divwp_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xr(W(RS))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divwp_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xm(W(MS), W(DS))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem (G = G % S)
 * set-flags: undefined */

#define remwx_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x000000DB | MRM(REG(RG), REG(RG), TIxx))

#define remwx_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        EMITW(0x000000DB | MRM(REG(RG), REG(RG), REG(RS)))

#define remwx_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x000000DB | MRM(REG(RG), REG(RG), TMxx))


#define remwn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x000000DA | MRM(REG(RG), REG(RG), TIxx))

#define remwn_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        EMITW(0x000000DA | MRM(REG(RG), REG(RG), REG(RS)))

#define remwn_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x000000DA | MRM(REG(RG), REG(RG), TMxx))


#define remwx_xx()          /* to be placed immediately prior divwx_x* */   \
        movwx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remwx_xr(RS)        /* to be placed immediately after divwx_xr */   \
        EMITW(0x000000DB | MRM(Tedx,    Tedx,    REG(RS)))/* Redx<-rem */

#define remwx_xm(MS, DS)    /* to be placed immediately after divwx_xm */   \
        EMITW(0x000000DB | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */


#define remwn_xx()          /* to be placed immediately prior divwn_x* */   \
        movwx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remwn_xr(RS)        /* to be placed immediately after divwn_xr */   \
        EMITW(0x000000DA | MRM(Tedx,    Tedx,    REG(RS)))/* Redx<-rem */

#define remwn_xm(MS, DS)    /* to be placed immediately after divwn_xm */   \
        EMITW(0x000000DA | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */

#endif /* r6 */

/* arj (G = G op S, if cc G then jump lb)
 * set-flags: undefined
 * refer to individual instruction descriptions
 * to stay within special register limitations */

#define and_x   and
#define ann_x   ann
#define orr_x   orr
#define orn_x   orn
#define xor_x   xor
#define neg_x   neg
#define add_x   add
#define sub_x   sub
#define shl_x   shl
#define shr_x   shr
#define ror_x   ror

#define EZ_x    J0
#define NZ_x    J1

#define arjwx_rx(RG, op, cc, lb)                                            \
        AR1(W(RG), op, wx_rx)                                               \
        CMZ(cc, MOD(RG), lb)

#define arjwx_mx(MG, DG, op, cc, lb)                                        \
        AR2(W(MG), W(DG), op, wz_mx)                                        \
        CMZ(cc, $t8,     lb)

#define arjwx_ri(RG, IS, op, cc, lb)                                        \
        AR2(W(RG), W(IS), op, wx_ri)                                        \
        CMZ(cc, MOD(RG), lb)

#define arjwx_mi(MG, DG, IS, op, cc, lb)                                    \
        AR3(W(MG), W(DG), W(IS), op, wz_mi)                                 \
        CMZ(cc, $t8,     lb)

#define arjwx_rr(RG, RS, op, cc, lb)                                        \
        AR2(W(RG), W(RS), op, wx_rr)                                        \
        CMZ(cc, MOD(RG), lb)

#define arjwx_ld(RG, MS, DS, op, cc, lb)                                    \
        AR3(W(RG), W(MS), W(DS), op, wx_ld)                                 \
        CMZ(cc, MOD(RG), lb)

#define arjwx_st(RS, MG, DG, op, cc, lb)                                    \
        AR3(W(RS), W(MG), W(DG), op, wz_st)                                 \
        CMZ(cc, $t8,     lb)

#define arjwx_mr(MG, DG, RS, op, cc, lb)                                    \
        arjwx_st(W(RS), W(MG), W(DG), op, cc, lb)

/* internal definitions for combined-arithmetic-jump (arj) */

#define AR1(P1, op, sg)                                                     \
        op##sg(W(P1))

#define AR2(P1, P2, op, sg)                                                 \
        op##sg(W(P1), W(P2))

#define AR3(P1, P2, P3, op, sg)                                             \
        op##sg(W(P1), W(P2), W(P3))

/* cmj (flags = S ? T, if cc flags then jump lb)
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

#define cmjwx_rz(RS, cc, lb)                                                \
        CMZ(cc, MOD(RS), lb)

#define cmjwx_mz(MS, DS, cc, lb)                                            \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        CMZ(cc, $t8,     lb)

#define cmjwx_ri(RS, IT, cc, lb)                                            \
        CMI(cc, MOD(RS), REG(RS), W(IT), lb)

#define cmjwx_mi(MS, DS, IT, cc, lb)                                        \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        CMI(cc, $t8,     TMxx,    W(IT), lb)

#define cmjwx_rr(RS, RT, cc, lb)                                            \
        CMR(cc, MOD(RS), MOD(RT), lb)

#define cmjwx_rm(RS, MT, DT, cc, lb)                                        \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        CMR(cc, MOD(RS), $t8,     lb)

#define cmjwx_mr(MS, DS, RT, cc, lb)                                        \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        CMR(cc, $t8,     MOD(RT), lb)

/* cmp (flags = S ? T)
 * set-flags: yes */

#define cmpwx_ri(RS, IT)                                                    \
        AUW(EMPTY,    VAL(IT), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IT))   \
        EMITW(0x00000025 | MRM(TLxx,    REG(RS), TZxx))

#define cmpwx_mi(MS, DS, IT)                                                \
        AUW(SIB(MS),  VAL(IT), TRxx,    MOD(MS), VAL(DS), C1(DS), G3(IT))   \
        EMITW(0x8C000000 | MDM(TLxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))

#define cmpwx_rr(RS, RT)                                                    \
        EMITW(0x00000025 | MRM(TRxx,    REG(RT), TZxx))                     \
        EMITW(0x00000025 | MRM(TLxx,    REG(RS), TZxx))

#define cmpwx_rm(RS, MT, DT)                                                \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TRxx,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x00000025 | MRM(TLxx,    REG(RS), TZxx))

#define cmpwx_mr(MS, DS, RT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TLxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x00000025 | MRM(TRxx,    REG(RT), TZxx))

/* ver (Mebp/inf_VER = SIMD-version)
 * set-flags: no
 * 0th byte - 128-bit version, 1st byte - 256-bit version,
 * 2nd byte - 512-bit version, 3rd byte - upper, reserved */

#define verxx_xx() /* destroys Reax, Recx, Rebx, Redx, Resi, Redi */        \
        movwx_mi(Mebp, inf_VER, IM(0x101)) /* <- MSA to bit0, bit8 */

/************************* address-sized instructions *************************/

/* adr (D = adr S)
 * set-flags: no */

#define adrxx_ld(RD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C3(DS), EMPTY2)   \
        EMITW(0x00000021 | MRM(REG(RD), MOD(MS), TDxx) | ADR)

     /* adrpx_ld(RD, MS, DS) in 32-bit rtarch_***_***.h files, SIMD-aligned */

/************************* pointer-sized instructions *************************/

/* label (D = Reax = adr lb)
 * set-flags: no */

     /* label_ld(lb) is defined in rtarch.h file, loads label to Reax */

     /* label_st(lb, MD, DD) is defined in rtarch.h file, destroys Reax */

/* pre-r6 */
#if (defined (RT_M32) && RT_M32 < 6) || (defined (RT_M64) && RT_M64 < 6)

/* jmp (if unconditional jump S/lb, else if cc flags then jump lb)
 * set-flags: no
 * maximum byte-address-range for un/conditional jumps is signed 18/16-bit
 * based on minimum natively-encoded offset across supported targets (u/c)
 * MIPS:18-bit, Power:26-bit, AArch32:26-bit, AArch64:28-bit, x86:32-bit /
 * MIPS:18-bit, Power:16-bit, AArch32:26-bit, AArch64:21-bit, x86:32-bit */

#define jmpxx_xr(RS)           /* register-targeted unconditional jump */   \
        EMITW(0x00000008 | MRM(0x00,    REG(RS), 0x00))                     \
        EMITW(0x00000025 | MRM(TPxx,    TPxx,    TZxx)) /* <- branch delay */

#if   defined (RT_M32)

#define jmpxx_xm(MS, DS)         /* memory-targeted unconditional jump */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x00000008 | MRM(0x00,    TMxx,    0x00))                     \
        EMITW(0x00000025 | MRM(TPxx,    TPxx,    TZxx)) /* <- branch delay */

#elif defined (RT_M64)

#define jmpxx_xm(MS, DS)         /* memory-targeted unconditional jump */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xDC000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
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

#else  /* r6 */

/* jmp (if unconditional jump S/lb, else if cc flags then jump lb)
 * set-flags: no
 * maximum byte-address-range for un/conditional jumps is signed 18/16-bit
 * based on minimum natively-encoded offset across supported targets (u/c)
 * MIPS:18-bit, Power:26-bit, AArch32:26-bit, AArch64:28-bit, x86:32-bit /
 * MIPS:18-bit, Power:16-bit, AArch32:26-bit, AArch64:21-bit, x86:32-bit */

#define jmpxx_xr(RS)           /* register-targeted unconditional jump */   \
        EMITW(0x00000009 | MRM(0x00,    REG(RS), 0x00))                     \
        EMITW(0x00000025 | MRM(TPxx,    TPxx,    TZxx)) /* <- branch delay */

#if   defined (RT_M32)

#define jmpxx_xm(MS, DS)         /* memory-targeted unconditional jump */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x00000009 | MRM(0x00,    TMxx,    0x00))                     \
        EMITW(0x00000025 | MRM(TPxx,    TPxx,    TZxx)) /* <- branch delay */

#elif defined (RT_M64)

#define jmpxx_xm(MS, DS)         /* memory-targeted unconditional jump */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xDC000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
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

#endif /* r6 */

/************************* register-size instructions *************************/

#if   defined (RT_M32)

/* stack (push stack = S, D = pop stack)
 * set-flags: no (sequence cmp/stack_la/jmp is not allowed on MIPS & Power)
 * adjust stack pointer with 8-byte (64-bit) steps on all current targets */

#define stack_st(RS)                                                        \
        EMITW(0x24000000 | MRM(0x00,    SPxx,    SPxx) | (-0x08 & 0xFFFF))  \
        EMITW(0xAC000000 | MRM(0x00,    SPxx,    REG(RS)))

#define stack_ld(RD)                                                        \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,    REG(RD)))                  \
        EMITW(0x24000000 | MRM(0x00,    SPxx,    SPxx) | (+0x08 & 0xFFFF))

#define stack_sa()   /* save all, [Reax - RegE] + 8 temps, 22 regs total */ \
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
        EMITW(0xAC000000 | MRM(0x00,    SPxx,    TDxx) | (+0x40 & 0xFFFF))  \
        EMITW(0xAC000000 | MRM(0x00,    SPxx,    TPxx) | (+0x44 & 0xFFFF))  \
        EMITW(0xAC000000 | MRM(0x00,    SPxx,    TNxx) | (+0x48 & 0xFFFF))  \
        EMITW(0xAC000000 | MRM(0x00,    SPxx,  1+TNxx) | (+0x4C & 0xFFFF))  \
        EMITW(0xAC000000 | MRM(0x00,    SPxx,  2+TNxx) | (+0x50 & 0xFFFF))  \
        EMITW(0xAC000000 | MRM(0x00,    SPxx,  3+TNxx) | (+0x54 & 0xFFFF))

#define stack_la()   /* load all, 8 temps + [RegE - Reax], 22 regs total */ \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,  3+TNxx) | (+0x54 & 0xFFFF))  \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,  2+TNxx) | (+0x50 & 0xFFFF))  \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,  1+TNxx) | (+0x4C & 0xFFFF))  \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,    TNxx) | (+0x48 & 0xFFFF))  \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,    TPxx) | (+0x44 & 0xFFFF))  \
        EMITW(0x8C000000 | MRM(0x00,    SPxx,    TDxx) | (+0x40 & 0xFFFF))  \
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

#endif /* defined (RT_M32) */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/* pre-r6 */
#if (defined (RT_M32) && RT_M32 < 6) || (defined (RT_M64) && RT_M64 < 6)

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

#define CMZ(cc, r1, lb)                                                     \
        Z##cc(r1, lb)


#define IJ0(r1, p1, IS, lb)                                                 \
        AUW(EMPTY,    VAL(IS), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        ASM_BEG ASM_OP3(beq, r1, $t9, lb) ASM_END

#define IJ1(r1, p1, IS, lb)                                                 \
        AUW(EMPTY,    VAL(IS), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        ASM_BEG ASM_OP3(bne, r1, $t9, lb) ASM_END

#define IJ2(r1, p1, IS, lb)                                                 \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(TLxx,    p1,      VAL(IS), T1(IS), M1(IS)) | \
        (+(TP1(IS) == 0) & 0x2C000000) | (+(TP1(IS) != 0) & 0x0000002B))    \
        ASM_BEG ASM_OP2(bnez, $t8, lb) ASM_END

#define IJ3(r1, p1, IS, lb)                                                 \
        AUW(EMPTY,    VAL(IS), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        ASM_BEG ASM_OP3(sltu, $t8, $t9, r1) ASM_END                         \
        ASM_BEG ASM_OP2(beqz, $t8, lb) ASM_END

#define IJ4(r1, p1, IS, lb)                                                 \
        AUW(EMPTY,    VAL(IS), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        ASM_BEG ASM_OP3(sltu, $t8, $t9, r1) ASM_END                         \
        ASM_BEG ASM_OP2(bnez, $t8, lb) ASM_END

#define IJ5(r1, p1, IS, lb)                                                 \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(TLxx,    p1,      VAL(IS), T1(IS), M1(IS)) | \
        (+(TP1(IS) == 0) & 0x2C000000) | (+(TP1(IS) != 0) & 0x0000002B))    \
        ASM_BEG ASM_OP2(beqz, $t8, lb) ASM_END

#define IJ6(r1, p1, IS, lb)                                                 \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(TLxx,    p1,      VAL(IS), T1(IS), M1(IS)) | \
        (+(TP1(IS) == 0) & 0x28000000) | (+(TP1(IS) != 0) & 0x0000002A))    \
        ASM_BEG ASM_OP2(bnez, $t8, lb) ASM_END

#define IJ7(r1, p1, IS, lb)                                                 \
        AUW(EMPTY,    VAL(IS), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        ASM_BEG ASM_OP3(slt,  $t8, $t9, r1) ASM_END                         \
        ASM_BEG ASM_OP2(beqz, $t8, lb) ASM_END

#define IJ8(r1, p1, IS, lb)                                                 \
        AUW(EMPTY,    VAL(IS), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        ASM_BEG ASM_OP3(slt,  $t8, $t9, r1) ASM_END                         \
        ASM_BEG ASM_OP2(bnez, $t8, lb) ASM_END

#define IJ9(r1, p1, IS, lb)                                                 \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(TLxx,    p1,      VAL(IS), T1(IS), M1(IS)) | \
        (+(TP1(IS) == 0) & 0x28000000) | (+(TP1(IS) != 0) & 0x0000002A))    \
        ASM_BEG ASM_OP2(beqz, $t8, lb) ASM_END

#define CMI(cc, r1, p1, IS, lb)                                             \
        I##cc(r1, p1, W(IS), lb)


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

#define CMR(cc, r1, r2, lb)                                                 \
        R##cc(r1, r2, lb)

#else  /* r6 */

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

#define CMZ(cc, r1, lb)                                                     \
        Z##cc(r1, lb)


#define IJ0(r1, p1, IS, lb)                                                 \
        AUW(EMPTY,    VAL(IS), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        RJ0(r1, $t9, lb)

#define IJ1(r1, p1, IS, lb)                                                 \
        AUW(EMPTY,    VAL(IS), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        RJ1(r1, $t9, lb)

#define IJ2(r1, p1, IS, lb)                                                 \
        AUW(EMPTY,    VAL(IS), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        RJ2(r1, $t9, lb)

#define IJ3(r1, p1, IS, lb)                                                 \
        AUW(EMPTY,    VAL(IS), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        RJ3(r1, $t9, lb)

#define IJ4(r1, p1, IS, lb)                                                 \
        AUW(EMPTY,    VAL(IS), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        RJ4(r1, $t9, lb)

#define IJ5(r1, p1, IS, lb)                                                 \
        AUW(EMPTY,    VAL(IS), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        RJ5(r1, $t9, lb)

#define IJ6(r1, p1, IS, lb)                                                 \
        AUW(EMPTY,    VAL(IS), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        RJ6(r1, $t9, lb)

#define IJ7(r1, p1, IS, lb)                                                 \
        AUW(EMPTY,    VAL(IS), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        RJ7(r1, $t9, lb)

#define IJ8(r1, p1, IS, lb)                                                 \
        AUW(EMPTY,    VAL(IS), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        RJ8(r1, $t9, lb)

#define IJ9(r1, p1, IS, lb)                                                 \
        AUW(EMPTY,    VAL(IS), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        RJ9(r1, $t9, lb)

#define CMI(cc, r1, p1, IS, lb)                                             \
        I##cc(r1, p1, W(IS), lb)


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

#define CMR(cc, r1, r2, lb)                                                 \
        R##cc(r1, r2, lb)

#endif /* r6 */

#endif /* RT_RTARCH_M32_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

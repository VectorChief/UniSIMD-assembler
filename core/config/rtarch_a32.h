/******************************************************************************/
/* Copyright (c) 2013-2018 VectorChief (at github, bitbucket, sourceforge)    */
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

#define MRM(reg, ren, rem)                                                  \
        ((rem) << 16 | (ren) << 5 | (reg))

#define MDM(reg, brm, vdp, bxx, pxx)                                        \
        (pxx(vdp) | bxx(brm) << 5 | (reg))

#define MIM(reg, ren, vim, txx, mxx)                                        \
        (mxx(vim) | (ren) << 5 | txx(reg))

#define AUW(sib, vim, reg, brm, vdp, cdp, cim)                              \
            sib  cdp(brm, vdp)  cim(reg, vim)

#define ADR ((A-1)*0x80000000)

#define EMPTY1(em1) em1
#define EMPTY2(em1, em2) em1 em2

/* selectors  */

#define REG(reg, mod, sib)  reg
#define RXG(reg, mod, sib)  ((reg) + 32)
#define RYG(reg, mod, sib)  ((reg) + 16)
#define MOD(reg, mod, sib)  mod
#define SIB(reg, mod, sib)  sib

#define VAL(val, tp1, tp2)  val
#define VXL(val, tp1, tp2)  (((val) >> 1) & 0x3FFC)
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

#define T10(tr) (tr)
#define M10(im) (0x10000000 | (im) << 10)
#define G10(rg, im) EMPTY

#define T11(tr) (tr)
#define M11(im) (0x0A000000 | TIxx << 16)
#define G11(rg, im) G31(rg, im)
#define T21(tr) (tr)
#define M21(im) (0x0A000000 | TIxx << 16)
#define G21(rg, im) G31(rg, im)
#define G31(rg, im) EMITW(0x52800000 | MRM(((rg)&0x1F),    0x00,    0x00) | \
                     ((rg)&0x20)<<26 | ((rg)&0x20)<<24 | ((rg)&0x20)<<17  | \
                             (0xFFFF & (im)) << 5)

#define T12(tr) (tr)
#define M12(im) (0x0A000000 | TIxx << 16)
#define G12(rg, im) G32(rg, im)
#define T22(tr) (tr)
#define M22(im) (0x0A000000 | TIxx << 16)
#define G22(rg, im) G32(rg, im)
#define G32(rg, im) EMITW(0x52800000 | MRM(((rg)&0x1F),    0x00,    0x00) | \
                     ((rg)&0x20)<<26 | ((rg)&0x20)<<24 | ((rg)&0x20)<<17  | \
                             (0xFFFF & (im)) << 5)                          \
                    EMITW(0x72A00000 | MRM(((rg)&0x1F),    0x00,    0x00) | \
                     ((rg)&0x20)<<26 | ((rg)&0x20)<<17 |                    \
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
                    EMITW(0x0B000000 | MRM(TPxx,    (br),    TDxx) | ADR)

#define B12(br) TPxx
#define P12(dp) (0x00000000)
#define C12(br, dp) C32(br, dp)                                             \
                    EMITW(0x0B000000 | MRM(TPxx,    (br),    TDxx) | ADR)
#define C32(br, dp) EMITW(0x52800000 | MRM(TDxx,    0x00,    0x00) |        \
                             (0xFFFC & (dp)) << 5)                          \
                    EMITW(0x72A00000 | MRM(TDxx,    0x00,    0x00) |        \
                             (0x7FFF & (dp) >> 16) << 5)

/* registers    REG   (check mapping with ASM_ENTER/ASM_LEAVE in rtarch.h) */

#define TNxx    0x16  /* w22, default FCTRL round mode */
#define TAxx    0x17  /* w23, extra reg for FAST_FCTRL */
#define TCxx    0x18  /* w24, extra reg for FAST_FCTRL */
#define TExx    0x19  /* w25, extra reg for FAST_FCTRL */

#define TMxx    0x1A  /* w26 */
#define TIxx    0x1B  /* w27, not used at the same time with TDxx */
#define TDxx    0x1B  /* w27, not used at the same time with TIxx */
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

#define Iecx    Tecx, TPxx, EMITW(0x0B000000 | MRM(TPxx, Tecx, Teax) | ADR)
#define Iedx    Tedx, TPxx, EMITW(0x0B000000 | MRM(TPxx, Tedx, Teax) | ADR)
#define Iebx    Tebx, TPxx, EMITW(0x0B000000 | MRM(TPxx, Tebx, Teax) | ADR)
#define Iebp    Tebp, TPxx, EMITW(0x0B000000 | MRM(TPxx, Tebp, Teax) | ADR)
#define Iesi    Tesi, TPxx, EMITW(0x0B000000 | MRM(TPxx, Tesi, Teax) | ADR)
#define Iedi    Tedi, TPxx, EMITW(0x0B000000 | MRM(TPxx, Tedi, Teax) | ADR)
#define Ieg8    Teg8, TPxx, EMITW(0x0B000000 | MRM(TPxx, Teg8, Teax) | ADR)
#define Ieg9    Teg9, TPxx, EMITW(0x0B000000 | MRM(TPxx, Teg9, Teax) | ADR)
#define IegA    TegA, TPxx, EMITW(0x0B000000 | MRM(TPxx, TegA, Teax) | ADR)
#define IegB    TegB, TPxx, EMITW(0x0B000000 | MRM(TPxx, TegB, Teax) | ADR)
#define IegC    TegC, TPxx, EMITW(0x0B000000 | MRM(TPxx, TegC, Teax) | ADR)
#define IegD    TegD, TPxx, EMITW(0x0B000000 | MRM(TPxx, TegD, Teax) | ADR)
#define IegE    TegE, TPxx, EMITW(0x0B000000 | MRM(TPxx, TegE, Teax) | ADR)

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
#define DF(dp)  ((dp) & 0x3FFC),     1, 0   /* native AArch64 BASE ld/st */
#define DG(dp)  ((dp) & 0x7FFC),     1, 0      /* native MIPS BASE ld/st */
#define DH(dp)  ((dp) & 0xFFFC),     1, 0   /* second native on all ARMs */
#define DV(dp)  ((dp) & 0x7FFFFFFC), 2, 2        /* native x64 long mode */
#define PLAIN   DP(0)           /* special type for Oeax addressing mode */

/* triplet pass-through wrapper */

#define W(p1, p2, p3)       p1,  p2,  p3

/******************************************************************************/
/**********************************   A32   ***********************************/
/******************************************************************************/

/* mov (D = S)
 * set-flags: no */

#define movwx_ri(RD, IS)                                                    \
        AUW(EMPTY,    VAL(IS), REG(RD), EMPTY,   EMPTY,   EMPTY2, G3(IS))

#define movwx_mi(MD, DD, IS)                                                \
        AUW(SIB(MD),  VAL(IS), TIxx,    MOD(MD), VAL(DD), C1(DD), G3(IS))   \
        EMITW(0xB9000000 | MDM(TIxx,    MOD(MD), VAL(DD), B1(DD), P1(DD)))

#define movwx_rr(RD, RS)                                                    \
        EMITW(0x2A000000 | MRM(REG(RD), TZxx,    REG(RS)))

#define movwx_ld(RD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(REG(RD), MOD(MS), VAL(DS), B1(DS), P1(DS)))

#define movwx_st(RS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0xB9000000 | MDM(REG(RS), MOD(MD), VAL(DD), B1(DD), P1(DD)))


#define movwx_rj(RD, IT, IS)     /* IT - upper 32-bit, IS - lower 32-bit */ \
        movwx_ri(W(RD), W(IS))

#define movwx_mj(MD, DD, IT, IS) /* IT - upper 32-bit, IS - lower 32-bit */ \
        movwx_mi(W(MD), W(DD), W(IS))

/* and (G = G & S)
 * set-flags: undefined (*x), yes (*z) */

#define andwx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define andwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define andwx_rr(RG, RS)                                                    \
        EMITW(0x0A000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define andwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0A000000 | MRM(REG(RG), REG(RG), TMxx))

#define andwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x0A000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define andwx_mr(MG, DG, RS)                                                \
        andwx_st(W(RS), W(MG), W(DG))


#define andwz_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x60000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define andwz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x60000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define andwz_rr(RG, RS)                                                    \
        EMITW(0x6A000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define andwz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x6A000000 | MRM(REG(RG), REG(RG), TMxx))

#define andwz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x6A000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define andwz_mr(MG, DG, RS)                                                \
        andwz_st(W(RS), W(MG), W(DG))

/* ann (G = ~G & S)
 * set-flags: undefined (*x), yes (*z) */

#define annwx_ri(RG, IS)                                                    \
        notwx_rx(W(RG))                                                     \
        andwx_ri(W(RG), W(IS))

#define annwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define annwx_rr(RG, RS)                                                    \
        EMITW(0x0A200000 | MRM(REG(RG), REG(RS), REG(RG)))

#define annwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0A200000 | MRM(REG(RG), TMxx,    REG(RG)))

#define annwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x0A200000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define annwx_mr(MG, DG, RS)                                                \
        annwx_st(W(RS), W(MG), W(DG))


#define annwz_ri(RG, IS)                                                    \
        notwx_rx(W(RG))                                                     \
        andwz_ri(W(RG), W(IS))

#define annwz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x60000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define annwz_rr(RG, RS)                                                    \
        EMITW(0x6A200000 | MRM(REG(RG), REG(RS), REG(RG)))

#define annwz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x6A200000 | MRM(REG(RG), TMxx,    REG(RG)))

#define annwz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x6A200000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define annwz_mr(MG, DG, RS)                                                \
        annwz_st(W(RS), W(MG), W(DG))

/* orr (G = G | S)
 * set-flags: undefined (*x), yes (*z) */

#define orrwx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x20000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define orrwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x20000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define orrwx_rr(RG, RS)                                                    \
        EMITW(0x2A000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define orrwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x2A000000 | MRM(REG(RG), REG(RG), TMxx))

#define orrwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define orrwx_mr(MG, DG, RS)                                                \
        orrwx_st(W(RS), W(MG), W(DG))


#define orrwz_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x20000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define orrwz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x20000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x71000000 | TMxx << 5)                  /* <- set flags (Z) */

#define orrwz_rr(RG, RS)                                                    \
        EMITW(0x2A000000 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define orrwz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x2A000000 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define orrwz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x71000000 | TMxx << 5)                  /* <- set flags (Z) */

#define orrwz_mr(MG, DG, RS)                                                \
        orrwz_st(W(RS), W(MG), W(DG))

/* orn (G = ~G | S)
 * set-flags: undefined (*x), yes (*z) */

#define ornwx_ri(RG, IS)                                                    \
        notwx_rx(W(RG))                                                     \
        orrwx_ri(W(RG), W(IS))

#define ornwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x20000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define ornwx_rr(RG, RS)                                                    \
        EMITW(0x2A200000 | MRM(REG(RG), REG(RS), REG(RG)))

#define ornwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x2A200000 | MRM(REG(RG), TMxx,    REG(RG)))

#define ornwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A200000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define ornwx_mr(MG, DG, RS)                                                \
        ornwx_st(W(RS), W(MG), W(DG))


#define ornwz_ri(RG, IS)                                                    \
        notwx_rx(W(RG))                                                     \
        orrwz_ri(W(RG), W(IS))

#define ornwz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x20000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x71000000 | TMxx << 5)                  /* <- set flags (Z) */

#define ornwz_rr(RG, RS)                                                    \
        EMITW(0x2A200000 | MRM(REG(RG), REG(RS), REG(RG)))                  \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define ornwz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x2A200000 | MRM(REG(RG), TMxx,    REG(RG)))                  \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define ornwz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A200000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x71000000 | TMxx << 5)                  /* <- set flags (Z) */

#define ornwz_mr(MG, DG, RS)                                                \
        ornwz_st(W(RS), W(MG), W(DG))

/* xor (G = G ^ S)
 * set-flags: undefined (*x), yes (*z) */

#define xorwx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x40000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define xorwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x40000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define xorwx_rr(RG, RS)                                                    \
        EMITW(0x4A000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define xorwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x4A000000 | MRM(REG(RG), REG(RG), TMxx))

#define xorwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x4A000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define xorwx_mr(MG, DG, RS)                                                \
        xorwx_st(W(RS), W(MG), W(DG))


#define xorwz_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x40000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define xorwz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x40000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x71000000 | TMxx << 5)                  /* <- set flags (Z) */

#define xorwz_rr(RG, RS)                                                    \
        EMITW(0x4A000000 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define xorwz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x4A000000 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define xorwz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x4A000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x71000000 | TMxx << 5)                  /* <- set flags (Z) */

#define xorwz_mr(MG, DG, RS)                                                \
        xorwz_st(W(RS), W(MG), W(DG))

/* not (G = ~G)
 * set-flags: no */

#define notwx_rx(RG)                                                        \
        EMITW(0x2A200000 | MRM(REG(RG), TZxx,    REG(RG)))

#define notwx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

/* neg (G = -G)
 * set-flags: undefined (*x), yes (*z) */

#define negwx_rx(RG)                                                        \
        EMITW(0x4B000000 | MRM(REG(RG), TZxx,    REG(RG)))

#define negwx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x4B000000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))


#define negwz_rx(RG)                                                        \
        EMITW(0x6B000000 | MRM(REG(RG), TZxx,    REG(RG)))

#define negwz_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x6B000000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

/* add (G = G + S)
 * set-flags: undefined (*x), yes (*z) */

#define addwx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x01000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define addwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x01000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define addwx_rr(RG, RS)                                                    \
        EMITW(0x0B000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define addwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0B000000 | MRM(REG(RG), REG(RG), TMxx))

#define addwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x0B000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define addwx_mr(MG, DG, RS)                                                \
        addwx_st(W(RS), W(MG), W(DG))


#define addwz_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x21000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define addwz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x21000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define addwz_rr(RG, RS)                                                    \
        EMITW(0x2B000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define addwz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x2B000000 | MRM(REG(RG), REG(RG), TMxx))

#define addwz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2B000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define addwz_mr(MG, DG, RS)                                                \
        addwz_st(W(RS), W(MG), W(DG))

/* sub (G = G - S)
 * set-flags: undefined (*x), yes (*z) */

#define subwx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x41000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define subwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x41000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define subwx_rr(RG, RS)                                                    \
        EMITW(0x4B000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define subwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x4B000000 | MRM(REG(RG), REG(RG), TMxx))

#define subwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x4B000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define subwx_mr(MG, DG, RS)                                                \
        subwx_st(W(RS), W(MG), W(DG))


#define subwz_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x61000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define subwz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x61000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define subwz_rr(RG, RS)                                                    \
        EMITW(0x6B000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define subwz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x6B000000 | MRM(REG(RG), REG(RG), TMxx))

#define subwz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x6B000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define subwz_mr(MG, DG, RS)                                                \
        subwz_st(W(RS), W(MG), W(DG))

/* shl (G = G << S)
 * set-flags: undefined (*x), yes (*z) */

#define shlwx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x1AC02000 | MRM(REG(RG), REG(RG), Tecx))

#define shlwx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02000 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define shlwx_ri(RG, IS)                                                    \
        EMITW(0x53000000 | MRM(REG(RG), REG(RG), 0x00) |                    \
        (-VAL(IS)&0x1F)<<16 | (31-(VAL(IS)&0x1F))<<10)

#define shlwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x53000000 | MRM(TMxx,    TMxx,    0x00) |                    \
        (-VAL(IS)&0x1F)<<16 | (31-(VAL(IS)&0x1F))<<10)                      \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define shlwx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x1AC02000 | MRM(REG(RG), REG(RG), REG(RS)))

#define shlwx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1AC02000 | MRM(REG(RG), REG(RG), TMxx))

#define shlwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define shlwx_mr(MG, DG, RS)                                                \
        shlwx_st(W(RS), W(MG), W(DG))


#define shlwz_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x1AC02000 | MRM(REG(RG), REG(RG), Tecx))                     \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shlwz_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02000 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x71000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shlwz_ri(RG, IS)                                                    \
        EMITW(0x53000000 | MRM(REG(RG), REG(RG), 0x00) |                    \
        (-VAL(IS)&0x1F)<<16 | (31-(VAL(IS)&0x1F))<<10)                      \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shlwz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x53000000 | MRM(TMxx,    TMxx,    0x00) |                    \
        (-VAL(IS)&0x1F)<<16 | (31-(VAL(IS)&0x1F))<<10)                      \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x71000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shlwz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x1AC02000 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shlwz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1AC02000 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shlwz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x71000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shlwz_mr(MG, DG, RS)                                                \
        shlwz_st(W(RS), W(MG), W(DG))

/* shr (G = G >> S)
 * set-flags: undefined (*x), yes (*z) */

#define shrwx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x1AC02400 | MRM(REG(RG), REG(RG), Tecx))

#define shrwx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02400 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define shrwx_ri(RG, IS)                                                    \
        EMITW(0x53007C00 | MRM(REG(RG), REG(RG), 0x00) |(VAL(IS)&0x1F)<<16)

#define shrwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x53007C00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IS)&0x1F)<<16) \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define shrwx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x1AC02400 | MRM(REG(RG), REG(RG), REG(RS)))

#define shrwx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1AC02400 | MRM(REG(RG), REG(RG), TMxx))

#define shrwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02400 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define shrwx_mr(MG, DG, RS)                                                \
        shrwx_st(W(RS), W(MG), W(DG))


#define shrwz_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x1AC02400 | MRM(REG(RG), REG(RG), Tecx))                     \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shrwz_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02400 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x71000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shrwz_ri(RG, IS)                                                    \
        EMITW(0x53007C00 | MRM(REG(RG), REG(RG), 0x00) |(VAL(IS)&0x1F)<<16) \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shrwz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x53007C00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IS)&0x1F)<<16) \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x71000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shrwz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x1AC02400 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shrwz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1AC02400 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shrwz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02400 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x71000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shrwz_mr(MG, DG, RS)                                                \
        shrwz_st(W(RS), W(MG), W(DG))


#define shrwn_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x1AC02800 | MRM(REG(RG), REG(RG), Tecx))

#define shrwn_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02800 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define shrwn_ri(RG, IS)                                                    \
        EMITW(0x13007C00 | MRM(REG(RG), REG(RG), 0x00) |(VAL(IS)&0x1F)<<16)

#define shrwn_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x13007C00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IS)&0x1F)<<16) \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define shrwn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x1AC02800 | MRM(REG(RG), REG(RG), REG(RS)))

#define shrwn_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1AC02800 | MRM(REG(RG), REG(RG), TMxx))

#define shrwn_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02800 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define shrwn_mr(MG, DG, RS)                                                \
        shrwn_st(W(RS), W(MG), W(DG))

/* ror (G = G >> S | G << 32 - S)
 * set-flags: undefined (*x), yes (*z) */

#define rorwx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x1AC02C00 | MRM(REG(RG), REG(RG), Tecx))

#define rorwx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02C00 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define rorwx_ri(RG, IS)                                                    \
        EMITW(0x13800000 | MRM(REG(RG), REG(RG), REG(RG)) |                 \
                                        (VAL(IS)&0x1F)<<10)

#define rorwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x13800000 | MRM(TMxx,    TMxx,    TMxx) |(VAL(IS)&0x1F)<<10) \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define rorwx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x1AC02C00 | MRM(REG(RG), REG(RG), REG(RS)))

#define rorwx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1AC02C00 | MRM(REG(RG), REG(RG), TMxx))

#define rorwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02C00 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))

#define rorwx_mr(MG, DG, RS)                                                \
        rorwx_st(W(RS), W(MG), W(DG))


#define rorwz_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x1AC02C00 | MRM(REG(RG), REG(RG), Tecx))                     \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define rorwz_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02C00 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x71000000 | TMxx << 5)                  /* <- set flags (Z) */

#define rorwz_ri(RG, IS)                                                    \
        EMITW(0x13800000 | MRM(REG(RG), REG(RG), REG(RG)) |                 \
                                        (VAL(IS)&0x1F)<<10)                 \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define rorwz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x13800000 | MRM(TMxx,    TMxx,    TMxx) |(VAL(IS)&0x1F)<<10) \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x71000000 | TMxx << 5)                  /* <- set flags (Z) */

#define rorwz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x1AC02C00 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define rorwz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1AC02C00 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define rorwz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02C00 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xB9000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x71000000 | TMxx << 5)                  /* <- set flags (Z) */

#define rorwz_mr(MG, DG, RS)                                                \
        rorwz_st(W(RS), W(MG), W(DG))

/* mul (G = G * S)
 * set-flags: undefined */

#define mulwx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x1B007C00 | MRM(REG(RG), REG(RG), TIxx))

#define mulwx_rr(RG, RS)                                                    \
        EMITW(0x1B007C00 | MRM(REG(RG), REG(RG), REG(RS)))

#define mulwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1B007C00 | MRM(REG(RG), REG(RG), TMxx))


#define mulwx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0x9BA07C00 | MRM(Teax,    Teax,    REG(RS)))                  \
        EMITW(0xD360FC00 | MRM(Tedx,    Teax,    0x00))

#define mulwx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x9BA07C00 | MRM(Teax,    Teax,    TMxx))                     \
        EMITW(0xD360FC00 | MRM(Tedx,    Teax,    0x00))


#define mulwn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0x9B207C00 | MRM(Teax,    Teax,    REG(RS)))                  \
        EMITW(0xD360FC00 | MRM(Tedx,    Teax,    0x00))

#define mulwn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x9B207C00 | MRM(Teax,    Teax,    TMxx))                     \
        EMITW(0xD360FC00 | MRM(Tedx,    Teax,    0x00))


#define mulwp_xr(RS)     /* Reax is in/out, prepares Redx for divwn_x* */   \
        mulwx_rr(Reax, W(RS)) /* product must not exceed operands size */

#define mulwp_xm(MS, DS) /* Reax is in/out, prepares Redx for divwn_x* */   \
        mulwx_ld(Reax, W(MS), W(DS))  /* must not exceed operands size */

/* div (G = G / S)
 * set-flags: undefined */

#define divwx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x1AC00800 | MRM(REG(RG), REG(RG), TIxx))

#define divwx_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        EMITW(0x1AC00800 | MRM(REG(RG), REG(RG), REG(RS)))

#define divwx_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1AC00800 | MRM(REG(RG), REG(RG), TMxx))


#define divwn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x1AC00C00 | MRM(REG(RG), REG(RG), TIxx))

#define divwn_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        EMITW(0x1AC00C00 | MRM(REG(RG), REG(RG), REG(RS)))

#define divwn_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1AC00C00 | MRM(REG(RG), REG(RG), TMxx))


#define prewx_xx()          /* to be placed immediately prior divwx_x* */   \
                                     /* to prepare Redx for int-divide */

#define prewn_xx()          /* to be placed immediately prior divwn_x* */   \
                                     /* to prepare Redx for int-divide */


#define divwx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(0x1AC00800 | MRM(Teax,    Teax,    REG(RS)))

#define divwx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1AC00800 | MRM(Teax,    Teax,    TMxx))


#define divwn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(0x1AC00C00 | MRM(Teax,    Teax,    REG(RS)))

#define divwn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1AC00C00 | MRM(Teax,    Teax,    TMxx))


#define divwp_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xr(W(RS))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divwp_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xm(W(MS), W(DS))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem (G = G % S)
 * set-flags: undefined */

#define remwx_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movwx_rr(Redx, W(RG))                                               \
        divwx_ri(W(RG), W(IS))                                              \
        EMITW(0x1B008000 | MRM(REG(RG), REG(RG), TIxx) | Tedx << 10)        \
        stack_ld(Redx)

#define remwx_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        stack_st(Redx)                                                      \
        movwx_rr(Redx, W(RG))                                               \
        divwx_rr(W(RG), W(RS))                                              \
        EMITW(0x1B008000 | MRM(REG(RG), REG(RG), REG(RS)) | Tedx << 10)     \
        stack_ld(Redx)

#define remwx_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        movwx_rr(Redx, W(RG))                                               \
        divwx_ld(W(RG), W(MS), W(DS))                                       \
        EMITW(0x1B008000 | MRM(REG(RG), REG(RG), TMxx) | Tedx << 10)        \
        stack_ld(Redx)


#define remwn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movwx_rr(Redx, W(RG))                                               \
        divwn_ri(W(RG), W(IS))                                              \
        EMITW(0x1B008000 | MRM(REG(RG), REG(RG), TIxx) | Tedx << 10)        \
        stack_ld(Redx)

#define remwn_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        stack_st(Redx)                                                      \
        movwx_rr(Redx, W(RG))                                               \
        divwn_rr(W(RG), W(RS))                                              \
        EMITW(0x1B008000 | MRM(REG(RG), REG(RG), REG(RS)) | Tedx << 10)     \
        stack_ld(Redx)

#define remwn_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        movwx_rr(Redx, W(RG))                                               \
        divwn_ld(W(RG), W(MS), W(DS))                                       \
        EMITW(0x1B008000 | MRM(REG(RG), REG(RG), TMxx) | Tedx << 10)        \
        stack_ld(Redx)


#define remwx_xx()          /* to be placed immediately prior divwx_x* */   \
        movwx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remwx_xr(RS)        /* to be placed immediately after divwx_xr */   \
        EMITW(0x1B008000 | MRM(Tedx,    Teax,    REG(RS)) | Tedx << 10)     \
                                                          /* Redx<-rem */

#define remwx_xm(MS, DS)    /* to be placed immediately after divwx_xm */   \
        EMITW(0x1B008000 | MRM(Tedx,    Teax,    TMxx) | Tedx << 10)        \
                                                          /* Redx<-rem */


#define remwn_xx()          /* to be placed immediately prior divwn_x* */   \
        movwx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remwn_xr(RS)        /* to be placed immediately after divwn_xr */   \
        EMITW(0x1B008000 | MRM(Tedx,    Teax,    REG(RS)) | Tedx << 10)     \
                                                          /* Redx<-rem */

#define remwn_xm(MS, DS)    /* to be placed immediately after divwn_xm */   \
        EMITW(0x1B008000 | MRM(Tedx,    Teax,    TMxx) | Tedx << 10)        \
                                                          /* Redx<-rem */

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

#define EZ_x    jezxx_lb
#define NZ_x    jnzxx_lb

#define arjwx_rx(RG, op, cc, lb)                                            \
        AR1(W(RG), op, wz_rx)                                               \
        CMJ(cc, lb)

#define arjwx_mx(MG, DG, op, cc, lb)                                        \
        AR2(W(MG), W(DG), op, wz_mx)                                        \
        CMJ(cc, lb)

#define arjwx_ri(RG, IS, op, cc, lb)                                        \
        AR2(W(RG), W(IS), op, wz_ri)                                        \
        CMJ(cc, lb)

#define arjwx_mi(MG, DG, IS, op, cc, lb)                                    \
        AR3(W(MG), W(DG), W(IS), op, wz_mi)                                 \
        CMJ(cc, lb)

#define arjwx_rr(RG, RS, op, cc, lb)                                        \
        AR2(W(RG), W(RS), op, wz_rr)                                        \
        CMJ(cc, lb)

#define arjwx_ld(RG, MS, DS, op, cc, lb)                                    \
        AR3(W(RG), W(MS), W(DS), op, wz_ld)                                 \
        CMJ(cc, lb)

#define arjwx_st(RS, MG, DG, op, cc, lb)                                    \
        AR3(W(RS), W(MG), W(DG), op, wz_st)                                 \
        CMJ(cc, lb)

#define arjwx_mr(MG, DG, RS, op, cc, lb)                                    \
        arjwx_st(W(RS), W(MG), W(DG), op, cc, lb)

/* cmj (flags = S ? T, if cc flags then jump lb)
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

#define cmjwx_rz(RS, cc, lb)                                                \
        cmjwx_ri(W(RS), IC(0), cc, lb)

#define cmjwx_mz(MS, DS, cc, lb)                                            \
        cmjwx_mi(W(MS), W(DS), IC(0), cc, lb)

#define cmjwx_ri(RS, IT, cc, lb)                                            \
        cmpwx_ri(W(RS), W(IT))                                              \
        CMJ(cc, lb)

#define cmjwx_mi(MS, DS, IT, cc, lb)                                        \
        cmpwx_mi(W(MS), W(DS), W(IT))                                       \
        CMJ(cc, lb)

#define cmjwx_rr(RS, RT, cc, lb)                                            \
        cmpwx_rr(W(RS), W(RT))                                              \
        CMJ(cc, lb)

#define cmjwx_rm(RS, MT, DT, cc, lb)                                        \
        cmpwx_rm(W(RS), W(MT), W(DT))                                       \
        CMJ(cc, lb)

#define cmjwx_mr(MS, DS, RT, cc, lb)                                        \
        cmpwx_mr(W(MS), W(DS), W(RT))                                       \
        CMJ(cc, lb)

/* cmp (flags = S ? T)
 * set-flags: yes */

#define cmpwx_ri(RS, IT)                                                    \
        AUW(EMPTY,    VAL(IT), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IT))   \
        EMITW(0x61000000 | MIM(TZxx,    REG(RS), VAL(IT), T1(IT), M1(IT)))

#define cmpwx_mi(MS, DS, IT)                                                \
        AUW(SIB(MS),  VAL(IT), TIxx,    MOD(MS), VAL(DS), C1(DS), G1(IT))   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x61000000 | MIM(TZxx,    TMxx,    VAL(IT), T1(IT), M1(IT)))

#define cmpwx_rr(RS, RT)                                                    \
        EMITW(0x6B000000 | MRM(TZxx,    REG(RS), REG(RT)))

#define cmpwx_rm(RS, MT, DT)                                                \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x6B000000 | MRM(TZxx,    REG(RS), TMxx))

#define cmpwx_mr(MS, DS, RT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x6B000000 | MRM(TZxx,    TMxx,    REG(RT)))

/* ver (Mebp/inf_VER = SIMD-version)
 * set-flags: no
 * For interpretation of SIMD build flags check compatibility layer in rtzero.h
 * 0th byte - 128-bit version, 1st byte - 256-bit version, | plus _R8/_RX slots
 * 2nd byte - 512-bit version, 3rd byte - 1K4-bit version, | in upper halves */

#define verxx_xx() /* destroys Reax, Recx, Rebx, Redx, Resi, Redi */        \
        movwx_mi(Mebp, inf_VER, IW(0x51145)) /* NEON: 0,2,6,8; SVE: 12,16,18 */

/************************* address-sized instructions *************************/

/* adr (D = adr S)
 * set-flags: no */

#define adrxx_ld(RD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C3(DS), EMPTY2)   \
        EMITW(0x0B000000 | MRM(REG(RD), MOD(MS), TDxx) | ADR)

/************************* pointer-sized instructions *************************/

/* label (D = Reax = adr lb)
 * set-flags: no */

     /* label_ld(lb) is defined in rtarch.h file, loads label to Reax */

     /* label_st(lb, MD, DD) is defined in rtarch.h file, destroys Reax */

/* jmp (if unconditional jump S/lb, else if cc flags then jump lb)
 * set-flags: no
 * maximum byte-address-range for un/conditional jumps is signed 18/16-bit
 * based on minimum natively-encoded offset across supported targets (u/c)
 * MIPS:18-bit, Power:26-bit, AArch32:26-bit, AArch64:28-bit, x86:32-bit /
 * MIPS:18-bit, Power:16-bit, AArch32:26-bit, AArch64:21-bit, x86:32-bit */

#define jmpxx_xr(RS)           /* register-targeted unconditional jump */   \
        EMITW(0xD61F0000 | MRM(0x00,    REG(RS), 0x00))

#if   (defined RT_A32)

#define jmpxx_xm(MS, DS)         /* memory-targeted unconditional jump */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0xD61F0000 | MRM(0x00,    TMxx,    0x00))

#elif (defined RT_A64)

#define jmpxx_xm(MS, DS)         /* memory-targeted unconditional jump */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0xD61F0000 | MRM(0x00,    TMxx,    0x00))

#endif /* defined (RT_A32, RT_A64) */

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

/************************* register-size instructions *************************/

/* stack (push stack = S, D = pop stack)
 * set-flags: no (sequence cmp/stack_la/jmp is not allowed on MIPS & Power)
 * adjust stack pointer with 8-byte (64-bit) steps on all current targets */

#define stack_st(RS)                                                        \
        EMITW(0xA9BF0000 | MRM(REG(RS), SPxx,    0x00) | TZxx << 10)

#define stack_ld(RD)                                                        \
        EMITW(0xA8C10000 | MRM(REG(RD), SPxx,    0x00) | TZxx << 10)

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
        EMITW(0xA9BF0000 | MRM(TNxx+3,  SPxx,    0x00) | TZxx << 10)

#define stack_la()   /* load all, 7 temps + [RegE - Reax], 21 regs total */ \
        EMITW(0xA8C10000 | MRM(TNxx+3,  SPxx,    0x00) | TZxx << 10)        \
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

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/* internal definitions for combined-arithmetic-jump (arj) */

#define AR1(P1, op, sg)                                                     \
        op##sg(W(P1))

#define AR2(P1, P2, op, sg)                                                 \
        op##sg(W(P1), W(P2))

#define AR3(P1, P2, P3, op, sg)                                             \
        op##sg(W(P1), W(P2), W(P3))

#define CMJ(cc, lb)                                                         \
        cc(lb)

#endif /* RT_RTARCH_A32_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_ARM_H
#define RT_RTARCH_ARM_H

#define RT_BASE_REGS        8

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_arm.h: Implementation of ARMv7/8:AArch32 BASE instructions.
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
 * Mixing of 64/32-bit fields in backend structures may lead to misalignment
 * of 64-bit fields to 4-byte boundary, which is not supported on some targets.
 * Place fields carefully to ensure proper alignment for all data types.
 * Note that within cmdx*_** subset most of the instructions follow in-heap
 * address size (RT_ADDRESS or A) and only label_ld/st, jmpxx_xr/xm follow
 * pointer size (RT_POINTER or P) as code/data/stack segments are fixed.
 * In 64/32-bit (ptr/adr) hybrid mode there is no way to move 64-bit registers,
 * thus label_ld has very limited use as jmpxx_xr(Reax) is the only matching op.
 * Stack ops always work with full registers regardless of the mode chosen.
 *
 * 32-bit and 64-bit BASE subsets are not easily compatible on all targets,
 * thus any register modified with 32-bit op cannot be used in 64-bit subset.
 * Alternatively, data flow must not exceed 31-bit range for 32-bit operations
 * to produce consistent results usable in 64-bit subset across all targets.
 * Only a64 and x64 have a complete 32-bit support in 64-bit mode both zeroing
 * the upper half of the result, while m64 sign-extending all 32-bit operations
 * and p64 overflowing 32-bit arithmetic into the upper half. Similar reasons
 * of inconsistency prohibit use of IW immediate type within 64-bit subset,
 * where a64 and p64 zero-extend, while x64 and m64 sign-extend 32-bit value.
 *
 * Note that offset correction for endianness E is only applicable for addresses
 * within pointer fields, when (in-heap) address and pointer sizes don't match.
 * Working with 32-bit data in 64-bit fields in any other circumstances must be
 * done consistently within a subset of one size (32-bit, 64-bit or C/C++).
 * Alternatively, data written natively in C/C++ can be worked on from within
 * a given (one) subset if appropriate offset correction is used from rtarch.h.
 *
 * Setting-flags instructions' naming scheme may change again in the future for
 * better orthogonality with operands size, type and args-list. It is therefore
 * recommended to use combined-arithmetic-jump (arj) for better API stability
 * and maximum efficiency across all supported targets. For similar reasons
 * of higher performance on certain targets use combined-compare-jump (cmj).
 * Not all canonical forms of BASE instructions have efficient implementation.
 * For example, some forms of shifts and division use stack ops on x86 targets,
 * while standalone remainder operations can only be done natively on MIPS.
 * Consider using special fixed-register forms for maximum performance.
 * Argument x-register (implied) is fixed by the implementation.
 * Some formal definitions are not given below to encourage
 * use of friendly aliases for better code readability.
 */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/* structural */

#define MRM(reg, ren, rem)                                                  \
        ((ren) << 16 | (reg) << 12 | (rem))

#define MDM(reg, brm, vdp, bxx, pxx)                                        \
        (bxx(brm) << 16 | (reg) << 12 | pxx(vdp))

#define MIM(reg, ren, vim, txx, mxx)                                        \
        ((ren) << 16 | txx(reg) | mxx(vim))

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

#define T10(tr) ((tr) << 12)
#define M10(im) (0x02000000 | (im))
#define G10(rg, im) EMPTY
#define T20(tr) ((tr) << 12)
#define M20(im) (0x02000000 | (im))
#define G20(rg, im) EMPTY
#define G30(rg, im) EMITW(0xE3000000 | MRM((rg),    0x00,    0x00) |        \
                            (0xF0000 & (im) <<  4) | (0xFFF & (im)))

#define T11(tr) ((tr) << 12)
#define M11(im) (0x00000000 | TIxx)
#define G11(rg, im) G31(rg, im)
#define T21(tr) ((tr) << 12)
#define M21(im) (0x00000000 | TIxx)
#define G21(rg, im) G31(rg, im)
#define G31(rg, im) EMITW(0xE3000000 | MRM((rg),    0x00,    0x00) |        \
                            (0xF0000 & (im) <<  4) | (0xFFF & (im)))

#define T12(tr) ((tr) << 12)
#define M12(im) (0x00000000 | TIxx)
#define G12(rg, im) G32(rg, im)
#define T22(tr) ((tr) << 12)
#define M22(im) (0x00000000 | TIxx)
#define G22(rg, im) G32(rg, im)
#define G32(rg, im) EMITW(0xE3000000 | MRM((rg),    0x00,    0x00) |        \
                            (0xF0000 & (im) <<  4) | (0xFFF & (im)))        \
                    EMITW(0xE3400000 | MRM((rg),    0x00,    0x00) |        \
                            (0xF0000 & (im) >> 12) | (0xFFF & (im) >> 16))

/* displacement encoding BASE(TP1), adr(TP3) */

#define B10(br) (br)
#define P10(dp) (0x00000000 | (dp))
#define C10(br, dp) EMPTY
#define C30(br, dp) EMITW(0xE3000000 | MRM(TDxx,    0x00,    0x00) |        \
                            (0xF0000 & (dp) <<  4) | (0xFFC & (dp)))

#define B11(br) TPxx
#define P11(dp) (0x00000000)
#define C11(br, dp) C30(br, dp)                                             \
                    EMITW(0xE0800000 | MRM(TPxx,    (br),    TDxx))
#define C31(br, dp) EMITW(0xE3000000 | MRM(TDxx,    0x00,    0x00) |        \
                            (0xF0000 & (dp) <<  4) | (0xFFC & (dp)))

#define B12(br) TPxx
#define P12(dp) (0x00000000)
#define C12(br, dp) C32(br, dp)                                             \
                    EMITW(0xE0800000 | MRM(TPxx,    (br),    TDxx))
#define C32(br, dp) EMITW(0xE3000000 | MRM(TDxx,    0x00,    0x00) |        \
                            (0xF0000 & (dp) <<  4) | (0xFFC & (dp)))        \
                    EMITW(0xE3400000 | MRM(TDxx,    0x00,    0x00) |        \
                            (0x70000 & (dp) >> 12) | (0xFFF & (dp) >> 16))

/* registers    REG   (check mapping with ASM_ENTER/ASM_LEAVE in rtarch.h) */

#define TNxx    0x08  /* r8,  default FCTRL round mode */
#define TAxx    0x0A  /* r10, extra reg for FAST_FCTRL */
#define TCxx    0x0C  /* r12, extra reg for FAST_FCTRL */
#define TExx    0x0E  /* r14, extra reg for FAST_FCTRL */

#define TMxx    0x04  /* r4 */
#define TIxx    0x09  /* r9, not used at the same time with TDxx */
#define TDxx    0x09  /* r9, not used at the same time with TIxx */
#define TPxx    0x0B  /* r11 */
#define SPxx    0x0D  /* r13 */
#define PCxx    0x0F  /* r15 */

#define Teax    0x00  /* r0 */
#define Tecx    0x01  /* r1 */
#define Tedx    0x02  /* r2 */
#define Tebx    0x03  /* r3 */
#define Tebp    0x05  /* r5 */
#define Tesi    0x06  /* r6 */
#define Tedi    0x07  /* r7 */

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

/* addressing   REG,  MOD,  SIB */

#define Oeax    Teax, Teax, EMPTY

#define Mecx    Tecx, Tecx, EMPTY
#define Medx    Tedx, Tedx, EMPTY
#define Mebx    Tebx, Tebx, EMPTY
#define Mebp    Tebp, Tebp, EMPTY
#define Mesi    Tesi, Tesi, EMPTY
#define Medi    Tedi, Tedi, EMPTY

#define Iecx    Tecx, TPxx, EMITW(0xE0800000 | MRM(TPxx,    Tecx,    Teax))
#define Iedx    Tedx, TPxx, EMITW(0xE0800000 | MRM(TPxx,    Tedx,    Teax))
#define Iebx    Tebx, TPxx, EMITW(0xE0800000 | MRM(TPxx,    Tebx,    Teax))
#define Iebp    Tebp, TPxx, EMITW(0xE0800000 | MRM(TPxx,    Tebp,    Teax))
#define Iesi    Tesi, TPxx, EMITW(0xE0800000 | MRM(TPxx,    Tesi,    Teax))
#define Iedi    Tedi, TPxx, EMITW(0xE0800000 | MRM(TPxx,    Tedi,    Teax))

/* immediate    VAL,  TP1,  TP2       (all immediate types are unsigned) */

#define IC(im)  ((im) & 0x7F),       0, 0      /* drop sign-ext (in x86) */
#define IB(im)  ((im) & 0xFF),       0, 0        /* 32-bit word (in x86) */
#define IM(im)  ((im) & 0xFFF),      1, 1  /* native AArch64 add/sub/cmp */
#define IG(im)  ((im) & 0x7FFF),     1, 1  /* native on MIPS add/sub/cmp */
#define IH(im)  ((im) & 0xFFFF),     1, 1  /* second native on ARMs/MIPS */
#define IV(im)  ((im) & 0x7FFFFFFF), 2, 2        /* native x64 long mode */
#define IW(im)  ((im) & 0xFFFFFFFF), 2, 2       /* only for cmdw*_** set */

/* displacement VAL,  TP1,  TP2    (all displacement types are unsigned) */

#define DP(dp)  ((dp) & 0xFFC),      0, 0    /* native on all ARMs, MIPS */
#define DF(dp)  ((dp) & 0x3FFC),     1, 1   /* native AArch64 BASE ld/st */
#define DG(dp)  ((dp) & 0x7FFC),     1, 1      /* native MIPS BASE ld/st */
#define DH(dp)  ((dp) & 0xFFFC),     1, 1   /* second native on all ARMs */
#define DV(dp)  ((dp) & 0x7FFFFFFC), 2, 2        /* native x64 long mode */
#define PLAIN   DP(0)           /* special type for Oeax addressing mode */

/* triplet pass-through wrapper */

#define W(p1, p2, p3)       p1,  p2,  p3

/******************************************************************************/
/**********************************   ARM   ***********************************/
/******************************************************************************/

/* mov
 * set-flags: no */

#define movwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), REG(RM), EMPTY,   EMPTY,   EMPTY2, G3(IM))   \

#define movwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G3(IM))   \
        EMITW(0xE5800000 | MDM(TIxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define movwx_rr(RG, RM)                                                    \
        EMITW(0xE1A00000 | MRM(REG(RG), 0x00,    REG(RM)))

#define movwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(REG(RG), MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define movwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5800000 | MDM(REG(RG), MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define adrxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C3(DP), EMPTY2)   \
        EMITW(0xE0800000 | MRM(REG(RG), MOD(RM), TDxx))

/* and
 * set-flags: undefined (*x), yes (*z) */

#define andwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0xE0000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)))

#define andwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)))  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define andwx_rr(RG, RM)                                                    \
        EMITW(0xE0000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define andwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0000000 | MRM(REG(RG), REG(RG), TMxx))

#define andwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define andwz_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0xE0100000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)))

#define andwz_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0100000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)))  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define andwz_rr(RG, RM)                                                    \
        EMITW(0xE0100000 | MRM(REG(RG), REG(RG), REG(RM)))

#define andwz_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0100000 | MRM(REG(RG), REG(RG), TMxx))

#define andwz_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0100000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* orr
 * set-flags: undefined (*x), yes (*z) */

#define orrwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0xE1800000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)))

#define orrwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1800000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)))  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define orrwx_rr(RG, RM)                                                    \
        EMITW(0xE1800000 | MRM(REG(RG), REG(RG), REG(RM)))

#define orrwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1800000 | MRM(REG(RG), REG(RG), TMxx))

#define orrwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1800000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define orrwz_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0xE1900000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)))

#define orrwz_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1900000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)))  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define orrwz_rr(RG, RM)                                                    \
        EMITW(0xE1900000 | MRM(REG(RG), REG(RG), REG(RM)))

#define orrwz_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1900000 | MRM(REG(RG), REG(RG), TMxx))

#define orrwz_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1900000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* xor
 * set-flags: undefined (*x), yes (*z) */

#define xorwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0xE0200000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)))

#define xorwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0200000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)))  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define xorwx_rr(RG, RM)                                                    \
        EMITW(0xE0200000 | MRM(REG(RG), REG(RG), REG(RM)))

#define xorwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0200000 | MRM(REG(RG), REG(RG), TMxx))

#define xorwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0200000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define xorwz_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0xE0300000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)))

#define xorwz_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0300000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)))  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define xorwz_rr(RG, RM)                                                    \
        EMITW(0xE0300000 | MRM(REG(RG), REG(RG), REG(RM)))

#define xorwz_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0300000 | MRM(REG(RG), REG(RG), TMxx))

#define xorwz_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0300000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* not
 * set-flags: no */

#define notwx_rx(RM)                                                        \
        EMITW(0xE1E00000 | MRM(REG(RM), 0x00,    REG(RM)))

#define notwx_mx(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1E00000 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* neg
 * set-flags: undefined (*x), yes (*z) */

#define negwx_rx(RM)                                                        \
        EMITW(0xE2600000 | MRM(REG(RM), REG(RM), 0x00))

#define negwx_mx(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE2600000 | MRM(TMxx,    TMxx,    0x00))                     \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define negwz_rx(RM)                                                        \
        EMITW(0xE2700000 | MRM(REG(RM), REG(RM), 0x00))

#define negwz_mx(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE2700000 | MRM(TMxx,    TMxx,    0x00))                     \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* add
 * set-flags: undefined (*x), yes (*z) */

#define addwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0xE0800000 | MIM(REG(RM), REG(RM), VAL(IM), T1(IM), M1(IM)))

#define addwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0800000 | MIM(TMxx,    TMxx,    VAL(IM), T1(IM), M1(IM)))  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define addwx_rr(RG, RM)                                                    \
        EMITW(0xE0800000 | MRM(REG(RG), REG(RG), REG(RM)))

#define addwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0800000 | MRM(REG(RG), REG(RG), TMxx))

#define addwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0800000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))


#define addwz_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0xE0900000 | MIM(REG(RM), REG(RM), VAL(IM), T1(IM), M1(IM)))

#define addwz_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0900000 | MIM(TMxx,    TMxx,    VAL(IM), T1(IM), M1(IM)))  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define addwz_rr(RG, RM)                                                    \
        EMITW(0xE0900000 | MRM(REG(RG), REG(RG), REG(RM)))

#define addwz_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0900000 | MRM(REG(RG), REG(RG), TMxx))

#define addwz_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0900000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

/* sub
 * set-flags: undefined (*x), yes (*z) */

#define subwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0xE0400000 | MIM(REG(RM), REG(RM), VAL(IM), T1(IM), M1(IM)))

#define subwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0400000 | MIM(TMxx,    TMxx,    VAL(IM), T1(IM), M1(IM)))  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define subwx_rr(RG, RM)                                                    \
        EMITW(0xE0400000 | MRM(REG(RG), REG(RG), REG(RM)))

#define subwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0400000 | MRM(REG(RG), REG(RG), TMxx))

#define subwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0400000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define subwx_mr(RM, DP, RG)                                                \
        subwx_st(W(RG), W(RM), W(DP))


#define subwz_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0xE0500000 | MIM(REG(RM), REG(RM), VAL(IM), T1(IM), M1(IM)))

#define subwz_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0500000 | MIM(TMxx,    TMxx,    VAL(IM), T1(IM), M1(IM)))  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define subwz_rr(RG, RM)                                                    \
        EMITW(0xE0500000 | MRM(REG(RG), REG(RG), REG(RM)))

#define subwz_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0500000 | MRM(REG(RG), REG(RG), TMxx))

#define subwz_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0500000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define subwz_mr(RM, DP, RG)                                                \
        subwz_st(W(RG), W(RM), W(DP))

/* shl
 * set-flags: undefined (*x), yes (*z) */

#define shlwx_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0xE1A00010 | MRM(REG(RM), 0x00,    REG(RM)) | Tecx << 8)

#define shlwx_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1A00010 | MRM(TMxx,    0x00,    TMxx) | Tecx << 8)         \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shlwx_ri(RM, IM)                                                    \
        EMITW(0xE1A00000 | MRM(REG(RM), 0x00,    REG(RM)) |                 \
                                                 (0x1F & VAL(IM)) << 7)

#define shlwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1A00000 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IM)) << 7)     \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shlwx_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        EMITW(0xE1A00010 | MRM(REG(RG), 0x00,    REG(RG)) | REG(RM) << 8)

#define shlwx_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1A00010 | MRM(REG(RG), 0x00,    REG(RG)) | TMxx << 8)

#define shlwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1A00010 | MRM(TMxx,    0x00,    TMxx) | REG(RG) << 8)      \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shlwx_mr(RM, DP, RG)                                                \
        shlwx_st(W(RG), W(RM), W(DP))


#define shlwz_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0xE1B00010 | MRM(REG(RM), 0x00,    REG(RM)) | Tecx << 8)

#define shlwz_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1B00010 | MRM(TMxx,    0x00,    TMxx) | Tecx << 8)         \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shlwz_ri(RM, IM)                                                    \
        EMITW(0xE1B00000 | MRM(REG(RM), 0x00,    REG(RM)) |                 \
                                                 (0x1F & VAL(IM)) << 7)

#define shlwz_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1B00000 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IM)) << 7)     \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shlwz_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        EMITW(0xE1B00010 | MRM(REG(RG), 0x00,    REG(RG)) | REG(RM) << 8)

#define shlwz_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1B00010 | MRM(REG(RG), 0x00,    REG(RG)) | TMxx << 8)

#define shlwz_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1B00010 | MRM(TMxx,    0x00,    TMxx) | REG(RG) << 8)      \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shlwz_mr(RM, DP, RG)                                                \
        shlwz_st(W(RG), W(RM), W(DP))

/* shr
 * set-flags: undefined (*x), yes (*z) */

#define shrwx_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0xE1A00030 | MRM(REG(RM), 0x00,    REG(RM)) | Tecx << 8)

#define shrwx_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1A00030 | MRM(TMxx,    0x00,    TMxx) | Tecx << 8)         \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrwx_ri(RM, IM)                                                    \
        EMITW(0xE1A00020 | MRM(REG(RM), 0x00,    REG(RM)) |                 \
                                                 (0x1F & VAL(IM)) << 7)

#define shrwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1A00020 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IM)) << 7)     \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrwx_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        EMITW(0xE1A00030 | MRM(REG(RG), 0x00,    REG(RG)) | REG(RM) << 8)

#define shrwx_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1A00030 | MRM(REG(RG), 0x00,    REG(RG)) | TMxx << 8)

#define shrwx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1A00030 | MRM(TMxx,    0x00,    TMxx) | REG(RG) << 8)      \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrwx_mr(RM, DP, RG)                                                \
        shrwx_st(W(RG), W(RM), W(DP))


#define shrwz_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0xE1B00030 | MRM(REG(RM), 0x00,    REG(RM)) | Tecx << 8)

#define shrwz_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1B00030 | MRM(TMxx,    0x00,    TMxx) | Tecx << 8)         \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrwz_ri(RM, IM)                                                    \
        EMITW(0xE1B00020 | MRM(REG(RM), 0x00,    REG(RM)) |                 \
                                                 (0x1F & VAL(IM)) << 7)

#define shrwz_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1B00020 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IM)) << 7)     \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrwz_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        EMITW(0xE1B00030 | MRM(REG(RG), 0x00,    REG(RG)) | REG(RM) << 8)

#define shrwz_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1B00030 | MRM(REG(RG), 0x00,    REG(RG)) | TMxx << 8)

#define shrwz_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1B00030 | MRM(TMxx,    0x00,    TMxx) | REG(RG) << 8)      \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrwz_mr(RM, DP, RG)                                                \
        shrwz_st(W(RG), W(RM), W(DP))


#define shrwn_rx(RM)                     /* reads Recx for shift value */   \
        EMITW(0xE1A00050 | MRM(REG(RM), 0x00,    REG(RM)) | Tecx << 8)

#define shrwn_mx(RM, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1A00050 | MRM(TMxx,    0x00,    TMxx) | Tecx << 8)         \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrwn_ri(RM, IM)                                                    \
        EMITW(0xE1A00040 | MRM(REG(RM), 0x00,    REG(RM)) |                 \
                                                 (0x1F & VAL(IM)) << 7)

#define shrwn_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1A00040 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IM)) << 7)     \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrwn_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        EMITW(0xE1A00050 | MRM(REG(RG), 0x00,    REG(RG)) | REG(RM) << 8)

#define shrwn_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1A00050 | MRM(REG(RG), 0x00,    REG(RG)) | TMxx << 8)

#define shrwn_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1A00050 | MRM(TMxx,    0x00,    TMxx) | REG(RG) << 8)      \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define shrwn_mr(RM, DP, RG)                                                \
        shrwn_st(W(RG), W(RM), W(DP))

/* mul
 * set-flags: undefined */

#define mulwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0xE0000090 | REG(RM) << 16 | REG(RM) << 8| TIxx)

#define mulwx_rr(RG, RM)                                                    \
        EMITW(0xE0000090 | REG(RG) << 16 | REG(RG) << 8| REG(RM))

#define mulwx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0000090 | REG(RG) << 16 | REG(RG) << 8| TMxx)


#define mulwx_xr(RM)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0xE0800090 | MRM(Teax,    Tedx,    REG(RM)) | Teax << 8)

#define mulwx_xm(RM, DP) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0800090 | MRM(Teax,    Tedx,    TMxx) | Teax << 8)


#define mulwn_xr(RM)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0xE0C00090 | MRM(Teax,    Tedx,    REG(RM)) | Teax << 8)

#define mulwn_xm(RM, DP) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0C00090 | MRM(Teax,    Tedx,    TMxx) | Teax << 8)


#define mulwp_xr(RM)     /* Reax is in/out, prepares Redx for divwn_x* */   \
        mulwx_rr(Reax, W(RM)) /* product must not exceed operands size */

#define mulwp_xm(RM, DP) /* Reax is in/out, prepares Redx for divwn_x* */   \
        mulwx_ld(Reax, W(RM), W(DP))  /* must not exceed operands size */

/* div
 * set-flags: undefined */

#if (RT_128 < 2) /* hw int-div is available in processors with ASIMDv2 */

#define divwx_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0xEC400B10 | MRM(REG(RM), TIxx,    Tmm0+0))                   \
        EMITW(0xEEB80B60 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* full-range */   \
        EMITW(0xEEB80B40 | MRM(Tmm0+0,  0x00,    Tmm0+0))/* 32-bit int */   \
        EMITW(0xEE800B00 | MRM(Tmm0+0,  Tmm0+0,  Tmm0+1))/* <-fp64 div */   \
        EMITW(0xEEBC0BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))                   \
        EMITW(0xEE100B10 | MRM(REG(RM), Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))

#define divwx_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        EMITW(0xEC400B10 | MRM(REG(RG), REG(RM), Tmm0+0))                   \
        EMITW(0xEEB80B60 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* full-range */   \
        EMITW(0xEEB80B40 | MRM(Tmm0+0,  0x00,    Tmm0+0))/* 32-bit int */   \
        EMITW(0xEE800B00 | MRM(Tmm0+0,  Tmm0+0,  Tmm0+1))/* <-fp64 div */   \
        EMITW(0xEEBC0BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))                   \
        EMITW(0xEE100B10 | MRM(REG(RG), Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))

#define divwx_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xEC400B10 | MRM(REG(RG), TMxx,    Tmm0+0))                   \
        EMITW(0xEEB80B60 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* full-range */   \
        EMITW(0xEEB80B40 | MRM(Tmm0+0,  0x00,    Tmm0+0))/* 32-bit int */   \
        EMITW(0xEE800B00 | MRM(Tmm0+0,  Tmm0+0,  Tmm0+1))/* <-fp64 div */   \
        EMITW(0xEEBC0BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))                   \
        EMITW(0xEE100B10 | MRM(REG(RG), Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))


#define divwn_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0xEC400B10 | MRM(REG(RM), TIxx,    Tmm0+0))                   \
        EMITW(0xEEB80BE0 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* full-range */   \
        EMITW(0xEEB80BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))/* 32-bit int */   \
        EMITW(0xEE800B00 | MRM(Tmm0+0,  Tmm0+0,  Tmm0+1))/* <-fp64 div */   \
        EMITW(0xEEBD0BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))                   \
        EMITW(0xEE100B10 | MRM(REG(RM), Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))

#define divwn_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        EMITW(0xEC400B10 | MRM(REG(RG), REG(RM), Tmm0+0))                   \
        EMITW(0xEEB80BE0 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* full-range */   \
        EMITW(0xEEB80BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))/* 32-bit int */   \
        EMITW(0xEE800B00 | MRM(Tmm0+0,  Tmm0+0,  Tmm0+1))/* <-fp64 div */   \
        EMITW(0xEEBD0BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))                   \
        EMITW(0xEE100B10 | MRM(REG(RG), Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))

#define divwn_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xEC400B10 | MRM(REG(RG), TMxx,    Tmm0+0))                   \
        EMITW(0xEEB80BE0 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* full-range */   \
        EMITW(0xEEB80BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))/* 32-bit int */   \
        EMITW(0xEE800B00 | MRM(Tmm0+0,  Tmm0+0,  Tmm0+1))/* <-fp64 div */   \
        EMITW(0xEEBD0BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))                   \
        EMITW(0xEE100B10 | MRM(REG(RG), Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))


#define prewx_xx()          /* to be placed immediately prior divwx_x* */   \
                                     /* to prepare Redx for int-divide */

#define prewn_xx()          /* to be placed immediately prior divwn_x* */   \
                                     /* to prepare Redx for int-divide */


#define divwx_xr(RM)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        EMITW(0xEC400B10 | MRM(Teax,    REG(RM), Tmm0+0))                   \
        EMITW(0xEEB80B60 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* full-range */   \
        EMITW(0xEEB80B40 | MRM(Tmm0+0,  0x00,    Tmm0+0))/* 32-bit int */   \
        EMITW(0xEE800B00 | MRM(Tmm0+0,  Tmm0+0,  Tmm0+1))/* <-fp64 div */   \
        EMITW(0xEEBC0BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))                   \
        EMITW(0xEE100B10 | MRM(Teax,    Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))

#define divwx_xm(RM, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xEC400B10 | MRM(Teax,    TMxx,    Tmm0+0))                   \
        EMITW(0xEEB80B60 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* full-range */   \
        EMITW(0xEEB80B40 | MRM(Tmm0+0,  0x00,    Tmm0+0))/* 32-bit int */   \
        EMITW(0xEE800B00 | MRM(Tmm0+0,  Tmm0+0,  Tmm0+1))/* <-fp64 div */   \
        EMITW(0xEEBC0BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))                   \
        EMITW(0xEE100B10 | MRM(Teax,    Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))


#define divwn_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        EMITW(0xEC400B10 | MRM(Teax,    REG(RM), Tmm0+0))                   \
        EMITW(0xEEB80BE0 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* full-range */   \
        EMITW(0xEEB80BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))/* 32-bit int */   \
        EMITW(0xEE800B00 | MRM(Tmm0+0,  Tmm0+0,  Tmm0+1))/* <-fp64 div */   \
        EMITW(0xEEBD0BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))                   \
        EMITW(0xEE100B10 | MRM(Teax,    Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))

#define divwn_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xEC400B10 | MRM(Teax,    TMxx,    Tmm0+0))                   \
        EMITW(0xEEB80BE0 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* full-range */   \
        EMITW(0xEEB80BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))/* 32-bit int */   \
        EMITW(0xEE800B00 | MRM(Tmm0+0,  Tmm0+0,  Tmm0+1))/* <-fp64 div */   \
        EMITW(0xEEBD0BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))                   \
        EMITW(0xEE100B10 | MRM(Teax,    Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))


#define divwp_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(0xEC400B10 | MRM(Teax,    REG(RM), Tmm0+0))/* part-range */   \
        EMITW(0xF3BB0600 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* 24-bit int */   \
        EMITW(0xEE800A20 | MRM(Tmm0+1,  Tmm0+1,  Tmm0+1))/* <-fp32 div */   \
        EMITW(0xF3BB0700 | MRM(Tmm0+0,  0x00,    Tmm0+1))/* Xmm0<-junk */   \
        EMITW(0xEE100B10 | MRM(Teax,    Tmm0+0,  0x00)) /* fallback to VFP */

#define divwp_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xEC400B10 | MRM(Teax,    TMxx,    Tmm0+0))/* part-range */   \
        EMITW(0xF3BB0600 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* 24-bit int */   \
        EMITW(0xEE800A20 | MRM(Tmm0+1,  Tmm0+1,  Tmm0+1))/* <-fp32 div */   \
        EMITW(0xF3BB0700 | MRM(Tmm0+0,  0x00,    Tmm0+1))/* Xmm0<-junk */   \
        EMITW(0xEE100B10 | MRM(Teax,    Tmm0+0,  0x00)) /* fallback to VFP */

#else /* RT_128 >= 2 */

#define divwx_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0xE730F010 | MRM(0x00,    REG(RM), REG(RM)) | TIxx << 8)

#define divwx_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        EMITW(0xE730F010 | MRM(0x00,    REG(RG), REG(RG)) | REG(RM) << 8)

#define divwx_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE730F010 | MRM(0x00,    REG(RG), REG(RG)) | TMxx << 8)


#define divwn_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0xE710F010 | MRM(0x00,    REG(RM), REG(RM)) | TIxx << 8)

#define divwn_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        EMITW(0xE710F010 | MRM(0x00,    REG(RG), REG(RG)) | REG(RM) << 8)

#define divwn_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE710F010 | MRM(0x00,    REG(RG), REG(RG)) | TMxx << 8)


#define prewx_xx()          /* to be placed immediately prior divwx_x* */   \
                                     /* to prepare Redx for int-divide */

#define prewn_xx()          /* to be placed immediately prior divwn_x* */   \
                                     /* to prepare Redx for int-divide */


#define divwx_xr(RM)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(0xE730F010 | MRM(0x00,    Teax,    Teax) | REG(RM) << 8)

#define divwx_xm(RM, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE730F010 | MRM(0x00,    Teax,    Teax) | TMxx << 8)


#define divwn_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(0xE710F010 | MRM(0x00,    Teax,    Teax) | REG(RM) << 8)

#define divwn_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE710F010 | MRM(0x00,    Teax,    Teax) | TMxx << 8)


#define divwp_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xr(W(RM))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divwp_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xm(W(RM), W(DP))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#endif /* RT_128 >= 2 */

/* rem
 * set-flags: undefined */

#define remwx_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movwx_rr(Redx, W(RM))                                               \
        divwx_ri(W(RM), W(IM))                                              \
        EMITW(0xE0600090 | MRM(Tedx,    REG(RM), REG(RM)) | TIxx << 8)      \
        stack_ld(Redx)

#define remwx_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        stack_st(Redx)                                                      \
        movwx_rr(Redx, W(RG))                                               \
        divwx_rr(W(RG), W(RM))                                              \
        EMITW(0xE0600090 | MRM(Tedx,    REG(RG), REG(RG)) | REG(RM) << 8)   \
        stack_ld(Redx)

#define remwx_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movwx_rr(Redx, W(RG))                                               \
        divwx_ld(W(RG), W(RM), W(DP))                                       \
        EMITW(0xE0600090 | MRM(Tedx,    REG(RG), REG(RG)) | TMxx << 8)      \
        stack_ld(Redx)


#define remwn_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movwx_rr(Redx, W(RM))                                               \
        divwn_ri(W(RM), W(IM))                                              \
        EMITW(0xE0600090 | MRM(Tedx,    REG(RM), REG(RM)) | TIxx << 8)      \
        stack_ld(Redx)

#define remwn_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        stack_st(Redx)                                                      \
        movwx_rr(Redx, W(RG))                                               \
        divwn_rr(W(RG), W(RM))                                              \
        EMITW(0xE0600090 | MRM(Tedx,    REG(RG), REG(RG)) | REG(RM) << 8)   \
        stack_ld(Redx)

#define remwn_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movwx_rr(Redx, W(RG))                                               \
        divwn_ld(W(RG), W(RM), W(DP))                                       \
        EMITW(0xE0600090 | MRM(Tedx,    REG(RG), REG(RG)) | TMxx << 8)      \
        stack_ld(Redx)


#define remwx_xx()          /* to be placed immediately prior divwx_x* */   \
        movwx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remwx_xr(RM)        /* to be placed immediately after divwx_xr */   \
        EMITW(0xE0600090 | MRM(Tedx,    Tedx,    REG(RM)) | Teax << 8)      \
                                                          /* Redx<-rem */

#define remwx_xm(RM, DP)    /* to be placed immediately after divwx_xm */   \
        EMITW(0xE0600090 | MRM(Tedx,    Tedx,    TMxx) | Teax << 8)         \
                                                          /* Redx<-rem */


#define remwn_xx()          /* to be placed immediately prior divwn_x* */   \
        movwx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remwn_xr(RM)        /* to be placed immediately after divwn_xr */   \
        EMITW(0xE0600090 | MRM(Tedx,    Tedx,    REG(RM)) | Teax << 8)      \
                                                          /* Redx<-rem */

#define remwn_xm(RM, DP)    /* to be placed immediately after divwn_xm */   \
        EMITW(0xE0600090 | MRM(Tedx,    Tedx,    TMxx) | Teax << 8)         \
                                                          /* Redx<-rem */

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


#define cmjwx_rz(RM, cc, lb)                                                \
        cmjwx_ri(W(RM), IC(0), cc, lb)

#define cmjwx_mz(RM, DP, cc, lb)                                            \
        cmjwx_mi(W(RM), W(DP), IC(0), cc, lb)

#define cmjwx_ri(RM, IM, cc, lb)                                            \
        cmpwx_ri(W(RM), W(IM))                                              \
        CMJ(cc, lb)

#define cmjwx_mi(RM, DP, IM, cc, lb)                                        \
        cmpwx_mi(W(RM), W(DP), W(IM))                                       \
        CMJ(cc, lb)

#define cmjwx_rr(RG, RM, cc, lb)                                            \
        cmpwx_rr(W(RG), W(RM))                                              \
        CMJ(cc, lb)

#define cmjwx_rm(RG, RM, DP, cc, lb)                                        \
        cmpwx_rm(W(RG), W(RM), W(DP))                                       \
        CMJ(cc, lb)

#define cmjwx_mr(RM, DP, RG, cc, lb)                                        \
        cmpwx_mr(W(RM), W(DP), W(RG))                                       \
        CMJ(cc, lb)

/* cmp
 * set-flags: yes */

#define cmpwx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0xE1500000 | MIM(0x00,    REG(RM), VAL(IM), T1(IM), M1(IM)))

#define cmpwx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1500000 | MIM(0x00,    TMxx,    VAL(IM), T1(IM), M1(IM)))

#define cmpwx_rr(RG, RM)                                                    \
        EMITW(0xE1500000 | MRM(0x00,    REG(RG), REG(RM)))

#define cmpwx_rm(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1500000 | MRM(0x00,    REG(RG), TMxx))

#define cmpwx_mr(RM, DP, RG)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1500000 | MRM(0x00,    TMxx,    REG(RG)))

/***************** pointer-sized instructions for 32-bit mode *****************/

/* jmp
 * set-flags: no
 * maximum byte-address-range for un/conditional jumps is signed 18/16-bit
 * based on minimum natively-encoded offset across supported targets (u/c)
 * MIPS:18-bit, Power:26-bit, AArch32:26-bit, AArch64:28-bit, x86:32-bit /
 * MIPS:18-bit, Power:16-bit, AArch32:26-bit, AArch64:21-bit, x86:32-bit */

#define jmpxx_xr(RM)           /* register-targeted unconditional jump */   \
        EMITW(0xE1A00000 | MRM(PCxx,    0x00,    REG(RM)))

#define jmpxx_xm(RM, DP)         /* memory-targeted unconditional jump */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5900000 | MDM(PCxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#define jmpxx_lb(lb)              /* label-targeted unconditional jump */   \
        ASM_BEG ASM_OP1(b,   lb) ASM_END

#define jezxx_lb(lb)               /* setting-flags-arithmetic -> jump */   \
        ASM_BEG ASM_OP1(beq, lb) ASM_END

#define jnzxx_lb(lb)               /* setting-flags-arithmetic -> jump */   \
        ASM_BEG ASM_OP1(bne, lb) ASM_END

#define jeqxx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(beq, lb) ASM_END

#define jnexx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(bne, lb) ASM_END

#define jltxx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(blo, lb) ASM_END

#define jlexx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(bls, lb) ASM_END

#define jgtxx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(bhi, lb) ASM_END

#define jgexx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(bhs, lb) ASM_END

#define jltxn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(blt, lb) ASM_END

#define jlexn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(ble, lb) ASM_END

#define jgtxn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(bgt, lb) ASM_END

#define jgexn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP1(bge, lb) ASM_END

#define LBL(lb)                                          /* code label */   \
        ASM_BEG ASM_OP0(lb:) ASM_END

/* stack
 * set-flags: no (sequence cmp/stack_la/jmp is not allowed on MIPS & Power)
 * adjust stack pointer with 4-byte (32-bit) steps on legacy 32-bit targets */

#define stack_st(RM)                                                        \
        EMITW(0xE5200004 | MRM(REG(RM), SPxx,    0x00))

#define stack_ld(RM)                                                        \
        EMITW(0xE4900004 | MRM(REG(RM), SPxx,    0x00))

#if RT_SIMD_FAST_FCTRL == 0

#define stack_sa()   /* save all, [Reax - Redi] + 4 temps, 11 regs total */ \
        EMITW(0xE9200BFF | MRM(0x00,    SPxx,    0x00))

#define stack_la()   /* load all, 4 temps + [Redi - Reax], 11 regs total */ \
        EMITW(0xE8B00BFF | MRM(0x00,    SPxx,    0x00))

#else /* RT_SIMD_FAST_FCTRL */

#define stack_sa()   /* save all, [Reax - Redi] + 7 temps, 14 regs total */ \
        EMITW(0xE9205FFF | MRM(0x00,    SPxx,    0x00))

#define stack_la()   /* load all, 7 temps + [Redi - Reax], 14 regs total */ \
        EMITW(0xE8B05FFF | MRM(0x00,    SPxx,    0x00))

#endif /* RT_SIMD_FAST_FCTRL */

/* ver
 * set-flags: no */

#define verxx_xx() /* destroys Reax, Recx, Rebx, Redx, Resi, Redi (in x86)*/\
        movwx_mi(Mebp, inf_VER, IB(7)) /* <- NEON to bit0, bit1, bit2 */

#endif /* RT_RTARCH_ARM_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

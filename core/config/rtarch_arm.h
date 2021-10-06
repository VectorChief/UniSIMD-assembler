/******************************************************************************/
/* Copyright (c) 2013-2021 VectorChief (at github, bitbucket, sourceforge)    */
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
 * rtarch_arm.h: Implementation of ARMv7 32-bit BASE instructions.
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
 * cmd**Z** - applies [cmd] while setting condition flags, [Z] - zero flag.
 * Regular cmd*x_**, cmd*n_** instructions may or may not set flags depending
 * on the target architecture, thus no assumptions can be made for jezxx/jnzxx.
 *
 * Interpretation of instruction parameters:
 *
 * upper-case params have triplet structure and require W to pass-forward
 * lower-case params are singular and can be used/passed as such directly
 *
 * RD - BASE register serving as destination only, if present
 * RG - BASE register serving as destination and first source
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
 * Configurable BASE/SIMD subsets (cmdx*, cmdy*, cmdp*) are defined in rtconf.h.
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
 * a given (one) subset if appropriate offset correction is used from rtbase.h.
 *
 * Setting-flags instruction naming scheme has been changed again recently for
 * better orthogonality with operand size, type and args-list. It is therefore
 * recommended to use combined-arithmetic-jump (arj) for better API stability
 * and maximum efficiency across all supported targets. For similar reasons
 * of higher performance on MIPS and POWER use combined-compare-jump (cmj).
 * Not all canonical forms of BASE instructions have efficient implementation.
 * For example, some forms of shifts and division use stack ops on x86 targets,
 * while standalone remainders can only be done natively on MIPSr6 and POWER9.
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
        ((ren) << 16 | (reg) << 12 | (rem))

#define MDM(reg, brm, vdp, bxx, pxx)                                        \
        (bxx(brm) << 16 | (reg) << 12 | pxx(vdp))

#define MIM(reg, ren, vim, txx, mxx)                                        \
        ((ren) << 16 | txx(reg) | mxx(vim))

#define AUW(sib, vim, reg, brm, vdp, cdp, cim)                              \
            sib  cdp(brm, vdp)  cim(reg, vim)

#ifdef RT_BASE_COMPAT_DIV
#undef  RT_ARM
#define RT_ARM RT_BASE_COMPAT_DIV /* 0/1 - int-div emul, 2 - hardware int-div */
#endif /* RT_BASE_COMPAT_DIV */

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
#define  B3(val, tp1, tp2)  B3##tp1
#define  BH(val, tp1, tp2)  BH##tp1
#define  P1(val, tp1, tp2)  P1##tp1
#define  PH(val, tp1, tp2)  PH##tp1
#define  C1(val, tp1, tp2)  C1##tp1
#define  A1(val, tp1, tp2)  A1##tp1
#define  AH(val, tp1, tp2)  AH##tp1
#define  C3(val, tp1, tp2)  C3##tp2 /* <- "C3##tp2" not a bug */

/* immediate encoding add/sub/cmp(TP1), and/orr/xor(TP2), mov/mul(TP3) */

#define T10(tr) ((tr) << 12)
#define M10(im) (0x02000000 |(0x00FF & (im)))
#define G10(rg, im) EMPTY
#define T20(tr) ((tr) << 12)
#define M20(im) (0x02000000 |(0x00FF & (im)))
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
#define B30(br) (br)
#define BH0(br) TPxx
#define P10(dp) (0x00000000 |(0x0FFC & (dp)))
#define PH0(dp) (0x00000000)
#define C10(br, dp) EMPTY
#define A10(br, dp) EMPTY
#define AH0(br, dp) C30(br, dp)                                             \
                    EMITW(0xE0800000 | MRM(TPxx,    (br),    TDxx))
#define C30(br, dp) EMITW(0xE3000000 | MRM(TDxx,    0x00,    0x00) |        \
                            (0xF0000 & (dp) <<  4) | (0xFFF & (dp)))

#define B11(br) (br)
#define B31(br) TPxx
#define BH1(br) TPxx
#define P11(dp) (0x00000000)
#define PH1(dp) (0x00000000)
#define C11(br, dp) C31(br, dp)
#define A11(br, dp) C31(br, dp)                                             \
                    EMITW(0xE0800000 | MRM(TPxx,    (br),    TDxx))
#define AH1(br, dp) A11(br, dp)
#define C31(br, dp) EMITW(0xE3000000 | MRM(TDxx,    0x00,    0x00) |        \
                            (0xF0000 & (dp) <<  4) | (0xFFF & (dp)))

#define B12(br) (br)
#define B32(br) TPxx
#define BH2(br) TPxx
#define P12(dp) (0x00000000)
#define PH2(dp) (0x00000000)
#define C12(br, dp) C32(br, dp)
#define A12(br, dp) C32(br, dp)                                             \
                    EMITW(0xE0800000 | MRM(TPxx,    (br),    TDxx))
#define AH2(br, dp) A12(br, dp)
#define C32(br, dp) EMITW(0xE3000000 | MRM(TDxx,    0x00,    0x00) |        \
                            (0xF0000 & (dp) <<  4) | (0xFFF & (dp)))        \
                    EMITW(0xE3400000 | MRM(TDxx,    0x00,    0x00) |        \
                            (0x70000 & (dp) >> 12) | (0xFFF & (dp) >> 16))

/* registers    REG   (check mapping with ASM_ENTER/ASM_LEAVE in rtarch.h) */
/* four registers TNxx,TAxx,TCxx,TExx below must retain their current mapping */

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

#define Jecx    Tecx, TPxx, EMITW(0xE0800080 | MRM(TPxx,    Tecx,    Teax))
#define Jedx    Tedx, TPxx, EMITW(0xE0800080 | MRM(TPxx,    Tedx,    Teax))
#define Jebx    Tebx, TPxx, EMITW(0xE0800080 | MRM(TPxx,    Tebx,    Teax))
#define Jebp    Tebp, TPxx, EMITW(0xE0800080 | MRM(TPxx,    Tebp,    Teax))
#define Jesi    Tesi, TPxx, EMITW(0xE0800080 | MRM(TPxx,    Tesi,    Teax))
#define Jedi    Tedi, TPxx, EMITW(0xE0800080 | MRM(TPxx,    Tedi,    Teax))

#define Kecx    Tecx, TPxx, EMITW(0xE0800100 | MRM(TPxx,    Tecx,    Teax))
#define Kedx    Tedx, TPxx, EMITW(0xE0800100 | MRM(TPxx,    Tedx,    Teax))
#define Kebx    Tebx, TPxx, EMITW(0xE0800100 | MRM(TPxx,    Tebx,    Teax))
#define Kebp    Tebp, TPxx, EMITW(0xE0800100 | MRM(TPxx,    Tebp,    Teax))
#define Kesi    Tesi, TPxx, EMITW(0xE0800100 | MRM(TPxx,    Tesi,    Teax))
#define Kedi    Tedi, TPxx, EMITW(0xE0800100 | MRM(TPxx,    Tedi,    Teax))

#define Lecx    Tecx, TPxx, EMITW(0xE0800180 | MRM(TPxx,    Tecx,    Teax))
#define Ledx    Tedx, TPxx, EMITW(0xE0800180 | MRM(TPxx,    Tedx,    Teax))
#define Lebx    Tebx, TPxx, EMITW(0xE0800180 | MRM(TPxx,    Tebx,    Teax))
#define Lebp    Tebp, TPxx, EMITW(0xE0800180 | MRM(TPxx,    Tebp,    Teax))
#define Lesi    Tesi, TPxx, EMITW(0xE0800180 | MRM(TPxx,    Tesi,    Teax))
#define Ledi    Tedi, TPxx, EMITW(0xE0800180 | MRM(TPxx,    Tedi,    Teax))

/* immediate    VAL,  TP1,  TP2            (all immediate types are unsigned) */
/* full-size IW type is only applicable within cmdw* subset, can set sign-bit */
/* within cmdz* subset use of IW type is limited to movzx_rj/_mj instructions */

#define  IC(im) ((im) & 0x7F),          0, 0        /* drop sign-ext (on x86) */
#define  IB(im) ((im) & 0xFF),          0, 0          /* 32-bit word (on x86) */
#define  IM(im) ((im) & 0xFFF),         1, 1    /* native AArch64 add/sub/cmp */
#define  IG(im) ((im) & 0x7FFF),        1, 1 /* native MIPS/POWER add/sub/cmp */
#define  IH(im) ((im) & 0xFFFF),        1, 1    /* second native on ARMs/MIPS */
#define  IV(im) ((im) & 0x7FFFFFFF),    2, 2       /* native x86_64 long mode */
#define  IW(im) ((im) & 0xFFFFFFFF),    2, 2          /* for cmdw* subset, *j */

/* displacement VAL,  TP1,  TP2         (all displacement types are unsigned) */
/* public scalable DP/DE/DF/DG/DH/DV definitions are now provided in rtbase.h */
/* as D* are used for BASE and SIMD instructions, only limits are SIMD-scaled */

#define _DP(dp) ((dp) & 0xFFF),         0, 0      /* native on all ARMs, MIPS */
#define _DE(dp) ((dp) & 0x1FFF),        1, 1     /* AArch64 256-bit SVE ld/st */
#define _DF(dp) ((dp) & 0x3FFF),        1, 1     /* native AArch64 BASE ld/st */
#define _DG(dp) ((dp) & 0x7FFF),        1, 1  /* native MIPS/POWER BASE ld/st */
#define _DH(dp) ((dp) & 0xFFFF),        1, 1     /* second native on all ARMs */
#define _DV(dp) ((dp) & 0x7FFFFFFF),    2, 2       /* native x86_64 long mode */
#define  PLAIN  DP(0)                /* special type for Oeax addressing mode */

/* triplet pass-through wrapper */

#define W(p1, p2, p3)       p1,  p2,  p3

/******************************************************************************/
/**********************************   BASE   **********************************/
/******************************************************************************/

/* mov (D = S)
 * set-flags: no */

#define movwx_ri(RD, IS)                                                    \
        AUW(EMPTY,    VAL(IS), REG(RD), EMPTY,   EMPTY,   EMPTY2, G3(IS))   \

#define movwx_mi(MD, DD, IS)                                                \
        AUW(SIB(MD),  VAL(IS), TIxx,    MOD(MD), VAL(DD), A1(DD), G3(IS))   \
        EMITW(0xE5800000 | MDM(TIxx,    MOD(MD), VAL(DD), B3(DD), P1(DD)))

#define movwx_rr(RD, RS)                                                    \
        EMITW(0xE1A00000 | MRM(REG(RD), 0x00,    REG(RS)))

#define movhn_rr(RD, RS)       /* move 16-bit to 32-bit with sign-extend */ \
        EMITW(0xE6BF0070 | MRM(REG(RD), 0x00,    REG(RS)))

#define movhz_rr(RD, RS)       /* move 16-bit to 32-bit with zero-extend */ \
        EMITW(0xE6FF0070 | MRM(REG(RD), 0x00,    REG(RS)))

#define movbn_rr(RD, RS)       /* move  8-bit to 32-bit with sign-extend */ \
        EMITW(0xE6AF0070 | MRM(REG(RD), 0x00,    REG(RS)))

#define movbz_rr(RD, RS)       /* move  8-bit to 32-bit with zero-extend */ \
        EMITW(0xE6EF0070 | MRM(REG(RD), 0x00,    REG(RS)))

#define movwx_ld(RD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(REG(RD), MOD(MS), VAL(DS), B3(DS), P1(DS)))

#define movhn_ld(RD, MS, DS)   /* load 16-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000F0 | MDM(REG(RD), MOD(MS), VAL(DS), BH(DS), PH(DS)))

#define movhz_ld(RD, MS, DS)   /* load 16-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(REG(RD), MOD(MS), VAL(DS), BH(DS), PH(DS)))

#define movbn_ld(RD, MS, DS)   /* load  8-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000D0 | MDM(REG(RD), MOD(MS), VAL(DS), BH(DS), PH(DS)))

#define movbz_ld(RD, MS, DS)   /* load  8-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(REG(RD), MOD(MS), VAL(DS), B3(DS), P1(DS)))

#define movwx_st(RS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), A1(DD), EMPTY2)   \
        EMITW(0xE5800000 | MDM(REG(RS), MOD(MD), VAL(DD), B3(DD), P1(DD)))


#define movwx_rj(RD, IT, IS)     /* IT - upper 32-bit, IS - lower 32-bit */ \
        movwx_ri(W(RD), W(IS))

#define movwx_mj(MD, DD, IT, IS) /* IT - upper 32-bit, IS - lower 32-bit */ \
        movwx_mi(W(MD), W(DD), W(IS))

/* and (G = G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define andwx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0xE0000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define andwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), A1(DG), G2(IS))   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define andwx_rr(RG, RS)                                                    \
        EMITW(0xE0000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define andwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE0000000 | MRM(REG(RG), REG(RG), TMxx))

#define andwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define andwx_mr(MG, DG, RS)                                                \
        andwx_st(W(RS), W(MG), W(DG))


#define andwxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0xE0100000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define andwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), A1(DG), G2(IS))   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0100000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define andwxZrr(RG, RS)                                                    \
        EMITW(0xE0100000 | MRM(REG(RG), REG(RG), REG(RS)))

#define andwxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE0100000 | MRM(REG(RG), REG(RG), TMxx))

#define andwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0100000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define andwxZmr(MG, DG, RS)                                                \
        andwxZst(W(RS), W(MG), W(DG))

/* ann (G = ~G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define annwx_ri(RG, IS)                                                    \
        notwx_rx(W(RG))                                                     \
        andwx_ri(W(RG), W(IS))

#define annwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), A1(DG), G2(IS))   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1E00000 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xE0000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define annwx_rr(RG, RS)                                                    \
        EMITW(0xE1C00000 | MRM(REG(RG), REG(RS), REG(RG)))

#define annwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE1C00000 | MRM(REG(RG), TMxx,    REG(RG)))

#define annwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1C00000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define annwx_mr(MG, DG, RS)                                                \
        annwx_st(W(RS), W(MG), W(DG))


#define annwxZri(RG, IS)                                                    \
        notwx_rx(W(RG))                                                     \
        andwxZri(W(RG), W(IS))

#define annwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), A1(DG), G2(IS))   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1E00000 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xE0100000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define annwxZrr(RG, RS)                                                    \
        EMITW(0xE1D00000 | MRM(REG(RG), REG(RS), REG(RG)))

#define annwxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE1D00000 | MRM(REG(RG), TMxx,    REG(RG)))

#define annwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1D00000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define annwxZmr(MG, DG, RS)                                                \
        annwxZst(W(RS), W(MG), W(DG))

/* orr (G = G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define orrwx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0xE1800000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define orrwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), A1(DG), G2(IS))   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1800000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define orrwx_rr(RG, RS)                                                    \
        EMITW(0xE1800000 | MRM(REG(RG), REG(RG), REG(RS)))

#define orrwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE1800000 | MRM(REG(RG), REG(RG), TMxx))

#define orrwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1800000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define orrwx_mr(MG, DG, RS)                                                \
        orrwx_st(W(RS), W(MG), W(DG))


#define orrwxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0xE1900000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define orrwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), A1(DG), G2(IS))   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1900000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define orrwxZrr(RG, RS)                                                    \
        EMITW(0xE1900000 | MRM(REG(RG), REG(RG), REG(RS)))

#define orrwxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE1900000 | MRM(REG(RG), REG(RG), TMxx))

#define orrwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1900000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define orrwxZmr(MG, DG, RS)                                                \
        orrwxZst(W(RS), W(MG), W(DG))

/* orn (G = ~G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define ornwx_ri(RG, IS)                                                    \
        notwx_rx(W(RG))                                                     \
        orrwx_ri(W(RG), W(IS))

#define ornwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), A1(DG), G2(IS))   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1E00000 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xE1800000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define ornwx_rr(RG, RS)                                                    \
        notwx_rx(W(RG))                                                     \
        orrwx_rr(W(RG), W(RS))

#define ornwx_ld(RG, MS, DS)                                                \
        notwx_rx(W(RG))                                                     \
        orrwx_ld(W(RG), W(MS), W(DS))

#define ornwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1E00000 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xE1800000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define ornwx_mr(MG, DG, RS)                                                \
        ornwx_st(W(RS), W(MG), W(DG))


#define ornwxZri(RG, IS)                                                    \
        notwx_rx(W(RG))                                                     \
        orrwxZri(W(RG), W(IS))

#define ornwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), A1(DG), G2(IS))   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1E00000 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xE1900000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define ornwxZrr(RG, RS)                                                    \
        notwx_rx(W(RG))                                                     \
        orrwxZrr(W(RG), W(RS))

#define ornwxZld(RG, MS, DS)                                                \
        notwx_rx(W(RG))                                                     \
        orrwxZld(W(RG), W(MS), W(DS))

#define ornwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1E00000 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xE1900000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define ornwxZmr(MG, DG, RS)                                                \
        ornwxZst(W(RS), W(MG), W(DG))

/* xor (G = G ^ S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define xorwx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0xE0200000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define xorwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), A1(DG), G2(IS))   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0200000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define xorwx_rr(RG, RS)                                                    \
        EMITW(0xE0200000 | MRM(REG(RG), REG(RG), REG(RS)))

#define xorwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE0200000 | MRM(REG(RG), REG(RG), TMxx))

#define xorwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0200000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define xorwx_mr(MG, DG, RS)                                                \
        xorwx_st(W(RS), W(MG), W(DG))


#define xorwxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0xE0300000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define xorwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), A1(DG), G2(IS))   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0300000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define xorwxZrr(RG, RS)                                                    \
        EMITW(0xE0300000 | MRM(REG(RG), REG(RG), REG(RS)))

#define xorwxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE0300000 | MRM(REG(RG), REG(RG), TMxx))

#define xorwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0300000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define xorwxZmr(MG, DG, RS)                                                \
        xorwxZst(W(RS), W(MG), W(DG))

/* not (G = ~G)
 * set-flags: no */

#define notwx_rx(RG)                                                        \
        EMITW(0xE1E00000 | MRM(REG(RG), 0x00,    REG(RG)))

#define notwx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1E00000 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

/* neg (G = -G)
 * set-flags: undefined (*_*), yes (*Z*) */

#define negwx_rx(RG)                                                        \
        EMITW(0xE2600000 | MRM(REG(RG), REG(RG), 0x00))

#define negwx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE2600000 | MRM(TMxx,    TMxx,    0x00))                     \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))


#define negwxZrx(RG)                                                        \
        EMITW(0xE2700000 | MRM(REG(RG), REG(RG), 0x00))

#define negwxZmx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE2700000 | MRM(TMxx,    TMxx,    0x00))                     \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

/* add (G = G + S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define addwx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0xE0800000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define addwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), A1(DG), G1(IS))   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0800000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define addwx_rr(RG, RS)                                                    \
        EMITW(0xE0800000 | MRM(REG(RG), REG(RG), REG(RS)))

#define addwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE0800000 | MRM(REG(RG), REG(RG), TMxx))

#define addhn_ld(RG, MS, DS)    /* add 16-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000F0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE0800000 | MRM(REG(RG), REG(RG), TMxx))

#define addhz_ld(RG, MS, DS)    /* add 16-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE0800000 | MRM(REG(RG), REG(RG), TMxx))

#define addbn_ld(RG, MS, DS)    /* add  8-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000D0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE0800000 | MRM(REG(RG), REG(RG), TMxx))

#define addbz_ld(RG, MS, DS)    /* add  8-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE0800000 | MRM(REG(RG), REG(RG), TMxx))

#define addwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0800000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define addwx_mr(MG, DG, RS)                                                \
        addwx_st(W(RS), W(MG), W(DG))


#define addwxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0xE0900000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define addwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), A1(DG), G1(IS))   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0900000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define addwxZrr(RG, RS)                                                    \
        EMITW(0xE0900000 | MRM(REG(RG), REG(RG), REG(RS)))

#define addwxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE0900000 | MRM(REG(RG), REG(RG), TMxx))

#define addhnZld(RG, MS, DS)    /* add 16-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000F0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE0900000 | MRM(REG(RG), REG(RG), TMxx))

#define addhzZld(RG, MS, DS)    /* add 16-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE0900000 | MRM(REG(RG), REG(RG), TMxx))

#define addbnZld(RG, MS, DS)    /* add  8-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000D0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE0900000 | MRM(REG(RG), REG(RG), TMxx))

#define addbzZld(RG, MS, DS)    /* add  8-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE0900000 | MRM(REG(RG), REG(RG), TMxx))

#define addwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0900000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define addwxZmr(MG, DG, RS)                                                \
        addwxZst(W(RS), W(MG), W(DG))

/* sub (G = G - S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define subwx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0xE0400000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define subwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), A1(DG), G1(IS))   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0400000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define subwx_rr(RG, RS)                                                    \
        EMITW(0xE0400000 | MRM(REG(RG), REG(RG), REG(RS)))

#define subwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE0400000 | MRM(REG(RG), REG(RG), TMxx))

#define subhn_ld(RG, MS, DS)  /* sub 16-bit from 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000F0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE0400000 | MRM(REG(RG), REG(RG), TMxx))

#define subhz_ld(RG, MS, DS)  /* sub 16-bit from 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE0400000 | MRM(REG(RG), REG(RG), TMxx))

#define subbn_ld(RG, MS, DS)  /* sub  8-bit from 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000D0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE0400000 | MRM(REG(RG), REG(RG), TMxx))

#define subbz_ld(RG, MS, DS)  /* sub  8-bit from 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE0400000 | MRM(REG(RG), REG(RG), TMxx))

#define subwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0400000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define subwx_mr(MG, DG, RS)                                                \
        subwx_st(W(RS), W(MG), W(DG))


#define subwxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0xE0500000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define subwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), A1(DG), G1(IS))   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0500000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define subwxZrr(RG, RS)                                                    \
        EMITW(0xE0500000 | MRM(REG(RG), REG(RG), REG(RS)))

#define subwxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE0500000 | MRM(REG(RG), REG(RG), TMxx))

#define subhnZld(RG, MS, DS)  /* sub 16-bit from 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000F0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE0500000 | MRM(REG(RG), REG(RG), TMxx))

#define subhzZld(RG, MS, DS)  /* sub 16-bit from 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE0500000 | MRM(REG(RG), REG(RG), TMxx))

#define subbnZld(RG, MS, DS)  /* sub  8-bit from 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000D0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE0500000 | MRM(REG(RG), REG(RG), TMxx))

#define subbzZld(RG, MS, DS)  /* sub  8-bit from 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE0500000 | MRM(REG(RG), REG(RG), TMxx))

#define subwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0500000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define subwxZmr(MG, DG, RS)                                                \
        subwxZst(W(RS), W(MG), W(DG))

/* shl (G = G << S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlwx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0xE1A00010 | MRM(REG(RG), 0x00,    REG(RG)) | Tecx << 8)

#define shlwx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1A00010 | MRM(TMxx,    0x00,    TMxx) | Tecx << 8)         \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shlwx_ri(RG, IS)                                                    \
        EMITW(0xE1A00000 | MRM(REG(RG), 0x00,    REG(RG)) |                 \
                                                 (0x1F & VAL(IS)) << 7)

#define shlwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1A00000 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IS)) << 7)     \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shlwx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0xE1A00010 | MRM(REG(RG), 0x00,    REG(RG)) | REG(RS) << 8)

#define shlwx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE1A00010 | MRM(REG(RG), 0x00,    REG(RG)) | TMxx << 8)

#define shlwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1A00010 | MRM(TMxx,    0x00,    TMxx) | REG(RS) << 8)      \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shlwx_mr(MG, DG, RS)                                                \
        shlwx_st(W(RS), W(MG), W(DG))


#define shlwxZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0xE1B00010 | MRM(REG(RG), 0x00,    REG(RG)) | Tecx << 8)

#define shlwxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1B00010 | MRM(TMxx,    0x00,    TMxx) | Tecx << 8)         \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shlwxZri(RG, IS)                                                    \
        EMITW(0xE1B00000 | MRM(REG(RG), 0x00,    REG(RG)) |                 \
                                                 (0x1F & VAL(IS)) << 7)

#define shlwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1B00000 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IS)) << 7)     \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shlwxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0xE1B00010 | MRM(REG(RG), 0x00,    REG(RG)) | REG(RS) << 8)

#define shlwxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE1B00010 | MRM(REG(RG), 0x00,    REG(RG)) | TMxx << 8)

#define shlwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1B00010 | MRM(TMxx,    0x00,    TMxx) | REG(RS) << 8)      \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shlwxZmr(MG, DG, RS)                                                \
        shlwxZst(W(RS), W(MG), W(DG))

/* shr (G = G >> S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrwx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0xE1A00030 | MRM(REG(RG), 0x00,    REG(RG)) | Tecx << 8)

#define shrwx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1A00030 | MRM(TMxx,    0x00,    TMxx) | Tecx << 8)         \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shrwx_ri(RG, IS)                                                    \
        EMITW(0xE1A00000 | MRM(REG(RG), 0x00,    REG(RG)) |                 \
        (M(VAL(IS) == 0) & 0x00000000) | (M(VAL(IS) != 0) & 0x00000020) |   \
                                                 (0x1F & VAL(IS)) << 7)

#define shrwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1A00020 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IS)) << 7)     \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shrwx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0xE1A00030 | MRM(REG(RG), 0x00,    REG(RG)) | REG(RS) << 8)

#define shrwx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE1A00030 | MRM(REG(RG), 0x00,    REG(RG)) | TMxx << 8)

#define shrwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1A00030 | MRM(TMxx,    0x00,    TMxx) | REG(RS) << 8)      \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shrwx_mr(MG, DG, RS)                                                \
        shrwx_st(W(RS), W(MG), W(DG))


#define shrwxZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0xE1B00030 | MRM(REG(RG), 0x00,    REG(RG)) | Tecx << 8)

#define shrwxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1B00030 | MRM(TMxx,    0x00,    TMxx) | Tecx << 8)         \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shrwxZri(RG, IS)                                                    \
        EMITW(0xE1B00000 | MRM(REG(RG), 0x00,    REG(RG)) |                 \
        (M(VAL(IS) == 0) & 0x00000000) | (M(VAL(IS) != 0) & 0x00000020) |   \
                                                 (0x1F & VAL(IS)) << 7)

#define shrwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1B00020 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IS)) << 7)     \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shrwxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0xE1B00030 | MRM(REG(RG), 0x00,    REG(RG)) | REG(RS) << 8)

#define shrwxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE1B00030 | MRM(REG(RG), 0x00,    REG(RG)) | TMxx << 8)

#define shrwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1B00030 | MRM(TMxx,    0x00,    TMxx) | REG(RS) << 8)      \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shrwxZmr(MG, DG, RS)                                                \
        shrwxZst(W(RS), W(MG), W(DG))


#define shrwn_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0xE1A00050 | MRM(REG(RG), 0x00,    REG(RG)) | Tecx << 8)

#define shrwn_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1A00050 | MRM(TMxx,    0x00,    TMxx) | Tecx << 8)         \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shrwn_ri(RG, IS)                                                    \
        EMITW(0xE1A00000 | MRM(REG(RG), 0x00,    REG(RG)) |                 \
        (M(VAL(IS) == 0) & 0x00000000) | (M(VAL(IS) != 0) & 0x00000040) |   \
                                                 (0x1F & VAL(IS)) << 7)

#define shrwn_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1A00040 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IS)) << 7)     \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shrwn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0xE1A00050 | MRM(REG(RG), 0x00,    REG(RG)) | REG(RS) << 8)

#define shrwn_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE1A00050 | MRM(REG(RG), 0x00,    REG(RG)) | TMxx << 8)

#define shrwn_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1A00050 | MRM(TMxx,    0x00,    TMxx) | REG(RS) << 8)      \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shrwn_mr(MG, DG, RS)                                                \
        shrwn_st(W(RS), W(MG), W(DG))


#define shrwnZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0xE1B00050 | MRM(REG(RG), 0x00,    REG(RG)) | Tecx << 8)

#define shrwnZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1B00050 | MRM(TMxx,    0x00,    TMxx) | Tecx << 8)         \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shrwnZri(RG, IS)                                                    \
        EMITW(0xE1B00000 | MRM(REG(RG), 0x00,    REG(RG)) |                 \
        (M(VAL(IS) == 0) & 0x00000000) | (M(VAL(IS) != 0) & 0x00000040) |   \
                                                 (0x1F & VAL(IS)) << 7)

#define shrwnZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1B00040 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IS)) << 7)     \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shrwnZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0xE1B00050 | MRM(REG(RG), 0x00,    REG(RG)) | REG(RS) << 8)

#define shrwnZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE1B00050 | MRM(REG(RG), 0x00,    REG(RG)) | TMxx << 8)

#define shrwnZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1B00050 | MRM(TMxx,    0x00,    TMxx) | REG(RS) << 8)      \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shrwnZmr(MG, DG, RS)                                                \
        shrwnZst(W(RS), W(MG), W(DG))

/* ror (G = G >> S | G << 32 - S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define rorwx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0xE1A00070 | MRM(REG(RG), 0x00,    REG(RG)) | Tecx << 8)

#define rorwx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1A00070 | MRM(TMxx,    0x00,    TMxx) | Tecx << 8)         \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define rorwx_ri(RG, IS)                                                    \
        EMITW(0xE1A00060 | MRM(REG(RG), 0x00,    REG(RG)) |                 \
                                                 (0x1F & VAL(IS)) << 7)

#define rorwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1A00060 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IS)) << 7)     \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define rorwx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0xE1A00070 | MRM(REG(RG), 0x00,    REG(RG)) | REG(RS) << 8)

#define rorwx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE1A00070 | MRM(REG(RG), 0x00,    REG(RG)) | TMxx << 8)

#define rorwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1A00070 | MRM(TMxx,    0x00,    TMxx) | REG(RS) << 8)      \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define rorwx_mr(MG, DG, RS)                                                \
        rorwx_st(W(RS), W(MG), W(DG))


#define rorwxZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0xE1B00070 | MRM(REG(RG), 0x00,    REG(RG)) | Tecx << 8)

#define rorwxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1B00070 | MRM(TMxx,    0x00,    TMxx) | Tecx << 8)         \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define rorwxZri(RG, IS)                                                    \
        EMITW(0xE1B00060 | MRM(REG(RG), 0x00,    REG(RG)) |                 \
                                                 (0x1F & VAL(IS)) << 7)

#define rorwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1B00060 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IS)) << 7)     \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define rorwxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0xE1B00070 | MRM(REG(RG), 0x00,    REG(RG)) | REG(RS) << 8)

#define rorwxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE1B00070 | MRM(REG(RG), 0x00,    REG(RG)) | TMxx << 8)

#define rorwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1B00070 | MRM(TMxx,    0x00,    TMxx) | REG(RS) << 8)      \
        EMITW(0xE5800000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define rorwxZmr(MG, DG, RS)                                                \
        rorwxZst(W(RS), W(MG), W(DG))

/* mul (G = G * S)
 * set-flags: undefined */

#define mulwx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0xE0000090 | REG(RG) << 16 | REG(RG) << 8| TIxx)

#define mulwx_rr(RG, RS)                                                    \
        EMITW(0xE0000090 | REG(RG) << 16 | REG(RG) << 8| REG(RS))

#define mulwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE0000090 | REG(RG) << 16 | REG(RG) << 8| TMxx)

#define mulhn_ld(RG, MS, DS)  /* mul 32-bit with 16-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000F0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE0000090 | REG(RG) << 16 | REG(RG) << 8| TMxx)

#define mulhz_ld(RG, MS, DS)  /* mul 32-bit with 16-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE0000090 | REG(RG) << 16 | REG(RG) << 8| TMxx)

#define mulbn_ld(RG, MS, DS)  /* mul 32-bit with  8-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000D0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE0000090 | REG(RG) << 16 | REG(RG) << 8| TMxx)

#define mulbz_ld(RG, MS, DS)  /* mul 32-bit with  8-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE0000090 | REG(RG) << 16 | REG(RG) << 8| TMxx)


#define mulwx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0xE0800090 | MRM(Teax,    Tedx,    REG(RS)) | Teax << 8)

#define mulwx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE0800090 | MRM(Teax,    Tedx,    TMxx) | Teax << 8)


#define mulwn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0xE0C00090 | MRM(Teax,    Tedx,    REG(RS)) | Teax << 8)

#define mulwn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE0C00090 | MRM(Teax,    Tedx,    TMxx) | Teax << 8)


#define mulwp_xr(RS)     /* Reax is in/out, prepares Redx for divwn_x* */   \
        mulwx_rr(Reax, W(RS)) /* product must not exceed operands size */

#define mulwp_xm(MS, DS) /* Reax is in/out, prepares Redx for divwn_x* */   \
        mulwx_ld(Reax, W(MS), W(DS))  /* must not exceed operands size */

/* div (G = G / S)
 * set-flags: undefined */

#if (RT_BASE_COMPAT_DIV < 2) /* no int-div for Cortex-A8/A9 + NEONv1, fp-emul */

#define divwx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0xEC400B10 | MRM(REG(RG), TIxx,    Tmm0+0))                   \
        EMITW(0xEEB80B60 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* full-range */   \
        EMITW(0xEEB80B40 | MRM(Tmm0+0,  0x00,    Tmm0+0))/* 32-bit int */   \
        EMITW(0xEE800B00 | MRM(Tmm0+0,  Tmm0+0,  Tmm0+1))/* <-fp64 div */   \
        EMITW(0xEEBC0BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))                   \
        EMITW(0xEE100B10 | MRM(REG(RG), Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))

#define divwx_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        EMITW(0xEC400B10 | MRM(REG(RG), REG(RS), Tmm0+0))                   \
        EMITW(0xEEB80B60 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* full-range */   \
        EMITW(0xEEB80B40 | MRM(Tmm0+0,  0x00,    Tmm0+0))/* 32-bit int */   \
        EMITW(0xEE800B00 | MRM(Tmm0+0,  Tmm0+0,  Tmm0+1))/* <-fp64 div */   \
        EMITW(0xEEBC0BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))                   \
        EMITW(0xEE100B10 | MRM(REG(RG), Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))

#define divwx_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xEC400B10 | MRM(REG(RG), TMxx,    Tmm0+0))                   \
        EMITW(0xEEB80B60 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* full-range */   \
        EMITW(0xEEB80B40 | MRM(Tmm0+0,  0x00,    Tmm0+0))/* 32-bit int */   \
        EMITW(0xEE800B00 | MRM(Tmm0+0,  Tmm0+0,  Tmm0+1))/* <-fp64 div */   \
        EMITW(0xEEBC0BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))                   \
        EMITW(0xEE100B10 | MRM(REG(RG), Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))


#define divwn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0xEC400B10 | MRM(REG(RG), TIxx,    Tmm0+0))                   \
        EMITW(0xEEB80BE0 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* full-range */   \
        EMITW(0xEEB80BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))/* 32-bit int */   \
        EMITW(0xEE800B00 | MRM(Tmm0+0,  Tmm0+0,  Tmm0+1))/* <-fp64 div */   \
        EMITW(0xEEBD0BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))                   \
        EMITW(0xEE100B10 | MRM(REG(RG), Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))

#define divwn_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        EMITW(0xEC400B10 | MRM(REG(RG), REG(RS), Tmm0+0))                   \
        EMITW(0xEEB80BE0 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* full-range */   \
        EMITW(0xEEB80BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))/* 32-bit int */   \
        EMITW(0xEE800B00 | MRM(Tmm0+0,  Tmm0+0,  Tmm0+1))/* <-fp64 div */   \
        EMITW(0xEEBD0BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))                   \
        EMITW(0xEE100B10 | MRM(REG(RG), Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))

#define divwn_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
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


#define divwx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        EMITW(0xEC400B10 | MRM(Teax,    REG(RS), Tmm0+0))                   \
        EMITW(0xEEB80B60 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* full-range */   \
        EMITW(0xEEB80B40 | MRM(Tmm0+0,  0x00,    Tmm0+0))/* 32-bit int */   \
        EMITW(0xEE800B00 | MRM(Tmm0+0,  Tmm0+0,  Tmm0+1))/* <-fp64 div */   \
        EMITW(0xEEBC0BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))                   \
        EMITW(0xEE100B10 | MRM(Teax,    Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))

#define divwx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xEC400B10 | MRM(Teax,    TMxx,    Tmm0+0))                   \
        EMITW(0xEEB80B60 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* full-range */   \
        EMITW(0xEEB80B40 | MRM(Tmm0+0,  0x00,    Tmm0+0))/* 32-bit int */   \
        EMITW(0xEE800B00 | MRM(Tmm0+0,  Tmm0+0,  Tmm0+1))/* <-fp64 div */   \
        EMITW(0xEEBC0BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))                   \
        EMITW(0xEE100B10 | MRM(Teax,    Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))


#define divwn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        EMITW(0xEC400B10 | MRM(Teax,    REG(RS), Tmm0+0))                   \
        EMITW(0xEEB80BE0 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* full-range */   \
        EMITW(0xEEB80BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))/* 32-bit int */   \
        EMITW(0xEE800B00 | MRM(Tmm0+0,  Tmm0+0,  Tmm0+1))/* <-fp64 div */   \
        EMITW(0xEEBD0BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))                   \
        EMITW(0xEE100B10 | MRM(Teax,    Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))

#define divwn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xEC400B10 | MRM(Teax,    TMxx,    Tmm0+0))                   \
        EMITW(0xEEB80BE0 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* full-range */   \
        EMITW(0xEEB80BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))/* 32-bit int */   \
        EMITW(0xEE800B00 | MRM(Tmm0+0,  Tmm0+0,  Tmm0+1))/* <-fp64 div */   \
        EMITW(0xEEBD0BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))                   \
        EMITW(0xEE100B10 | MRM(Teax,    Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))


#define divwp_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(0xEC400B10 | MRM(Teax,    REG(RS), Tmm0+0))/* part-range */   \
        EMITW(0xF3BB0600 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* 24-bit int */   \
        EMITW(0xEE800A20 | MRM(Tmm0+1,  Tmm0+1,  Tmm0+1))/* <-fp32 div */   \
        EMITW(0xF3BB0700 | MRM(Tmm0+0,  0x00,    Tmm0+1))/* Xmm0<-junk */   \
        EMITW(0xEE100B10 | MRM(Teax,    Tmm0+0,  0x00)) /* fallback to VFP */

#define divwp_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xEC400B10 | MRM(Teax,    TMxx,    Tmm0+0))/* part-range */   \
        EMITW(0xF3BB0600 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* 24-bit int */   \
        EMITW(0xEE800A20 | MRM(Tmm0+1,  Tmm0+1,  Tmm0+1))/* <-fp32 div */   \
        EMITW(0xF3BB0700 | MRM(Tmm0+0,  0x00,    Tmm0+1))/* Xmm0<-junk */   \
        EMITW(0xEE100B10 | MRM(Teax,    Tmm0+0,  0x00)) /* fallback to VFP */

#else /* RT_BASE_COMPAT_DIV >= 2, hw int-div for Cortex-A7/A15 + NEONv2 */

#define divwx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0xE730F010 | MRM(0x00,    REG(RG), REG(RG)) | TIxx << 8)

#define divwx_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        EMITW(0xE730F010 | MRM(0x00,    REG(RG), REG(RG)) | REG(RS) << 8)

#define divwx_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE730F010 | MRM(0x00,    REG(RG), REG(RG)) | TMxx << 8)


#define divwn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0xE710F010 | MRM(0x00,    REG(RG), REG(RG)) | TIxx << 8)

#define divwn_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        EMITW(0xE710F010 | MRM(0x00,    REG(RG), REG(RG)) | REG(RS) << 8)

#define divwn_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE710F010 | MRM(0x00,    REG(RG), REG(RG)) | TMxx << 8)


#define prewx_xx()          /* to be placed immediately prior divwx_x* */   \
                                     /* to prepare Redx for int-divide */

#define prewn_xx()          /* to be placed immediately prior divwn_x* */   \
                                     /* to prepare Redx for int-divide */


#define divwx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(0xE730F010 | MRM(0x00,    Teax,    Teax) | REG(RS) << 8)

#define divwx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE730F010 | MRM(0x00,    Teax,    Teax) | TMxx << 8)


#define divwn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(0xE710F010 | MRM(0x00,    Teax,    Teax) | REG(RS) << 8)

#define divwn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE710F010 | MRM(0x00,    Teax,    Teax) | TMxx << 8)


#define divwp_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xr(W(RS))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divwp_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xm(W(MS), W(DS))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#endif /* RT_BASE_COMPAT_DIV >= 2, hw int-div for Cortex-A7/A15 + NEONv2 */

/* rem (G = G % S)
 * set-flags: undefined */

#define remwx_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        EMITW(0xE1A00000 | MRM(TMxx,    0x00,    REG(RG)))                  \
        divwx_ri(W(RG), W(IS))                                              \
        EMITW(0xE0600090 | MRM(TMxx,    REG(RG), REG(RG)) | TIxx << 8)

#define remwx_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        EMITW(0xE1A00000 | MRM(TMxx,    0x00,    REG(RG)))                  \
        divwx_rr(W(RG), W(RS))                                              \
        EMITW(0xE0600090 | MRM(TMxx,    REG(RG), REG(RG)) | REG(RS) << 8)

#define remwx_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        EMITW(0xE1A00000 | MRM(TIxx,    0x00,    REG(RG)))                  \
        divwx_ld(W(RG), W(MS), W(DS))                                       \
        EMITW(0xE0600090 | MRM(TIxx,    REG(RG), REG(RG)) | TMxx << 8)


#define remwn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        EMITW(0xE1A00000 | MRM(TMxx,    0x00,    REG(RG)))                  \
        divwn_ri(W(RG), W(IS))                                              \
        EMITW(0xE0600090 | MRM(TMxx,    REG(RG), REG(RG)) | TIxx << 8)

#define remwn_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        EMITW(0xE1A00000 | MRM(TMxx,    0x00,    REG(RG)))                  \
        divwn_rr(W(RG), W(RS))                                              \
        EMITW(0xE0600090 | MRM(TMxx,    REG(RG), REG(RG)) | REG(RS) << 8)

#define remwn_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        EMITW(0xE1A00000 | MRM(TIxx,    0x00,    REG(RG)))                  \
        divwn_ld(W(RG), W(MS), W(DS))                                       \
        EMITW(0xE0600090 | MRM(TIxx,    REG(RG), REG(RG)) | TMxx << 8)


#define remwx_xx()          /* to be placed immediately prior divwx_x* */   \
        movwx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remwx_xr(RS)        /* to be placed immediately after divwx_xr */   \
        EMITW(0xE0600090 | MRM(Tedx,    Tedx,    REG(RS)) | Teax << 8)      \
                                                          /* Redx<-rem */

#define remwx_xm(MS, DS)    /* to be placed immediately after divwx_xm */   \
        EMITW(0xE0600090 | MRM(Tedx,    Tedx,    TMxx) | Teax << 8)         \
                                                          /* Redx<-rem */


#define remwn_xx()          /* to be placed immediately prior divwn_x* */   \
        movwx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remwn_xr(RS)        /* to be placed immediately after divwn_xr */   \
        EMITW(0xE0600090 | MRM(Tedx,    Tedx,    REG(RS)) | Teax << 8)      \
                                                          /* Redx<-rem */

#define remwn_xm(MS, DS)    /* to be placed immediately after divwn_xm */   \
        EMITW(0xE0600090 | MRM(Tedx,    Tedx,    TMxx) | Teax << 8)         \
                                                          /* Redx<-rem */

/* arj (G = G op S, if cc G then jump lb)
 * set-flags: undefined
 * refer to individual instruction descriptions
 * to stay within special register limitations */

#define and_x   AL0
#define ann_x   AL1
#define orr_x   AL2
#define orn_x   AL3
#define xor_x   AL4
#define neg_x   AM0
#define add_x   AM1
#define sub_x   AM2
#define shl_x   AN0
#define shr_x   AN1
#define shr_n   AN2
#define ror_x   AN3

#define EZ_x    jezxx_lb
#define NZ_x    jnzxx_lb

#define arjwx_rx(RG, op, cc, lb)                                            \
        AR1(W(RG), op, w, Zrx)                                              \
        CMJ(cc, lb)

#define arjwx_mx(MG, DG, op, cc, lb)                                        \
        AR2(W(MG), W(DG), op, w, Zmx)                                       \
        CMJ(cc, lb)

#define arjwx_ri(RG, IS, op, cc, lb)                                        \
        AR2(W(RG), W(IS), op, w, Zri)                                       \
        CMJ(cc, lb)

#define arjwx_mi(MG, DG, IS, op, cc, lb)                                    \
        AR3(W(MG), W(DG), W(IS), op, w, Zmi)                                \
        CMJ(cc, lb)

#define arjwx_rr(RG, RS, op, cc, lb)                                        \
        AR2(W(RG), W(RS), op, w, Zrr)                                       \
        CMJ(cc, lb)

#define arjwx_ld(RG, MS, DS, op, cc, lb)                                    \
        AR3(W(RG), W(MS), W(DS), op, w, Zld)                                \
        CMJ(cc, lb)

#define arjwx_st(RS, MG, DG, op, cc, lb)                                    \
        AR3(W(RS), W(MG), W(DG), op, w, Zst)                                \
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

#define cmjhn_rm(RS, MT, DT, cc, lb)   /* cmj 32/16-bit with sign-extend */ \
        cmphn_rm(W(RS), W(MT), W(DT))                                       \
        CMJ(cc, lb)

#define cmjhz_rm(RS, MT, DT, cc, lb)   /* cmj 32/16-bit with zero-extend */ \
        cmphz_rm(W(RS), W(MT), W(DT))                                       \
        CMJ(cc, lb)

#define cmjbn_rm(RS, MT, DT, cc, lb)   /* cmj 32/8-bit  with sign-extend */ \
        cmpbn_rm(W(RS), W(MT), W(DT))                                       \
        CMJ(cc, lb)

#define cmjbz_rm(RS, MT, DT, cc, lb)   /* cmj 32/8-bit  with zero-extend */ \
        cmpbz_rm(W(RS), W(MT), W(DT))                                       \
        CMJ(cc, lb)

#define cmjwx_mr(MS, DS, RT, cc, lb)                                        \
        cmpwx_mr(W(MS), W(DS), W(RT))                                       \
        CMJ(cc, lb)

#define cmjhn_mr(MS, DS, RT, cc, lb)   /* cmj 16/32-bit with sign-extend */ \
        cmphn_mr(W(MS), W(DS), W(RT))                                       \
        CMJ(cc, lb)

#define cmjhz_mr(MS, DS, RT, cc, lb)   /* cmj 16/32-bit with zero-extend */ \
        cmphz_mr(W(MS), W(DS), W(RT))                                       \
        CMJ(cc, lb)

#define cmjbn_mr(MS, DS, RT, cc, lb)   /* cmj  8/32-bit with sign-extend */ \
        cmpbn_mr(W(MS), W(DS), W(RT))                                       \
        CMJ(cc, lb)

#define cmjbz_mr(MS, DS, RT, cc, lb)   /* cmj  8/32-bit with zero-extend */ \
        cmpbz_mr(W(MS), W(DS), W(RT))                                       \
        CMJ(cc, lb)

/* cmp (flags = S ? T)
 * set-flags: yes */

#define cmpwx_ri(RS, IT)                                                    \
        AUW(EMPTY,    VAL(IT), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IT))   \
        EMITW(0xE1500000 | MIM(0x00,    REG(RS), VAL(IT), T1(IT), M1(IT)))

#define cmpwx_mi(MS, DS, IT)                                                \
        AUW(SIB(MS),  VAL(IT), TIxx,    MOD(MS), VAL(DS), A1(DS), G1(IT))   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE1500000 | MIM(0x00,    TMxx,    VAL(IT), T1(IT), M1(IT)))

#define cmpwx_rr(RS, RT)                                                    \
        EMITW(0xE1500000 | MRM(0x00,    REG(RS), REG(RT)))

#define cmpwx_rm(RS, MT, DT)                                                \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MT), VAL(DT), B3(DT), P1(DT)))  \
        EMITW(0xE1500000 | MRM(0x00,    REG(RS), TMxx))

#define cmphn_rm(RS, MT, DT)    /* cmp 32-bit to 16-bit with sign-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), AH(DT), EMPTY2)   \
        EMITW(0xE1D000F0 | MDM(TMxx,    MOD(MT), VAL(DT), BH(DT), PH(DT)))  \
        EMITW(0xE1500000 | MRM(0x00,    REG(RS), TMxx))

#define cmphz_rm(RS, MT, DT)    /* cmp 32-bit to 16-bit with zero-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), AH(DT), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MT), VAL(DT), BH(DT), PH(DT)))  \
        EMITW(0xE1500000 | MRM(0x00,    REG(RS), TMxx))

#define cmpbn_rm(RS, MT, DT)    /* cmp 32-bit to  8-bit with sign-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), AH(DT), EMPTY2)   \
        EMITW(0xE1D000D0 | MDM(TMxx,    MOD(MT), VAL(DT), BH(DT), PH(DT)))  \
        EMITW(0xE1500000 | MRM(0x00,    REG(RS), TMxx))

#define cmpbz_rm(RS, MT, DT)    /* cmp 32-bit to  8-bit with zero-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MT), VAL(DT), B3(DT), P1(DT)))  \
        EMITW(0xE1500000 | MRM(0x00,    REG(RS), TMxx))

#define cmpwx_mr(MS, DS, RT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE1500000 | MRM(0x00,    TMxx,    REG(RT)))

#define cmphn_mr(MS, DS, RT)    /* cmp 16-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000F0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE1500000 | MRM(0x00,    TMxx,    REG(RT)))

#define cmphz_mr(MS, DS, RT)    /* cmp 16-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE1500000 | MRM(0x00,    TMxx,    REG(RT)))

#define cmpbn_mr(MS, DS, RT)    /* cmp  8-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000D0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE1500000 | MRM(0x00,    TMxx,    REG(RT)))

#define cmpbz_mr(MS, DS, RT)    /* cmp  8-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE1500000 | MRM(0x00,    TMxx,    REG(RT)))

/* ver (Mebp/inf_VER = SIMD-version)
 * set-flags: no
 * For interpretation of SIMD build flags check compatibility layer in rtzero.h
 * 0th byte - 128-bit version, 1st byte - 256-bit version, | plus _R8/_RX slots
 * 2nd byte - 512-bit version, 3rd byte - 1K4-bit version, | in upper halves */

#define verxx_xx() /* destroys Reax, Recx, Rebx, Redx, Resi, Redi */        \
        movwx_mi(Mebp, inf_VER, IB(0xF)) /* <- NEON to bits: 0,1,2,3 */

/************************* address-sized instructions *************************/

/* adr (D = adr S)
 * set-flags: no */

#define adrxx_ld(RD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C3(DS), EMPTY2)   \
        EMITW(0xE0800000 | MRM(REG(RD), MOD(MS), TDxx))

/************************* pointer-sized instructions *************************/

/* label (D = Reax = adr lb)
 * set-flags: no */

     /* label_ld(lb) is defined in rtarch.h file, loads label to Reax */

     /* label_st(lb, MD, DD) is defined in rtarch.h file, destroys Reax */

/* jmp (if unconditional jump S/lb, else if cc flags then jump lb)
 * set-flags: no
 * maximum byte-address-range for un/conditional jumps is signed 18/16-bit
 * based on minimum natively-encoded offset across supported targets (u/c)
 * MIPS:18-bit, POWER:26-bit, AArch32:26-bit, AArch64:28-bit, x86:32-bit /
 * MIPS:18-bit, POWER:16-bit, AArch32:26-bit, AArch64:21-bit, x86:32-bit */

#define jmpxx_xr(RS)           /* register-targeted unconditional jump */   \
        EMITW(0xE1A00000 | MRM(PCxx,    0x00,    REG(RS)))

#define jmpxx_xm(MS, DS)         /* memory-targeted unconditional jump */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5900000 | MDM(PCxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))

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

/************************* register-size instructions *************************/

/* stack (push stack = S, D = pop stack)
 * set-flags: no (sequence cmp/stack_la/jmp is not allowed on MIPS & POWER)
 * adjust stack pointer with 4-byte (32-bit) steps on legacy 32-bit targets */

#define stack_st(RS)                                                        \
        EMITW(0xE5200004 | MRM(REG(RS), SPxx,    0x00))

#define stack_ld(RD)                                                        \
        EMITW(0xE4900004 | MRM(REG(RD), SPxx,    0x00))

#define stack_sa()   /* save all, [Reax - Redi] + 7 temps, 14 regs total */ \
        EMITW(0xE9205FFF | MRM(0x00,    SPxx,    0x00))

#define stack_la()   /* load all, 7 temps + [Redi - Reax], 14 regs total */ \
        EMITW(0xE8B05FFF | MRM(0x00,    SPxx,    0x00))

/************************* 16-bit subset instructions *************************/

/* mov (D = S)
 * set-flags: no */

#define movhx_ri(RD, IS)                                                    \
        AUW(EMPTY,    VAL(IS), REG(RD), EMPTY,   EMPTY,   EMPTY2, G3(IS))   \

#define movhx_mi(MD, DD, IS)                                                \
        AUW(SIB(MD),  VAL(IS), TIxx,    MOD(MD), VAL(DD), AH(DD), G3(IS))   \
        EMITW(0xE1C000B0 | MDM(TIxx,    MOD(MD), VAL(DD), BH(DD), PH(DD)))

#define movhx_rr(RD, RS)                                                    \
        EMITW(0xE1A00000 | MRM(REG(RD), 0x00,    REG(RS)))

#define movhx_ld(RD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(REG(RD), MOD(MS), VAL(DS), BH(DS), PH(DS)))

#define movhx_st(RS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), AH(DD), EMPTY2)   \
        EMITW(0xE1C000B0 | MDM(REG(RS), MOD(MD), VAL(DD), BH(DD), PH(DD)))

/* and (G = G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define andhx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0xE0000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define andhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), AH(DG), G2(IS))   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE0000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define andhx_rr(RG, RS)                                                    \
        EMITW(0xE0000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define andhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE0000000 | MRM(REG(RG), REG(RG), TMxx))

#define andhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE0000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define andhx_mr(MG, DG, RS)                                                \
        andhx_st(W(RS), W(MG), W(DG))


#define andhxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0xE0100000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define andhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), AH(DG), G2(IS))   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE0100000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define andhxZrr(RG, RS)                                                    \
        EMITW(0xE0100000 | MRM(REG(RG), REG(RG), REG(RS)))

#define andhxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE0100000 | MRM(REG(RG), REG(RG), TMxx))

#define andhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE0100000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define andhxZmr(MG, DG, RS)                                                \
        andhxZst(W(RS), W(MG), W(DG))

/* ann (G = ~G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define annhx_ri(RG, IS)                                                    \
        nothx_rx(W(RG))                                                     \
        andhx_ri(W(RG), W(IS))

#define annhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), AH(DG), G2(IS))   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1E00000 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xE0000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define annhx_rr(RG, RS)                                                    \
        EMITW(0xE1C00000 | MRM(REG(RG), REG(RS), REG(RG)))

#define annhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE1C00000 | MRM(REG(RG), TMxx,    REG(RG)))

#define annhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1C00000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define annhx_mr(MG, DG, RS)                                                \
        annhx_st(W(RS), W(MG), W(DG))


#define annhxZri(RG, IS)                                                    \
        nothx_rx(W(RG))                                                     \
        andhxZri(W(RG), W(IS))

#define annhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), AH(DG), G2(IS))   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1E00000 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xE0100000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define annhxZrr(RG, RS)                                                    \
        EMITW(0xE1D00000 | MRM(REG(RG), REG(RS), REG(RG)))

#define annhxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE1D00000 | MRM(REG(RG), TMxx,    REG(RG)))

#define annhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1D00000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define annhxZmr(MG, DG, RS)                                                \
        annhxZst(W(RS), W(MG), W(DG))

/* orr (G = G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define orrhx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0xE1800000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define orrhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), AH(DG), G2(IS))   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1800000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define orrhx_rr(RG, RS)                                                    \
        EMITW(0xE1800000 | MRM(REG(RG), REG(RG), REG(RS)))

#define orrhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE1800000 | MRM(REG(RG), REG(RG), TMxx))

#define orrhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1800000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define orrhx_mr(MG, DG, RS)                                                \
        orrhx_st(W(RS), W(MG), W(DG))


#define orrhxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0xE1900000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define orrhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), AH(DG), G2(IS))   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1900000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define orrhxZrr(RG, RS)                                                    \
        EMITW(0xE1900000 | MRM(REG(RG), REG(RG), REG(RS)))

#define orrhxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE1900000 | MRM(REG(RG), REG(RG), TMxx))

#define orrhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1900000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define orrhxZmr(MG, DG, RS)                                                \
        orrhxZst(W(RS), W(MG), W(DG))

/* orn (G = ~G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define ornhx_ri(RG, IS)                                                    \
        nothx_rx(W(RG))                                                     \
        orrhx_ri(W(RG), W(IS))

#define ornhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), AH(DG), G2(IS))   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1E00000 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xE1800000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define ornhx_rr(RG, RS)                                                    \
        nothx_rx(W(RG))                                                     \
        orrhx_rr(W(RG), W(RS))

#define ornhx_ld(RG, MS, DS)                                                \
        nothx_rx(W(RG))                                                     \
        orrhx_ld(W(RG), W(MS), W(DS))

#define ornhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1E00000 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xE1800000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define ornhx_mr(MG, DG, RS)                                                \
        ornhx_st(W(RS), W(MG), W(DG))


#define ornhxZri(RG, IS)                                                    \
        nothx_rx(W(RG))                                                     \
        orrhxZri(W(RG), W(IS))

#define ornhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), AH(DG), G2(IS))   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1E00000 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xE1900000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define ornhxZrr(RG, RS)                                                    \
        nothx_rx(W(RG))                                                     \
        orrhxZrr(W(RG), W(RS))

#define ornhxZld(RG, MS, DS)                                                \
        nothx_rx(W(RG))                                                     \
        orrhxZld(W(RG), W(MS), W(DS))

#define ornhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1E00000 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xE1900000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define ornhxZmr(MG, DG, RS)                                                \
        ornhxZst(W(RS), W(MG), W(DG))

/* xor (G = G ^ S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define xorhx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0xE0200000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define xorhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), AH(DG), G2(IS))   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE0200000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define xorhx_rr(RG, RS)                                                    \
        EMITW(0xE0200000 | MRM(REG(RG), REG(RG), REG(RS)))

#define xorhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE0200000 | MRM(REG(RG), REG(RG), TMxx))

#define xorhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE0200000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define xorhx_mr(MG, DG, RS)                                                \
        xorhx_st(W(RS), W(MG), W(DG))


#define xorhxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0xE0300000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define xorhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), AH(DG), G2(IS))   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE0300000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define xorhxZrr(RG, RS)                                                    \
        EMITW(0xE0300000 | MRM(REG(RG), REG(RG), REG(RS)))

#define xorhxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE0300000 | MRM(REG(RG), REG(RG), TMxx))

#define xorhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE0300000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define xorhxZmr(MG, DG, RS)                                                \
        xorhxZst(W(RS), W(MG), W(DG))

/* not (G = ~G)
 * set-flags: no */

#define nothx_rx(RG)                                                        \
        EMITW(0xE1E00000 | MRM(REG(RG), 0x00,    REG(RG)))

#define nothx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1E00000 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

/* neg (G = -G)
 * set-flags: undefined (*_*), yes (*Z*) */

#define neghx_rx(RG)                                                        \
        EMITW(0xE2600000 | MRM(REG(RG), REG(RG), 0x00))

#define neghx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE2600000 | MRM(TMxx,    TMxx,    0x00))                     \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))


#define neghxZrx(RG)                                                        \
        EMITW(0xE2700000 | MRM(REG(RG), REG(RG), 0x00))

#define neghxZmx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE2700000 | MRM(TMxx,    TMxx,    0x00))                     \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

/* add (G = G + S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define addhx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0xE0800000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define addhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), AH(DG), G1(IS))   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE0800000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define addhx_rr(RG, RS)                                                    \
        EMITW(0xE0800000 | MRM(REG(RG), REG(RG), REG(RS)))

#define addhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE0800000 | MRM(REG(RG), REG(RG), TMxx))

#define addhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE0800000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define addhx_mr(MG, DG, RS)                                                \
        addhx_st(W(RS), W(MG), W(DG))


#define addhxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0xE0900000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define addhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), AH(DG), G1(IS))   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE0900000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define addhxZrr(RG, RS)                                                    \
        EMITW(0xE0900000 | MRM(REG(RG), REG(RG), REG(RS)))

#define addhxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE0900000 | MRM(REG(RG), REG(RG), TMxx))

#define addhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE0900000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define addhxZmr(MG, DG, RS)                                                \
        addhxZst(W(RS), W(MG), W(DG))

/* sub (G = G - S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define subhx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0xE0400000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define subhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), AH(DG), G1(IS))   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE0400000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define subhx_rr(RG, RS)                                                    \
        EMITW(0xE0400000 | MRM(REG(RG), REG(RG), REG(RS)))

#define subhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE0400000 | MRM(REG(RG), REG(RG), TMxx))

#define subhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE0400000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define subhx_mr(MG, DG, RS)                                                \
        subhx_st(W(RS), W(MG), W(DG))


#define subhxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0xE0500000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define subhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), AH(DG), G1(IS))   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE0500000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define subhxZrr(RG, RS)                                                    \
        EMITW(0xE0500000 | MRM(REG(RG), REG(RG), REG(RS)))

#define subhxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE0500000 | MRM(REG(RG), REG(RG), TMxx))

#define subhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE0500000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define subhxZmr(MG, DG, RS)                                                \
        subhxZst(W(RS), W(MG), W(DG))

/* shl (G = G << S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlhx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0xE1A00010 | MRM(REG(RG), 0x00,    REG(RG)) | Tecx << 8)

#define shlhx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1A00010 | MRM(TMxx,    0x00,    TMxx) | Tecx << 8)         \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define shlhx_ri(RG, IS)                                                    \
        EMITW(0xE1A00000 | MRM(REG(RG), 0x00,    REG(RG)) |                 \
                                                 (0x1F & VAL(IS)) << 7)

#define shlhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1A00000 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IS)) << 7)     \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define shlhx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0xE1A00010 | MRM(REG(RG), 0x00,    REG(RG)) | REG(RS) << 8)

#define shlhx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE1A00010 | MRM(REG(RG), 0x00,    REG(RG)) | TMxx << 8)

#define shlhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1A00010 | MRM(TMxx,    0x00,    TMxx) | REG(RS) << 8)      \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define shlhx_mr(MG, DG, RS)                                                \
        shlhx_st(W(RS), W(MG), W(DG))


#define shlhxZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0xE1B00010 | MRM(REG(RG), 0x00,    REG(RG)) | Tecx << 8)

#define shlhxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1B00010 | MRM(TMxx,    0x00,    TMxx) | Tecx << 8)         \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define shlhxZri(RG, IS)                                                    \
        EMITW(0xE1B00000 | MRM(REG(RG), 0x00,    REG(RG)) |                 \
                                                 (0x1F & VAL(IS)) << 7)

#define shlhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1B00000 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IS)) << 7)     \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define shlhxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0xE1B00010 | MRM(REG(RG), 0x00,    REG(RG)) | REG(RS) << 8)

#define shlhxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE1B00010 | MRM(REG(RG), 0x00,    REG(RG)) | TMxx << 8)

#define shlhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1B00010 | MRM(TMxx,    0x00,    TMxx) | REG(RS) << 8)      \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define shlhxZmr(MG, DG, RS)                                                \
        shlhxZst(W(RS), W(MG), W(DG))

/* shr (G = G >> S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrhx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0xE6FF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1A00030 | MRM(REG(RG), 0x00,    REG(RG)) | Tecx << 8)

#define shrhx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1A00030 | MRM(TMxx,    0x00,    TMxx) | Tecx << 8)         \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define shrhx_ri(RG, IS)                                                    \
        EMITW(0xE6FF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1A00000 | MRM(REG(RG), 0x00,    REG(RG)) |                 \
        (M(VAL(IS) == 0) & 0x00000000) | (M(VAL(IS) != 0) & 0x00000020) |   \
                                                 (0x1F & VAL(IS)) << 7)

#define shrhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1A00020 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IS)) << 7)     \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define shrhx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0xE6FF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1A00030 | MRM(REG(RG), 0x00,    REG(RG)) | REG(RS) << 8)

#define shrhx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE6FF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1A00030 | MRM(REG(RG), 0x00,    REG(RG)) | TMxx << 8)

#define shrhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1A00030 | MRM(TMxx,    0x00,    TMxx) | REG(RS) << 8)      \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define shrhx_mr(MG, DG, RS)                                                \
        shrhx_st(W(RS), W(MG), W(DG))


#define shrhxZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0xE6FF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1B00030 | MRM(REG(RG), 0x00,    REG(RG)) | Tecx << 8)

#define shrhxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1B00030 | MRM(TMxx,    0x00,    TMxx) | Tecx << 8)         \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define shrhxZri(RG, IS)                                                    \
        EMITW(0xE6FF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1B00000 | MRM(REG(RG), 0x00,    REG(RG)) |                 \
        (M(VAL(IS) == 0) & 0x00000000) | (M(VAL(IS) != 0) & 0x00000020) |   \
                                                 (0x1F & VAL(IS)) << 7)

#define shrhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1B00020 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IS)) << 7)     \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define shrhxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0xE6FF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1B00030 | MRM(REG(RG), 0x00,    REG(RG)) | REG(RS) << 8)

#define shrhxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE6FF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1B00030 | MRM(REG(RG), 0x00,    REG(RG)) | TMxx << 8)

#define shrhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1B00030 | MRM(TMxx,    0x00,    TMxx) | REG(RS) << 8)      \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define shrhxZmr(MG, DG, RS)                                                \
        shrhxZst(W(RS), W(MG), W(DG))


#define shrhn_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0xE6BF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1A00050 | MRM(REG(RG), 0x00,    REG(RG)) | Tecx << 8)

#define shrhn_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000F0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1A00050 | MRM(TMxx,    0x00,    TMxx) | Tecx << 8)         \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define shrhn_ri(RG, IS)                                                    \
        EMITW(0xE6BF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1A00000 | MRM(REG(RG), 0x00,    REG(RG)) |                 \
        (M(VAL(IS) == 0) & 0x00000000) | (M(VAL(IS) != 0) & 0x00000040) |   \
                                                 (0x1F & VAL(IS)) << 7)

#define shrhn_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000F0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1A00040 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IS)) << 7)     \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define shrhn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0xE6BF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1A00050 | MRM(REG(RG), 0x00,    REG(RG)) | REG(RS) << 8)

#define shrhn_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE6BF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1A00050 | MRM(REG(RG), 0x00,    REG(RG)) | TMxx << 8)

#define shrhn_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000F0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1A00050 | MRM(TMxx,    0x00,    TMxx) | REG(RS) << 8)      \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define shrhn_mr(MG, DG, RS)                                                \
        shrhn_st(W(RS), W(MG), W(DG))


#define shrhnZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0xE6BF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1B00050 | MRM(REG(RG), 0x00,    REG(RG)) | Tecx << 8)

#define shrhnZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000F0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1B00050 | MRM(TMxx,    0x00,    TMxx) | Tecx << 8)         \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define shrhnZri(RG, IS)                                                    \
        EMITW(0xE6BF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1B00000 | MRM(REG(RG), 0x00,    REG(RG)) |                 \
        (M(VAL(IS) == 0) & 0x00000000) | (M(VAL(IS) != 0) & 0x00000040) |   \
                                                 (0x1F & VAL(IS)) << 7)

#define shrhnZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000F0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1B00040 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IS)) << 7)     \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define shrhnZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0xE6BF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1B00050 | MRM(REG(RG), 0x00,    REG(RG)) | REG(RS) << 8)

#define shrhnZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE6BF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1B00050 | MRM(REG(RG), 0x00,    REG(RG)) | TMxx << 8)

#define shrhnZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000F0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1B00050 | MRM(TMxx,    0x00,    TMxx) | REG(RS) << 8)      \
        EMITW(0xE1C000B0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define shrhnZmr(MG, DG, RS)                                                \
        shrhnZst(W(RS), W(MG), W(DG))

/* mul (G = G * S)
 * set-flags: undefined */

#define mulhx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0xE0000090 | REG(RG) << 16 | REG(RG) << 8| TIxx)

#define mulhx_rr(RG, RS)                                                    \
        EMITW(0xE0000090 | REG(RG) << 16 | REG(RG) << 8| REG(RS))

#define mulhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE0000090 | REG(RG) << 16 | REG(RG) << 8| TMxx)


#define mulhx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0xE0000090 | Teax << 16 | Teax << 8 | REG(RS))                \
        EMITW(0xE1A00020 | MRM(Tedx,    0x00,    Teax) | 16 << 7)

#define mulhx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE0000090 | Teax << 16 | Teax << 8 | TMxx)                   \
        EMITW(0xE1A00020 | MRM(Tedx,    0x00,    Teax) | 16 << 7)


#define mulhn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0xE0000090 | Teax << 16 | Teax << 8 | REG(RS))                \
        EMITW(0xE1A00040 | MRM(Tedx,    0x00,    Teax) | 16 << 7)

#define mulhn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000F0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE0000090 | Teax << 16 | Teax << 8 | TMxx)                   \
        EMITW(0xE1A00040 | MRM(Tedx,    0x00,    Teax) | 16 << 7)

/* div (G = G / S)
 * set-flags: undefined */

#if (RT_BASE_COMPAT_DIV < 2) /* no int-div for Cortex-A8/A9 + NEONv1, fp-emul */

#define divhx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0xE6FF0070 | MRM(TIxx,    0x00,    TIxx))                     \
        EMITW(0xE6FF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xEC400B10 | MRM(REG(RG), TIxx,    Tmm0+0))                   \
        EMITW(0xF3BB0680 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* 24-bit int */   \
        EMITW(0xEE800A20 | MRM(Tmm0+1,  Tmm0+1,  Tmm0+1))/* <-fp32 div */   \
        EMITW(0xF3BB0780 | MRM(Tmm0+0,  0x00,    Tmm0+1))                   \
        EMITW(0xEE100B10 | MRM(REG(RG), Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))

#define divhx_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        EMITW(0xE6FF0070 | MRM(TIxx,    0x00,    REG(RS)))                  \
        EMITW(0xE6FF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xEC400B10 | MRM(REG(RG), TIxx,    Tmm0+0))                   \
        EMITW(0xF3BB0680 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* 24-bit int */   \
        EMITW(0xEE800A20 | MRM(Tmm0+1,  Tmm0+1,  Tmm0+1))/* <-fp32 div */   \
        EMITW(0xF3BB0780 | MRM(Tmm0+0,  0x00,    Tmm0+1))                   \
        EMITW(0xEE100B10 | MRM(REG(RG), Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))

#define divhx_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE6FF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xEC400B10 | MRM(REG(RG), TMxx,    Tmm0+0))                   \
        EMITW(0xF3BB0680 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* 24-bit int */   \
        EMITW(0xEE800A20 | MRM(Tmm0+1,  Tmm0+1,  Tmm0+1))/* <-fp32 div */   \
        EMITW(0xF3BB0780 | MRM(Tmm0+0,  0x00,    Tmm0+1))                   \
        EMITW(0xEE100B10 | MRM(REG(RG), Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))


#define divhn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0xE6BF0070 | MRM(TIxx,    0x00,    TIxx))                     \
        EMITW(0xE6BF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xEC400B10 | MRM(REG(RG), TIxx,    Tmm0+0))                   \
        EMITW(0xF3BB0600 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* 24-bit int */   \
        EMITW(0xEE800A20 | MRM(Tmm0+1,  Tmm0+1,  Tmm0+1))/* <-fp32 div */   \
        EMITW(0xF3BB0700 | MRM(Tmm0+0,  0x00,    Tmm0+1))                   \
        EMITW(0xEE100B10 | MRM(REG(RG), Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))

#define divhn_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        EMITW(0xE6BF0070 | MRM(TIxx,    0x00,    REG(RS)))                  \
        EMITW(0xE6BF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xEC400B10 | MRM(REG(RG), TIxx,    Tmm0+0))                   \
        EMITW(0xF3BB0600 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* 24-bit int */   \
        EMITW(0xEE800A20 | MRM(Tmm0+1,  Tmm0+1,  Tmm0+1))/* <-fp32 div */   \
        EMITW(0xF3BB0700 | MRM(Tmm0+0,  0x00,    Tmm0+1))                   \
        EMITW(0xEE100B10 | MRM(REG(RG), Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))

#define divhn_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000F0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE6BF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xEC400B10 | MRM(REG(RG), TMxx,    Tmm0+0))                   \
        EMITW(0xF3BB0600 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* 24-bit int */   \
        EMITW(0xEE800A20 | MRM(Tmm0+1,  Tmm0+1,  Tmm0+1))/* <-fp32 div */   \
        EMITW(0xF3BB0700 | MRM(Tmm0+0,  0x00,    Tmm0+1))                   \
        EMITW(0xEE100B10 | MRM(REG(RG), Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))


#define prehx_xx()          /* to be placed immediately prior divhx_x* */   \
                                     /* to prepare Redx for int-divide */

#define prehn_xx()          /* to be placed immediately prior divhn_x* */   \
                                     /* to prepare Redx for int-divide */


#define divhx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        EMITW(0xE6FF0070 | MRM(TIxx,    0x00,    REG(RS)))                  \
        EMITW(0xE6FF0070 | MRM(Teax,    0x00,    Teax))                     \
        EMITW(0xEC400B10 | MRM(Teax,    TIxx,    Tmm0+0))/* part-range */   \
        EMITW(0xF3BB0680 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* 24-bit int */   \
        EMITW(0xEE800A20 | MRM(Tmm0+1,  Tmm0+1,  Tmm0+1))/* <-fp32 div */   \
        EMITW(0xF3BB0780 | MRM(Tmm0+0,  0x00,    Tmm0+1))                   \
        EMITW(0xEE100B10 | MRM(Teax,    Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))

#define divhx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE6FF0070 | MRM(Teax,    0x00,    Teax))                     \
        EMITW(0xEC400B10 | MRM(Teax,    TMxx,    Tmm0+0))/* part-range */   \
        EMITW(0xF3BB0680 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* 24-bit int */   \
        EMITW(0xEE800A20 | MRM(Tmm0+1,  Tmm0+1,  Tmm0+1))/* <-fp32 div */   \
        EMITW(0xF3BB0780 | MRM(Tmm0+0,  0x00,    Tmm0+1))                   \
        EMITW(0xEE100B10 | MRM(Teax,    Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))


#define divhn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        EMITW(0xE6BF0070 | MRM(TIxx,    0x00,    REG(RS)))                  \
        EMITW(0xE6BF0070 | MRM(Teax,    0x00,    Teax))                     \
        EMITW(0xEC400B10 | MRM(Teax,    TIxx,    Tmm0+0))/* part-range */   \
        EMITW(0xF3BB0600 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* 24-bit int */   \
        EMITW(0xEE800A20 | MRM(Tmm0+1,  Tmm0+1,  Tmm0+1))/* <-fp32 div */   \
        EMITW(0xF3BB0700 | MRM(Tmm0+0,  0x00,    Tmm0+1))                   \
        EMITW(0xEE100B10 | MRM(Teax,    Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))

#define divhn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000F0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE6BF0070 | MRM(Teax,    0x00,    Teax))                     \
        EMITW(0xEC400B10 | MRM(Teax,    TMxx,    Tmm0+0))/* part-range */   \
        EMITW(0xF3BB0600 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* 24-bit int */   \
        EMITW(0xEE800A20 | MRM(Tmm0+1,  Tmm0+1,  Tmm0+1))/* <-fp32 div */   \
        EMITW(0xF3BB0700 | MRM(Tmm0+0,  0x00,    Tmm0+1))                   \
        EMITW(0xEE100B10 | MRM(Teax,    Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))

#else /* RT_BASE_COMPAT_DIV >= 2, hw int-div for Cortex-A7/A15 + NEONv2 */

#define divhx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0xE6FF0070 | MRM(TIxx,    0x00,    TIxx))                     \
        EMITW(0xE6FF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE730F010 | MRM(0x00,    REG(RG), REG(RG)) | TIxx << 8)

#define divhx_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        EMITW(0xE6FF0070 | MRM(TIxx,    0x00,    REG(RS)))                  \
        EMITW(0xE6FF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE730F010 | MRM(0x00,    REG(RG), REG(RG)) | TIxx << 8)

#define divhx_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE6FF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE730F010 | MRM(0x00,    REG(RG), REG(RG)) | TMxx << 8)


#define divhn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0xE6BF0070 | MRM(TIxx,    0x00,    TIxx))                     \
        EMITW(0xE6BF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE710F010 | MRM(0x00,    REG(RG), REG(RG)) | TIxx << 8)

#define divhn_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        EMITW(0xE6BF0070 | MRM(TIxx,    0x00,    REG(RS)))                  \
        EMITW(0xE6BF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE710F010 | MRM(0x00,    REG(RG), REG(RG)) | TIxx << 8)

#define divhn_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000F0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE6BF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE710F010 | MRM(0x00,    REG(RG), REG(RG)) | TMxx << 8)


#define prehx_xx()          /* to be placed immediately prior divhx_x* */   \
                                     /* to prepare Redx for int-divide */

#define prehn_xx()          /* to be placed immediately prior divhn_x* */   \
                                     /* to prepare Redx for int-divide */


#define divhx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(0xE6FF0070 | MRM(TIxx,    0x00,    REG(RS)))                  \
        EMITW(0xE6FF0070 | MRM(Teax,    0x00,    Teax))                     \
        EMITW(0xE730F010 | MRM(0x00,    Teax,    Teax) | TIxx << 8)

#define divhx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000B0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE6FF0070 | MRM(Teax,    0x00,    Teax))                     \
        EMITW(0xE730F010 | MRM(0x00,    Teax,    Teax) | TMxx << 8)


#define divhn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(0xE6BF0070 | MRM(TIxx,    0x00,    REG(RS)))                  \
        EMITW(0xE6BF0070 | MRM(Teax,    0x00,    Teax))                     \
        EMITW(0xE710F010 | MRM(0x00,    Teax,    Teax) | TIxx << 8)

#define divhn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000F0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE6BF0070 | MRM(Teax,    0x00,    Teax))                     \
        EMITW(0xE710F010 | MRM(0x00,    Teax,    Teax) | TMxx << 8)

#endif /* RT_BASE_COMPAT_DIV >= 2, hw int-div for Cortex-A7/A15 + NEONv2 */

/* rem (G = G % S)
 * set-flags: undefined */

#define remhx_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        EMITW(0x53003C00 | MRM(TMxx,    REG(RG), 0x00))                     \
        divhx_ri(W(RG), W(IS))                                              \
        EMITW(0xE0600090 | MRM(TMxx,    REG(RG), REG(RG)) | TIxx << 8)

#define remhx_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        EMITW(0x53003C00 | MRM(TMxx,    REG(RG), 0x00))                     \
        divhx_rr(W(RG), W(RS))                                              \
        EMITW(0xE0600090 | MRM(TMxx,    REG(RG), REG(RG)) | TIxx << 8)

#define remhx_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        EMITW(0x53003C00 | MRM(TIxx,    REG(RG), 0x00))                     \
        divhx_ld(W(RG), W(MS), W(DS))                                       \
        EMITW(0xE0600090 | MRM(TIxx,    REG(RG), REG(RG)) | TMxx << 8)


#define remhn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        EMITW(0x13003C00 | MRM(TMxx,    REG(RG), 0x00))                     \
        divhn_ri(W(RG), W(IS))                                              \
        EMITW(0xE0600090 | MRM(TMxx,    REG(RG), REG(RG)) | TIxx << 8)

#define remhn_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        EMITW(0x13003C00 | MRM(TMxx,    REG(RG), 0x00))                     \
        divhn_rr(W(RG), W(RS))                                              \
        EMITW(0xE0600090 | MRM(TMxx,    REG(RG), REG(RG)) | TIxx << 8)

#define remhn_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        EMITW(0x13003C00 | MRM(TIxx,    REG(RG), 0x00))                     \
        divhn_ld(W(RG), W(MS), W(DS))                                       \
        EMITW(0xE0600090 | MRM(TIxx,    REG(RG), REG(RG)) | TMxx << 8)


#define remhx_xx()          /* to be placed immediately prior divhx_x* */   \
        movhz_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remhx_xr(RS)        /* to be placed immediately after divhx_xr */   \
        EMITW(0xE0600090 | MRM(Tedx,    Tedx,    TIxx) | Teax << 8)         \
                                                          /* Redx<-rem */

#define remhx_xm(MS, DS)    /* to be placed immediately after divhx_xm */   \
        EMITW(0xE0600090 | MRM(Tedx,    Tedx,    TMxx) | Teax << 8)         \
                                                          /* Redx<-rem */


#define remhn_xx()          /* to be placed immediately prior divhn_x* */   \
        movhn_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remhn_xr(RS)        /* to be placed immediately after divhn_xr */   \
        EMITW(0xE0600090 | MRM(Tedx,    Tedx,    TIxx) | Teax << 8)         \
                                                          /* Redx<-rem */

#define remhn_xm(MS, DS)    /* to be placed immediately after divhn_xm */   \
        EMITW(0xE0600090 | MRM(Tedx,    Tedx,    TMxx) | Teax << 8)         \
                                                          /* Redx<-rem */

/* arj (G = G op S, if cc G then jump lb)
 * set-flags: undefined
 * refer to individual instruction descriptions
 * to stay within special register limitations */

#define add_n   AM3
#define sub_n   AM4

     /* Definitions for arj's "op" and "cc" parameters
      * are provided in 32-bit rtarch_***.h files. */

#define arjhx_rx(RG, op, cc, lb)                                            \
        AR1(W(RG), op, h, Zrx)                                              \
        CMJ(cc, lb)

#define arjhx_mx(MG, DG, op, cc, lb)                                        \
        AR2(W(MG), W(DG), op, h, Zmx)                                       \
        CMJ(cc, lb)

#define arjhx_ri(RG, IS, op, cc, lb)                                        \
        AR2(W(RG), W(IS), op, h, Zri)                                       \
        CMJ(cc, lb)

#define arjhx_mi(MG, DG, IS, op, cc, lb)                                    \
        AR3(W(MG), W(DG), W(IS), op, h, Zmi)                                \
        CMJ(cc, lb)

#define arjhx_rr(RG, RS, op, cc, lb)                                        \
        AR2(W(RG), W(RS), op, h, Zrr)                                       \
        CMJ(cc, lb)

#define arjhx_ld(RG, MS, DS, op, cc, lb)                                    \
        AR3(W(RG), W(MS), W(DS), op, h, Zld)                                \
        CMJ(cc, lb)

#define arjhx_st(RS, MG, DG, op, cc, lb)                                    \
        AR3(W(RS), W(MG), W(DG), op, h, Zst)                                \
        CMJ(cc, lb)

#define arjhx_mr(MG, DG, RS, op, cc, lb)                                    \
        arjhx_st(W(RS), W(MG), W(DG), op, cc, lb)

/* cmj (flags = S ? T, if cc flags then jump lb)
 * set-flags: undefined */

     /* Definitions for cmj's "cc" parameter
      * are provided in 32-bit rtarch_***.h files. */

#define cmjhx_rz(RS, cc, lb)                                                \
        cmjhx_ri(W(RS), IC(0), cc, lb)

#define cmjhx_mz(MS, DS, cc, lb)                                            \
        cmjhx_mi(W(MS), W(DS), IC(0), cc, lb)

#define cmjhx_ri(RS, IT, cc, lb)                                            \
        cmphx_ri(W(RS), W(IT))                                              \
        CMJ(cc, lb)

#define cmjhx_mi(MS, DS, IT, cc, lb)                                        \
        cmphx_mi(W(MS), W(DS), W(IT))                                       \
        CMJ(cc, lb)

#define cmjhx_rr(RS, RT, cc, lb)                                            \
        cmphx_rr(W(RS), W(RT))                                              \
        CMJ(cc, lb)

#define cmjhx_rm(RS, MT, DT, cc, lb)                                        \
        cmphx_rm(W(RS), W(MT), W(DT))                                       \
        CMJ(cc, lb)

#define cmjhx_mr(MS, DS, RT, cc, lb)                                        \
        cmphx_mr(W(MS), W(DS), W(RT))                                       \
        CMJ(cc, lb)

/* cmp (flags = S ? T)
 * set-flags: yes */

#define cmphx_ri(RS, IT)                                                    \
        AUW(EMPTY,    VAL(IT), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IT))   \
        EMITW(0xE6BF0070 | MRM(TIxx,    0x00,    TIxx))                     \
        EMITW(0xE6BF0070 | MRM(TMxx,    0x00,    REG(RS)))                  \
        EMITW(0xE1500000 | MRM(0x00,    TMxx,    TIxx))

#define cmphx_mi(MS, DS, IT)                                                \
        AUW(SIB(MS),  VAL(IT), TIxx,    MOD(MS), VAL(DS), AH(DS), G3(IT))   \
        EMITW(0xE1D000F0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE6BF0070 | MRM(TIxx,    0x00,    TIxx))                     \
        EMITW(0xE1500000 | MRM(0x00,    TMxx,    TIxx))

#define cmphx_rr(RS, RT)                                                    \
        EMITW(0xE6BF0070 | MRM(TIxx,    0x00,    REG(RT)))                  \
        EMITW(0xE6BF0070 | MRM(TMxx,    0x00,    REG(RS)))                  \
        EMITW(0xE1500000 | MRM(0x00,    TMxx,    TIxx))

#define cmphx_rm(RS, MT, DT)                                                \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), AH(DT), EMPTY2)   \
        EMITW(0xE1D000F0 | MDM(TMxx,    MOD(MT), VAL(DT), BH(DT), PH(DT)))  \
        EMITW(0xE6BF0070 | MRM(TIxx,    0x00,    REG(RS)))                  \
        EMITW(0xE1500000 | MRM(0x00,    TIxx,    TMxx))

#define cmphx_mr(MS, DS, RT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000F0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE6BF0070 | MRM(TIxx,    0x00,    REG(RT)))                  \
        EMITW(0xE1500000 | MRM(0x00,    TMxx,    TIxx))

/************************** 8-bit subset instructions *************************/

/* mov (D = S)
 * set-flags: no */

#define movbx_ri(RD, IS)                                                    \
        AUW(EMPTY,    VAL(IS), REG(RD), EMPTY,   EMPTY,   EMPTY2, G3(IS))   \

#define movbx_mi(MD, DD, IS)                                                \
        AUW(SIB(MD),  VAL(IS), TIxx,    MOD(MD), VAL(DD), A1(DD), G3(IS))   \
        EMITW(0xE5C00000 | MDM(TIxx,    MOD(MD), VAL(DD), B3(DD), P1(DD)))

#define movbx_rr(RD, RS)                                                    \
        EMITW(0xE1A00000 | MRM(REG(RD), 0x00,    REG(RS)))

#define movbx_ld(RD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(REG(RD), MOD(MS), VAL(DS), B3(DS), P1(DS)))

#define movbx_st(RS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), A1(DD), EMPTY2)   \
        EMITW(0xE5C00000 | MDM(REG(RS), MOD(MD), VAL(DD), B3(DD), P1(DD)))

/* and (G = G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define andbx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0xE0000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define andbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), A1(DG), G2(IS))   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define andbx_rr(RG, RS)                                                    \
        EMITW(0xE0000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define andbx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE0000000 | MRM(REG(RG), REG(RG), TMxx))

#define andbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define andbx_mr(MG, DG, RS)                                                \
        andbx_st(W(RS), W(MG), W(DG))


#define andbxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0xE0100000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define andbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), A1(DG), G2(IS))   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0100000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define andbxZrr(RG, RS)                                                    \
        EMITW(0xE0100000 | MRM(REG(RG), REG(RG), REG(RS)))

#define andbxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE0100000 | MRM(REG(RG), REG(RG), TMxx))

#define andbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0100000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define andbxZmr(MG, DG, RS)                                                \
        andbxZst(W(RS), W(MG), W(DG))

/* ann (G = ~G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define annbx_ri(RG, IS)                                                    \
        notbx_rx(W(RG))                                                     \
        andbx_ri(W(RG), W(IS))

#define annbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), A1(DG), G2(IS))   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1E00000 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xE0000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define annbx_rr(RG, RS)                                                    \
        EMITW(0xE1C00000 | MRM(REG(RG), REG(RS), REG(RG)))

#define annbx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE1C00000 | MRM(REG(RG), TMxx,    REG(RG)))

#define annbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1C00000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define annbx_mr(MG, DG, RS)                                                \
        annbx_st(W(RS), W(MG), W(DG))


#define annbxZri(RG, IS)                                                    \
        notbx_rx(W(RG))                                                     \
        andbxZri(W(RG), W(IS))

#define annbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), A1(DG), G2(IS))   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1E00000 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xE0100000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define annbxZrr(RG, RS)                                                    \
        EMITW(0xE1D00000 | MRM(REG(RG), REG(RS), REG(RG)))

#define annbxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE1D00000 | MRM(REG(RG), TMxx,    REG(RG)))

#define annbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1D00000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define annbxZmr(MG, DG, RS)                                                \
        annbxZst(W(RS), W(MG), W(DG))

/* orr (G = G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define orrbx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0xE1800000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define orrbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), A1(DG), G2(IS))   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1800000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define orrbx_rr(RG, RS)                                                    \
        EMITW(0xE1800000 | MRM(REG(RG), REG(RG), REG(RS)))

#define orrbx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE1800000 | MRM(REG(RG), REG(RG), TMxx))

#define orrbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1800000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define orrbx_mr(MG, DG, RS)                                                \
        orrbx_st(W(RS), W(MG), W(DG))


#define orrbxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0xE1900000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define orrbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), A1(DG), G2(IS))   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1900000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define orrbxZrr(RG, RS)                                                    \
        EMITW(0xE1900000 | MRM(REG(RG), REG(RG), REG(RS)))

#define orrbxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE1900000 | MRM(REG(RG), REG(RG), TMxx))

#define orrbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1900000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define orrbxZmr(MG, DG, RS)                                                \
        orrbxZst(W(RS), W(MG), W(DG))

/* orn (G = ~G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define ornbx_ri(RG, IS)                                                    \
        notbx_rx(W(RG))                                                     \
        orrbx_ri(W(RG), W(IS))

#define ornbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), A1(DG), G2(IS))   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1E00000 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xE1800000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define ornbx_rr(RG, RS)                                                    \
        notbx_rx(W(RG))                                                     \
        orrbx_rr(W(RG), W(RS))

#define ornbx_ld(RG, MS, DS)                                                \
        notbx_rx(W(RG))                                                     \
        orrbx_ld(W(RG), W(MS), W(DS))

#define ornbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1E00000 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xE1800000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define ornbx_mr(MG, DG, RS)                                                \
        ornbx_st(W(RS), W(MG), W(DG))


#define ornbxZri(RG, IS)                                                    \
        notbx_rx(W(RG))                                                     \
        orrbxZri(W(RG), W(IS))

#define ornbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), A1(DG), G2(IS))   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1E00000 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xE1900000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define ornbxZrr(RG, RS)                                                    \
        notbx_rx(W(RG))                                                     \
        orrbxZrr(W(RG), W(RS))

#define ornbxZld(RG, MS, DS)                                                \
        notbx_rx(W(RG))                                                     \
        orrbxZld(W(RG), W(MS), W(DS))

#define ornbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1E00000 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xE1900000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define ornbxZmr(MG, DG, RS)                                                \
        ornbxZst(W(RS), W(MG), W(DG))

/* xor (G = G ^ S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define xorbx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0xE0200000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define xorbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), A1(DG), G2(IS))   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0200000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define xorbx_rr(RG, RS)                                                    \
        EMITW(0xE0200000 | MRM(REG(RG), REG(RG), REG(RS)))

#define xorbx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE0200000 | MRM(REG(RG), REG(RG), TMxx))

#define xorbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0200000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define xorbx_mr(MG, DG, RS)                                                \
        xorbx_st(W(RS), W(MG), W(DG))


#define xorbxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0xE0300000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define xorbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), A1(DG), G2(IS))   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0300000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define xorbxZrr(RG, RS)                                                    \
        EMITW(0xE0300000 | MRM(REG(RG), REG(RG), REG(RS)))

#define xorbxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE0300000 | MRM(REG(RG), REG(RG), TMxx))

#define xorbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0300000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define xorbxZmr(MG, DG, RS)                                                \
        xorbxZst(W(RS), W(MG), W(DG))

/* not (G = ~G)
 * set-flags: no */

#define notbx_rx(RG)                                                        \
        EMITW(0xE1E00000 | MRM(REG(RG), 0x00,    REG(RG)))

#define notbx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1E00000 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

/* neg (G = -G)
 * set-flags: undefined (*_*), yes (*Z*) */

#define negbx_rx(RG)                                                        \
        EMITW(0xE2600000 | MRM(REG(RG), REG(RG), 0x00))

#define negbx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE2600000 | MRM(TMxx,    TMxx,    0x00))                     \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))


#define negbxZrx(RG)                                                        \
        EMITW(0xE2700000 | MRM(REG(RG), REG(RG), 0x00))

#define negbxZmx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE2700000 | MRM(TMxx,    TMxx,    0x00))                     \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

/* add (G = G + S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define addbx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0xE0800000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define addbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), A1(DG), G1(IS))   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0800000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define addbx_rr(RG, RS)                                                    \
        EMITW(0xE0800000 | MRM(REG(RG), REG(RG), REG(RS)))

#define addbx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE0800000 | MRM(REG(RG), REG(RG), TMxx))

#define addbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0800000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define addbx_mr(MG, DG, RS)                                                \
        addbx_st(W(RS), W(MG), W(DG))


#define addbxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0xE0900000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define addbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), A1(DG), G1(IS))   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0900000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define addbxZrr(RG, RS)                                                    \
        EMITW(0xE0900000 | MRM(REG(RG), REG(RG), REG(RS)))

#define addbxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE0900000 | MRM(REG(RG), REG(RG), TMxx))

#define addbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0900000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define addbxZmr(MG, DG, RS)                                                \
        addbxZst(W(RS), W(MG), W(DG))

/* sub (G = G - S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define subbx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0xE0400000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define subbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), A1(DG), G1(IS))   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0400000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define subbx_rr(RG, RS)                                                    \
        EMITW(0xE0400000 | MRM(REG(RG), REG(RG), REG(RS)))

#define subbx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE0400000 | MRM(REG(RG), REG(RG), TMxx))

#define subbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0400000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define subbx_mr(MG, DG, RS)                                                \
        subbx_st(W(RS), W(MG), W(DG))


#define subbxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0xE0500000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define subbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), A1(DG), G1(IS))   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0500000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define subbxZrr(RG, RS)                                                    \
        EMITW(0xE0500000 | MRM(REG(RG), REG(RG), REG(RS)))

#define subbxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE0500000 | MRM(REG(RG), REG(RG), TMxx))

#define subbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE0500000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define subbxZmr(MG, DG, RS)                                                \
        subbxZst(W(RS), W(MG), W(DG))

/* shl (G = G << S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlbx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0xE1A00010 | MRM(REG(RG), 0x00,    REG(RG)) | Tecx << 8)

#define shlbx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1A00010 | MRM(TMxx,    0x00,    TMxx) | Tecx << 8)         \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shlbx_ri(RG, IS)                                                    \
        EMITW(0xE1A00000 | MRM(REG(RG), 0x00,    REG(RG)) |                 \
                                                 (0x1F & VAL(IS)) << 7)

#define shlbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1A00000 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IS)) << 7)     \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shlbx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0xE1A00010 | MRM(REG(RG), 0x00,    REG(RG)) | REG(RS) << 8)

#define shlbx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE1A00010 | MRM(REG(RG), 0x00,    REG(RG)) | TMxx << 8)

#define shlbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1A00010 | MRM(TMxx,    0x00,    TMxx) | REG(RS) << 8)      \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shlbx_mr(MG, DG, RS)                                                \
        shlbx_st(W(RS), W(MG), W(DG))


#define shlbxZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0xE1B00010 | MRM(REG(RG), 0x00,    REG(RG)) | Tecx << 8)

#define shlbxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1B00010 | MRM(TMxx,    0x00,    TMxx) | Tecx << 8)         \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shlbxZri(RG, IS)                                                    \
        EMITW(0xE1B00000 | MRM(REG(RG), 0x00,    REG(RG)) |                 \
                                                 (0x1F & VAL(IS)) << 7)

#define shlbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1B00000 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IS)) << 7)     \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shlbxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0xE1B00010 | MRM(REG(RG), 0x00,    REG(RG)) | REG(RS) << 8)

#define shlbxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE1B00010 | MRM(REG(RG), 0x00,    REG(RG)) | TMxx << 8)

#define shlbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1B00010 | MRM(TMxx,    0x00,    TMxx) | REG(RS) << 8)      \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shlbxZmr(MG, DG, RS)                                                \
        shlbxZst(W(RS), W(MG), W(DG))

/* shr (G = G >> S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrbx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0xE6EF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1A00030 | MRM(REG(RG), 0x00,    REG(RG)) | Tecx << 8)

#define shrbx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1A00030 | MRM(TMxx,    0x00,    TMxx) | Tecx << 8)         \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shrbx_ri(RG, IS)                                                    \
        EMITW(0xE6EF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1A00000 | MRM(REG(RG), 0x00,    REG(RG)) |                 \
        (M(VAL(IS) == 0) & 0x00000000) | (M(VAL(IS) != 0) & 0x00000020) |   \
                                                 (0x1F & VAL(IS)) << 7)

#define shrbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1A00020 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IS)) << 7)     \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shrbx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0xE6EF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1A00030 | MRM(REG(RG), 0x00,    REG(RG)) | REG(RS) << 8)

#define shrbx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE6EF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1A00030 | MRM(REG(RG), 0x00,    REG(RG)) | TMxx << 8)

#define shrbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1A00030 | MRM(TMxx,    0x00,    TMxx) | REG(RS) << 8)      \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shrbx_mr(MG, DG, RS)                                                \
        shrbx_st(W(RS), W(MG), W(DG))


#define shrbxZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0xE6EF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1B00030 | MRM(REG(RG), 0x00,    REG(RG)) | Tecx << 8)

#define shrbxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1B00030 | MRM(TMxx,    0x00,    TMxx) | Tecx << 8)         \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shrbxZri(RG, IS)                                                    \
        EMITW(0xE6EF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1B00000 | MRM(REG(RG), 0x00,    REG(RG)) |                 \
        (M(VAL(IS) == 0) & 0x00000000) | (M(VAL(IS) != 0) & 0x00000020) |   \
                                                 (0x1F & VAL(IS)) << 7)

#define shrbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1B00020 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IS)) << 7)     \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shrbxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0xE6EF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1B00030 | MRM(REG(RG), 0x00,    REG(RG)) | REG(RS) << 8)

#define shrbxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE6EF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1B00030 | MRM(REG(RG), 0x00,    REG(RG)) | TMxx << 8)

#define shrbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))  \
        EMITW(0xE1B00030 | MRM(TMxx,    0x00,    TMxx) | REG(RS) << 8)      \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), B3(DG), P1(DG)))

#define shrbxZmr(MG, DG, RS)                                                \
        shrbxZst(W(RS), W(MG), W(DG))


#define shrbn_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0xE6AF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1A00050 | MRM(REG(RG), 0x00,    REG(RG)) | Tecx << 8)

#define shrbn_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000D0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1A00050 | MRM(TMxx,    0x00,    TMxx) | Tecx << 8)         \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define shrbn_ri(RG, IS)                                                    \
        EMITW(0xE6AF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1A00000 | MRM(REG(RG), 0x00,    REG(RG)) |                 \
        (M(VAL(IS) == 0) & 0x00000000) | (M(VAL(IS) != 0) & 0x00000040) |   \
                                                 (0x1F & VAL(IS)) << 7)

#define shrbn_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000D0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1A00040 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IS)) << 7)     \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define shrbn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0xE6AF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1A00050 | MRM(REG(RG), 0x00,    REG(RG)) | REG(RS) << 8)

#define shrbn_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE6AF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1A00050 | MRM(REG(RG), 0x00,    REG(RG)) | TMxx << 8)

#define shrbn_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000D0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1A00050 | MRM(TMxx,    0x00,    TMxx) | REG(RS) << 8)      \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define shrbn_mr(MG, DG, RS)                                                \
        shrbn_st(W(RS), W(MG), W(DG))


#define shrbnZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0xE6AF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1B00050 | MRM(REG(RG), 0x00,    REG(RG)) | Tecx << 8)

#define shrbnZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000D0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1B00050 | MRM(TMxx,    0x00,    TMxx) | Tecx << 8)         \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define shrbnZri(RG, IS)                                                    \
        EMITW(0xE6AF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1B00000 | MRM(REG(RG), 0x00,    REG(RG)) |                 \
        (M(VAL(IS) == 0) & 0x00000000) | (M(VAL(IS) != 0) & 0x00000040) |   \
                                                 (0x1F & VAL(IS)) << 7)

#define shrbnZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000D0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1B00040 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IS)) << 7)     \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define shrbnZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0xE6AF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1B00050 | MRM(REG(RG), 0x00,    REG(RG)) | REG(RS) << 8)

#define shrbnZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE6AF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE1B00050 | MRM(REG(RG), 0x00,    REG(RG)) | TMxx << 8)

#define shrbnZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), AH(DG), EMPTY2)   \
        EMITW(0xE1D000D0 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))  \
        EMITW(0xE1B00050 | MRM(TMxx,    0x00,    TMxx) | REG(RS) << 8)      \
        EMITW(0xE5C00000 | MDM(TMxx,    MOD(MG), VAL(DG), BH(DG), PH(DG)))

#define shrbnZmr(MG, DG, RS)                                                \
        shrbnZst(W(RS), W(MG), W(DG))

/* mul (G = G * S)
 * set-flags: undefined */

#define mulbx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0xE0000090 | REG(RG) << 16 | REG(RG) << 8| TIxx)

#define mulbx_rr(RG, RS)                                                    \
        EMITW(0xE0000090 | REG(RG) << 16 | REG(RG) << 8| REG(RS))

#define mulbx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE0000090 | REG(RG) << 16 | REG(RG) << 8| TMxx)


#define mulbx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0xE0000090 | Teax << 16 | Teax << 8 | REG(RS))                \
        EMITW(0xE1A00020 | MRM(Tedx,    0x00,    Teax) | 8 << 7)

#define mulbx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE0000090 | Teax << 16 | Teax << 8 | TMxx)                   \
        EMITW(0xE1A00020 | MRM(Tedx,    0x00,    Teax) | 8 << 7)


#define mulbn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0xE0000090 | Teax << 16 | Teax << 8 | REG(RS))                \
        EMITW(0xE1A00040 | MRM(Tedx,    0x00,    Teax) | 8 << 7)

#define mulbn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000D0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE0000090 | Teax << 16 | Teax << 8 | TMxx)                   \
        EMITW(0xE1A00040 | MRM(Tedx,    0x00,    Teax) | 8 << 7)

/* div (G = G / S)
 * set-flags: undefined */

#if (RT_BASE_COMPAT_DIV < 2) /* no int-div for Cortex-A8/A9 + NEONv1, fp-emul */

#define divbx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0xE6EF0070 | MRM(TIxx,    0x00,    TIxx))                     \
        EMITW(0xE6EF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xEC400B10 | MRM(REG(RG), TIxx,    Tmm0+0))                   \
        EMITW(0xF3BB0680 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* 24-bit int */   \
        EMITW(0xEE800A20 | MRM(Tmm0+1,  Tmm0+1,  Tmm0+1))/* <-fp32 div */   \
        EMITW(0xF3BB0780 | MRM(Tmm0+0,  0x00,    Tmm0+1))                   \
        EMITW(0xEE100B10 | MRM(REG(RG), Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))

#define divbx_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        EMITW(0xE6EF0070 | MRM(TIxx,    0x00,    REG(RS)))                  \
        EMITW(0xE6EF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xEC400B10 | MRM(REG(RG), TIxx,    Tmm0+0))                   \
        EMITW(0xF3BB0680 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* 24-bit int */   \
        EMITW(0xEE800A20 | MRM(Tmm0+1,  Tmm0+1,  Tmm0+1))/* <-fp32 div */   \
        EMITW(0xF3BB0780 | MRM(Tmm0+0,  0x00,    Tmm0+1))                   \
        EMITW(0xEE100B10 | MRM(REG(RG), Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))

#define divbx_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE6EF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xEC400B10 | MRM(REG(RG), TMxx,    Tmm0+0))                   \
        EMITW(0xF3BB0680 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* 24-bit int */   \
        EMITW(0xEE800A20 | MRM(Tmm0+1,  Tmm0+1,  Tmm0+1))/* <-fp32 div */   \
        EMITW(0xF3BB0780 | MRM(Tmm0+0,  0x00,    Tmm0+1))                   \
        EMITW(0xEE100B10 | MRM(REG(RG), Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))


#define divbn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0xE6AF0070 | MRM(TIxx,    0x00,    TIxx))                     \
        EMITW(0xE6AF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xEC400B10 | MRM(REG(RG), TIxx,    Tmm0+0))                   \
        EMITW(0xF3BB0600 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* 24-bit int */   \
        EMITW(0xEE800A20 | MRM(Tmm0+1,  Tmm0+1,  Tmm0+1))/* <-fp32 div */   \
        EMITW(0xF3BB0700 | MRM(Tmm0+0,  0x00,    Tmm0+1))                   \
        EMITW(0xEE100B10 | MRM(REG(RG), Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))

#define divbn_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        EMITW(0xE6AF0070 | MRM(TIxx,    0x00,    REG(RS)))                  \
        EMITW(0xE6AF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xEC400B10 | MRM(REG(RG), TIxx,    Tmm0+0))                   \
        EMITW(0xF3BB0600 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* 24-bit int */   \
        EMITW(0xEE800A20 | MRM(Tmm0+1,  Tmm0+1,  Tmm0+1))/* <-fp32 div */   \
        EMITW(0xF3BB0700 | MRM(Tmm0+0,  0x00,    Tmm0+1))                   \
        EMITW(0xEE100B10 | MRM(REG(RG), Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))

#define divbn_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000D0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE6AF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xEC400B10 | MRM(REG(RG), TMxx,    Tmm0+0))                   \
        EMITW(0xF3BB0600 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* 24-bit int */   \
        EMITW(0xEE800A20 | MRM(Tmm0+1,  Tmm0+1,  Tmm0+1))/* <-fp32 div */   \
        EMITW(0xF3BB0700 | MRM(Tmm0+0,  0x00,    Tmm0+1))                   \
        EMITW(0xEE100B10 | MRM(REG(RG), Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))


#define prebx_xx()          /* to be placed immediately prior divbx_x* */   \
                                     /* to prepare Redx for int-divide */

#define prebn_xx()          /* to be placed immediately prior divbn_x* */   \
                                     /* to prepare Redx for int-divide */


#define divbx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        EMITW(0xE6EF0070 | MRM(TIxx,    0x00,    REG(RS)))                  \
        EMITW(0xE6EF0070 | MRM(Teax,    0x00,    Teax))                     \
        EMITW(0xEC400B10 | MRM(Teax,    TIxx,    Tmm0+0))/* part-range */   \
        EMITW(0xF3BB0680 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* 24-bit int */   \
        EMITW(0xEE800A20 | MRM(Tmm0+1,  Tmm0+1,  Tmm0+1))/* <-fp32 div */   \
        EMITW(0xF3BB0780 | MRM(Tmm0+0,  0x00,    Tmm0+1))                   \
        EMITW(0xEE100B10 | MRM(Teax,    Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))

#define divbx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE6EF0070 | MRM(Teax,    0x00,    Teax))                     \
        EMITW(0xEC400B10 | MRM(Teax,    TMxx,    Tmm0+0))/* part-range */   \
        EMITW(0xF3BB0680 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* 24-bit int */   \
        EMITW(0xEE800A20 | MRM(Tmm0+1,  Tmm0+1,  Tmm0+1))/* <-fp32 div */   \
        EMITW(0xF3BB0780 | MRM(Tmm0+0,  0x00,    Tmm0+1))                   \
        EMITW(0xEE100B10 | MRM(Teax,    Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))


#define divbn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        EMITW(0xE6AF0070 | MRM(TIxx,    0x00,    REG(RS)))                  \
        EMITW(0xE6AF0070 | MRM(Teax,    0x00,    Teax))                     \
        EMITW(0xEC400B10 | MRM(Teax,    TIxx,    Tmm0+0))/* part-range */   \
        EMITW(0xF3BB0600 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* 24-bit int */   \
        EMITW(0xEE800A20 | MRM(Tmm0+1,  Tmm0+1,  Tmm0+1))/* <-fp32 div */   \
        EMITW(0xF3BB0700 | MRM(Tmm0+0,  0x00,    Tmm0+1))                   \
        EMITW(0xEE100B10 | MRM(Teax,    Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))

#define divbn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        movpx_st(Xmm0, Mebp, inf_SCR01(0))          /* fallback to VFP */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000D0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE6AF0070 | MRM(Teax,    0x00,    Teax))                     \
        EMITW(0xEC400B10 | MRM(Teax,    TMxx,    Tmm0+0))/* part-range */   \
        EMITW(0xF3BB0600 | MRM(Tmm0+1,  0x00,    Tmm0+0))/* 24-bit int */   \
        EMITW(0xEE800A20 | MRM(Tmm0+1,  Tmm0+1,  Tmm0+1))/* <-fp32 div */   \
        EMITW(0xF3BB0700 | MRM(Tmm0+0,  0x00,    Tmm0+1))                   \
        EMITW(0xEE100B10 | MRM(Teax,    Tmm0+0,  0x00))                     \
        movpx_ld(Xmm0, Mebp, inf_SCR01(0))

#else /* RT_BASE_COMPAT_DIV >= 2, hw int-div for Cortex-A7/A15 + NEONv2 */

#define divbx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0xE6EF0070 | MRM(TIxx,    0x00,    TIxx))                     \
        EMITW(0xE6EF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE730F010 | MRM(0x00,    REG(RG), REG(RG)) | TIxx << 8)

#define divbx_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        EMITW(0xE6EF0070 | MRM(TIxx,    0x00,    REG(RS)))                  \
        EMITW(0xE6EF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE730F010 | MRM(0x00,    REG(RG), REG(RG)) | TIxx << 8)

#define divbx_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE6EF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE730F010 | MRM(0x00,    REG(RG), REG(RG)) | TMxx << 8)


#define divbn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0xE6AF0070 | MRM(TIxx,    0x00,    TIxx))                     \
        EMITW(0xE6AF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE710F010 | MRM(0x00,    REG(RG), REG(RG)) | TIxx << 8)

#define divbn_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        EMITW(0xE6AF0070 | MRM(TIxx,    0x00,    REG(RS)))                  \
        EMITW(0xE6AF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE710F010 | MRM(0x00,    REG(RG), REG(RG)) | TIxx << 8)

#define divbn_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000D0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE6AF0070 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0xE710F010 | MRM(0x00,    REG(RG), REG(RG)) | TMxx << 8)


#define prebx_xx()          /* to be placed immediately prior divbx_x* */   \
                                     /* to prepare Redx for int-divide */

#define prebn_xx()          /* to be placed immediately prior divbn_x* */   \
                                     /* to prepare Redx for int-divide */


#define divbx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(0xE6EF0070 | MRM(TIxx,    0x00,    REG(RS)))                  \
        EMITW(0xE6EF0070 | MRM(Teax,    0x00,    Teax))                     \
        EMITW(0xE730F010 | MRM(0x00,    Teax,    Teax) | TIxx << 8)

#define divbx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xE5D00000 | MDM(TMxx,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0xE6EF0070 | MRM(Teax,    0x00,    Teax))                     \
        EMITW(0xE730F010 | MRM(0x00,    Teax,    Teax) | TMxx << 8)


#define divbn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(0xE6AF0070 | MRM(TIxx,    0x00,    REG(RS)))                  \
        EMITW(0xE6AF0070 | MRM(Teax,    0x00,    Teax))                     \
        EMITW(0xE710F010 | MRM(0x00,    Teax,    Teax) | TIxx << 8)

#define divbn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000D0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE6AF0070 | MRM(Teax,    0x00,    Teax))                     \
        EMITW(0xE710F010 | MRM(0x00,    Teax,    Teax) | TMxx << 8)

#endif /* RT_BASE_COMPAT_DIV >= 2, hw int-div for Cortex-A7/A15 + NEONv2 */

/* rem (G = G % S)
 * set-flags: undefined */

#define rembx_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        EMITW(0x53003C00 | MRM(TMxx,    REG(RG), 0x00))                     \
        divbx_ri(W(RG), W(IS))                                              \
        EMITW(0xE0600090 | MRM(TMxx,    REG(RG), REG(RG)) | TIxx << 8)

#define rembx_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        EMITW(0x53003C00 | MRM(TMxx,    REG(RG), 0x00))                     \
        divbx_rr(W(RG), W(RS))                                              \
        EMITW(0xE0600090 | MRM(TMxx,    REG(RG), REG(RG)) | TIxx << 8)

#define rembx_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        EMITW(0x53003C00 | MRM(TIxx,    REG(RG), 0x00))                     \
        divbx_ld(W(RG), W(MS), W(DS))                                       \
        EMITW(0xE0600090 | MRM(TIxx,    REG(RG), REG(RG)) | TMxx << 8)


#define rembn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        EMITW(0x13003C00 | MRM(TMxx,    REG(RG), 0x00))                     \
        divbn_ri(W(RG), W(IS))                                              \
        EMITW(0xE0600090 | MRM(TMxx,    REG(RG), REG(RG)) | TIxx << 8)

#define rembn_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        EMITW(0x13003C00 | MRM(TMxx,    REG(RG), 0x00))                     \
        divbn_rr(W(RG), W(RS))                                              \
        EMITW(0xE0600090 | MRM(TMxx,    REG(RG), REG(RG)) | TIxx << 8)

#define rembn_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        EMITW(0x13003C00 | MRM(TIxx,    REG(RG), 0x00))                     \
        divbn_ld(W(RG), W(MS), W(DS))                                       \
        EMITW(0xE0600090 | MRM(TIxx,    REG(RG), REG(RG)) | TMxx << 8)


#define rembx_xx()          /* to be placed immediately prior divbx_x* */   \
        movbz_rr(Redx, Reax)         /* to prepare for rem calculation */

#define rembx_xr(RS)        /* to be placed immediately after divbx_xr */   \
        EMITW(0xE0600090 | MRM(Tedx,    Tedx,    TIxx) | Teax << 8)         \
                                                          /* Redx<-rem */

#define rembx_xm(MS, DS)    /* to be placed immediately after divbx_xm */   \
        EMITW(0xE0600090 | MRM(Tedx,    Tedx,    TMxx) | Teax << 8)         \
                                                          /* Redx<-rem */


#define rembn_xx()          /* to be placed immediately prior divbn_x* */   \
        movbn_rr(Redx, Reax)         /* to prepare for rem calculation */

#define rembn_xr(RS)        /* to be placed immediately after divbn_xr */   \
        EMITW(0xE0600090 | MRM(Tedx,    Tedx,    TIxx) | Teax << 8)         \
                                                          /* Redx<-rem */

#define rembn_xm(MS, DS)    /* to be placed immediately after divbn_xm */   \
        EMITW(0xE0600090 | MRM(Tedx,    Tedx,    TMxx) | Teax << 8)         \
                                                          /* Redx<-rem */

/* arj (G = G op S, if cc G then jump lb)
 * set-flags: undefined
 * refer to individual instruction descriptions
 * to stay within special register limitations */

#define add_n   AM3
#define sub_n   AM4

     /* Definitions for arj's "op" and "cc" parameters
      * are provided in 32-bit rtarch_***.h files. */

#define arjbx_rx(RG, op, cc, lb)                                            \
        AR1(W(RG), op, h, Zrx)                                              \
        CMJ(cc, lb)

#define arjbx_mx(MG, DG, op, cc, lb)                                        \
        AR2(W(MG), W(DG), op, h, Zmx)                                       \
        CMJ(cc, lb)

#define arjbx_ri(RG, IS, op, cc, lb)                                        \
        AR2(W(RG), W(IS), op, h, Zri)                                       \
        CMJ(cc, lb)

#define arjbx_mi(MG, DG, IS, op, cc, lb)                                    \
        AR3(W(MG), W(DG), W(IS), op, h, Zmi)                                \
        CMJ(cc, lb)

#define arjbx_rr(RG, RS, op, cc, lb)                                        \
        AR2(W(RG), W(RS), op, h, Zrr)                                       \
        CMJ(cc, lb)

#define arjbx_ld(RG, MS, DS, op, cc, lb)                                    \
        AR3(W(RG), W(MS), W(DS), op, h, Zld)                                \
        CMJ(cc, lb)

#define arjbx_st(RS, MG, DG, op, cc, lb)                                    \
        AR3(W(RS), W(MG), W(DG), op, h, Zst)                                \
        CMJ(cc, lb)

#define arjbx_mr(MG, DG, RS, op, cc, lb)                                    \
        arjbx_st(W(RS), W(MG), W(DG), op, cc, lb)

/* cmj (flags = S ? T, if cc flags then jump lb)
 * set-flags: undefined */

     /* Definitions for cmj's "cc" parameter
      * are provided in 32-bit rtarch_***.h files. */

#define cmjbx_rz(RS, cc, lb)                                                \
        cmjbx_ri(W(RS), IC(0), cc, lb)

#define cmjbx_mz(MS, DS, cc, lb)                                            \
        cmjbx_mi(W(MS), W(DS), IC(0), cc, lb)

#define cmjbx_ri(RS, IT, cc, lb)                                            \
        cmpbx_ri(W(RS), W(IT))                                              \
        CMJ(cc, lb)

#define cmjbx_mi(MS, DS, IT, cc, lb)                                        \
        cmpbx_mi(W(MS), W(DS), W(IT))                                       \
        CMJ(cc, lb)

#define cmjbx_rr(RS, RT, cc, lb)                                            \
        cmpbx_rr(W(RS), W(RT))                                              \
        CMJ(cc, lb)

#define cmjbx_rm(RS, MT, DT, cc, lb)                                        \
        cmpbx_rm(W(RS), W(MT), W(DT))                                       \
        CMJ(cc, lb)

#define cmjbx_mr(MS, DS, RT, cc, lb)                                        \
        cmpbx_mr(W(MS), W(DS), W(RT))                                       \
        CMJ(cc, lb)

/* cmp (flags = S ? T)
 * set-flags: yes */

#define cmpbx_ri(RS, IT)                                                    \
        AUW(EMPTY,    VAL(IT), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IT))   \
        EMITW(0xE6AF0070 | MRM(TIxx,    0x00,    TIxx))                     \
        EMITW(0xE6AF0070 | MRM(TMxx,    0x00,    REG(RS)))                  \
        EMITW(0xE1500000 | MRM(0x00,    TMxx,    TIxx))

#define cmpbx_mi(MS, DS, IT)                                                \
        AUW(SIB(MS),  VAL(IT), TIxx,    MOD(MS), VAL(DS), AH(DS), G3(IT))   \
        EMITW(0xE1D000D0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE6AF0070 | MRM(TIxx,    0x00,    TIxx))                     \
        EMITW(0xE1500000 | MRM(0x00,    TMxx,    TIxx))

#define cmpbx_rr(RS, RT)                                                    \
        EMITW(0xE6AF0070 | MRM(TIxx,    0x00,    REG(RT)))                  \
        EMITW(0xE6AF0070 | MRM(TMxx,    0x00,    REG(RS)))                  \
        EMITW(0xE1500000 | MRM(0x00,    TMxx,    TIxx))

#define cmpbx_rm(RS, MT, DT)                                                \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), AH(DT), EMPTY2)   \
        EMITW(0xE1D000D0 | MDM(TMxx,    MOD(MT), VAL(DT), BH(DT), PH(DT)))  \
        EMITW(0xE6AF0070 | MRM(TIxx,    0x00,    REG(RS)))                  \
        EMITW(0xE1500000 | MRM(0x00,    TIxx,    TMxx))

#define cmpbx_mr(MS, DS, RT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), AH(DS), EMPTY2)   \
        EMITW(0xE1D000D0 | MDM(TMxx,    MOD(MS), VAL(DS), BH(DS), PH(DS)))  \
        EMITW(0xE6AF0070 | MRM(TIxx,    0x00,    REG(RT)))                  \
        EMITW(0xE1500000 | MRM(0x00,    TMxx,    TIxx))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/* internal definitions for combined-arithmetic-jump (arj) */

#define AL0(sz, sg) and##sz##x##sg
#define AL1(sz, sg) ann##sz##x##sg
#define AL2(sz, sg) orr##sz##x##sg
#define AL3(sz, sg) orn##sz##x##sg
#define AL4(sz, sg) xor##sz##x##sg
#define AM0(sz, sg) neg##sz##x##sg
#define AM1(sz, sg) add##sz##x##sg
#define AM2(sz, sg) sub##sz##x##sg
#define AM3(sz, sg) add##sz##n##sg
#define AM4(sz, sg) sub##sz##n##sg
#define AN0(sz, sg) shl##sz##x##sg
#define AN1(sz, sg) shr##sz##x##sg
#define AN2(sz, sg) shr##sz##n##sg
#define AN3(sz, sg) ror##sz##x##sg

#define AR1(P1, op, sz, sg)                                                 \
        op(sz,sg)(W(P1))

#define AR2(P1, P2, op, sz, sg)                                             \
        op(sz,sg)(W(P1), W(P2))

#define AR3(P1, P2, P3, op, sz, sg)                                         \
        op(sz,sg)(W(P1), W(P2), W(P3))

#define CMJ(cc, lb)                                                         \
        cc(lb)

#endif /* RT_RTARCH_ARM_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/* Copyright (c) 2013-2023 VectorChief (at github, bitbucket, sourceforge)    */
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
 * rtarch_p32.h: Implementation of POWER 32-bit BASE instructions.
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
 * stack_st - applies [mov] to stack from full register (push)
 * stack_ld - applies [mov] to full register from stack (pop)
 * stack_sa - applies [mov] to stack from all full registers
 * stack_la - applies [mov] to all full registers from stack
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
 * 64/32-bit subsets are both self-consistent within themselves, 32-bit results
 * cannot be used in 64-bit subset without proper sign/zero-extend bridges,
 * cmdwn/wz bridges for 32-bit subset are provided in 64-bit headers.
 * 16/8-bit subsets are both self-consistent within themselves, their results
 * cannot be used in larger subsets without proper sign/zero-extend bridges,
 * cmdhn/hz and cmdbn/bz bridges for 16/8-bit are provided in 32-bit headers.
 * The results of 8-bit subset cannot be used within 16-bit subset consistently.
 * There is no sign/zero-extend bridge from 8-bit to 16-bit, use 32-bit instead.
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
 * Setting-flags instruction naming scheme was changed twice in the past for
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
 *
 * Only the first 4 registers are available for byte BASE logic/arithmetic and
 * shifts on legacy 32-bit targets with 8 BASE registers (ARMv7, x86).
 */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/* structural */

#define MRM(reg, ren, rem) /* arithmetic */                                 \
        ((reg) << 21 | (ren) << 11 | (rem) << 16)

#define MSM(reg, ren, rem) /* logic, shifts */                              \
        ((reg) << 16 | (ren) << 21 | (rem) << 11)

#define MTM(reg, ren, rem) /* divide, stack */                              \
        ((reg) << 21 | (ren) << 16 | (rem) << 11)

#define MDM(reg, brm, vdp, bxx, pxx)                                        \
        (pxx(vdp) | bxx(brm) << 16 | (reg) << 21)

#define MIM(reg, ren, vim, txx, mxx)                                        \
        (mxx(vim) | txx(reg, ren))

#define AUW(sib, vim, reg, brm, vdp, cdp, cim)                              \
            sib  cdp(brm, vdp)  cim(reg, vim)

#if   (defined RT_P32)

#ifdef RT_BASE_COMPAT_REM
#undef  RT_P32
#define RT_P32 RT_BASE_COMPAT_REM /* 0-8 - generic, 9 - POWER9 (ISA 3.0) */
#endif /* RT_BASE_COMPAT_REM */

#elif (defined RT_P64)

#ifdef RT_BASE_COMPAT_REM
#undef  RT_P64
#define RT_P64 RT_BASE_COMPAT_REM /* 0-8 - generic, 9 - POWER9 (ISA 3.0) */
#endif /* RT_BASE_COMPAT_REM */

#endif /* defined (RT_P32, RT_P64) */

#define EMPTY1(em1) em1
#define EMPTY2(em1, em2) em1 em2

/* selectors  */

#define REG(reg, mod, sib)  reg
#define RXG(reg, mod, sib)  ((reg) + 32)
#define RYG(reg, mod, sib)  ((reg) + 16)
#define MOD(reg, mod, sib)  mod
#define SIB(reg, mod, sib)  sib

#define VAL(val, tp1, tp2)  val
#define VXL(val, tp1, tp2)  ((val) | 0x20)
#define VYL(val, tp1, tp2)  ((val) | 0x10)
#define VZL(val, tp1, tp2)  ((val) | 0x30)
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
#define  B3(val, tp1, tp2)  B3##tp1
#define  P1(val, tp1, tp2)  P1##tp1
#define  PW(val, tp1, tp2)  PW##tp1
#define  PH(val, tp1, tp2)  PH##tp1
#define  PS(val, tp1, tp2)  PS##tp1
#define  PB(val, tp1, tp2)  PB##tp1
#define  F1(val, tp1, tp2)  F1##tp1
#define  O1(val, tp1, tp2)  O1##tp1
#define  OH(val, tp1, tp2)  OH##tp1
#define  OB(val, tp1, tp2)  OB##tp1
#define  Q1(val, tp1, tp2)  Q1##tp1
#define  C1(val, tp1, tp2)  C1##tp1
#define  A1(val, tp1, tp2)  A1##tp1
#define  C3(val, tp1, tp2)  C3##tp2 /* <- "C3##tp2" not a bug */

/* immediate encoding add/sub/cmp(TP1), and/orr/xor(TP2), mov/mul(TP3) */

#define T10(tr, sr) ((tr) << 21 | (sr) << 16)
#define M10(im) (0x00000000 |(0x7FFF & (im)))
#define G10(rg, im) EMPTY
#define T20(tr, sr) ((tr) << 16 | (sr) << 21)
#define M20(im) (0x00000000 |(0xFFFF & (im)))
#define G20(rg, im) EMPTY
#define T30(tr, sr) ((tr) << 16 | (sr) << 21)
#define M30(im) (0x00000000 |(0x7FFF & (im)))
#define G30(rg, im) EMITW(0x60000000 | ((rg)&0x1F) << 16 |                  \
                         ((((rg)&0x1F) << 21) & M(((rg)&0x20) != 0)) |      \
                                                    (0xFFFF & (im)))

#define T11(tr, sr) ((tr) << 21 | (sr) << 11)
#define M11(im) (0x00000000 | TIxx << 16)
#define G11(rg, im) G31(rg, im)
#define T21(tr, sr) ((tr) << 16 | (sr) << 21)
#define M21(im) (0x00000000 | TIxx << 11)
#define G21(rg, im) G31(rg, im)
#define T31(tr, sr) ((tr) << 16 | (sr) << 21)
#define M31(im) (0x00000000 | TIxx << 11)
#define G31(rg, im) EMITW(0x60000000 | ((rg)&0x1F) << 16 |                  \
                         ((((rg)&0x1F) << 21) & M(((rg)&0x20) != 0)) |      \
                                                    (0xFFFF & (im)))

#define T12(tr, sr) ((tr) << 21 | (sr) << 11)
#define M12(im) (0x00000000 | TIxx << 16)
#define G12(rg, im) G32(rg, im)
#define T22(tr, sr) ((tr) << 16 | (sr) << 21)
#define M22(im) (0x00000000 | TIxx << 11)
#define G22(rg, im) G32(rg, im)
#define T32(tr, sr) ((tr) << 16 | (sr) << 21)
#define M32(im) (0x00000000 | TIxx << 11)
#define G32(rg, im) EMITW(0x64000000 | ((rg)&0x1F) << 16 |                  \
                         ((((rg)&0x1F) << 21) & M(((rg)&0x20) != 0)) |      \
                                                    (0xFFFF & (im) >> 16))  \
                    EMITW(0x60000000 | ((rg)&0x1F) << 16 |                  \
                           ((rg)&0x1F) << 21  |     (0xFFFF & (im)))

/* displacement encoding BASE(TP1), adr(TP3) */

#define B10(br) (br)
#define B30(br) (br)
#define P10(dp) (0x80000000 |(0x7FFC & (dp)))
#define PW0(dp) (0xE8000002 |(0x7FFC & (dp)))
#define PH0(dp) (0xA0000000 |(0x7FFE & (dp)))
#define PS0(dp) (0xA8000000 |(0x7FFE & (dp)))
#define PB0(dp) (0x88000000 |(0x7FFF & (dp)))
#define F10(dp) (0xE8000000 |(0x7FFC & (dp)))
#define O10(dp) (0x90000000 |(0x7FFC & (dp)))
#define OH0(dp) (0xB0000000 |(0x7FFE & (dp)))
#define OB0(dp) (0x98000000 |(0x7FFF & (dp)))
#define Q10(dp) (0xF8000000 |(0x7FFC & (dp)))
#define C10(br, dp) EMPTY
#define A10(br, dp) EMPTY
#define C30(br, dp) EMITW(0x60000000 | TDxx << 16 | (0xFFFF & (dp)))

#define B11(br) (br)
#define B31(br) TPxx
#define P11(dp) (0x7C00002E | TDxx << 11)
#define PW1(dp) (0x7C0002AA | TDxx << 11)
#define PH1(dp) (0x7C00022E | TDxx << 11)
#define PS1(dp) (0x7C0002AE | TDxx << 11)
#define PB1(dp) (0x7C0000AE | TDxx << 11)
#define F11(dp) (0x7C00002A | TDxx << 11)
#define O11(dp) (0x7C00012E | TDxx << 11)
#define OH1(dp) (0x7C00032E | TDxx << 11)
#define OB1(dp) (0x7C0001AE | TDxx << 11)
#define Q11(dp) (0x7C00012A | TDxx << 11)
#define C11(br, dp) C31(br, dp)
#define A11(br, dp) C31(br, dp)                                             \
                    EMITW(0x7C000214 | MRM(TPxx,    (br),    TDxx))
#define C31(br, dp) EMITW(0x60000000 | TDxx << 16 | (0xFFFF & (dp)))

#define B12(br) (br)
#define B32(br) TPxx
#define P12(dp) (0x7C00002E | TDxx << 11)
#define PW2(dp) (0x7C0002AA | TDxx << 11)
#define PH2(dp) (0x7C00022E | TDxx << 11)
#define PS2(dp) (0x7C0002AE | TDxx << 11)
#define PB2(dp) (0x7C0000AE | TDxx << 11)
#define F12(dp) (0x7C00002A | TDxx << 11)
#define O12(dp) (0x7C00012E | TDxx << 11)
#define OH2(dp) (0x7C00032E | TDxx << 11)
#define OB2(dp) (0x7C0001AE | TDxx << 11)
#define Q12(dp) (0x7C00012A | TDxx << 11)
#define C12(br, dp) C32(br, dp)
#define A12(br, dp) C32(br, dp)                                             \
                    EMITW(0x7C000214 | MRM(TPxx,    (br),    TDxx))
#define C32(br, dp) EMITW(0x64000000 | TDxx << 16 | (0x7FFF & (dp) >> 16))  \
                    EMITW(0x60000000 | TDxx << 16 | TDxx << 21 |            \
                                                    (0xFFFF & (dp)))

/* splatters for SIMD shifts and scalars */

#define SPLT    ((1-RT_ENDIAN)*3)
#define SP16    ((1-RT_ENDIAN)*7)
#define SP08    ((1-RT_ENDIAN)*15)
#define SPL(DT) (SPLT+(2*RT_ENDIAN-1)*(3 & VAL(DT) >> 2))

/* registers    REG   (check mapping with ASM_ENTER/ASM_LEAVE in rtarch.h) */
/* four registers T0xx,T1xx,T2xx,T3xx below must occupy consecutive indices */

#define T0xx    0x14  /* r20, extra reg for fast SIMD-load */
#define T1xx    0x15  /* r21, extra reg for fast SIMD-load */
#define T2xx    0x16  /* r22, extra reg for fast SIMD-load */
#define T3xx    0x17  /* r23, extra reg for fast SIMD-load */

#define TLxx    0x18  /* r24, left  arg for compare */
#define TRxx    0x19  /* r25, right arg for compare */
#define TMxx    0x18  /* r24 */
#define TIxx    0x19  /* r25 */
#define TDxx    0x1A  /* r26 */
#define TPxx    0x1B  /* r27 */
#define TCxx    0x1C  /* r28 */
#define TVxx    0x1D  /* r29 */
#define TWxx    0x1E  /* r30 */
#define TZxx    0x00  /* r0, must be zero for logic ops */
#define SPxx    0x01  /* r1 */

#define Teax    0x04  /* r4, must be non-zero reg-index */
#define Tecx    0x0F  /* r15 */
#define Tedx    0x10  /* r16 */
#define Tebx    0x03  /* r3 */
#define Tebp    0x05  /* r5 */
#define Tesi    0x06  /* r6 */
#define Tedi    0x07  /* r7 */
#define Teg8    0x08  /* r8 */
#define Teg9    0x09  /* r9 */
#define TegA    0x0A  /* r10 */
#define TegB    0x11  /* r17 */
#define TegC    0x12  /* r18 */
#define TegD    0x13  /* r19 */
#define TegE    0x0E  /* r14 */

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/* registers    REG,  MOD,  SIB */

#define Reax    Teax, %%r4,  EMPTY
#define Recx    Tecx, %%r15, EMPTY
#define Redx    Tedx, %%r16, EMPTY
#define Rebx    Tebx, %%r3,  EMPTY
#define Rebp    Tebp, %%r5,  EMPTY
#define Resi    Tesi, %%r6,  EMPTY
#define Redi    Tedi, %%r7,  EMPTY
#define Reg8    Teg8, %%r8,  EMPTY
#define Reg9    Teg9, %%r9,  EMPTY
#define RegA    TegA, %%r10, EMPTY
#define RegB    TegB, %%r17, EMPTY
#define RegC    TegC, %%r18, EMPTY
#define RegD    TegD, %%r19, EMPTY
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

#if   (defined RT_P32)

#define Jecx    Tecx, TPxx, EMITW(0x5400083C | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tecx,    TPxx))
#define Jedx    Tedx, TPxx, EMITW(0x5400083C | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tedx,    TPxx))
#define Jebx    Tebx, TPxx, EMITW(0x5400083C | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tebx,    TPxx))
#define Jebp    Tebp, TPxx, EMITW(0x5400083C | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tebp,    TPxx))
#define Jesi    Tesi, TPxx, EMITW(0x5400083C | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tesi,    TPxx))
#define Jedi    Tedi, TPxx, EMITW(0x5400083C | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tedi,    TPxx))
#define Jeg8    Teg8, TPxx, EMITW(0x5400083C | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Teg8,    TPxx))
#define Jeg9    Teg9, TPxx, EMITW(0x5400083C | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Teg9,    TPxx))
#define JegA    TegA, TPxx, EMITW(0x5400083C | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegA,    TPxx))
#define JegB    TegB, TPxx, EMITW(0x5400083C | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegB,    TPxx))
#define JegC    TegC, TPxx, EMITW(0x5400083C | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegC,    TPxx))
#define JegD    TegD, TPxx, EMITW(0x5400083C | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegD,    TPxx))
#define JegE    TegE, TPxx, EMITW(0x5400083C | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegE,    TPxx))

#define Kecx    Tecx, TPxx, EMITW(0x5400103A | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tecx,    TPxx))
#define Kedx    Tedx, TPxx, EMITW(0x5400103A | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tedx,    TPxx))
#define Kebx    Tebx, TPxx, EMITW(0x5400103A | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tebx,    TPxx))
#define Kebp    Tebp, TPxx, EMITW(0x5400103A | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tebp,    TPxx))
#define Kesi    Tesi, TPxx, EMITW(0x5400103A | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tesi,    TPxx))
#define Kedi    Tedi, TPxx, EMITW(0x5400103A | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tedi,    TPxx))
#define Keg8    Teg8, TPxx, EMITW(0x5400103A | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Teg8,    TPxx))
#define Keg9    Teg9, TPxx, EMITW(0x5400103A | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Teg9,    TPxx))
#define KegA    TegA, TPxx, EMITW(0x5400103A | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegA,    TPxx))
#define KegB    TegB, TPxx, EMITW(0x5400103A | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegB,    TPxx))
#define KegC    TegC, TPxx, EMITW(0x5400103A | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegC,    TPxx))
#define KegD    TegD, TPxx, EMITW(0x5400103A | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegD,    TPxx))
#define KegE    TegE, TPxx, EMITW(0x5400103A | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegE,    TPxx))

#define Lecx    Tecx, TPxx, EMITW(0x54001838 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tecx,    TPxx))
#define Ledx    Tedx, TPxx, EMITW(0x54001838 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tedx,    TPxx))
#define Lebx    Tebx, TPxx, EMITW(0x54001838 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tebx,    TPxx))
#define Lebp    Tebp, TPxx, EMITW(0x54001838 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tebp,    TPxx))
#define Lesi    Tesi, TPxx, EMITW(0x54001838 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tesi,    TPxx))
#define Ledi    Tedi, TPxx, EMITW(0x54001838 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tedi,    TPxx))
#define Leg8    Teg8, TPxx, EMITW(0x54001838 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Teg8,    TPxx))
#define Leg9    Teg9, TPxx, EMITW(0x54001838 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Teg9,    TPxx))
#define LegA    TegA, TPxx, EMITW(0x54001838 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegA,    TPxx))
#define LegB    TegB, TPxx, EMITW(0x54001838 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegB,    TPxx))
#define LegC    TegC, TPxx, EMITW(0x54001838 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegC,    TPxx))
#define LegD    TegD, TPxx, EMITW(0x54001838 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegD,    TPxx))
#define LegE    TegE, TPxx, EMITW(0x54001838 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegE,    TPxx))

#elif (defined RT_P64)

#define Jecx    Tecx, TPxx, EMITW(0x78000808 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tecx,    TPxx))
#define Jedx    Tedx, TPxx, EMITW(0x78000808 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tedx,    TPxx))
#define Jebx    Tebx, TPxx, EMITW(0x78000808 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tebx,    TPxx))
#define Jebp    Tebp, TPxx, EMITW(0x78000808 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tebp,    TPxx))
#define Jesi    Tesi, TPxx, EMITW(0x78000808 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tesi,    TPxx))
#define Jedi    Tedi, TPxx, EMITW(0x78000808 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tedi,    TPxx))
#define Jeg8    Teg8, TPxx, EMITW(0x78000808 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Teg8,    TPxx))
#define Jeg9    Teg9, TPxx, EMITW(0x78000808 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Teg9,    TPxx))
#define JegA    TegA, TPxx, EMITW(0x78000808 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegA,    TPxx))
#define JegB    TegB, TPxx, EMITW(0x78000808 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegB,    TPxx))
#define JegC    TegC, TPxx, EMITW(0x78000808 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegC,    TPxx))
#define JegD    TegD, TPxx, EMITW(0x78000808 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegD,    TPxx))
#define JegE    TegE, TPxx, EMITW(0x78000808 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegE,    TPxx))

#define Kecx    Tecx, TPxx, EMITW(0x78001008 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tecx,    TPxx))
#define Kedx    Tedx, TPxx, EMITW(0x78001008 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tedx,    TPxx))
#define Kebx    Tebx, TPxx, EMITW(0x78001008 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tebx,    TPxx))
#define Kebp    Tebp, TPxx, EMITW(0x78001008 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tebp,    TPxx))
#define Kesi    Tesi, TPxx, EMITW(0x78001008 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tesi,    TPxx))
#define Kedi    Tedi, TPxx, EMITW(0x78001008 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tedi,    TPxx))
#define Keg8    Teg8, TPxx, EMITW(0x78001008 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Teg8,    TPxx))
#define Keg9    Teg9, TPxx, EMITW(0x78001008 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Teg9,    TPxx))
#define KegA    TegA, TPxx, EMITW(0x78001008 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegA,    TPxx))
#define KegB    TegB, TPxx, EMITW(0x78001008 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegB,    TPxx))
#define KegC    TegC, TPxx, EMITW(0x78001008 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegC,    TPxx))
#define KegD    TegD, TPxx, EMITW(0x78001008 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegD,    TPxx))
#define KegE    TegE, TPxx, EMITW(0x78001008 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegE,    TPxx))

#define Lecx    Tecx, TPxx, EMITW(0x78001808 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tecx,    TPxx))
#define Ledx    Tedx, TPxx, EMITW(0x78001808 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tedx,    TPxx))
#define Lebx    Tebx, TPxx, EMITW(0x78001808 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tebx,    TPxx))
#define Lebp    Tebp, TPxx, EMITW(0x78001808 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tebp,    TPxx))
#define Lesi    Tesi, TPxx, EMITW(0x78001808 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tesi,    TPxx))
#define Ledi    Tedi, TPxx, EMITW(0x78001808 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Tedi,    TPxx))
#define Leg8    Teg8, TPxx, EMITW(0x78001808 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Teg8,    TPxx))
#define Leg9    Teg9, TPxx, EMITW(0x78001808 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    Teg9,    TPxx))
#define LegA    TegA, TPxx, EMITW(0x78001808 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegA,    TPxx))
#define LegB    TegB, TPxx, EMITW(0x78001808 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegB,    TPxx))
#define LegC    TegC, TPxx, EMITW(0x78001808 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegC,    TPxx))
#define LegD    TegD, TPxx, EMITW(0x78001808 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegD,    TPxx))
#define LegE    TegE, TPxx, EMITW(0x78001808 | MSM(TPxx,    Teax,    0x00)) \
                            EMITW(0x7C000214 | MRM(TPxx,    TegE,    TPxx))

#endif /* (defined RT_P64) */

/* immediate    VAL,  TP1,  TP2            (all immediate types are unsigned) */
/* full-size IW type is only applicable within cmdw* subset, can set sign-bit */
/* within cmdz* subset use of IW type is limited to movzx_rj/_mj instructions */

#define  IC(im) ((im) & 0x7F),          0, 0        /* drop sign-ext (on x86) */
#define  IB(im) ((im) & 0xFF),          0, 0          /* 32-bit word (on x86) */
#define  IM(im) ((im) & 0xFFF),         0, 0    /* native AArch64 add/sub/cmp */
#define  IG(im) ((im) & 0x7FFF),        0, 0 /* native MIPS/POWER add/sub/cmp */
#define  IH(im) ((im) & 0xFFFF),        1, 0    /* second native on ARMs/MIPS */
#define  IV(im) ((im) & 0x7FFFFFFF),    2, 2       /* native x86_64 long mode */
#define  IW(im) ((im) & 0xFFFFFFFF),    2, 2          /* for cmdw* subset, *j */

/* displacement VAL,  TP1,  TP2         (all displacement types are unsigned) */
/* public scalable DP/DE/DF/DG/DH/DV definitions are now provided in rtbase.h */
/* as D* are used for BASE and SIMD instructions, only limits are SIMD-scaled */

#define _DP(dp) ((dp) & 0xFFF),         0, 0      /* native on all ARMs, MIPS */
#define _DE(dp) ((dp) & 0x1FFF),        0, 0     /* AArch64 256-bit SVE ld/st */
#define _DF(dp) ((dp) & 0x3FFF),        0, 0     /* native AArch64 BASE ld/st */
#define _DG(dp) ((dp) & 0x7FFF),        0, 0  /* native MIPS/POWER BASE ld/st */
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
        AUW(EMPTY,    VAL(IS), REG(RD), EMPTY,   EMPTY,   EMPTY2, G3(IS))

#define movwx_mi(MD, DD, IS)                                                \
        AUW(SIB(MD),  VAL(IS), TWxx,    MOD(MD), VAL(DD), C1(DD), G3(IS))   \
        EMITW(0x00000000 | MDM(TWxx,    MOD(MD), VAL(DD), B1(DD), O1(DD)))

#define movwx_rr(RD, RS)                                                    \
        EMITW(0x7C000378 | MSM(REG(RD), REG(RS), REG(RS)))

#define movhn_rr(RD, RS)      /* move 16-bit to 32/64-bit w/ sign-extend */ \
        EMITW(0x7C000734 | MSM(REG(RD), REG(RS), 0x00))

#define movhz_rr(RD, RS)      /* move 16-bit to 32/64-bit w/ zero-extend */ \
        EMITW(0x5400043E | MSM(REG(RD), REG(RS), 0x00))

#define movbn_rr(RD, RS)      /* move  8-bit to 32/64-bit w/ sign-extend */ \
        EMITW(0x7C000774 | MSM(REG(RD), REG(RS), 0x00))

#define movbz_rr(RD, RS)      /* move  8-bit to 32/64-bit w/ zero-extend */ \
        EMITW(0x5400063E | MSM(REG(RD), REG(RS), 0x00))

#define movwx_ld(RD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(REG(RD), MOD(MS), VAL(DS), B1(DS), P1(DS)))

#define movhn_ld(RD, MS, DS)  /* load 16-bit to 32/64-bit w/ sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(REG(RD), MOD(MS), VAL(DS), B1(DS), PS(DS)))

#define movhz_ld(RD, MS, DS)  /* load 16-bit to 32/64-bit w/ zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(REG(RD), MOD(MS), VAL(DS), B1(DS), PH(DS)))

#define movbn_ld(RD, MS, DS)  /* load  8-bit to 32/64-bit w/ sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(REG(RD), MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000774 | MSM(REG(RD), REG(RD), 0x00))

#define movbz_ld(RD, MS, DS)  /* load  8-bit to 32/64-bit w/ zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(REG(RD), MOD(MS), VAL(DS), B1(DS), PB(DS)))

#define movwx_st(RS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0x00000000 | MDM(REG(RS), MOD(MD), VAL(DD), B1(DD), O1(DD)))


#define movwx_rj(RD, IT, IS)     /* IT - upper 32-bit, IS - lower 32-bit */ \
        movwx_ri(W(RD), W(IS))

#define movwx_mj(MD, DD, IT, IS) /* IT - upper 32-bit, IS - lower 32-bit */ \
        movwx_mi(W(MD), W(DD), W(IS))

/* and (G = G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define andwx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000038))

#define andwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000038))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define andwx_rr(RG, RS)                                                    \
        EMITW(0x7C000038 | MSM(REG(RG), REG(RG), REG(RS)))

#define andwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000038 | MSM(REG(RG), REG(RG), TMxx))

#define andwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000038 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define andwx_mr(MG, DG, RS)                                                \
        andwx_st(W(RS), W(MG), W(DG))


#if RT_BASE_COMPAT_ZFL == 0

#define andwxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000039))

#define andwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000039))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define andwxZrr(RG, RS)                                                    \
        EMITW(0x7C000039 | MSM(REG(RG), REG(RG), REG(RS)))

#define andwxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000039 | MSM(REG(RG), REG(RG), TMxx))

#define andwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000039 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define andwxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000038))    \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define andwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000038))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#define andwxZrr(RG, RS)                                                    \
        EMITW(0x7C000038 | MSM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define andwxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000038 | MSM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define andwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000038 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

#define andwxZmr(MG, DG, RS)                                                \
        andwxZst(W(RS), W(MG), W(DG))

/* ann (G = ~G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define annwx_ri(RG, IS)                                                    \
        notwx_rx(W(RG))                                                     \
        andwx_ri(W(RG), W(IS))

#define annwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C0000F8 | MSM(TMxx,    TMxx,    TMxx))                     \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000038))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define annwx_rr(RG, RS)                                                    \
        EMITW(0x7C000078 | MSM(REG(RG), REG(RS), REG(RG)))

#define annwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000078 | MSM(REG(RG), TMxx,    REG(RG)))

#define annwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000078 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define annwx_mr(MG, DG, RS)                                                \
        annwx_st(W(RS), W(MG), W(DG))


#define annwxZri(RG, IS)                                                    \
        notwx_rx(W(RG))                                                     \
        andwxZri(W(RG), W(IS))

#if RT_BASE_COMPAT_ZFL == 0

#define annwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C0000F8 | MSM(TMxx,    TMxx,    TMxx))                     \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000039))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define annwxZrr(RG, RS)                                                    \
        EMITW(0x7C000079 | MSM(REG(RG), REG(RS), REG(RG)))

#define annwxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000079 | MSM(REG(RG), TMxx,    REG(RG)))

#define annwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000079 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define annwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C0000F8 | MSM(TMxx,    TMxx,    TMxx))                     \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000038))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#define annwxZrr(RG, RS)                                                    \
        EMITW(0x7C000078 | MSM(REG(RG), REG(RS), REG(RG)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define annwxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000078 | MSM(REG(RG), TMxx,    REG(RG)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define annwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000078 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

#define annwxZmr(MG, DG, RS)                                                \
        annwxZst(W(RS), W(MG), W(DG))

/* orr (G = G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define orrwx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), RXG(RG), EMPTY,   EMPTY,   EMPTY2, G3(IS))

#define orrwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        AUW(EMPTY,    VAL(IS), TMxx+32, EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define orrwx_rr(RG, RS)                                                    \
        EMITW(0x7C000378 | MSM(REG(RG), REG(RG), REG(RS)))

#define orrwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000378 | MSM(REG(RG), REG(RG), TMxx))

#define orrwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000378 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define orrwx_mr(MG, DG, RS)                                                \
        orrwx_st(W(RS), W(MG), W(DG))


#define orrwxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), RXG(RG), EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define orrwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        AUW(EMPTY,    VAL(IS), TMxx+32, EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#if RT_BASE_COMPAT_ZFL == 0

#define orrwxZrr(RG, RS)                                                    \
        EMITW(0x7C000379 | MSM(REG(RG), REG(RG), REG(RS)))

#define orrwxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000379 | MSM(REG(RG), REG(RG), TMxx))

#define orrwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000379 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define orrwxZrr(RG, RS)                                                    \
        EMITW(0x7C000378 | MSM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define orrwxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000378 | MSM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define orrwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000378 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

#define orrwxZmr(MG, DG, RS)                                                \
        orrwxZst(W(RS), W(MG), W(DG))

/* orn (G = ~G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define ornwx_ri(RG, IS)                                                    \
        notwx_rx(W(RG))                                                     \
        orrwx_ri(W(RG), W(IS))

#define ornwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C0000F8 | MSM(TMxx,    TMxx,    TMxx))                     \
        AUW(EMPTY,    VAL(IS), TMxx+32, EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define ornwx_rr(RG, RS)                                                    \
        EMITW(0x7C000338 | MSM(REG(RG), REG(RS), REG(RG)))

#define ornwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000338 | MSM(REG(RG), TMxx,    REG(RG)))

#define ornwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000338 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define ornwx_mr(MG, DG, RS)                                                \
        ornwx_st(W(RS), W(MG), W(DG))


#define ornwxZri(RG, IS)                                                    \
        notwx_rx(W(RG))                                                     \
        orrwxZri(W(RG), W(IS))

#define ornwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C0000F8 | MSM(TMxx,    TMxx,    TMxx))                     \
        AUW(EMPTY,    VAL(IS), TMxx+32, EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#if RT_BASE_COMPAT_ZFL == 0

#define ornwxZrr(RG, RS)                                                    \
        EMITW(0x7C000339 | MSM(REG(RG), REG(RS), REG(RG)))

#define ornwxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000339 | MSM(REG(RG), TMxx,    REG(RG)))

#define ornwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000339 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define ornwxZrr(RG, RS)                                                    \
        EMITW(0x7C000338 | MSM(REG(RG), REG(RS), REG(RG)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define ornwxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000338 | MSM(REG(RG), TMxx,    REG(RG)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define ornwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000338 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

#define ornwxZmr(MG, DG, RS)                                                \
        ornwxZst(W(RS), W(MG), W(DG))

/* xor (G = G ^ S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define xorwx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x68000000) | (M(TP2(IS) != 0) & 0x7C000278))

#define xorwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x68000000) | (M(TP2(IS) != 0) & 0x7C000278))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define xorwx_rr(RG, RS)                                                    \
        EMITW(0x7C000278 | MSM(REG(RG), REG(RG), REG(RS)))

#define xorwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000278 | MSM(REG(RG), REG(RG), TMxx))

#define xorwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000278 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define xorwx_mr(MG, DG, RS)                                                \
        xorwx_st(W(RS), W(MG), W(DG))


#define xorwxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x68000000) | (M(TP2(IS) != 0) & 0x7C000278))    \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define xorwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x68000000) | (M(TP2(IS) != 0) & 0x7C000278))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#if RT_BASE_COMPAT_ZFL == 0

#define xorwxZrr(RG, RS)                                                    \
        EMITW(0x7C000279 | MSM(REG(RG), REG(RG), REG(RS)))

#define xorwxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000279 | MSM(REG(RG), REG(RG), TMxx))

#define xorwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000279 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define xorwxZrr(RG, RS)                                                    \
        EMITW(0x7C000278 | MSM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define xorwxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000278 | MSM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define xorwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000278 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

#define xorwxZmr(MG, DG, RS)                                                \
        xorwxZst(W(RS), W(MG), W(DG))

/* not (G = ~G)
 * set-flags: no */

#define notwx_rx(RG)                                                        \
        EMITW(0x7C0000F8 | MSM(REG(RG), REG(RG), REG(RG)))

#define notwx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TWxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C0000F8 | MSM(TWxx,    TWxx,    TWxx))                     \
        EMITW(0x00000000 | MDM(TWxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

/* neg (G = -G)
 * set-flags: undefined (*_*), yes (*Z*) */

#define negwx_rx(RG)                                                        \
        EMITW(0x7C0000D0 | MRM(REG(RG), 0x00,    REG(RG)))

#define negwx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C0000D0 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))


#if RT_BASE_COMPAT_ZFL == 0

#define negwxZrx(RG)                                                        \
        EMITW(0x7C0000D1 | MRM(REG(RG), 0x00,    REG(RG)))

#define negwxZmx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C0000D1 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define negwxZrx(RG)                                                        \
        EMITW(0x7C0000D0 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define negwxZmx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C0000D0 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

/* add (G = G + S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define addwx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)) | \
        (M(TP1(IS) == 0) & 0x38000000) | (M(TP1(IS) != 0) & 0x7C000214))

#define addwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)) | \
        (M(TP1(IS) == 0) & 0x38000000) | (M(TP1(IS) != 0) & 0x7C000214))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define addwx_rr(RG, RS)                                                    \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), REG(RS)))

#define addwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), TMxx))

#define addhn_ld(RG, MS, DS)    /* add 16-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PS(DS)))  \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), TMxx))

#define addhz_ld(RG, MS, DS)    /* add 16-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), TMxx))

#define addbn_ld(RG, MS, DS)    /* add  8-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000774 | MSM(TMxx,    TMxx,    0x00))                     \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), TMxx))

#define addbz_ld(RG, MS, DS)    /* add  8-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), TMxx))

#define addwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000214 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define addwx_mr(MG, DG, RS)                                                \
        addwx_st(W(RS), W(MG), W(DG))


#if RT_BASE_COMPAT_ZFL == 0

#define addwxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)) | \
        (M(TP1(IS) == 0) & 0x34000000) | (M(TP1(IS) != 0) & 0x7C000215))

#define addwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)) | \
        (M(TP1(IS) == 0) & 0x34000000) | (M(TP1(IS) != 0) & 0x7C000215))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define addwxZrr(RG, RS)                                                    \
        EMITW(0x7C000215 | MRM(REG(RG), REG(RG), REG(RS)))

#define addwxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000215 | MRM(REG(RG), REG(RG), TMxx))

#define addhnZld(RG, MS, DS)    /* add 16-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PS(DS)))  \
        EMITW(0x7C000215 | MRM(REG(RG), REG(RG), TMxx))

#define addhzZld(RG, MS, DS)    /* add 16-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000215 | MRM(REG(RG), REG(RG), TMxx))

#define addbnZld(RG, MS, DS)    /* add  8-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000774 | MSM(TMxx,    TMxx,    0x00))                     \
        EMITW(0x7C000215 | MRM(REG(RG), REG(RG), TMxx))

#define addbzZld(RG, MS, DS)    /* add  8-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000215 | MRM(REG(RG), REG(RG), TMxx))

#define addwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000215 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define addwxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)) | \
        (M(TP1(IS) == 0) & 0x38000000) | (M(TP1(IS) != 0) & 0x7C000214))    \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define addwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)) | \
        (M(TP1(IS) == 0) & 0x38000000) | (M(TP1(IS) != 0) & 0x7C000214))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#define addwxZrr(RG, RS)                                                    \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define addwxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define addhnZld(RG, MS, DS)    /* add 16-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PS(DS)))  \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define addhzZld(RG, MS, DS)    /* add 16-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define addbnZld(RG, MS, DS)    /* add  8-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000774 | MSM(TMxx,    TMxx,    0x00))                     \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define addbzZld(RG, MS, DS)    /* add  8-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define addwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000214 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

#define addwxZmr(MG, DG, RS)                                                \
        addwxZst(W(RS), W(MG), W(DG))

/* sub (G = G - S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define subwx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), 0x00,    T1(IS), EMPTY1) | \
        (M(TP1(IS) == 0) & (0x38000000 | (0xFFFF & -VAL(IS)))) |            \
        (M(TP1(IS) != 0) & (0x7C000050 | TIxx << 16)))

#define subwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    0x00,    T1(IS), EMPTY1) | \
        (M(TP1(IS) == 0) & (0x38000000 | (0xFFFF & -VAL(IS)))) |            \
        (M(TP1(IS) != 0) & (0x7C000050 | TIxx << 16)))                      \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define subwx_rr(RG, RS)                                                    \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), REG(RS)))

#define subwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), TMxx))

#define subhn_ld(RG, MS, DS)  /* sub 16-bit from 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PS(DS)))  \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), TMxx))

#define subhz_ld(RG, MS, DS)  /* sub 16-bit from 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), TMxx))

#define subbn_ld(RG, MS, DS)  /* sub  8-bit from 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000774 | MSM(TMxx,    TMxx,    0x00))                     \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), TMxx))

#define subbz_ld(RG, MS, DS)  /* sub  8-bit from 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), TMxx))

#define subwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000050 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define subwx_mr(MG, DG, RS)                                                \
        subwx_st(W(RS), W(MG), W(DG))


#if RT_BASE_COMPAT_ZFL == 0

#define subwxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), 0x00,    T1(IS), EMPTY1) | \
        (M(TP1(IS) == 0) & (0x34000000 | (0xFFFF & -VAL(IS)))) |            \
        (M(TP1(IS) != 0) & (0x7C000051 | TIxx << 16)))

#define subwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    0x00,    T1(IS), EMPTY1) | \
        (M(TP1(IS) == 0) & (0x34000000 | (0xFFFF & -VAL(IS)))) |            \
        (M(TP1(IS) != 0) & (0x7C000051 | TIxx << 16)))                      \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define subwxZrr(RG, RS)                                                    \
        EMITW(0x7C000051 | MRM(REG(RG), REG(RG), REG(RS)))

#define subwxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000051 | MRM(REG(RG), REG(RG), TMxx))

#define subhnZld(RG, MS, DS)  /* sub 16-bit from 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PS(DS)))  \
        EMITW(0x7C000051 | MRM(REG(RG), REG(RG), TMxx))

#define subhzZld(RG, MS, DS)  /* sub 16-bit from 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000051 | MRM(REG(RG), REG(RG), TMxx))

#define subbnZld(RG, MS, DS)  /* sub  8-bit from 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000774 | MSM(TMxx,    TMxx,    0x00))                     \
        EMITW(0x7C000051 | MRM(REG(RG), REG(RG), TMxx))

#define subbzZld(RG, MS, DS)  /* sub  8-bit from 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000051 | MRM(REG(RG), REG(RG), TMxx))

#define subwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000051 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define subwxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), 0x00,    T1(IS), EMPTY1) | \
        (M(TP1(IS) == 0) & (0x38000000 | (0xFFFF & -VAL(IS)))) |            \
        (M(TP1(IS) != 0) & (0x7C000050 | TIxx << 16)))                      \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define subwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    0x00,    T1(IS), EMPTY1) | \
        (M(TP1(IS) == 0) & (0x38000000 | (0xFFFF & -VAL(IS)))) |            \
        (M(TP1(IS) != 0) & (0x7C000050 | TIxx << 16)))                      \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#define subwxZrr(RG, RS)                                                    \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define subwxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define subhnZld(RG, MS, DS)  /* sub 16-bit from 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PS(DS)))  \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define subhzZld(RG, MS, DS)  /* sub 16-bit from 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define subbnZld(RG, MS, DS)  /* sub  8-bit from 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000774 | MSM(TMxx,    TMxx,    0x00))                     \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define subbzZld(RG, MS, DS)  /* sub  8-bit from 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define subwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000050 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

#define subwxZmr(MG, DG, RS)                                                \
        subwxZst(W(RS), W(MG), W(DG))

/* shl (G = G << S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlwx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x7C000030 | MSM(REG(RG), REG(RG), Tecx))

#define shlwx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000030 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define shlwx_ri(RG, IS)                                                    \
        EMITW(0x54000000 | MSM(REG(RG), REG(RG), VAL(IS) & 0x1F) |          \
                                           ((31-VAL(IS)) & 0x1F) << 1)

#define shlwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x54000000 | MSM(TMxx,    TMxx,    VAL(IS) & 0x1F) |          \
                                           ((31-VAL(IS)) & 0x1F) << 1)      \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define shlwx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000030 | MSM(REG(RG), REG(RG), REG(RS)))

#define shlwx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000030 | MSM(REG(RG), REG(RG), TMxx))

#define shlwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000030 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define shlwx_mr(MG, DG, RS)                                                \
        shlwx_st(W(RS), W(MG), W(DG))


#define shlwxZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x7C000031 | MSM(REG(RG), REG(RG), Tecx))

#define shlwxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000031 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define shlwxZri(RG, IS)                                                    \
        EMITW(0x54000001 | MSM(REG(RG), REG(RG), VAL(IS) & 0x1F) |          \
                                           ((31-VAL(IS)) & 0x1F) << 1)

#define shlwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x54000001 | MSM(TMxx,    TMxx,    VAL(IS) & 0x1F) |          \
                                           ((31-VAL(IS)) & 0x1F) << 1)      \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define shlwxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000031 | MSM(REG(RG), REG(RG), REG(RS)))

#define shlwxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000031 | MSM(REG(RG), REG(RG), TMxx))

#define shlwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000031 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define shlwxZmr(MG, DG, RS)                                                \
        shlwxZst(W(RS), W(MG), W(DG))

/* shr (G = G >> S), unsigned (logical)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrwx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x7C000430 | MSM(REG(RG), REG(RG), Tecx))

#define shrwx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000430 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define shrwx_ri(RG, IS)                                                    \
        EMITW(0x5400003E | MSM(REG(RG), REG(RG), (32-VAL(IS)) & 0x1F) |     \
                                                     (VAL(IS) & 0x1F) << 6)

#define shrwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x5400003E | MSM(TMxx,    TMxx,    (32-VAL(IS)) & 0x1F) |     \
                                                     (VAL(IS) & 0x1F) << 6) \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define shrwx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000430 | MSM(REG(RG), REG(RG), REG(RS)))

#define shrwx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000430 | MSM(REG(RG), REG(RG), TMxx))

#define shrwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000430 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define shrwx_mr(MG, DG, RS)                                                \
        shrwx_st(W(RS), W(MG), W(DG))


#define shrwxZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x7C000431 | MSM(REG(RG), REG(RG), Tecx))

#define shrwxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000431 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define shrwxZri(RG, IS)                                                    \
        EMITW(0x5400003F | MSM(REG(RG), REG(RG), (32-VAL(IS)) & 0x1F) |     \
                                                     (VAL(IS) & 0x1F) << 6)

#define shrwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x5400003F | MSM(TMxx,    TMxx,    (32-VAL(IS)) & 0x1F) |     \
                                                     (VAL(IS) & 0x1F) << 6) \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define shrwxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000431 | MSM(REG(RG), REG(RG), REG(RS)))

#define shrwxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000431 | MSM(REG(RG), REG(RG), TMxx))

#define shrwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000431 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define shrwxZmr(MG, DG, RS)                                                \
        shrwxZst(W(RS), W(MG), W(DG))

/* shr (G = G >> S), signed (arithmetic)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrwn_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x7C000630 | MSM(REG(RG), REG(RG), Tecx))

#define shrwn_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000630 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define shrwn_ri(RG, IS)                                                    \
        EMITW(0x7C000670 | MSM(REG(RG), REG(RG), VAL(IS) & 0x1F))

#define shrwn_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000670 | MSM(TMxx,    TMxx,    VAL(IS) & 0x1F))           \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define shrwn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000630 | MSM(REG(RG), REG(RG), REG(RS)))

#define shrwn_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000630 | MSM(REG(RG), REG(RG), TMxx))

#define shrwn_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000630 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define shrwn_mr(MG, DG, RS)                                                \
        shrwn_st(W(RS), W(MG), W(DG))


#define shrwnZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x7C000631 | MSM(REG(RG), REG(RG), Tecx))

#define shrwnZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000631 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define shrwnZri(RG, IS)                                                    \
        EMITW(0x7C000671 | MSM(REG(RG), REG(RG), VAL(IS) & 0x1F))

#define shrwnZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000671 | MSM(TMxx,    TMxx,    VAL(IS) & 0x1F))           \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define shrwnZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000631 | MSM(REG(RG), REG(RG), REG(RS)))

#define shrwnZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000631 | MSM(REG(RG), REG(RG), TMxx))

#define shrwnZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x7C000631 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define shrwnZmr(MG, DG, RS)                                                \
        shrwnZst(W(RS), W(MG), W(DG))

/* ror (G = G >> S | G << 32 - S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define rorwx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x20000020 | MRM(TIxx,    0x00,    Tecx))                     \
        EMITW(0x5C00003E | MSM(REG(RG), REG(RG), TIxx))

#define rorwx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x20000020 | MRM(TIxx,    0x00,    Tecx))                     \
        EMITW(0x5C00003E | MSM(TMxx,    TMxx,    TIxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define rorwx_ri(RG, IS)                                                    \
        EMITW(0x5400003E | MSM(REG(RG), REG(RG), (32-VAL(IS)) & 0x1F))

#define rorwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x5400003E | MSM(TMxx,    TMxx,    (32-VAL(IS)) & 0x1F))      \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define rorwx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x20000020 | MRM(TIxx,    0x00,    REG(RS)))                  \
        EMITW(0x5C00003E | MSM(REG(RG), REG(RG), TIxx))

#define rorwx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x20000020 | MRM(TIxx,    0x00,    TMxx))                     \
        EMITW(0x5C00003E | MSM(REG(RG), REG(RG), TIxx))

#define rorwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x20000020 | MRM(TIxx,    0x00,    REG(RS)))                  \
        EMITW(0x5C00003E | MSM(TMxx,    TMxx,    TIxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define rorwx_mr(MG, DG, RS)                                                \
        rorwx_st(W(RS), W(MG), W(DG))


#define rorwxZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x20000020 | MRM(TIxx,    0x00,    Tecx))                     \
        EMITW(0x5C00003F | MSM(REG(RG), REG(RG), TIxx))

#define rorwxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x20000020 | MRM(TIxx,    0x00,    Tecx))                     \
        EMITW(0x5C00003F | MSM(TMxx,    TMxx,    TIxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define rorwxZri(RG, IS)                                                    \
        EMITW(0x5400003F | MSM(REG(RG), REG(RG), (32-VAL(IS)) & 0x1F))

#define rorwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x5400003F | MSM(TMxx,    TMxx,    (32-VAL(IS)) & 0x1F))      \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define rorwxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x20000020 | MRM(TIxx,    0x00,    REG(RS)))                  \
        EMITW(0x5C00003F | MSM(REG(RG), REG(RG), TIxx))

#define rorwxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x20000020 | MRM(TIxx,    0x00,    TMxx))                     \
        EMITW(0x5C00003F | MSM(REG(RG), REG(RG), TIxx))

#define rorwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0x20000020 | MRM(TIxx,    0x00,    REG(RS)))                  \
        EMITW(0x5C00003F | MSM(TMxx,    TMxx,    TIxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), O1(DG)))

#define rorwxZmr(MG, DG, RS)                                                \
        rorwxZst(W(RS), W(MG), W(DG))

/* mul (G = G * S)
 * set-flags: undefined */

#define mulwx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x7C0001D6 | MRM(REG(RG), REG(RG), TIxx))

#define mulwx_rr(RG, RS)                                                    \
        EMITW(0x7C0001D6 | MRM(REG(RG), REG(RG), REG(RS)))

#define mulwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C0001D6 | MRM(REG(RG), REG(RG), TMxx))

#define mulhn_ld(RG, MS, DS)  /* mul 32-bit with 16-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PS(DS)))  \
        EMITW(0x7C0001D6 | MRM(REG(RG), REG(RG), TMxx))

#define mulhz_ld(RG, MS, DS)  /* mul 32-bit with 16-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C0001D6 | MRM(REG(RG), REG(RG), TMxx))

#define mulbn_ld(RG, MS, DS)  /* mul 32-bit with  8-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000774 | MSM(TMxx,    TMxx,    0x00))                     \
        EMITW(0x7C0001D6 | MRM(REG(RG), REG(RG), TMxx))

#define mulbz_ld(RG, MS, DS)  /* mul 32-bit with  8-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C0001D6 | MRM(REG(RG), REG(RG), TMxx))


#define mulwx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0x7C000016 | MRM(Tedx,    Teax,    REG(RS)))                  \
        EMITW(0x7C0001D6 | MRM(Teax,    Teax,    REG(RS)))

#define mulwx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000016 | MRM(Tedx,    Teax,    TMxx))                     \
        EMITW(0x7C0001D6 | MRM(Teax,    Teax,    TMxx))


#define mulwn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0x7C000096 | MRM(Tedx,    Teax,    REG(RS)))                  \
        EMITW(0x7C0001D6 | MRM(Teax,    Teax,    REG(RS)))

#define mulwn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000096 | MRM(Tedx,    Teax,    TMxx))                     \
        EMITW(0x7C0001D6 | MRM(Teax,    Teax,    TMxx))


#define mulwp_xr(RS)     /* Reax is in/out, prepares Redx for divwn_x* */   \
        mulwx_rr(Reax, W(RS)) /* product must not exceed operands size */

#define mulwp_xm(MS, DS) /* Reax is in/out, prepares Redx for divwn_x* */   \
        mulwx_ld(Reax, W(MS), W(DS))  /* must not exceed operands size */

/* div (G = G / S)
 * set-flags: undefined */

#define divwx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x7C000396 | MTM(REG(RG), REG(RG), TIxx))

#define divwx_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        EMITW(0x7C000396 | MTM(REG(RG), REG(RG), REG(RS)))

#define divwx_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000396 | MTM(REG(RG), REG(RG), TMxx))


#define divwn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x7C0003D6 | MTM(REG(RG), REG(RG), TIxx))

#define divwn_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        EMITW(0x7C0003D6 | MTM(REG(RG), REG(RG), REG(RS)))

#define divwn_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C0003D6 | MTM(REG(RG), REG(RG), TMxx))


#define prewx_xx()   /* to be placed right before divwx_x* or remwx_xx */   \
                                     /* to prepare Redx for int-divide */

#define prewn_xx()   /* to be placed right before divwn_x* or remwn_xx */   \
                                     /* to prepare Redx for int-divide */


#define divwx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(0x7C000396 | MTM(Teax,    Teax,    REG(RS)))

#define divwx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000396 | MTM(Teax,    Teax,    TMxx))


#define divwn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(0x7C0003D6 | MTM(Teax,    Teax,    REG(RS)))

#define divwn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C0003D6 | MTM(Teax,    Teax,    TMxx))


#define divwp_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xr(W(RS))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divwp_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xm(W(MS), W(DS))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem (G = G % S)
 * set-flags: undefined */

#if RT_BASE_COMPAT_REM == 0

#define remwx_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        EMITW(0x7C000378 | MSM(TWxx,    REG(RG), REG(RG)))                  \
        divwx_ri(W(RG), W(IS))                                              \
        EMITW(0x7C0001D6 | MRM(TMxx,    REG(RG), TIxx))                     \
        EMITW(0x7C000050 | MRM(REG(RG), TWxx,    TMxx))

#define remwx_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        EMITW(0x7C000378 | MSM(TWxx,    REG(RG), REG(RG)))                  \
        divwx_rr(W(RG), W(RS))                                              \
        EMITW(0x7C0001D6 | MRM(TMxx,    REG(RG), REG(RS)))                  \
        EMITW(0x7C000050 | MRM(REG(RG), TWxx,    TMxx))

#define remwx_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        EMITW(0x7C000378 | MSM(TWxx,    REG(RG), REG(RG)))                  \
        divwx_ld(W(RG), W(MS), W(DS))                                       \
        EMITW(0x7C0001D6 | MRM(TMxx,    REG(RG), TMxx))                     \
        EMITW(0x7C000050 | MRM(REG(RG), TWxx,    TMxx))


#define remwn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        EMITW(0x7C000378 | MSM(TWxx,    REG(RG), REG(RG)))                  \
        divwn_ri(W(RG), W(IS))                                              \
        EMITW(0x7C0001D6 | MRM(TMxx,    REG(RG), TIxx))                     \
        EMITW(0x7C000050 | MRM(REG(RG), TWxx,    TMxx))

#define remwn_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        EMITW(0x7C000378 | MSM(TWxx,    REG(RG), REG(RG)))                  \
        divwn_rr(W(RG), W(RS))                                              \
        EMITW(0x7C0001D6 | MRM(TMxx,    REG(RG), REG(RS)))                  \
        EMITW(0x7C000050 | MRM(REG(RG), TWxx,    TMxx))

#define remwn_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        EMITW(0x7C000378 | MSM(TWxx,    REG(RG), REG(RG)))                  \
        divwn_ld(W(RG), W(MS), W(DS))                                       \
        EMITW(0x7C0001D6 | MRM(TMxx,    REG(RG), TMxx))                     \
        EMITW(0x7C000050 | MRM(REG(RG), TWxx,    TMxx))


#define remwx_xx() /* to be placed before divwx_x*, but after prewx_xx */   \
        movwx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remwx_xr(RS)        /* to be placed immediately after divwx_xr */   \
        EMITW(0x7C0001D6 | MRM(TMxx,    Teax,    REG(RS)))                  \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */

#define remwx_xm(MS, DS)    /* to be placed immediately after divwx_xm */   \
        EMITW(0x7C0001D6 | MRM(TMxx,    Teax,    TMxx))                     \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */


#define remwn_xx() /* to be placed before divwn_x*, but after prewn_xx */   \
        movwx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remwn_xr(RS)        /* to be placed immediately after divwn_xr */   \
        EMITW(0x7C0001D6 | MRM(TMxx,    Teax,    REG(RS)))                  \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */

#define remwn_xm(MS, DS)    /* to be placed immediately after divwn_xm */   \
        EMITW(0x7C0001D6 | MRM(TMxx,    Teax,    TMxx))                     \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */

#else /* RT_BASE_COMPAT_REM != 0 */

#define remwx_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x7C000216 | MTM(REG(RG), REG(RG), TIxx))

#define remwx_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        EMITW(0x7C000216 | MTM(REG(RG), REG(RG), REG(RS)))

#define remwx_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000216 | MTM(REG(RG), REG(RG), TMxx))


#define remwn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x7C000616 | MTM(REG(RG), REG(RG), TIxx))

#define remwn_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        EMITW(0x7C000616 | MTM(REG(RG), REG(RG), REG(RS)))

#define remwn_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000616 | MTM(REG(RG), REG(RG), TMxx))


#define remwx_xx() /* to be placed before divwx_x*, but after prewx_xx */   \
        movwx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remwx_xr(RS)        /* to be placed immediately after divwx_xr */   \
        EMITW(0x7C000216 | MTM(Tedx,    Tedx,    REG(RS)))/* Redx<-rem */

#define remwx_xm(MS, DS)    /* to be placed immediately after divwx_xm */   \
        EMITW(0x7C000216 | MTM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */


#define remwn_xx() /* to be placed before divwn_x*, but after prewn_xx */   \
        movwx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remwn_xr(RS)        /* to be placed immediately after divwn_xr */   \
        EMITW(0x7C000616 | MTM(Tedx,    Tedx,    REG(RS)))/* Redx<-rem */

#define remwn_xm(MS, DS)    /* to be placed immediately after divwn_xm */   \
        EMITW(0x7C000616 | MTM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */

#endif /* RT_BASE_COMPAT_REM != 0 */

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
#define add_n   AM3
#define sub_n   AM4
#define add_z   AM5
#define sub_z   AM6
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
        cmjwx_ri(W(RS), IC(0), cc, lb)

#define cmjwx_mz(MS, DS, cc, lb)                                            \
        cmjwx_mi(W(MS), W(DS), IC(0), cc, lb)

#define cmjwx_ri(RS, IT, cc, lb)                                            \
        CWI(cc, MOD(RS), REG(RS), W(IT), lb)

#define cmjwx_mi(MS, DS, IT, cc, lb)                                        \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        CWI(cc, %%r24,   TMxx,    W(IT), lb)

#define cmjwx_rr(RS, RT, cc, lb)                                            \
        CWR(cc, MOD(RS), MOD(RT), lb)

#define cmjwx_rm(RS, MT, DT, cc, lb)                                        \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        CWR(cc, MOD(RS), %%r24,   lb)

#define cmjhn_rm(RS, MT, DT, cc, lb)   /* cmj 32/16-bit with sign-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MT), VAL(DT), B1(DT), PS(DT)))  \
        CWR(cc, MOD(RS), %%r24,   lb)

#define cmjhz_rm(RS, MT, DT, cc, lb)   /* cmj 32/16-bit with zero-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MT), VAL(DT), B1(DT), PH(DT)))  \
        CWR(cc, MOD(RS), %%r24,   lb)

#define cmjbn_rm(RS, MT, DT, cc, lb)   /* cmj 32/8-bit  with sign-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MT), VAL(DT), B1(DT), PB(DT)))  \
        EMITW(0x7C000774 | MSM(TMxx,    TMxx,    0x00))                     \
        CWR(cc, MOD(RS), %%r24,   lb)

#define cmjbz_rm(RS, MT, DT, cc, lb)   /* cmj 32/8-bit  with zero-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MT), VAL(DT), B1(DT), PB(DT)))  \
        CWR(cc, MOD(RS), %%r24,   lb)

#define cmjwx_mr(MS, DS, RT, cc, lb)                                        \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        CWR(cc, %%r24,   MOD(RT), lb)

#define cmjhn_mr(MS, DS, RT, cc, lb)   /* cmj 16/32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PS(DS)))  \
        CWR(cc, %%r24,   MOD(RT), lb)

#define cmjhz_mr(MS, DS, RT, cc, lb)   /* cmj 16/32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        CWR(cc, %%r24,   MOD(RT), lb)

#define cmjbn_mr(MS, DS, RT, cc, lb)   /* cmj  8/32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000774 | MSM(TMxx,    TMxx,    0x00))                     \
        CWR(cc, %%r24,   MOD(RT), lb)

#define cmjbz_mr(MS, DS, RT, cc, lb)   /* cmj  8/32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        CWR(cc, %%r24,   MOD(RT), lb)

#if   (defined RT_P32)

/* cmp (flags = S ? T)
 * set-flags: yes */

#define cmpwx_ri(RS, IT)                                                    \
        AUW(EMPTY,    VAL(IT), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IT))   \
        EMITW(0x7C000378 | MSM(TLxx,    REG(RS), REG(RS)))

#define cmpwx_mi(MS, DS, IT)                                                \
        AUW(SIB(MS),  VAL(IT), TRxx,    MOD(MS), VAL(DS), C1(DS), G3(IT))   \
        EMITW(0x00000000 | MDM(TLxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))

#define cmpwx_rr(RS, RT)                                                    \
        EMITW(0x7C000378 | MSM(TRxx,    REG(RT), REG(RT)))                  \
        EMITW(0x7C000378 | MSM(TLxx,    REG(RS), REG(RS)))

#define cmpwx_rm(RS, MT, DT)                                                \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x00000000 | MDM(TRxx,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x7C000378 | MSM(TLxx,    REG(RS), REG(RS)))

#define cmphn_rm(RS, MT, DT)    /* cmp 32-bit to 16-bit with sign-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x00000000 | MDM(TRxx,    MOD(MT), VAL(DT), B1(DT), PS(DT)))  \
        EMITW(0x7C000378 | MSM(TLxx,    REG(RS), REG(RS)))

#define cmphz_rm(RS, MT, DT)    /* cmp 32-bit to 16-bit with zero-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x00000000 | MDM(TRxx,    MOD(MT), VAL(DT), B1(DT), PH(DT)))  \
        EMITW(0x7C000378 | MSM(TLxx,    REG(RS), REG(RS)))

#define cmpbn_rm(RS, MT, DT)    /* cmp 32-bit to  8-bit with sign-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x00000000 | MDM(TRxx,    MOD(MT), VAL(DT), B1(DT), PB(DT)))  \
        EMITW(0x7C000774 | MSM(TRxx,    TRxx,    0x00))                     \
        EMITW(0x7C000378 | MSM(TLxx,    REG(RS), REG(RS)))

#define cmpbz_rm(RS, MT, DT)    /* cmp 32-bit to  8-bit with zero-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x00000000 | MDM(TRxx,    MOD(MT), VAL(DT), B1(DT), PB(DT)))  \
        EMITW(0x7C000378 | MSM(TLxx,    REG(RS), REG(RS)))

#define cmpwx_mr(MS, DS, RT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TLxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C000378 | MSM(TRxx,    REG(RT), REG(RT)))

#define cmphn_mr(MS, DS, RT)    /* cmp 16-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TLxx,    MOD(MS), VAL(DS), B1(DS), PS(DS)))  \
        EMITW(0x7C000378 | MSM(TRxx,    REG(RT), REG(RT)))

#define cmphz_mr(MS, DS, RT)    /* cmp 16-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TLxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000378 | MSM(TRxx,    REG(RT), REG(RT)))

#define cmpbn_mr(MS, DS, RT)    /* cmp  8-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TLxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000774 | MSM(TLxx,    TLxx,    0x00))                     \
        EMITW(0x7C000378 | MSM(TRxx,    REG(RT), REG(RT)))

#define cmpbz_mr(MS, DS, RT)    /* cmp  8-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TLxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000378 | MSM(TRxx,    REG(RT), REG(RT)))

#endif /* (defined RT_P32) */

/* ver (Mebp/inf_VER = SIMD-version)
 * set-flags: no
 * For interpretation of SIMD build flags check compatibility layer in rtzero.h
 * 0th byte - 128-bit version, 1st byte - 256-bit version, | plus _R8/_RX slots
 * 2nd byte - 512-bit version, 3rd byte - 1K4-bit version, | in upper halves */

#define verxx_xx() /* destroys Reax, Recx, Rebx, Redx, Resi, Redi */        \
        movwx_mi(Mebp, inf_VER, IV(0x033F4F)) /* <- VSX1+2/3/VMX: 128,256,512 */

/************************* address-sized instructions *************************/

/* adr (D = adr S)
 * set-flags: no */

#define adrxx_ld(RD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C3(DS), EMPTY2)   \
        EMITW(0x7C000214 | MRM(REG(RD), MOD(MS), TDxx))

/************************* pointer-sized instructions *************************/

/* label (D = Reax = adr lb)
 * set-flags: no */

     /* label_ld(lb) is defined in rtarch.h file, loads label to Reax */

     /* label_st(lb, MD, DD) is defined in rtarch.h file, destroys Reax */

#if   (defined RT_P32)

/* jmp (if unconditional jump S/lb, else if cc flags then jump lb)
 * set-flags: no
 * maximum byte-address-range for un/conditional jumps is signed 18/16-bit
 * based on minimum natively-encoded offset across supported targets (u/c)
 * MIPS:18-bit, POWER:26-bit, AArch32:26-bit, AArch64:28-bit, x86:32-bit /
 * MIPS:18-bit, POWER:16-bit, AArch32:26-bit, AArch64:21-bit, x86:32-bit */

#define jmpxx_xr(RS)           /* register-targeted unconditional jump */   \
        EMITW(0x7C0003A6 | MRM(REG(RS), 0x00,    0x09)) /* ctr <- reg */    \
        EMITW(0x4C000420 | MTM(0x0C,    0x0A,    0x00)) /* beqctr cr2 */

#define jmpxx_xm(MS, DS)         /* memory-targeted unconditional jump */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7C0003A6 | MRM(TMxx,    0x00,    0x09)) /* ctr <- r24 */    \
        EMITW(0x4C000420 | MTM(0x0C,    0x0A,    0x00)) /* beqctr cr2 */

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

/************************* register-size instructions *************************/

/* stack (push stack = S, D = pop stack)
 * set-flags: no (sequence cmp/stack_la/jmp is not allowed on MIPS & POWER)
 * adjust stack pointer with 8-byte (64-bit) steps on all current targets */

#define stack_st(RS)                                                        \
        EMITW(0x38000000 | MTM(SPxx,    SPxx,    0x00) | (-0x08 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(REG(RS), SPxx,    0x00))

#define stack_ld(RD)                                                        \
        EMITW(0x80000000 | MTM(REG(RD), SPxx,    0x00))                     \
        EMITW(0x38000000 | MTM(SPxx,    SPxx,    0x00) | (+0x08 & 0xFFFF))

#define stack_sa()  /* save all, [Reax - RegE] + 12 temps, 26 regs total */ \
        EMITW(0x38000000 | MTM(SPxx,    SPxx,    0x00) | (-0x70 & 0xFFFF))  \
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
        EMITW(0x90000000 | MTM(TDxx,    SPxx,    0x00) | (+0x40 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(TPxx,    SPxx,    0x00) | (+0x44 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(TCxx,    SPxx,    0x00) | (+0x48 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(TVxx,    SPxx,    0x00) | (+0x4C & 0xFFFF))  \
        EMITW(0x90000000 | MTM(T0xx,    SPxx,    0x00) | (+0x50 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(T1xx,    SPxx,    0x00) | (+0x54 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(T2xx,    SPxx,    0x00) | (+0x58 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(T3xx,    SPxx,    0x00) | (+0x5C & 0xFFFF))  \
        EMITW(0x90000000 | MTM(TZxx,    SPxx,    0x00) | (+0x60 & 0xFFFF))  \
        EMITW(0x90000000 | MTM(TWxx,    SPxx,    0x00) | (+0x64 & 0xFFFF))

#define stack_la()  /* load all, 12 temps + [RegE - Reax], 26 regs total */ \
        EMITW(0x80000000 | MTM(TWxx,    SPxx,    0x00) | (+0x64 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(TZxx,    SPxx,    0x00) | (+0x60 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(T3xx,    SPxx,    0x00) | (+0x5C & 0xFFFF))  \
        EMITW(0x80000000 | MTM(T2xx,    SPxx,    0x00) | (+0x58 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(T1xx,    SPxx,    0x00) | (+0x54 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(T0xx,    SPxx,    0x00) | (+0x50 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(TVxx,    SPxx,    0x00) | (+0x4C & 0xFFFF))  \
        EMITW(0x80000000 | MTM(TCxx,    SPxx,    0x00) | (+0x48 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(TPxx,    SPxx,    0x00) | (+0x44 & 0xFFFF))  \
        EMITW(0x80000000 | MTM(TDxx,    SPxx,    0x00) | (+0x40 & 0xFFFF))  \
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
        EMITW(0x38000000 | MTM(SPxx,    SPxx,    0x00) | (+0x70 & 0xFFFF))

#endif /* (defined RT_P32) */

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
#define AM5(sz, sg) add##sz##z##sg
#define AM6(sz, sg) sub##sz##z##sg
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

/* internal definitions for combined-compare-jump (cmj) */

#define IWJ0(r1, p1, IS, lb)                                                \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x28000000) | (M(TP2(IS) != 0) & 0x7C000040))    \
        ASM_BEG ASM_OP1(beq,   lb) ASM_END

#define IWJ1(r1, p1, IS, lb)                                                \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x28000000) | (M(TP2(IS) != 0) & 0x7C000040))    \
        ASM_BEG ASM_OP1(bne,   lb) ASM_END

#define IWJ2(r1, p1, IS, lb)                                                \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x28000000) | (M(TP2(IS) != 0) & 0x7C000040))    \
        ASM_BEG ASM_OP1(blt,   lb) ASM_END

#define IWJ3(r1, p1, IS, lb)                                                \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x28000000) | (M(TP2(IS) != 0) & 0x7C000040))    \
        ASM_BEG ASM_OP1(ble,   lb) ASM_END

#define IWJ4(r1, p1, IS, lb)                                                \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x28000000) | (M(TP2(IS) != 0) & 0x7C000040))    \
        ASM_BEG ASM_OP1(bgt,   lb) ASM_END

#define IWJ5(r1, p1, IS, lb)                                                \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x28000000) | (M(TP2(IS) != 0) & 0x7C000040))    \
        ASM_BEG ASM_OP1(bge,   lb) ASM_END

#define IWJ6(r1, p1, IS, lb)                                                \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IS), T3(IS), M3(IS)) | \
        (M(TP1(IS) == 0) & 0x2C000000) | (M(TP1(IS) != 0) & 0x7C000000))    \
        ASM_BEG ASM_OP1(blt,   lb) ASM_END

#define IWJ7(r1, p1, IS, lb)                                                \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IS), T3(IS), M3(IS)) | \
        (M(TP1(IS) == 0) & 0x2C000000) | (M(TP1(IS) != 0) & 0x7C000000))    \
        ASM_BEG ASM_OP1(ble,   lb) ASM_END

#define IWJ8(r1, p1, IS, lb)                                                \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IS), T3(IS), M3(IS)) | \
        (M(TP1(IS) == 0) & 0x2C000000) | (M(TP1(IS) != 0) & 0x7C000000))    \
        ASM_BEG ASM_OP1(bgt,   lb) ASM_END

#define IWJ9(r1, p1, IS, lb)                                                \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IS), T3(IS), M3(IS)) | \
        (M(TP1(IS) == 0) & 0x2C000000) | (M(TP1(IS) != 0) & 0x7C000000))    \
        ASM_BEG ASM_OP1(bge,   lb) ASM_END

#define CWI(cc, r1, p1, IS, lb)                                             \
        IW##cc(r1, p1, W(IS), lb)


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

#endif /* RT_RTARCH_P32_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

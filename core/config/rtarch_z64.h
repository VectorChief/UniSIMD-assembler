/******************************************************************************/
/* Copyright (c) 2013-2026 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_Z64_H
#define RT_RTARCH_Z64_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_z64.h: Implementation of s390x BASE+SIMD instructions.
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

#if (defined RT_Z64)

/* structural */

#define MRM(cod, reg, ren, rem) /* arith32 */                               \
        ((reg) << 4 | (ren) | (rem) << 12 | (cod) << 8)

#define MGM(cod, reg, ren, rem) /* arith64 */                               \
        ((reg) << 4 | (ren) | (rem) << 12 | (0xB900 | (cod)) << 16)

#define MIM(cod, reg, ren, imm) /* arithIM */                               \
        ((reg) << 36 | (ren) << 32 | (imm) | (cod) << 40)

#define MSM(reg, ren, rem) /* logic, shifts */                              \
        ((reg) << 36 | (ren) << 32 | (rem) << 28)

#define MDM(cod, reg, bmd, brm, vdp, bxx, pxx)                              \
        (pxx(vdp) | (bmd) << 32 | bxx(brm) << 28 | (reg) << 36 | (cod))

#define MPM(cod, reg, bmd, brm, vdp, bxx, pxx)                              \
        (pxx(vdp) | (bmd) << 32 | bxx(brm) << 28 | (reg & 0x0F) << 36 |     \
         0xE70000000000 | (cod) | (reg & 0x10) <<  7)

#define MWM(cod, reg, ren, rem, bit)                                        \
        ((reg & 0x0F) << 36 | (ren & 0x0F) << 32 | (rem & 0x0F) << 28 |     \
         (reg & 0x10) <<  7 | (ren & 0x10) <<  6 | (rem & 0x10) <<  5 |     \
         0xE70000080000 | (bit) << 12 | (cod))

#define MXM(cod, reg, ren, rem, bit)                                        \
        ((reg & 0x0F) << 36 | (ren & 0x0F) << 32 | (rem & 0x0F) << 28 |     \
         (reg & 0x10) <<  7 | (ren & 0x10) <<  6 | (rem & 0x10) <<  5 |     \
         0xE70000000000 | (bit) << 12 | (cod))

#define MYM(cod, reg, ren, mod, bit)                                        \
        ((reg & 0x0F) << 36 | (ren & 0x0F) << 32 | (mod & 0x0F) << 20 |     \
         (reg & 0x10) <<  7 | (ren & 0x10) <<  6 |                          \
         0xE70000080000 | (bit) << 12 | (cod))

#define MZM(cod, reg, ren, mod, bit)                                        \
        ((reg & 0x0F) << 36 | (ren & 0x0F) << 32 | (mod & 0x0F) << 20 |     \
         (reg & 0x10) <<  7 | (ren & 0x10) <<  6 |                          \
         0xE70000000000 | (bit) << 12 | (cod))

#define AUW(sib, vim, reg, brm, vdp, cdp, cim)                              \
            sib  cdp(brm, vdp)  cim(reg, vim)

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

#define  B1(val, tp1, tp2)  B1##tp1
#define  B3(val, tp1, tp2)  B3##tp1
#define  P1(val, tp1, tp2)  P1##tp1
#define  C1(val, tp1, tp2)  C1##tp1
#define  A1(val, tp1, tp2)  A1##tp1
#define  C3(val, tp1, tp2)  C3##tp2 /* <- "C3##tp2" not a bug */

#define  B2(val, tp1, tp2)  B2##tp2
#define  B4(val, tp1, tp2)  B4##tp2
#define  P2(val, tp1, tp2)  P2##tp2
#define  L2(val, tp1, tp2)  L2##tp2
#define  C2(val, tp1, tp2)  C2##tp2
#define  A2(val, tp1, tp2)  A2##tp2

/* displacement encoding BASE(TP1), adr(TP3) */

#define B10(br) (br)
#define B30(br) (br)
#define P10(dp) (0xE30000000000 | (0x0FFF & (dp))<<16 | (0x0FF000 & (dp))>>4)
#define C10(br, dp) EMPTY
#define A10(br, dp) EMPTY
#define C30(br, dp) EMIT6(0xC00100000000 | TDxx << 36 | (0x7FFFFFFF & (dp)))

#define B11(br) TDxx
#define B31(br) TPxx
#define P11(dp) (0xE30000000000)
#define C11(br, dp) C31(br, dp)
#define A11(br, dp) C31(br, dp)                                             \
                    EMITW(0xB9080000 | TDxx << 4 | (br))
#define C31(br, dp) EMIT6(0xC00100000000 | TDxx << 36 | (0x7FFFFFFF & (dp)))

/* displacement encoding SIMD(TP2), ELEM(TP2) */

#define B20(br) (br)
#define B40(br) (br)
#define P20(dp) (0xE30000000000 | (0x0FFF & (dp))<<16)
#define L20(dp) (0xE30000000000 | (0x0FFF & (dp))<<16)
#define C20(br, dp) EMPTY
#define A20(br, dp) EMPTY
#define C40(br, dp) EMIT6(0xC00100000000 | TDxx << 36 | (0x7FFFFFFF & (dp)))

#define B21(br) TDxx
#define B41(br) TPxx
#define P21(dp) (0xE30000000000)
#define L21(dp) (0xE30000000000 | (0x0010 & (dp))<<16)
#define C21(br, dp) EMIT6(0xC00100000000 | TDxx << 36 | (0x7FFFFFFF & (dp)))
#define A21(br, dp) C41(br, dp)                                             \
                    EMITW(0xB9080000 | TDxx << 4 | (br))

/* internal     REG */

#define TMxx    0x00  /* r0 */
#define TDxx    0x01  /* r1 */
#define TPxx    0x02  /* r2 */
#define SPxx    0x0F  /* r15 */

#define TEax    0x04  /* r4, must be non-zero reg-index */
#define TEcx    0x0D  /* r13 */
#define TEdx    0x0E  /* r14 */
#define TEbx    0x03  /* r3 */
#define TEbp    0x05  /* r5 */
#define TEsi    0x06  /* r6 */
#define TEdi    0x07  /* r7 */
#define TEg8    0x08  /* r8 */
#define TEg9    0x09  /* r9 */
#define TEgA    0x0A  /* r10 */
#define TEgB    0x0B  /* r11 */
#define TEgC    0x0C  /* r12 */

#define Tmm0    0x00  /* v0 */
#define Tmm1    0x01  /* v1 */
#define Tmm2    0x02  /* v2 */
#define Tmm3    0x03  /* v3 */
#define Tmm4    0x04  /* v4 */
#define Tmm5    0x05  /* v5 */
#define Tmm6    0x06  /* v6 */
#define Tmm7    0x07  /* v7 */
#define Tmm8    0x08  /* v8 */
#define Tmm9    0x09  /* v9 */
#define TmmA    0x0A  /* v10 */
#define TmmB    0x0B  /* v11 */
#define TmmC    0x0C  /* v12 */
#define TmmD    0x0D  /* v13 */
#define TmmE    0x0E  /* v14 */

#define TmmF    0x0F  /* v15 */
#define TmmG    0x10  /* v16 */
#define TmmH    0x11  /* v17 */
#define TmmI    0x12  /* v18 */
#define TmmJ    0x13  /* v19 */
#define TmmK    0x14  /* v20 */
#define TmmL    0x15  /* v21 */
#define TmmW    0x16  /* v22 */
#define TmmN    0x17  /* v23 */
#define TmmO    0x18  /* v24 */
#define TmmP    0x19  /* v25 */
#define TmmQ    0x1A  /* v26 */
#define TmmR    0x1B  /* v27 */
#define TmmS    0x1C  /* v28 */
#define TmmT    0x1D  /* v29 */

#define TmmM    0x1F  /* v31 */

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/* registers    REG,  MOD,  SIB */

#define Reax    TEax, %r4,  EMPTY
#define Recx    TEcx, %r13, EMPTY
#define Redx    TEdx, %r14, EMPTY
#define Rebx    TEbx, %r3,  EMPTY
#define Rebp    TEbp, %r5,  EMPTY
#define Resi    TEsi, %r6,  EMPTY
#define Redi    TEdi, %r7,  EMPTY
#define Reg8    TEg8, %r8,  EMPTY
#define Reg9    TEg9, %r9,  EMPTY
#define RegA    TEgA, %r10, EMPTY
#define RegB    TEgB, %r11, EMPTY
#define RegC    TEgC, %r12, EMPTY

/* addressing   REG,  MOD,  SIB */

#define Oeax    TEax, 0x00, EMPTY

#define Mecx    TEcx, 0x00, EMPTY
#define Medx    TEdx, 0x00, EMPTY
#define Mebx    TEbx, 0x00, EMPTY
#define Mebp    TEbp, 0x00, EMPTY
#define Mesi    TEsi, 0x00, EMPTY
#define Medi    TEdi, 0x00, EMPTY
#define Meg8    TEg8, 0x00, EMPTY
#define Meg9    TEg9, 0x00, EMPTY
#define MegA    TEgA, 0x00, EMPTY
#define MegB    TEgB, 0x00, EMPTY
#define MegC    TEgC, 0x00, EMPTY

/* immediate    VAL,  TP1,  TP2            (all immediate types are unsigned) */
/* full-size IW type is only applicable within cmdw* subset, can set sign-bit */
/* within cmdz* subset use of IW type is limited to movzx_rj/_mj instructions */

#define  IC(im) ((im) & 0x7F),          0, 0        /* drop sign-ext (on x86) */
#define  IB(im) ((im) & 0xFF),          0, 0          /* 32-bit word (on x86) */
#define  IM(im) ((im) & 0xFFF),         0, 0    /* native AArch64 add/sub/cmp */
#define  IG(im) ((im) & 0x7FFF),        0, 0 /* native MIPS/POWER add/sub/cmp */
#define  IH(im) ((im) & 0xFFFF),        0, 0    /* second native on ARMs/MIPS */
#define  IV(im) ((im) & 0x7FFFFFFF),    0, 0       /* native x86_64 long mode */
#define  IW(im) ((im) & 0xFFFFFFFF),    0, 0          /* for cmdw* subset, *j */

/* public scalable I/J/K/L*** definitions are now provided in rtbase.h */
/* public scalable S/T/U/V*** definitions accept any register as index */
/* fully explicit N*** takes index register and scale (1,2,3) for 2x/4x/8x */

#define Secx(R) TEcx,REG(R),EMPTY
#define Sedx(R) TEdx,REG(R),EMPTY
#define Sebx(R) TEbx,REG(R),EMPTY
#define Sebp(R) TEbp,REG(R),EMPTY
#define Sesi(R) TEsi,REG(R),EMPTY
#define Sedi(R) TEdi,REG(R),EMPTY
#define Seg8(R) TEg8,REG(R),EMPTY
#define Seg9(R) TEg9,REG(R),EMPTY
#define SegA(R) TEgA,REG(R),EMPTY
#define SegB(R) TEgB,REG(R),EMPTY
#define SegC(R) TEgC,REG(R),EMPTY

#define Necx(R,n) TEcx,TPxx,EMIT6(0xEB000000000D|MSM(TPxx,REG(R),0x00)|(n)<<16)
#define Nedx(R,n) TEdx,TPxx,EMIT6(0xEB000000000D|MSM(TPxx,REG(R),0x00)|(n)<<16)
#define Nebx(R,n) TEbx,TPxx,EMIT6(0xEB000000000D|MSM(TPxx,REG(R),0x00)|(n)<<16)
#define Nebp(R,n) TEbp,TPxx,EMIT6(0xEB000000000D|MSM(TPxx,REG(R),0x00)|(n)<<16)
#define Nesi(R,n) TEsi,TPxx,EMIT6(0xEB000000000D|MSM(TPxx,REG(R),0x00)|(n)<<16)
#define Nedi(R,n) TEdi,TPxx,EMIT6(0xEB000000000D|MSM(TPxx,REG(R),0x00)|(n)<<16)
#define Neg8(R,n) TEg8,TPxx,EMIT6(0xEB000000000D|MSM(TPxx,REG(R),0x00)|(n)<<16)
#define Neg9(R,n) TEg9,TPxx,EMIT6(0xEB000000000D|MSM(TPxx,REG(R),0x00)|(n)<<16)
#define NegA(R,n) TEgA,TPxx,EMIT6(0xEB000000000D|MSM(TPxx,REG(R),0x00)|(n)<<16)
#define NegB(R,n) TEgB,TPxx,EMIT6(0xEB000000000D|MSM(TPxx,REG(R),0x00)|(n)<<16)
#define NegC(R,n) TEgC,TPxx,EMIT6(0xEB000000000D|MSM(TPxx,REG(R),0x00)|(n)<<16)

/* displacement VAL,  TP1,  TP2         (all displacement types are unsigned) */
/* public scalable DP/DE/DF/DG/DH/DV definitions are now provided in rtbase.h */
/* as D* are used for BASE and SIMD instructions, only limits are SIMD-scaled */

#define _DP(dp) ((dp) & 0xFFF),         0, 0      /* native on all ARMs, MIPS */
#define _DE(dp) ((dp) & 0x1FFF),        0, 1     /* AArch64 256-bit SVE ld/st */
#define _DF(dp) ((dp) & 0x3FFF),        0, 1     /* native AArch64 BASE ld/st */
#define _DG(dp) ((dp) & 0x7FFF),        0, 1  /* native MIPS/POWER BASE ld/st */
#define _DH(dp) ((dp) & 0xFFFF),        0, 1     /* second native on all ARMs */
#define _DV(dp) ((dp) & 0x7FFFFFFF),    1, 1       /* native x86_64 long mode */
#define  PLAIN  DP(0)                /* special type for Oeax addressing mode */

/* registers    REG,  MOD,  SIB */

#define Xmm0    Tmm0, 0x00, EMPTY
#define Xmm1    Tmm1, 0x00, EMPTY
#define Xmm2    Tmm2, 0x00, EMPTY
#define Xmm3    Tmm3, 0x00, EMPTY
#define Xmm4    Tmm4, 0x00, EMPTY
#define Xmm5    Tmm5, 0x00, EMPTY
#define Xmm6    Tmm6, 0x00, EMPTY
#define Xmm7    Tmm7, 0x00, EMPTY
#define Xmm8    Tmm8, 0x00, EMPTY
#define Xmm9    Tmm9, 0x00, EMPTY
#define XmmA    TmmA, 0x00, EMPTY
#define XmmB    TmmB, 0x00, EMPTY
#define XmmC    TmmC, 0x00, EMPTY
#define XmmD    TmmD, 0x00, EMPTY
#define XmmE    TmmE, 0x00, EMPTY

/* only for 128-bit instructions (save/restore in 256-bit header)
 * provided as an extension to common baseline of 15 registers */

#define XmmF    TmmF, 0x00, EMPTY
#define XmmG    TmmG, 0x00, EMPTY
#define XmmH    TmmH, 0x00, EMPTY
#define XmmI    TmmI, 0x00, EMPTY
#define XmmJ    TmmJ, 0x00, EMPTY
#define XmmK    TmmK, 0x00, EMPTY
#define XmmL    TmmL, 0x00, EMPTY
#define XmmM    TmmW, 0x00, EMPTY
#define XmmN    TmmN, 0x00, EMPTY
#define XmmO    TmmO, 0x00, EMPTY
#define XmmP    TmmP, 0x00, EMPTY
#define XmmQ    TmmQ, 0x00, EMPTY
#define XmmR    TmmR, 0x00, EMPTY
#define XmmS    TmmS, 0x00, EMPTY
#define XmmT    TmmT, 0x00, EMPTY

/* triplet pass-through wrapper */

#define W(p1, p2, p3)       p1,  p2,  p3

/******************************************************************************/
/**********************************   BASE   **********************************/
/******************************************************************************/

/*--------------------------------   32-bit   --------------------------------*/

/* mov (D = S)
 * set-flags: no */

#define movwx_ri(RD, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RD),0x01,VAL(IS)))

#define movwx_mi(MD, DD, IS)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    REG(MD), VAL(DD), A1(DD), EMPTY2)   \
        EMIT6(MIM(0xC0, TMxx,   0x01,VAL(IS)))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MD),REG(MD), VAL(DD), B1(DD), P1(DD)))

#define movwx_rr(RD, RS)                                                    \
        EMITH(MRM(0x18, REG(RD),REG(RS),0x00))

#define movhn_rr(RD, RS)      /* move 16-bit to 32/64-bit w/ sign-extend */ \
        EMITW(MGM(0x07, REG(RD),REG(RS),0x00))

#define movhz_rr(RD, RS)      /* move 16-bit to 32/64-bit w/ zero-extend */ \
        EMITW(MGM(0x85, REG(RD),REG(RS),0x00))

#define movbn_rr(RD, RS)      /* move  8-bit to 32/64-bit w/ sign-extend */ \
        EMITW(MGM(0x06, REG(RD),REG(RS),0x00))

#define movbz_rr(RD, RS)      /* move  8-bit to 32/64-bit w/ zero-extend */ \
        EMITW(MGM(0x84, REG(RD),REG(RS),0x00))

#define movwx_ld(RD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x58, REG(RD),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define movhn_ld(RD, MS, DS)  /* load 16-bit to 32/64-bit w/ sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x15, REG(RD),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define movhz_ld(RD, MS, DS)  /* load 16-bit to 32/64-bit w/ zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x91, REG(RD),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define movbn_ld(RD, MS, DS)  /* load  8-bit to 32/64-bit w/ sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x77, REG(RD),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define movbz_ld(RD, MS, DS)  /* load  8-bit to 32/64-bit w/ zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x90, REG(RD),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define movwx_st(RS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    REG(MD), VAL(DD), A1(DD), EMPTY2)   \
        EMIT6(MDM(0x50, REG(RS),MOD(MD),REG(MD), VAL(DD), B1(DD), P1(DD)))


#define movwx_rj(RD, IT, IS)     /* IT - upper 32-bit, IS - lower 32-bit */ \
        movwx_ri(W(RD), W(IS))

#define movwx_mj(MD, DD, IT, IS) /* IT - upper 32-bit, IS - lower 32-bit */ \
        movwx_mi(W(MD), W(DD), W(IS))

/* add (G = G + S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define addwx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC2, REG(RG),0x09,VAL(IS)))

#define addwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC2, TMxx,   0x09,VAL(IS)))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define addwx_rr(RG, RS)                                                    \
        EMITH(MRM(0x1A, REG(RG),REG(RS),0x00))

#define addwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x5A, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define addhn_ld(RG, MS, DS)    /* add 16-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x7A, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define addhz_ld(RG, MS, DS)    /* add 16-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x91, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x1A, REG(RG),TMxx,   0x00))

#define addbn_ld(RG, MS, DS)    /* add  8-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x77, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x1A, REG(RG),TMxx,   0x00))

#define addbz_ld(RG, MS, DS)    /* add  8-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x90, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x1A, REG(RG),TMxx,   0x00))

/* sub (G = G - S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define subwx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC2, REG(RG),0x05,VAL(IS)))

#define subwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC2, TMxx,   0x05,VAL(IS)))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define subwx_rr(RG, RS)                                                    \
        EMITH(MRM(0x1B, REG(RG),REG(RS),0x00))

#define subwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x5B, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define subhn_ld(RG, MS, DS)    /* add 16-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x7B, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define subhz_ld(RG, MS, DS)    /* add 16-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x91, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x1B, REG(RG),TMxx,   0x00))

#define subbn_ld(RG, MS, DS)    /* add  8-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x77, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x1B, REG(RG),TMxx,   0x00))

#define subbz_ld(RG, MS, DS)    /* add  8-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x90, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x1B, REG(RG),TMxx,   0x00))

/*--------------------------------   64-bit   --------------------------------*/

/* mov (D = S)
 * set-flags: no */

#define movzx_ri(RD, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RD),0x01,VAL(IS)))

#define movzx_mi(MD, DD, IS)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    REG(MD), VAL(DD), A1(DD), EMPTY2)   \
        EMIT6(MIM(0xC0, TMxx,   0x01,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MD),REG(MD), VAL(DD), B1(DD), P1(DD)))

#define movzx_rr(RD, RS)                                                    \
        EMITW(MGM(0x04, REG(RD),REG(RS),0x00))

#define movwn_rr(RD, RS)       /* move 32-bit to 64-bit with sign-extend */ \
        EMITW(MGM(0x14, REG(RD),REG(RS),0x00))

#define movwz_rr(RD, RS)       /* move 32-bit to 64-bit with zero-extend */ \
        EMITW(MGM(0x16, REG(RD),REG(RS),0x00))

#define movzx_ld(RD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x04, REG(RD),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define movwn_ld(RD, MS, DS)   /* load 32-bit to 64-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x14, REG(RD),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define movwz_ld(RD, MS, DS)   /* load 32-bit to 64-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x16, REG(RD),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define movzx_st(RS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    REG(MD), VAL(DD), A1(DD), EMPTY2)   \
        EMIT6(MDM(0x24, REG(RS),MOD(MD),REG(MD), VAL(DD), B1(DD), P1(DD)))


#define movzx_rj(RD, IT, IS)     /* IT - upper 32-bit, IS - lower 32-bit */ \
        EMIT6(MIM(0xC0, REG(RD),0x01,VAL(IS)))                              \
        EMIT6(MIM(0xC0, REG(RD),0x08,VAL(IT)))

#define movzx_mj(MD, DD, IT, IS) /* IT - upper 32-bit, IS - lower 32-bit */ \
        AUW(SIB(MD),  EMPTY,  EMPTY,    REG(MD), VAL(DD), A1(DD), EMPTY2)   \
        EMIT6(MIM(0xC0, TMxx,   0x01,VAL(IS)))                              \
        EMIT6(MIM(0xC0, TMxx,   0x08,VAL(IT)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MD),REG(MD), VAL(DD), B1(DD), P1(DD)))

/* add (G = G + S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define addzx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC2, REG(RG),0x08,VAL(IS)))

#define addzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC2, TMxx,   0x08,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define addzx_rr(RG, RS)                                                    \
        EMITW(MGM(0x08, REG(RG),REG(RS),0x00))

#define addzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x08, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define addwn_ld(RG, MS, DS)    /* add 32-bit to 64-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x18, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define addwz_ld(RG, MS, DS)    /* add 32-bit to 64-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x1A, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

/* sub (G = G - S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define subzx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC2, REG(RG),0x04,VAL(IS)))

#define subzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC2, TMxx,   0x04,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define subzx_rr(RG, RS)                                                    \
        EMITW(MGM(0x09, REG(RG),REG(RS),0x00))

#define subzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x09, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define subwn_ld(RG, MS, DS)  /* sub 32-bit from 64-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x19, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define subwz_ld(RG, MS, DS)  /* sub 32-bit from 64-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x1B, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

/************************* register-size instructions *************************/

/* stack (push stack = S, D = pop stack)
 * set-flags: no (sequence cmp/stack_la/jmp is not allowed on MIPS & POWER)
 * adjust stack pointer with 8-byte (64-bit) steps on all current targets */

#define stack_st(RS)                                                        \
        EMIT6(MIM(0xC2, SPxx,0x04,0x08)) /* SP -= 8 */                      \
        EMIT6(0xE30000000024 | MSM(REG(RS),SPxx,0x00))

#define stack_ld(RD)                                                        \
        EMIT6(0xE30000000004 | MSM(REG(RD),SPxx,0x00))                      \
        EMIT6(MIM(0xC2, SPxx,0x08,0x08)) /* SP += 8 */

#define stack_sa()  /* save all, [Reax - RegС] + 2 temps, 14 regs total */  \
        EMIT6(MIM(0xC2, SPxx,0x04,0x70)) /* SP -= 112 */                    \
        EMIT6(0xE30000000024 | MSM(TEax,SPxx,0x00) | (0x00 & 0xFFF) << 16)  \
        EMIT6(0xE30000000024 | MSM(TEcx,SPxx,0x00) | (0x08 & 0xFFF) << 16)  \
        EMIT6(0xE30000000024 | MSM(TEdx,SPxx,0x00) | (0x10 & 0xFFF) << 16)  \
        EMIT6(0xE30000000024 | MSM(TEbx,SPxx,0x00) | (0x18 & 0xFFF) << 16)  \
        EMIT6(0xE30000000024 | MSM(TEbp,SPxx,0x00) | (0x20 & 0xFFF) << 16)  \
        EMIT6(0xE30000000024 | MSM(TEsi,SPxx,0x00) | (0x28 & 0xFFF) << 16)  \
        EMIT6(0xE30000000024 | MSM(TEdi,SPxx,0x00) | (0x30 & 0xFFF) << 16)  \
        EMIT6(0xE30000000024 | MSM(TEg8,SPxx,0x00) | (0x38 & 0xFFF) << 16)  \
        EMIT6(0xE30000000024 | MSM(TEg9,SPxx,0x00) | (0x40 & 0xFFF) << 16)  \
        EMIT6(0xE30000000024 | MSM(TEgA,SPxx,0x00) | (0x48 & 0xFFF) << 16)  \
        EMIT6(0xE30000000024 | MSM(TEgB,SPxx,0x00) | (0x50 & 0xFFF) << 16)  \
        EMIT6(0xE30000000024 | MSM(TEgC,SPxx,0x00) | (0x58 & 0xFFF) << 16)  \
        EMIT6(0xE30000000024 | MSM(TDxx,SPxx,0x00) | (0x60 & 0xFFF) << 16)  \
        EMIT6(0xE30000000024 | MSM(TPxx,SPxx,0x00) | (0x68 & 0xFFF) << 16)

#define stack_la()  /* load all, 2 temps + [RegC - Reax], 14 regs total */  \
        EMIT6(0xE30000000004 | MSM(TPxx,SPxx,0x00) | (0x68 & 0xFFF) << 16)  \
        EMIT6(0xE30000000004 | MSM(TDxx,SPxx,0x00) | (0x60 & 0xFFF) << 16)  \
        EMIT6(0xE30000000004 | MSM(TEgC,SPxx,0x00) | (0x58 & 0xFFF) << 16)  \
        EMIT6(0xE30000000004 | MSM(TEgB,SPxx,0x00) | (0x50 & 0xFFF) << 16)  \
        EMIT6(0xE30000000004 | MSM(TEgA,SPxx,0x00) | (0x48 & 0xFFF) << 16)  \
        EMIT6(0xE30000000004 | MSM(TEg9,SPxx,0x00) | (0x40 & 0xFFF) << 16)  \
        EMIT6(0xE30000000004 | MSM(TEg8,SPxx,0x00) | (0x38 & 0xFFF) << 16)  \
        EMIT6(0xE30000000004 | MSM(TEdi,SPxx,0x00) | (0x30 & 0xFFF) << 16)  \
        EMIT6(0xE30000000004 | MSM(TEsi,SPxx,0x00) | (0x28 & 0xFFF) << 16)  \
        EMIT6(0xE30000000004 | MSM(TEbp,SPxx,0x00) | (0x20 & 0xFFF) << 16)  \
        EMIT6(0xE30000000004 | MSM(TEbx,SPxx,0x00) | (0x18 & 0xFFF) << 16)  \
        EMIT6(0xE30000000004 | MSM(TEdx,SPxx,0x00) | (0x10 & 0xFFF) << 16)  \
        EMIT6(0xE30000000004 | MSM(TEcx,SPxx,0x00) | (0x08 & 0xFFF) << 16)  \
        EMIT6(0xE30000000004 | MSM(TEax,SPxx,0x00) | (0x00 & 0xFFF) << 16)  \
        EMIT6(MIM(0xC2, SPxx,0x08,0x70)) /* SP += 112 */

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/*--------------------------------   32-bit   -------------------- 128-bit ---*/

/* mov (D = S) */

#define movix_rr(XD, XS)                                                    \
        EMIT6(MXM(0x56, REG(XD),REG(XS),   0x00, 0x00))

#define movix_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, REG(XD),MOD(MS),REG(MS), VAL(DS), B2(DS), P2(DS)))

#define movix_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    REG(MD), VAL(DD), A2(DD), EMPTY2)   \
        EMIT6(MPM(0x0E, REG(XS),MOD(MD),REG(MD), VAL(DD), B2(DD), P2(DD)))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andix_rr(XG, XS)                                                    \
        andix3rr(W(XG), W(XG), W(XS))

#define andix_ld(XG, MS, DS)                                                \
        andix3ld(W(XG), W(XG), W(MS), W(DS))

#define andix3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x68, REG(XD),REG(XS),REG(XT), 0x00))

#define andix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0x68, REG(XD),REG(XS),   TmmM, 0x00))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annix_rr(XG, XS)                                                    \
        annix3rr(W(XG), W(XG), W(XS))

#define annix_ld(XG, MS, DS)                                                \
        annix3ld(W(XG), W(XG), W(MS), W(DS))

#define annix3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x69, REG(XD),REG(XT),REG(XS), 0x00))

#define annix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0x69, REG(XD),TmmM,   REG(XS), 0x00))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrix_rr(XG, XS)                                                    \
        orrix3rr(W(XG), W(XG), W(XS))

#define orrix_ld(XG, MS, DS)                                                \
        orrix3ld(W(XG), W(XG), W(MS), W(DS))

#define orrix3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x6A, REG(XD),REG(XS),REG(XT), 0x00))

#define orrix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0x6A, REG(XD),REG(XS),   TmmM, 0x00))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define ornix_rr(XG, XS)                                                    \
        ornix3rr(W(XG), W(XG), W(XS))

#define ornix_ld(XG, MS, DS)                                                \
        ornix3ld(W(XG), W(XG), W(MS), W(DS))

#define ornix3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x6F, REG(XD),REG(XT),REG(XS), 0x00))

#define ornix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0x6F, REG(XD),TmmM,   REG(XS), 0x00))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorix_rr(XG, XS)                                                    \
        xorix3rr(W(XG), W(XG), W(XS))

#define xorix_ld(XG, MS, DS)                                                \
        xorix3ld(W(XG), W(XG), W(MS), W(DS))

#define xorix3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x6D, REG(XD),REG(XS),REG(XT), 0x00))

#define xorix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0x6D, REG(XD),REG(XS),   TmmM, 0x00))

/* not (G = ~G), (D = ~S) */

#define notix_rx(XG)                                                        \
        notix_rr(W(XG), W(XG))

#define notix_rr(XD, XS)                                                    \
        EMIT6(MXM(0x6B, REG(XD),REG(XS),REG(XS), 0x00))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addis_rr(XG, XS)                                                    \
        addis3rr(W(XG), W(XG), W(XS))

#define addis_ld(XG, MS, DS)                                                \
        addis3ld(W(XG), W(XG), W(MS), W(DS))

#define addis3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE3, REG(XD),REG(XS),REG(XT), 0x02))

#define addis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xE3, REG(XD),REG(XS),   TmmM, 0x02))

        /* adp, adh are defined in rtbase.h (first 15-regs only)
         * under "COMMON SIMD INSTRUCTIONS" section */

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subis_rr(XG, XS)                                                    \
        subis3rr(W(XG), W(XG), W(XS))

#define subis_ld(XG, MS, DS)                                                \
        subis3ld(W(XG), W(XG), W(MS), W(DS))

#define subis3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE2, REG(XD),REG(XS),REG(XT), 0x02))

#define subis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xE2, REG(XD),REG(XS),   TmmM, 0x02))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulis_rr(XG, XS)                                                    \
        mulis3rr(W(XG), W(XG), W(XS))

#define mulis_ld(XG, MS, DS)                                                \
        mulis3ld(W(XG), W(XG), W(MS), W(DS))

#define mulis3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE7, REG(XD),REG(XS),REG(XT), 0x02))

#define mulis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xE7, REG(XD),REG(XS),   TmmM, 0x02))

        /* mlp, mlh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divis_rr(XG, XS)                                                    \
        divis3rr(W(XG), W(XG), W(XS))

#define divis_ld(XG, MS, DS)                                                \
        divis3ld(W(XG), W(XG), W(MS), W(DS))

#define divis3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE5, REG(XD),REG(XS),REG(XT), 0x02))

#define divis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xE5, REG(XD),REG(XS),   TmmM, 0x02))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqis_rr(XG, XS)                                                    \
        ceqis3rr(W(XG), W(XG), W(XS))

#define ceqis_ld(XG, MS, DS)                                                \
        ceqis3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqis3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE8, REG(XD),REG(XS),REG(XT), 0x02))

#define ceqis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xE8, REG(XD),REG(XS),   TmmM, 0x02))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneis_rr(XG, XS)                                                    \
        cneis3rr(W(XG), W(XG), W(XS))

#define cneis_ld(XG, MS, DS)                                                \
        cneis3ld(W(XG), W(XG), W(MS), W(DS))

#define cneis3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE8, REG(XD),REG(XS),REG(XT), 0x02))                     \
        notix_rx(W(XD))

#define cneis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xE8, REG(XD),REG(XS),   TmmM, 0x02))                     \
        notix_rx(W(XD))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltis_rr(XG, XS)                                                    \
        cltis3rr(W(XG), W(XG), W(XS))

#define cltis_ld(XG, MS, DS)                                                \
        cltis3ld(W(XG), W(XG), W(MS), W(DS))

#define cltis3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEB, REG(XD),REG(XT),REG(XS), 0x02))

#define cltis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xEB, REG(XD),TmmM,   REG(XS), 0x02))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define cleis_rr(XG, XS)                                                    \
        cleis3rr(W(XG), W(XG), W(XS))

#define cleis_ld(XG, MS, DS)                                                \
        cleis3ld(W(XG), W(XG), W(MS), W(DS))

#define cleis3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEA, REG(XD),REG(XT),REG(XS), 0x02))

#define cleis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xEA, REG(XD),TmmM,   REG(XS), 0x02))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtis_rr(XG, XS)                                                    \
        cgtis3rr(W(XG), W(XG), W(XS))

#define cgtis_ld(XG, MS, DS)                                                \
        cgtis3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtis3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEB, REG(XD),REG(XS),REG(XT), 0x02))

#define cgtis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xEB, REG(XD),REG(XS),   TmmM, 0x02))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgeis_rr(XG, XS)                                                    \
        cgeis3rr(W(XG), W(XG), W(XS))

#define cgeis_ld(XG, MS, DS)                                                \
        cgeis3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeis3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEA, REG(XD),REG(XS),REG(XT), 0x02))

#define cgeis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xEA, REG(XD),REG(XS),   TmmM, 0x02))

/*************   packed single-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzis_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x05, 0x02))

#define rnzis_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x05, 0x02))

#define cvzis_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x05, 0x02))

#define cvzis_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x05, 0x02))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpis_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x06, 0x02))

#define rnpis_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x06, 0x02))

#define cvpis_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x06, 0x02))

#define cvpis_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x06, 0x02))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmis_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x07, 0x02))

#define rnmis_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x07, 0x02))

#define cvmis_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x07, 0x02))

#define cvmis_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x07, 0x02))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnis_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x04, 0x02))

#define rnnis_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x04, 0x02))

#define cvnis_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x04, 0x02))

#define cvnis_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x04, 0x02))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndis_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x00, 0x02))

#define rndis_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x00, 0x02))

#define cvtis_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x00, 0x02))

#define cvtis_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x00, 0x02))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnris_rr(XD, XS, mode)                                              \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),(RT_SIMD_MODE_##mode&3), 0x02))

#define cvris_rr(XD, XS, mode)                                              \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,   (RT_SIMD_MODE_##mode&3), 0x02))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnin_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC3, REG(XD),REG(XS),   0x04, 0x02))

#define cvnin_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC3, REG(XD),TmmM,      0x04, 0x02))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtin_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC3, REG(XD),REG(XS),   0x00, 0x02))

#define cvtin_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC3, REG(XD),TmmM,      0x00, 0x02))

/* cvn (D = unsigned-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnix_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC1, REG(XD),REG(XS),   0x04, 0x02))

#define cvnix_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC1, REG(XD),TmmM,      0x04, 0x02))

/* cvt (D = unsigned-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtix_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC1, REG(XD),REG(XS),   0x00, 0x02))

#define cvtix_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC1, REG(XD),TmmM,      0x00, 0x02))

/* cuz (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define ruzis_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x05, 0x02))

#define ruzis_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x05, 0x02))

#define cuzis_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x05, 0x02))

#define cuzis_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x05, 0x02))

/* cup (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rupis_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x06, 0x02))

#define rupis_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x06, 0x02))

#define cupis_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x06, 0x02))

#define cupis_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x06, 0x02))

/* cum (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rumis_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x07, 0x02))

#define rumis_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x07, 0x02))

#define cumis_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x07, 0x02))

#define cumis_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x07, 0x02))

/* cun (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define runis_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x04, 0x02))

#define runis_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x04, 0x02))

#define cunis_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x04, 0x02))

#define cunis_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x04, 0x02))

/* cut (D = fp-to-unsigned-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cuz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rudis_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x00, 0x02))

#define rudis_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x00, 0x02))

#define cutis_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x00, 0x02))

#define cutis_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x00, 0x02))

/* cur (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define ruris_rr(XD, XS, mode)                                              \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),(RT_SIMD_MODE_##mode&3), 0x02))

#define curis_rr(XD, XS, mode)                                              \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,   (RT_SIMD_MODE_##mode&3), 0x02))

/*--------------------------------   64-bit   -------------------- 128-bit ---*/

/* mov (D = S) */

#define movjx_rr(XD, XS)                                                    \
        EMIT6(MXM(0x56, REG(XD),REG(XS),   0x00, 0x00))

#define movjx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, REG(XD),MOD(MS),REG(MS), VAL(DS), B2(DS), P2(DS)))

#define movjx_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    REG(MD), VAL(DD), A2(DD), EMPTY2)   \
        EMIT6(MPM(0x0E, REG(XS),MOD(MD),REG(MD), VAL(DD), B2(DD), P2(DD)))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andjx_rr(XG, XS)                                                    \
        andjx3rr(W(XG), W(XG), W(XS))

#define andjx_ld(XG, MS, DS)                                                \
        andjx3ld(W(XG), W(XG), W(MS), W(DS))

#define andjx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x68, REG(XD),REG(XS),REG(XT), 0x00))

#define andjx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0x68, REG(XD),REG(XS),   TmmM, 0x00))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annjx_rr(XG, XS)                                                    \
        annjx3rr(W(XG), W(XG), W(XS))

#define annjx_ld(XG, MS, DS)                                                \
        annjx3ld(W(XG), W(XG), W(MS), W(DS))

#define annjx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x69, REG(XD),REG(XT),REG(XS), 0x00))

#define annjx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0x69, REG(XD),TmmM,   REG(XS), 0x00))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrjx_rr(XG, XS)                                                    \
        orrjx3rr(W(XG), W(XG), W(XS))

#define orrjx_ld(XG, MS, DS)                                                \
        orrjx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrjx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x6A, REG(XD),REG(XS),REG(XT), 0x00))

#define orrjx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0x6A, REG(XD),REG(XS),   TmmM, 0x00))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define ornjx_rr(XG, XS)                                                    \
        ornjx3rr(W(XG), W(XG), W(XS))

#define ornjx_ld(XG, MS, DS)                                                \
        ornjx3ld(W(XG), W(XG), W(MS), W(DS))

#define ornjx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x6F, REG(XD),REG(XT),REG(XS), 0x00))

#define ornjx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0x6F, REG(XD),TmmM,   REG(XS), 0x00))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorjx_rr(XG, XS)                                                    \
        xorjx3rr(W(XG), W(XG), W(XS))

#define xorjx_ld(XG, MS, DS)                                                \
        xorjx3ld(W(XG), W(XG), W(MS), W(DS))

#define xorjx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x6D, REG(XD),REG(XS),REG(XT), 0x00))

#define xorjx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0x6D, REG(XD),REG(XS),   TmmM, 0x00))

/* not (G = ~G), (D = ~S) */

#define notjx_rx(XG)                                                        \
        notjx_rr(W(XG), W(XG))

#define notjx_rr(XD, XS)                                                    \
        EMIT6(MXM(0x6B, REG(XD),REG(XS),REG(XS), 0x00))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addjs_rr(XG, XS)                                                    \
        addjs3rr(W(XG), W(XG), W(XS))

#define addjs_ld(XG, MS, DS)                                                \
        addjs3ld(W(XG), W(XG), W(MS), W(DS))

#define addjs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE3, REG(XD),REG(XS),REG(XT), 0x03))

#define addjs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xE3, REG(XD),REG(XS),   TmmM, 0x03))

        /* adp, adh are defined in rtbase.h (first 15-regs only)
         * under "COMMON SIMD INSTRUCTIONS" section */

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subjs_rr(XG, XS)                                                    \
        subjs3rr(W(XG), W(XG), W(XS))

#define subjs_ld(XG, MS, DS)                                                \
        subjs3ld(W(XG), W(XG), W(MS), W(DS))

#define subjs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE2, REG(XD),REG(XS),REG(XT), 0x03))

#define subjs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xE2, REG(XD),REG(XS),   TmmM, 0x03))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define muljs_rr(XG, XS)                                                    \
        muljs3rr(W(XG), W(XG), W(XS))

#define muljs_ld(XG, MS, DS)                                                \
        muljs3ld(W(XG), W(XG), W(MS), W(DS))

#define muljs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE7, REG(XD),REG(XS),REG(XT), 0x03))

#define muljs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xE7, REG(XD),REG(XS),   TmmM, 0x03))

        /* mlp, mlh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divjs_rr(XG, XS)                                                    \
        divjs3rr(W(XG), W(XG), W(XS))

#define divjs_ld(XG, MS, DS)                                                \
        divjs3ld(W(XG), W(XG), W(MS), W(DS))

#define divjs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE5, REG(XD),REG(XS),REG(XT), 0x03))

#define divjs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xE5, REG(XD),REG(XS),   TmmM, 0x03))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqjs_rr(XG, XS)                                                    \
        ceqjs3rr(W(XG), W(XG), W(XS))

#define ceqjs_ld(XG, MS, DS)                                                \
        ceqjs3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqjs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE8, REG(XD),REG(XS),REG(XT), 0x03))

#define ceqjs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xE8, REG(XD),REG(XS),   TmmM, 0x03))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnejs_rr(XG, XS)                                                    \
        cnejs3rr(W(XG), W(XG), W(XS))

#define cnejs_ld(XG, MS, DS)                                                \
        cnejs3ld(W(XG), W(XG), W(MS), W(DS))

#define cnejs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE8, REG(XD),REG(XS),REG(XT), 0x03))                     \
        notjx_rx(W(XD))

#define cnejs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xE8, REG(XD),REG(XS),   TmmM, 0x03))                     \
        notjx_rx(W(XD))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltjs_rr(XG, XS)                                                    \
        cltjs3rr(W(XG), W(XG), W(XS))

#define cltjs_ld(XG, MS, DS)                                                \
        cltjs3ld(W(XG), W(XG), W(MS), W(DS))

#define cltjs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEB, REG(XD),REG(XT),REG(XS), 0x03))

#define cltjs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xEB, REG(XD),TmmM,   REG(XS), 0x03))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define clejs_rr(XG, XS)                                                    \
        clejs3rr(W(XG), W(XG), W(XS))

#define clejs_ld(XG, MS, DS)                                                \
        clejs3ld(W(XG), W(XG), W(MS), W(DS))

#define clejs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEA, REG(XD),REG(XT),REG(XS), 0x03))

#define clejs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xEA, REG(XD),TmmM,   REG(XS), 0x03))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtjs_rr(XG, XS)                                                    \
        cgtjs3rr(W(XG), W(XG), W(XS))

#define cgtjs_ld(XG, MS, DS)                                                \
        cgtjs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtjs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEB, REG(XD),REG(XS),REG(XT), 0x03))

#define cgtjs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xEB, REG(XD),REG(XS),   TmmM, 0x03))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgejs_rr(XG, XS)                                                    \
        cgejs3rr(W(XG), W(XG), W(XS))

#define cgejs_ld(XG, MS, DS)                                                \
        cgejs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgejs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEA, REG(XD),REG(XS),REG(XT), 0x03))

#define cgejs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xEA, REG(XD),REG(XS),   TmmM, 0x03))

/*************   packed double-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzjs_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x05, 0x03))

#define rnzjs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x05, 0x03))

#define cvzjs_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x05, 0x03))

#define cvzjs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x05, 0x03))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpjs_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x06, 0x03))

#define rnpjs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x06, 0x03))

#define cvpjs_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x06, 0x03))

#define cvpjs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x06, 0x03))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmjs_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x07, 0x03))

#define rnmjs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x07, 0x03))

#define cvmjs_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x07, 0x03))

#define cvmjs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x07, 0x03))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnjs_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x04, 0x03))

#define rnnjs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x04, 0x03))

#define cvnjs_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x04, 0x03))

#define cvnjs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x04, 0x03))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndjs_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x00, 0x03))

#define rndjs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x00, 0x03))

#define cvtjs_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x00, 0x03))

#define cvtjs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x00, 0x03))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnrjs_rr(XD, XS, mode)                                              \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),(RT_SIMD_MODE_##mode&3), 0x03))

#define cvrjs_rr(XD, XS, mode)                                              \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,   (RT_SIMD_MODE_##mode&3), 0x03))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnjn_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC3, REG(XD),REG(XS),   0x04, 0x03))

#define cvnjn_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC3, REG(XD),TmmM,      0x04, 0x03))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtjn_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC3, REG(XD),REG(XS),   0x00, 0x03))

#define cvtjn_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC3, REG(XD),TmmM,      0x00, 0x03))

/* cvn (D = unsigned-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnjx_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC1, REG(XD),REG(XS),   0x04, 0x03))

#define cvnjx_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC1, REG(XD),TmmM,      0x04, 0x03))

/* cvt (D = unsigned-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtjx_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC1, REG(XD),REG(XS),   0x00, 0x03))

#define cvtjx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC1, REG(XD),TmmM,      0x00, 0x03))

/* cuz (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define ruzjs_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x05, 0x03))

#define ruzjs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x05, 0x03))

#define cuzjs_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x05, 0x03))

#define cuzjs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x05, 0x03))

/* cup (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rupjs_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x06, 0x03))

#define rupjs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x06, 0x03))

#define cupjs_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x06, 0x03))

#define cupjs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x06, 0x03))

/* cum (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rumjs_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x07, 0x03))

#define rumjs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x07, 0x03))

#define cumjs_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x07, 0x03))

#define cumjs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x07, 0x03))

/* cun (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define runjs_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x04, 0x03))

#define runjs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x04, 0x03))

#define cunjs_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x04, 0x03))

#define cunjs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x04, 0x03))

/* cut (D = fp-to-unsigned-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cuz
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rudjs_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x00, 0x03))

#define rudjs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x00, 0x03))

#define cutjs_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x00, 0x03))

#define cutjs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x00, 0x03))

/* cur (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rurjs_rr(XD, XS, mode)                                              \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),(RT_SIMD_MODE_##mode&3), 0x03))

#define curjs_rr(XD, XS, mode)                                              \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,   (RT_SIMD_MODE_##mode&3), 0x03))

/*--------------------------------   32-bit   -------------------- 256-bit ---*/

/* mov (D = S) */

#define movcx_rr(XD, XS)                                                    \
        EMIT6(MXM(0x56, REG(XD),REG(XS),   0x00, 0x00))                     \
        EMIT6(MXM(0x56, RYG(XD),RYG(XS),   0x00, 0x00))

#define movcx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, REG(XD),MOD(MS),REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MPM(0x06, RYG(XD),MOD(MS),REG(MS), VYL(DS), B2(DS), L2(DS)))

#define movcx_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    REG(MD), VAL(DD), A2(DD), EMPTY2)   \
        EMIT6(MPM(0x0E, REG(XS),MOD(MD),REG(MD), VAL(DD), B2(DD), L2(DD)))  \
        EMIT6(MPM(0x0E, RYG(XS),MOD(MD),REG(MD), VYL(DD), B2(DD), L2(DD)))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andcx_rr(XG, XS)                                                    \
        andcx3rr(W(XG), W(XG), W(XS))

#define andcx_ld(XG, MS, DS)                                                \
        andcx3ld(W(XG), W(XG), W(MS), W(DS))

#define andcx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x68, REG(XD),REG(XS),REG(XT), 0x00))                     \
        EMIT6(MXM(0x68, RYG(XD),RYG(XS),RYG(XT), 0x00))

#define andcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x68, REG(XD),REG(XS),   TmmM, 0x00))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x68, RYG(XD),RYG(XS),   TmmM, 0x00))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define anncx_rr(XG, XS)                                                    \
        anncx3rr(W(XG), W(XG), W(XS))

#define anncx_ld(XG, MS, DS)                                                \
        anncx3ld(W(XG), W(XG), W(MS), W(DS))

#define anncx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x69, REG(XD),REG(XT),REG(XS), 0x00))                     \
        EMIT6(MXM(0x69, RYG(XD),RYG(XT),RYG(XS), 0x00))

#define anncx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x69, REG(XD),TmmM,   REG(XS), 0x00))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x69, RYG(XD),TmmM,   RYG(XS), 0x00))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrcx_rr(XG, XS)                                                    \
        orrcx3rr(W(XG), W(XG), W(XS))

#define orrcx_ld(XG, MS, DS)                                                \
        orrcx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrcx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x6A, REG(XD),REG(XS),REG(XT), 0x00))                     \
        EMIT6(MXM(0x6A, RYG(XD),RYG(XS),RYG(XT), 0x00))

#define orrcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6A, REG(XD),REG(XS),   TmmM, 0x00))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6A, RYG(XD),RYG(XS),   TmmM, 0x00))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define orncx_rr(XG, XS)                                                    \
        orncx3rr(W(XG), W(XG), W(XS))

#define orncx_ld(XG, MS, DS)                                                \
        orncx3ld(W(XG), W(XG), W(MS), W(DS))

#define orncx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x6F, REG(XD),REG(XT),REG(XS), 0x00))                     \
        EMIT6(MXM(0x6F, RYG(XD),RYG(XT),RYG(XS), 0x00))

#define orncx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6F, REG(XD),TmmM,   REG(XS), 0x00))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6F, RYG(XD),TmmM,   RYG(XS), 0x00))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorcx_rr(XG, XS)                                                    \
        xorcx3rr(W(XG), W(XG), W(XS))

#define xorcx_ld(XG, MS, DS)                                                \
        xorcx3ld(W(XG), W(XG), W(MS), W(DS))

#define xorcx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x6D, REG(XD),REG(XS),REG(XT), 0x00))                     \
        EMIT6(MXM(0x6D, RYG(XD),RYG(XS),RYG(XT), 0x00))

#define xorcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6D, REG(XD),REG(XS),   TmmM, 0x00))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6D, RYG(XD),RYG(XS),   TmmM, 0x00))

/* not (G = ~G), (D = ~S) */

#define notcx_rx(XG)                                                        \
        notcx_rr(W(XG), W(XG))

#define notcx_rr(XD, XS)                                                    \
        EMIT6(MXM(0x6B, REG(XD),REG(XS),REG(XS), 0x00))                     \
        EMIT6(MXM(0x6B, RYG(XD),RYG(XS),RYG(XS), 0x00))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addcs_rr(XG, XS)                                                    \
        addcs3rr(W(XG), W(XG), W(XS))

#define addcs_ld(XG, MS, DS)                                                \
        addcs3ld(W(XG), W(XG), W(MS), W(DS))

#define addcs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE3, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xE3, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define addcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE3, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE3, RYG(XD),RYG(XS),   TmmM, 0x02))

        /* adp, adh are defined in rtbase.h (first 15-regs only)
         * under "COMMON SIMD INSTRUCTIONS" section */

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subcs_rr(XG, XS)                                                    \
        subcs3rr(W(XG), W(XG), W(XS))

#define subcs_ld(XG, MS, DS)                                                \
        subcs3ld(W(XG), W(XG), W(MS), W(DS))

#define subcs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE2, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xE2, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define subcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE2, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE2, RYG(XD),RYG(XS),   TmmM, 0x02))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulcs_rr(XG, XS)                                                    \
        mulcs3rr(W(XG), W(XG), W(XS))

#define mulcs_ld(XG, MS, DS)                                                \
        mulcs3ld(W(XG), W(XG), W(MS), W(DS))

#define mulcs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE7, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xE7, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define mulcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE7, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE7, RYG(XD),RYG(XS),   TmmM, 0x02))

        /* mlp, mlh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divcs_rr(XG, XS)                                                    \
        divcs3rr(W(XG), W(XG), W(XS))

#define divcs_ld(XG, MS, DS)                                                \
        divcs3ld(W(XG), W(XG), W(MS), W(DS))

#define divcs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE5, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xE5, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define divcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE5, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE5, RYG(XD),RYG(XS),   TmmM, 0x02))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqcs_rr(XG, XS)                                                    \
        ceqcs3rr(W(XG), W(XG), W(XS))

#define ceqcs_ld(XG, MS, DS)                                                \
        ceqcs3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqcs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE8, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xE8, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define ceqcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE8, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE8, RYG(XD),RYG(XS),   TmmM, 0x02))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnecs_rr(XG, XS)                                                    \
        cnecs3rr(W(XG), W(XG), W(XS))

#define cnecs_ld(XG, MS, DS)                                                \
        cnecs3ld(W(XG), W(XG), W(MS), W(DS))

#define cnecs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE8, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xE8, RYG(XD),RYG(XS),RYG(XT), 0x02))                     \
        notcx_rx(W(XD))

#define cnecs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE8, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE8, RYG(XD),RYG(XS),   TmmM, 0x02))                     \
        notcx_rx(W(XD))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltcs_rr(XG, XS)                                                    \
        cltcs3rr(W(XG), W(XG), W(XS))

#define cltcs_ld(XG, MS, DS)                                                \
        cltcs3ld(W(XG), W(XG), W(MS), W(DS))

#define cltcs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEB, REG(XD),REG(XT),REG(XS), 0x02))                     \
        EMIT6(MXM(0xEB, RYG(XD),RYG(XT),RYG(XS), 0x02))

#define cltcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEB, REG(XD),TmmM,   REG(XS), 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEB, RYG(XD),TmmM,   RYG(XS), 0x02))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define clecs_rr(XG, XS)                                                    \
        clecs3rr(W(XG), W(XG), W(XS))

#define clecs_ld(XG, MS, DS)                                                \
        clecs3ld(W(XG), W(XG), W(MS), W(DS))

#define clecs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEA, REG(XD),REG(XT),REG(XS), 0x02))                     \
        EMIT6(MXM(0xEA, RYG(XD),RYG(XT),RYG(XS), 0x02))

#define clecs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEA, REG(XD),TmmM,   REG(XS), 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEA, RYG(XD),TmmM,   RYG(XS), 0x02))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtcs_rr(XG, XS)                                                    \
        cgtcs3rr(W(XG), W(XG), W(XS))

#define cgtcs_ld(XG, MS, DS)                                                \
        cgtcs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtcs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEB, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xEB, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define cgtcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEB, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEB, RYG(XD),RYG(XS),   TmmM, 0x02))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgecs_rr(XG, XS)                                                    \
        cgecs3rr(W(XG), W(XG), W(XS))

#define cgecs_ld(XG, MS, DS)                                                \
        cgecs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgecs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEA, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xEA, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define cgecs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEA, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEA, RYG(XD),RYG(XS),   TmmM, 0x02))

/*************   packed single-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzcs_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x05, 0x02))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x05, 0x02))

#define rnzcs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x05, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x05, 0x02))

#define cvzcs_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x05, 0x02))                     \
        EMIT6(MZM(0xC2, RYG(XD),RYG(XS),   0x05, 0x02))

#define cvzcs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x05, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, RYG(XD),TmmM,      0x05, 0x02))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpcs_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x06, 0x02))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x06, 0x02))

#define rnpcs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x06, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x06, 0x02))

#define cvpcs_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x06, 0x02))                     \
        EMIT6(MZM(0xC2, RYG(XD),RYG(XS),   0x06, 0x02))

#define cvpcs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x06, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, RYG(XD),TmmM,      0x06, 0x02))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmcs_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x07, 0x02))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x07, 0x02))

#define rnmcs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x07, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x07, 0x02))

#define cvmcs_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x07, 0x02))                     \
        EMIT6(MZM(0xC2, RYG(XD),RYG(XS),   0x07, 0x02))

#define cvmcs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x07, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, RYG(XD),TmmM,      0x07, 0x02))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnncs_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x04, 0x02))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x04, 0x02))

#define rnncs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x04, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x04, 0x02))

#define cvncs_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x04, 0x02))                     \
        EMIT6(MZM(0xC2, RYG(XD),RYG(XS),   0x04, 0x02))

#define cvncs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x04, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, RYG(XD),TmmM,      0x04, 0x02))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndcs_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x00, 0x02))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x00, 0x02))

#define rndcs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x00, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x00, 0x02))

#define cvtcs_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x00, 0x02))                     \
        EMIT6(MZM(0xC2, RYG(XD),RYG(XS),   0x00, 0x02))

#define cvtcs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x00, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, RYG(XD),TmmM,      0x00, 0x02))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnrcs_rr(XD, XS, mode)                                              \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),(RT_SIMD_MODE_##mode&3), 0x02))     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),(RT_SIMD_MODE_##mode&3), 0x02))

#define cvrcs_rr(XD, XS, mode)                                              \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,   (RT_SIMD_MODE_##mode&3), 0x02))     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, RYG(XD),TmmM,   (RT_SIMD_MODE_##mode&3), 0x02))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvncn_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC3, REG(XD),REG(XS),   0x04, 0x02))                     \
        EMIT6(MZM(0xC3, RYG(XD),RYG(XS),   0x04, 0x02))

#define cvncn_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC3, REG(XD),TmmM,      0x04, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC3, RYG(XD),TmmM,      0x04, 0x02))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtcn_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC3, REG(XD),REG(XS),   0x00, 0x02))                     \
        EMIT6(MZM(0xC3, RYG(XD),RYG(XS),   0x00, 0x02))

#define cvtcn_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC3, REG(XD),TmmM,      0x00, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC3, RYG(XD),TmmM,      0x00, 0x02))

/* cvn (D = unsigned-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvncx_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC1, REG(XD),REG(XS),   0x04, 0x02))                     \
        EMIT6(MZM(0xC1, RYG(XD),RYG(XS),   0x04, 0x02))

#define cvncx_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC1, REG(XD),TmmM,      0x04, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC1, RYG(XD),TmmM,      0x04, 0x02))

/* cvt (D = unsigned-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtcx_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC1, REG(XD),REG(XS),   0x00, 0x02))                     \
        EMIT6(MZM(0xC1, RYG(XD),RYG(XS),   0x00, 0x02))

#define cvtcx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC1, REG(XD),TmmM,      0x00, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC1, RYG(XD),TmmM,      0x00, 0x02))

/* cuz (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define ruzcs_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x05, 0x02))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x05, 0x02))

#define ruzcs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x05, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x05, 0x02))

#define cuzcs_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x05, 0x02))                     \
        EMIT6(MZM(0xC0, RYG(XD),RYG(XS),   0x05, 0x02))

#define cuzcs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x05, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, RYG(XD),TmmM,      0x05, 0x02))

/* cup (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rupcs_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x06, 0x02))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x06, 0x02))

#define rupcs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x06, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x06, 0x02))

#define cupcs_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x06, 0x02))                     \
        EMIT6(MZM(0xC0, RYG(XD),RYG(XS),   0x06, 0x02))

#define cupcs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x06, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, RYG(XD),TmmM,      0x06, 0x02))

/* cum (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rumcs_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x07, 0x02))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x07, 0x02))

#define rumcs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x07, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x07, 0x02))

#define cumcs_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x07, 0x02))                     \
        EMIT6(MZM(0xC0, RYG(XD),RYG(XS),   0x07, 0x02))

#define cumcs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x07, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, RYG(XD),TmmM,      0x07, 0x02))

/* cun (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define runcs_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x04, 0x02))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x04, 0x02))

#define runcs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x04, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x04, 0x02))

#define cuncs_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x04, 0x02))                     \
        EMIT6(MZM(0xC0, RYG(XD),RYG(XS),   0x04, 0x02))

#define cuncs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x04, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, RYG(XD),TmmM,      0x04, 0x02))

/* cut (D = fp-to-unsigned-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cuz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rudcs_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x00, 0x02))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x00, 0x02))

#define rudcs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x00, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x00, 0x02))

#define cutcs_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x00, 0x02))                     \
        EMIT6(MZM(0xC0, RYG(XD),RYG(XS),   0x00, 0x02))

#define cutcs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x00, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, RYG(XD),TmmM,      0x00, 0x02))

/* cur (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rurcs_rr(XD, XS, mode)                                              \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),(RT_SIMD_MODE_##mode&3), 0x02))     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),(RT_SIMD_MODE_##mode&3), 0x02))

#define curcs_rr(XD, XS, mode)                                              \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,   (RT_SIMD_MODE_##mode&3), 0x02))     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, RYG(XD),TmmM,   (RT_SIMD_MODE_##mode&3), 0x02))

/*--------------------------------   64-bit   -------------------- 256-bit ---*/

/* mov (D = S) */

#define movdx_rr(XD, XS)                                                    \
        EMIT6(MXM(0x56, REG(XD),REG(XS),   0x00, 0x00))                     \
        EMIT6(MXM(0x56, RYG(XD),RYG(XS),   0x00, 0x00))

#define movdx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, REG(XD),MOD(MS),REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MPM(0x06, RYG(XD),MOD(MS),REG(MS), VYL(DS), B2(DS), L2(DS)))

#define movdx_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    REG(MD), VAL(DD), A2(DD), EMPTY2)   \
        EMIT6(MPM(0x0E, REG(XS),MOD(MD),REG(MD), VAL(DD), B2(DD), L2(DD)))  \
        EMIT6(MPM(0x0E, RYG(XS),MOD(MD),REG(MD), VYL(DD), B2(DD), L2(DD)))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define anddx_rr(XG, XS)                                                    \
        anddx3rr(W(XG), W(XG), W(XS))

#define anddx_ld(XG, MS, DS)                                                \
        anddx3ld(W(XG), W(XG), W(MS), W(DS))

#define anddx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x68, REG(XD),REG(XS),REG(XT), 0x00))                     \
        EMIT6(MXM(0x68, RYG(XD),RYG(XS),RYG(XT), 0x00))

#define anddx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x68, REG(XD),REG(XS),   TmmM, 0x00))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x68, RYG(XD),RYG(XS),   TmmM, 0x00))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define anndx_rr(XG, XS)                                                    \
        anndx3rr(W(XG), W(XG), W(XS))

#define anndx_ld(XG, MS, DS)                                                \
        anndx3ld(W(XG), W(XG), W(MS), W(DS))

#define anndx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x69, REG(XD),REG(XT),REG(XS), 0x00))                     \
        EMIT6(MXM(0x69, RYG(XD),RYG(XT),RYG(XS), 0x00))

#define anndx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x69, REG(XD),TmmM,   REG(XS), 0x00))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x69, RYG(XD),TmmM,   RYG(XS), 0x00))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrdx_rr(XG, XS)                                                    \
        orrdx3rr(W(XG), W(XG), W(XS))

#define orrdx_ld(XG, MS, DS)                                                \
        orrdx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrdx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x6A, REG(XD),REG(XS),REG(XT), 0x00))                     \
        EMIT6(MXM(0x6A, RYG(XD),RYG(XS),RYG(XT), 0x00))

#define orrdx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6A, REG(XD),REG(XS),   TmmM, 0x00))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6A, RYG(XD),RYG(XS),   TmmM, 0x00))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define orndx_rr(XG, XS)                                                    \
        orndx3rr(W(XG), W(XG), W(XS))

#define orndx_ld(XG, MS, DS)                                                \
        orndx3ld(W(XG), W(XG), W(MS), W(DS))

#define orndx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x6F, REG(XD),REG(XT),REG(XS), 0x00))                     \
        EMIT6(MXM(0x6F, RYG(XD),RYG(XT),RYG(XS), 0x00))

#define orndx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6F, REG(XD),TmmM,   REG(XS), 0x00))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6F, RYG(XD),TmmM,   RYG(XS), 0x00))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xordx_rr(XG, XS)                                                    \
        xordx3rr(W(XG), W(XG), W(XS))

#define xordx_ld(XG, MS, DS)                                                \
        xordx3ld(W(XG), W(XG), W(MS), W(DS))

#define xordx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x6D, REG(XD),REG(XS),REG(XT), 0x00))                     \
        EMIT6(MXM(0x6D, RYG(XD),RYG(XS),RYG(XT), 0x00))

#define xordx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6D, REG(XD),REG(XS),   TmmM, 0x00))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6D, RYG(XD),RYG(XS),   TmmM, 0x00))

/* not (G = ~G), (D = ~S) */

#define notdx_rx(XG)                                                        \
        notdx_rr(W(XG), W(XG))

#define notdx_rr(XD, XS)                                                    \
        EMIT6(MXM(0x6B, REG(XD),REG(XS),REG(XS), 0x00))                     \
        EMIT6(MXM(0x6B, RYG(XD),RYG(XS),RYG(XS), 0x00))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addds_rr(XG, XS)                                                    \
        addds3rr(W(XG), W(XG), W(XS))

#define addds_ld(XG, MS, DS)                                                \
        addds3ld(W(XG), W(XG), W(MS), W(DS))

#define addds3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE3, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xE3, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define addds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE3, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE3, RYG(XD),RYG(XS),   TmmM, 0x03))

        /* adp, adh are defined in rtbase.h (first 15-regs only)
         * under "COMMON SIMD INSTRUCTIONS" section */

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subds_rr(XG, XS)                                                    \
        subds3rr(W(XG), W(XG), W(XS))

#define subds_ld(XG, MS, DS)                                                \
        subds3ld(W(XG), W(XG), W(MS), W(DS))

#define subds3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE2, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xE2, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define subds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE2, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE2, RYG(XD),RYG(XS),   TmmM, 0x03))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulds_rr(XG, XS)                                                    \
        mulds3rr(W(XG), W(XG), W(XS))

#define mulds_ld(XG, MS, DS)                                                \
        mulds3ld(W(XG), W(XG), W(MS), W(DS))

#define mulds3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE7, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xE7, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define mulds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE7, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE7, RYG(XD),RYG(XS),   TmmM, 0x03))

        /* mlp, mlh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divds_rr(XG, XS)                                                    \
        divds3rr(W(XG), W(XG), W(XS))

#define divds_ld(XG, MS, DS)                                                \
        divds3ld(W(XG), W(XG), W(MS), W(DS))

#define divds3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE5, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xE5, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define divds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE5, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE5, RYG(XD),RYG(XS),   TmmM, 0x03))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqds_rr(XG, XS)                                                    \
        ceqds3rr(W(XG), W(XG), W(XS))

#define ceqds_ld(XG, MS, DS)                                                \
        ceqds3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqds3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE8, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xE8, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define ceqds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE8, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE8, RYG(XD),RYG(XS),   TmmM, 0x03))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneds_rr(XG, XS)                                                    \
        cneds3rr(W(XG), W(XG), W(XS))

#define cneds_ld(XG, MS, DS)                                                \
        cneds3ld(W(XG), W(XG), W(MS), W(DS))

#define cneds3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE8, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xE8, RYG(XD),RYG(XS),RYG(XT), 0x03))                     \
        notdx_rx(W(XD))

#define cneds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE8, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE8, RYG(XD),RYG(XS),   TmmM, 0x03))                     \
        notdx_rx(W(XD))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltds_rr(XG, XS)                                                    \
        cltds3rr(W(XG), W(XG), W(XS))

#define cltds_ld(XG, MS, DS)                                                \
        cltds3ld(W(XG), W(XG), W(MS), W(DS))

#define cltds3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEB, REG(XD),REG(XT),REG(XS), 0x03))                     \
        EMIT6(MXM(0xEB, RYG(XD),RYG(XT),RYG(XS), 0x03))

#define cltds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEB, REG(XD),TmmM,   REG(XS), 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEB, RYG(XD),TmmM,   RYG(XS), 0x03))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define cleds_rr(XG, XS)                                                    \
        cleds3rr(W(XG), W(XG), W(XS))

#define cleds_ld(XG, MS, DS)                                                \
        cleds3ld(W(XG), W(XG), W(MS), W(DS))

#define cleds3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEA, REG(XD),REG(XT),REG(XS), 0x03))                     \
        EMIT6(MXM(0xEA, RYG(XD),RYG(XT),RYG(XS), 0x03))

#define cleds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEA, REG(XD),TmmM,   REG(XS), 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEA, RYG(XD),TmmM,   RYG(XS), 0x03))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtds_rr(XG, XS)                                                    \
        cgtds3rr(W(XG), W(XG), W(XS))

#define cgtds_ld(XG, MS, DS)                                                \
        cgtds3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtds3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEB, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xEB, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define cgtds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEB, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEB, RYG(XD),RYG(XS),   TmmM, 0x03))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgeds_rr(XG, XS)                                                    \
        cgeds3rr(W(XG), W(XG), W(XS))

#define cgeds_ld(XG, MS, DS)                                                \
        cgeds3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeds3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEA, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xEA, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define cgeds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEA, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEA, RYG(XD),RYG(XS),   TmmM, 0x03))

/*************   packed double-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzds_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x05, 0x03))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x05, 0x03))

#define rnzds_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x05, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x05, 0x03))

#define cvzds_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x05, 0x03))                     \
        EMIT6(MZM(0xC2, RYG(XD),RYG(XS),   0x05, 0x03))

#define cvzds_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x05, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, RYG(XD),TmmM,      0x05, 0x03))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpds_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x06, 0x03))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x06, 0x03))

#define rnpds_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x06, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x06, 0x03))

#define cvpds_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x06, 0x03))                     \
        EMIT6(MZM(0xC2, RYG(XD),RYG(XS),   0x06, 0x03))

#define cvpds_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x06, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, RYG(XD),TmmM,      0x06, 0x03))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmds_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x07, 0x03))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x07, 0x03))

#define rnmds_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x07, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x07, 0x03))

#define cvmds_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x07, 0x03))                     \
        EMIT6(MZM(0xC2, RYG(XD),RYG(XS),   0x07, 0x03))

#define cvmds_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x07, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, RYG(XD),TmmM,      0x07, 0x03))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnds_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x04, 0x03))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x04, 0x03))

#define rnnds_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x04, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x04, 0x03))

#define cvnds_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x04, 0x03))                     \
        EMIT6(MZM(0xC2, RYG(XD),RYG(XS),   0x04, 0x03))

#define cvnds_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x04, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, RYG(XD),TmmM,      0x04, 0x03))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndds_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x00, 0x03))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x00, 0x03))

#define rndds_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x00, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x00, 0x03))

#define cvtds_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x00, 0x03))                     \
        EMIT6(MZM(0xC2, RYG(XD),RYG(XS),   0x00, 0x03))

#define cvtds_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x00, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, RYG(XD),TmmM,      0x00, 0x03))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnrds_rr(XD, XS, mode)                                              \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),(RT_SIMD_MODE_##mode&3), 0x03))     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),(RT_SIMD_MODE_##mode&3), 0x03))

#define cvrds_rr(XD, XS, mode)                                              \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,   (RT_SIMD_MODE_##mode&3), 0x03))     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, RYG(XD),TmmM,   (RT_SIMD_MODE_##mode&3), 0x03))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvndn_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC3, REG(XD),REG(XS),   0x04, 0x03))                     \
        EMIT6(MZM(0xC3, RYG(XD),RYG(XS),   0x04, 0x03))

#define cvndn_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC3, REG(XD),TmmM,      0x04, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC3, RYG(XD),TmmM,      0x04, 0x03))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtdn_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC3, REG(XD),REG(XS),   0x00, 0x03))                     \
        EMIT6(MZM(0xC3, RYG(XD),RYG(XS),   0x00, 0x03))

#define cvtdn_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC3, REG(XD),TmmM,      0x00, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC3, RYG(XD),TmmM,      0x00, 0x03))

/* cvn (D = unsigned-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvndx_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC1, REG(XD),REG(XS),   0x04, 0x03))                     \
        EMIT6(MZM(0xC1, RYG(XD),RYG(XS),   0x04, 0x03))

#define cvndx_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC1, REG(XD),TmmM,      0x04, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC1, RYG(XD),TmmM,      0x04, 0x03))

/* cvt (D = unsigned-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtdx_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC1, REG(XD),REG(XS),   0x00, 0x03))                     \
        EMIT6(MZM(0xC1, RYG(XD),RYG(XS),   0x00, 0x03))

#define cvtdx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC1, REG(XD),TmmM,      0x00, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC1, RYG(XD),TmmM,      0x00, 0x03))

/* cuz (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define ruzds_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x05, 0x03))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x05, 0x03))

#define ruzds_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x05, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x05, 0x03))

#define cuzds_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x05, 0x03))                     \
        EMIT6(MZM(0xC0, RYG(XD),RYG(XS),   0x05, 0x03))

#define cuzds_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x05, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, RYG(XD),TmmM,      0x05, 0x03))

/* cup (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rupds_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x06, 0x03))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x06, 0x03))

#define rupds_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x06, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x06, 0x03))

#define cupds_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x06, 0x03))                     \
        EMIT6(MZM(0xC0, RYG(XD),RYG(XS),   0x06, 0x03))

#define cupds_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x06, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, RYG(XD),TmmM,      0x06, 0x03))

/* cum (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rumds_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x07, 0x03))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x07, 0x03))

#define rumds_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x07, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x07, 0x03))

#define cumds_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x07, 0x03))                     \
        EMIT6(MZM(0xC0, RYG(XD),RYG(XS),   0x07, 0x03))

#define cumds_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x07, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, RYG(XD),TmmM,      0x07, 0x03))

/* cun (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define runds_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x04, 0x03))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x04, 0x03))

#define runds_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x04, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x04, 0x03))

#define cunds_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x04, 0x03))                     \
        EMIT6(MZM(0xC0, RYG(XD),RYG(XS),   0x04, 0x03))

#define cunds_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x04, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, RYG(XD),TmmM,      0x04, 0x03))

/* cut (D = fp-to-unsigned-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cuz
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rudds_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x00, 0x03))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x00, 0x03))

#define rudds_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x00, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x00, 0x03))

#define cutds_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x00, 0x03))                     \
        EMIT6(MZM(0xC0, RYG(XD),RYG(XS),   0x00, 0x03))

#define cutds_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x00, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, RYG(XD),TmmM,      0x00, 0x03))

/* cur (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rurds_rr(XD, XS, mode)                                              \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),(RT_SIMD_MODE_##mode&3), 0x03))     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),(RT_SIMD_MODE_##mode&3), 0x03))

#define curds_rr(XD, XS, mode)                                              \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,   (RT_SIMD_MODE_##mode&3), 0x03))     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, RYG(XD),TmmM,   (RT_SIMD_MODE_##mode&3), 0x03))

/******************************************************************************/
/**********************************   ELEM   **********************************/
/******************************************************************************/

/*--------------------------------   32-bit   --------------------------------*/

/* mov (D = S) */

#define movrs_rr(XD, XS)                                                    \
        EMIT6(MXM(0x56, REG(XD),REG(XS),   0x00, 0x00))

#define movrs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x03, REG(XD),MOD(MS),REG(MS), VAL(DS), B2(DS), P2(DS)))

#define movrs_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    REG(MD), VAL(DD), A2(DD), EMPTY2)   \
        EMIT6(MPM(0x0B, REG(XS),MOD(MD),REG(MD), VAL(DD), B2(DD), P2(DD)))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addrs_rr(XG, XS)                                                    \
        addrs3rr(W(XG), W(XG), W(XS))

#define addrs_ld(XG, MS, DS)                                                \
        addrs3ld(W(XG), W(XG), W(MS), W(DS))

#define addrs3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xE3, REG(XD),REG(XS),REG(XT), 0x02))

#define addrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xE3, REG(XD),REG(XS),   TmmM, 0x02))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subrs_rr(XG, XS)                                                    \
        subrs3rr(W(XG), W(XG), W(XS))

#define subrs_ld(XG, MS, DS)                                                \
        subrs3ld(W(XG), W(XG), W(MS), W(DS))

#define subrs3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xE2, REG(XD),REG(XS),REG(XT), 0x02))

#define subrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xE2, REG(XD),REG(XS),   TmmM, 0x02))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulrs_rr(XG, XS)                                                    \
        mulrs3rr(W(XG), W(XG), W(XS))

#define mulrs_ld(XG, MS, DS)                                                \
        mulrs3ld(W(XG), W(XG), W(MS), W(DS))

#define mulrs3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xE7, REG(XD),REG(XS),REG(XT), 0x02))

#define mulrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xE7, REG(XD),REG(XS),   TmmM, 0x02))

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divrs_rr(XG, XS)                                                    \
        divrs3rr(W(XG), W(XG), W(XS))

#define divrs_ld(XG, MS, DS)                                                \
        divrs3ld(W(XG), W(XG), W(MS), W(DS))

#define divrs3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xE5, REG(XD),REG(XS),REG(XT), 0x02))

#define divrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xE5, REG(XD),REG(XS),   TmmM, 0x02))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqrs_rr(XG, XS)                                                    \
        ceqrs3rr(W(XG), W(XG), W(XS))

#define ceqrs_ld(XG, MS, DS)                                                \
        ceqrs3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqrs3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xE8, REG(XD),REG(XS),REG(XT), 0x02))

#define ceqrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xE8, REG(XD),REG(XS),   TmmM, 0x02))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cners_rr(XG, XS)                                                    \
        cners3rr(W(XG), W(XG), W(XS))

#define cners_ld(XG, MS, DS)                                                \
        cners3ld(W(XG), W(XG), W(MS), W(DS))

#define cners3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xE8, REG(XD),REG(XS),REG(XT), 0x02))                     \
        notix_rx(W(XD))

#define cners3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xE8, REG(XD),REG(XS),   TmmM, 0x02))                     \
        notix_rx(W(XD))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltrs_rr(XG, XS)                                                    \
        cltrs3rr(W(XG), W(XG), W(XS))

#define cltrs_ld(XG, MS, DS)                                                \
        cltrs3ld(W(XG), W(XG), W(MS), W(DS))

#define cltrs3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xEB, REG(XD),REG(XT),REG(XS), 0x02))

#define cltrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xEB, REG(XD),TmmM,   REG(XS), 0x02))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define clers_rr(XG, XS)                                                    \
        clers3rr(W(XG), W(XG), W(XS))

#define clers_ld(XG, MS, DS)                                                \
        clers3ld(W(XG), W(XG), W(MS), W(DS))

#define clers3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xEA, REG(XD),REG(XT),REG(XS), 0x02))

#define clers3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xEA, REG(XD),TmmM,   REG(XS), 0x02))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtrs_rr(XG, XS)                                                    \
        cgtrs3rr(W(XG), W(XG), W(XS))

#define cgtrs_ld(XG, MS, DS)                                                \
        cgtrs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtrs3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xEB, REG(XD),REG(XS),REG(XT), 0x02))

#define cgtrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xEB, REG(XD),REG(XS),   TmmM, 0x02))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgers_rr(XG, XS)                                                    \
        cgers3rr(W(XG), W(XG), W(XS))

#define cgers_ld(XG, MS, DS)                                                \
        cgers3ld(W(XG), W(XG), W(MS), W(DS))

#define cgers3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xEA, REG(XD),REG(XS),REG(XT), 0x02))

#define cgers3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xEA, REG(XD),REG(XS),   TmmM, 0x02))

/*************   scalar single-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzrs_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x05, 0x02))

#define rnzrs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x05, 0x02))

#define cvzrs_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MYM(0xC2, REG(XD),REG(XS),   0x05, 0x02))

#define cvzrs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC2, REG(XD),TmmM,      0x05, 0x02))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnprs_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x06, 0x02))

#define rnprs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x06, 0x02))

#define cvprs_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MYM(0xC2, REG(XD),REG(XS),   0x06, 0x02))

#define cvprs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC2, REG(XD),TmmM,      0x06, 0x02))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmrs_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x07, 0x02))

#define rnmrs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x07, 0x02))

#define cvmrs_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MYM(0xC2, REG(XD),REG(XS),   0x07, 0x02))

#define cvmrs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC2, REG(XD),TmmM,      0x07, 0x02))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnrs_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x04, 0x02))

#define rnnrs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x04, 0x02))

#define cvnrs_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MYM(0xC2, REG(XD),REG(XS),   0x04, 0x02))

#define cvnrs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC2, REG(XD),TmmM,      0x04, 0x02))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndrs_rr(XD, XS)                                                    \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x00, 0x02))

#define rndrs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x00, 0x02))

#define cvtrs_rr(XD, XS)                                                    \
        EMIT6(MYM(0xC2, REG(XD),REG(XS),   0x00, 0x02))

#define cvtrs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC2, REG(XD),TmmM,      0x00, 0x02))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnrrs_rr(XD, XS, mode)                                              \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),(RT_SIMD_MODE_##mode&3), 0x02))

#define cvrrs_rr(XD, XS, mode)                                              \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC2, REG(XD),TmmM,   (RT_SIMD_MODE_##mode&3), 0x02))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnrn_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MYM(0xC3, REG(XD),REG(XS),   0x04, 0x02))

#define cvnrn_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC3, REG(XD),TmmM,      0x04, 0x02))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtrn_rr(XD, XS)                                                    \
        EMIT6(MYM(0xC3, REG(XD),REG(XS),   0x00, 0x02))

#define cvtrn_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC3, REG(XD),TmmM,      0x00, 0x02))

/* cvn (D = unsigned-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnrx_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MYM(0xC1, REG(XD),REG(XS),   0x04, 0x02))

#define cvnrx_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC1, REG(XD),TmmM,      0x04, 0x02))

/* cvt (D = unsigned-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtrx_rr(XD, XS)                                                    \
        EMIT6(MYM(0xC1, REG(XD),REG(XS),   0x00, 0x02))

#define cvtrx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC1, REG(XD),TmmM,      0x00, 0x02))

/* cuz (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define ruzrs_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x05, 0x02))

#define ruzrs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x05, 0x02))

#define cuzrs_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MYM(0xC0, REG(XD),REG(XS),   0x05, 0x02))

#define cuzrs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC0, REG(XD),TmmM,      0x05, 0x02))

/* cup (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define ruprs_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x06, 0x02))

#define ruprs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x06, 0x02))

#define cuprs_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MYM(0xC0, REG(XD),REG(XS),   0x06, 0x02))

#define cuprs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC0, REG(XD),TmmM,      0x06, 0x02))

/* cum (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rumrs_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x07, 0x02))

#define rumrs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x07, 0x02))

#define cumrs_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MYM(0xC0, REG(XD),REG(XS),   0x07, 0x02))

#define cumrs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC0, REG(XD),TmmM,      0x07, 0x02))

/* cun (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define runrs_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x04, 0x02))

#define runrs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x04, 0x02))

#define cunrs_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MYM(0xC0, REG(XD),REG(XS),   0x04, 0x02))

#define cunrs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC0, REG(XD),TmmM,      0x04, 0x02))

/* cut (D = fp-to-unsigned-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cuz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rudrs_rr(XD, XS)                                                    \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x00, 0x02))

#define rudrs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x00, 0x02))

#define cutrs_rr(XD, XS)                                                    \
        EMIT6(MYM(0xC0, REG(XD),REG(XS),   0x00, 0x02))

#define cutrs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC0, REG(XD),TmmM,      0x00, 0x02))

/* cur (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rurrs_rr(XD, XS, mode)                                              \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),(RT_SIMD_MODE_##mode&3), 0x02))

#define currs_rr(XD, XS, mode)                                              \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC0, REG(XD),TmmM,   (RT_SIMD_MODE_##mode&3), 0x02))

/*--------------------------------   64-bit   --------------------------------*/

/* mov (D = S) */

#define movts_rr(XD, XS)                                                    \
        EMIT6(MXM(0x56, REG(XD),REG(XS),   0x00, 0x00))

#define movts_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x02, REG(XD),MOD(MS),REG(MS), VAL(DS), B2(DS), P2(DS)))

#define movts_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    REG(MD), VAL(DD), A2(DD), EMPTY2)   \
        EMIT6(MPM(0x0A, REG(XS),MOD(MD),REG(MD), VAL(DD), B2(DD), P2(DD)))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addts_rr(XG, XS)                                                    \
        addts3rr(W(XG), W(XG), W(XS))

#define addts_ld(XG, MS, DS)                                                \
        addts3ld(W(XG), W(XG), W(MS), W(DS))

#define addts3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xE3, REG(XD),REG(XS),REG(XT), 0x03))

#define addts3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xE3, REG(XD),REG(XS),   TmmM, 0x03))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subts_rr(XG, XS)                                                    \
        subts3rr(W(XG), W(XG), W(XS))

#define subts_ld(XG, MS, DS)                                                \
        subts3ld(W(XG), W(XG), W(MS), W(DS))

#define subts3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xE2, REG(XD),REG(XS),REG(XT), 0x03))

#define subts3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xE2, REG(XD),REG(XS),   TmmM, 0x03))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mults_rr(XG, XS)                                                    \
        mults3rr(W(XG), W(XG), W(XS))

#define mults_ld(XG, MS, DS)                                                \
        mults3ld(W(XG), W(XG), W(MS), W(DS))

#define mults3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xE7, REG(XD),REG(XS),REG(XT), 0x03))

#define mults3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xE7, REG(XD),REG(XS),   TmmM, 0x03))

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divts_rr(XG, XS)                                                    \
        divts3rr(W(XG), W(XG), W(XS))

#define divts_ld(XG, MS, DS)                                                \
        divts3ld(W(XG), W(XG), W(MS), W(DS))

#define divts3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xE5, REG(XD),REG(XS),REG(XT), 0x03))

#define divts3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xE5, REG(XD),REG(XS),   TmmM, 0x03))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqts_rr(XG, XS)                                                    \
        ceqts3rr(W(XG), W(XG), W(XS))

#define ceqts_ld(XG, MS, DS)                                                \
        ceqts3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqts3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xE8, REG(XD),REG(XS),REG(XT), 0x03))

#define ceqts3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xE8, REG(XD),REG(XS),   TmmM, 0x03))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnets_rr(XG, XS)                                                    \
        cnets3rr(W(XG), W(XG), W(XS))

#define cnets_ld(XG, MS, DS)                                                \
        cnets3ld(W(XG), W(XG), W(MS), W(DS))

#define cnets3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xE8, REG(XD),REG(XS),REG(XT), 0x03))                     \
        notjx_rx(W(XD))

#define cnets3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xE8, REG(XD),REG(XS),   TmmM, 0x03))                     \
        notjx_rx(W(XD))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltts_rr(XG, XS)                                                    \
        cltts3rr(W(XG), W(XG), W(XS))

#define cltts_ld(XG, MS, DS)                                                \
        cltts3ld(W(XG), W(XG), W(MS), W(DS))

#define cltts3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xEB, REG(XD),REG(XT),REG(XS), 0x03))

#define cltts3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xEB, REG(XD),TmmM,   REG(XS), 0x03))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define clets_rr(XG, XS)                                                    \
        clets3rr(W(XG), W(XG), W(XS))

#define clets_ld(XG, MS, DS)                                                \
        clets3ld(W(XG), W(XG), W(MS), W(DS))

#define clets3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xEA, REG(XD),REG(XT),REG(XS), 0x03))

#define clets3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xEA, REG(XD),TmmM,   REG(XS), 0x03))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtts_rr(XG, XS)                                                    \
        cgtts3rr(W(XG), W(XG), W(XS))

#define cgtts_ld(XG, MS, DS)                                                \
        cgtts3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtts3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xEB, REG(XD),REG(XS),REG(XT), 0x03))

#define cgtts3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xEB, REG(XD),REG(XS),   TmmM, 0x03))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgets_rr(XG, XS)                                                    \
        cgets3rr(W(XG), W(XG), W(XS))

#define cgets_ld(XG, MS, DS)                                                \
        cgets3ld(W(XG), W(XG), W(MS), W(DS))

#define cgets3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xEA, REG(XD),REG(XS),REG(XT), 0x03))

#define cgets3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xEA, REG(XD),REG(XS),   TmmM, 0x03))

/*************   scalar double-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzts_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x05, 0x03))

#define rnzts_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x05, 0x03))

#define cvzts_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MYM(0xC2, REG(XD),REG(XS),   0x05, 0x03))

#define cvzts_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC2, REG(XD),TmmM,      0x05, 0x03))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpts_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x06, 0x03))

#define rnpts_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x06, 0x03))

#define cvpts_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MYM(0xC2, REG(XD),REG(XS),   0x06, 0x03))

#define cvpts_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC2, REG(XD),TmmM,      0x06, 0x03))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmts_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x07, 0x03))

#define rnmts_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x07, 0x03))

#define cvmts_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MYM(0xC2, REG(XD),REG(XS),   0x07, 0x03))

#define cvmts_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC2, REG(XD),TmmM,      0x07, 0x03))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnts_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x04, 0x03))

#define rnnts_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x04, 0x03))

#define cvnts_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MYM(0xC2, REG(XD),REG(XS),   0x04, 0x03))

#define cvnts_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC2, REG(XD),TmmM,      0x04, 0x03))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndts_rr(XD, XS)                                                    \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x00, 0x03))

#define rndts_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x00, 0x03))

#define cvtts_rr(XD, XS)                                                    \
        EMIT6(MYM(0xC2, REG(XD),REG(XS),   0x00, 0x03))

#define cvtts_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC2, REG(XD),TmmM,      0x00, 0x03))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnrts_rr(XD, XS, mode)                                              \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),(RT_SIMD_MODE_##mode&3), 0x03))

#define cvrts_rr(XD, XS, mode)                                              \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC2, REG(XD),TmmM,   (RT_SIMD_MODE_##mode&3), 0x03))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvntn_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MYM(0xC3, REG(XD),REG(XS),   0x04, 0x03))

#define cvntn_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC3, REG(XD),TmmM,      0x04, 0x03))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvttn_rr(XD, XS)                                                    \
        EMIT6(MYM(0xC3, REG(XD),REG(XS),   0x00, 0x03))

#define cvttn_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC3, REG(XD),TmmM,      0x00, 0x03))

/* cvn (D = unsigned-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvntx_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MYM(0xC1, REG(XD),REG(XS),   0x04, 0x03))

#define cvntx_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC1, REG(XD),TmmM,      0x04, 0x03))

/* cvt (D = unsigned-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvttx_rr(XD, XS)                                                    \
        EMIT6(MYM(0xC1, REG(XD),REG(XS),   0x00, 0x03))

#define cvttx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC1, REG(XD),TmmM,      0x00, 0x03))

/* cuz (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define ruzts_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x05, 0x03))

#define ruzts_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x05, 0x03))

#define cuzts_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MYM(0xC0, REG(XD),REG(XS),   0x05, 0x03))

#define cuzts_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC0, REG(XD),TmmM,      0x05, 0x03))

/* cup (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rupts_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x06, 0x03))

#define rupts_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x06, 0x03))

#define cupts_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MYM(0xC0, REG(XD),REG(XS),   0x06, 0x03))

#define cupts_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC0, REG(XD),TmmM,      0x06, 0x03))

/* cum (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rumts_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x07, 0x03))

#define rumts_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x07, 0x03))

#define cumts_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MYM(0xC0, REG(XD),REG(XS),   0x07, 0x03))

#define cumts_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC0, REG(XD),TmmM,      0x07, 0x03))

/* cun (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define runts_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x04, 0x03))

#define runts_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x04, 0x03))

#define cunts_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MYM(0xC0, REG(XD),REG(XS),   0x04, 0x03))

#define cunts_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC0, REG(XD),TmmM,      0x04, 0x03))

/* cut (D = fp-to-unsigned-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cuz
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rudts_rr(XD, XS)                                                    \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x00, 0x03))

#define rudts_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x00, 0x03))

#define cutts_rr(XD, XS)                                                    \
        EMIT6(MYM(0xC0, REG(XD),REG(XS),   0x00, 0x03))

#define cutts_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC0, REG(XD),TmmM,      0x00, 0x03))

/* cur (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rurts_rr(XD, XS, mode)                                              \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),(RT_SIMD_MODE_##mode&3), 0x03))

#define curts_rr(XD, XS, mode)                                              \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC0, REG(XD),TmmM,   (RT_SIMD_MODE_##mode&3), 0x03))

/************************* register-size instructions *************************/

/* sregs */

#undef  sregs_sa
#define sregs_sa() /* save all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        movix_st(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(Xmm8, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(Xmm9, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(XmmA, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(XmmB, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(XmmC, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(XmmD, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(XmmE, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(XmmF, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMIT6(0xE7000000080E | MSM(TmmM,  TEax,  0x00))

#undef  sregs_la
#define sregs_la() /* load all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        movix_ld(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(Xmm8, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(Xmm9, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(XmmA, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(XmmB, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(XmmC, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(XmmD, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(XmmE, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(XmmF, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMIT6(0xE70000000806 | MSM(TmmM,  TEax,  0x00))

/* verxx */

#define verxx_xx()                                                          \
        subxx_rr(Reax, Reax)                                                \
        addwx_ri(Reax, IV(0x00030F))                                        \
        movwx_st(Reax, Mebp, inf_VER)

/******************************************************************************/
/**********************************   MODE   **********************************/
/******************************************************************************/

/************************   helper macros (FPU mode)   ************************/

/* simd mode
 * set via FCTRL macros, *_F for faster non-IEEE mode (optional on MIPS/POWER),
 * original FCTRL blocks (FCTRL_ENTER/FCTRL_LEAVE) are defined in rtbase.h
 * NOTE: ARMv7 always uses ROUNDN non-IEEE mode for SIMD fp-arithmetic,
 * while fp<->int conversion takes ROUND* into account via VFP fallback */

#define RT_SIMD_MODE_ROUNDN     0x04    /* round towards near */
#define RT_SIMD_MODE_ROUNDM     0x07    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP     0x06    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ     0x05    /* round towards zero */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/*
 * rtarch_z64_128x*v1.h: Implementation of s390x SIMD instructions.
 *
 * This file is a part of the unified SIMD assembler framework (rtarch.h)
 * designed to be compatible with different processor architectures,
 * while maintaining strictly defined common API.
 *
 * Recommended naming scheme for instructions:
 *
 * cmdp*_ri - applies [cmd] to [p]acked: [r]egister from [i]mmediate
 * cmdp*_rr - applies [cmd] to [p]acked: [r]egister from [r]egister
 *
 * cmdp*_rm - applies [cmd] to [p]acked: [r]egister from [m]emory
 * cmdp*_ld - applies [cmd] to [p]acked: as above
 *
 * cmdi*_** - applies [cmd] to 32-bit elements SIMD args, packed-128-bit
 * cmdj*_** - applies [cmd] to 64-bit elements SIMD args, packed-128-bit
 * cmdl*_** - applies [cmd] to L-size elements SIMD args, packed-128-bit
 *
 * cmdc*_** - applies [cmd] to 32-bit elements SIMD args, packed-256-bit
 * cmdd*_** - applies [cmd] to 64-bit elements SIMD args, packed-256-bit
 * cmdf*_** - applies [cmd] to L-size elements SIMD args, packed-256-bit
 *
 * cmdo*_** - applies [cmd] to 32-bit elements SIMD args, packed-var-len
 * cmdp*_** - applies [cmd] to L-size elements SIMD args, packed-var-len
 * cmdq*_** - applies [cmd] to 64-bit elements SIMD args, packed-var-len
 *
 * cmdr*_** - applies [cmd] to 32-bit elements ELEM args, scalar-fp-only
 * cmds*_** - applies [cmd] to L-size elements ELEM args, scalar-fp-only
 * cmdt*_** - applies [cmd] to 64-bit elements ELEM args, scalar-fp-only
 *
 * cmd*x_** - applies [cmd] to SIMD/BASE unsigned integer args, [x] - default
 * cmd*n_** - applies [cmd] to SIMD/BASE   signed integer args, [n] - negatable
 * cmd*s_** - applies [cmd] to SIMD/ELEM floating point   args, [s] - scalable
 *
 * The cmdp*_** (rtconf.h) instructions are intended for SPMD programming model
 * and can be configured to work with 32/64-bit data elements (fp+int).
 * In this model data paths are fixed-width, BASE and SIMD data elements are
 * width-compatible, code path divergence is handled via mkj**_** pseudo-ops.
 * Matching element-sized BASE subset cmdy*_** is defined in rtconf.h as well.
 *
 * Note, when using fixed-data-size 128/256-bit SIMD subsets simultaneously
 * upper 128-bit halves of full 256-bit SIMD registers may end up undefined.
 * On RISC targets they remain unchanged, while on x86-AVX they are zeroed.
 * This happens when registers written in 128-bit subset are then used/read
 * from within 256-bit subset. The same rule applies to mixing with 512-bit
 * and wider vectors. Use of scalars may leave respective vector registers
 * undefined, as seen from the perspective of any particular vector subset.
 *
 * 256-bit vectors used with wider subsets may not be compatible with regards
 * to memory loads/stores when mixed in the code. It means that data loaded
 * with wider vector and stored within 256-bit subset at the same address may
 * result in changing the initial representation in memory. The same can be
 * said about mixing vector and scalar subsets. Scalars can be completely
 * detached on some architectures. Use elm*x_st to store 1st vector element.
 * 128-bit vectors should be memory-compatible with any wider vector subset.
 *
 * Handling of NaNs in the floating point pipeline may not be consistent
 * across different architectures. Avoid NaNs entering the data flow by using
 * masking or control flow instructions. Apply special care when dealing with
 * floating point compare and min/max input/output. The result of floating point
 * compare instructions can be considered a -QNaN, though it is also interpreted
 * as integer -1 and is often treated as a mask. Most arithmetic instructions
 * should propagate QNaNs unchanged, however this behavior hasn't been tested.
 *
 * Note, that instruction subsets operating on vectors of different length
 * may support different number of SIMD registers, therefore mixing them
 * in the same code needs to be done with register awareness in mind.
 * For example, AVX-512 supports 32 SIMD registers, while AVX2 only has 16,
 * as does 256-bit paired subset on ARMv8, while 128-bit and SVE have 32.
 * These numbers should be consistent across architectures if properly
 * mapped to SIMD target mask presented in rtzero.h (compatibility layer).
 *
 * Interpretation of instruction parameters:
 *
 * upper-case params have triplet structure and require W to pass-forward
 * lower-case params are singular and can be used/passed as such directly
 *
 * XD - SIMD register serving as destination only, if present
 * XG - SIMD register serving as destination and first source
 * XS - SIMD register serving as second source (first if any)
 * XT - SIMD register serving as third source (second if any)
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
 */

#endif /* (defined RT_Z64) */

#endif /* RT_RTARCH_Z64_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

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

#define MTM(cod, reg, ren, imm) /* shiftIM */                               \
        ((reg) << 36 | (ren) << 28 | (imm) << 16 | (reg) << 32 |            \
         0xEB0000000000 | (cod))

#define MDM(cod, reg, bmd, brm, vdp, bxx, pxx)                              \
        (pxx(vdp) | (bmd) << 32 | bxx(brm) << 28 | (reg) << 36 | (cod))

#define MPM(cod, reg, bmd, brm, vdp, bxx, pxx)                              \
        (pxx(vdp) | (bmd) << 32 | bxx(brm) << 28 | (reg & 0x0F) << 36 |     \
         0xE70000000000 | (cod) | (reg & 0x10) <<  7)

#define MUM(cod, reg, ren, rem, bit)                                        \
        ((reg & 0x0F) << 36 | (ren & 0x0F) << 32 | (rem & 0x0F) << 28 |     \
         (reg & 0x10) <<  7 | (ren & 0x10) <<  6 | (rem & 0x10) <<  5 |     \
         (reg & 0x0F) << 12 | (reg & 0x10) <<  4 |                          \
         0xE70000080000 | (bit) << 24 | (cod))

#define MVM(cod, reg, ren, rem, bit)                                        \
        ((reg & 0x0F) << 36 | (ren & 0x0F) << 32 | (rem & 0x0F) << 28 |     \
         (reg & 0x10) <<  7 | (ren & 0x10) <<  6 | (rem & 0x10) <<  5 |     \
         (reg & 0x0F) << 12 | (reg & 0x10) <<  4 |                          \
         0xE70000000000 | (bit) << 24 | (cod))

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

#define B21(br) TDxx
#define B41(br) TPxx
#define P21(dp) (0xE30000000000)
#define L21(dp) (0xE30000000000 | (0x0010 & (dp))<<16)
#define C21(br, dp) EMIT6(0xC00100000000 | TDxx << 36 | (0x7FFFFFFF & (dp)))
#define A21(br, dp) C21(br, dp)                                             \
                    EMITW(0xB9080000 | TDxx << 4 | (br))

/* internal     REG */

#define TLxx    0x00  /* r0,  left  arg for compare */
#define TRxx    0x0D  /* r13, right arg for compare */
#define TMxx    0x00  /* r0 */
#define TDxx    0x0D  /* r13, must be non-zero reg-index */
#define TPxx    0x0E  /* r14, must be non-zero reg-index */
#define SPxx    0x0F  /* r15 */

#define TEax    0x03  /* r3 */
#define TEcx    0x01  /* r1 */
#define TEdx    0x02  /* r2 */
#define TEbx    0x04  /* r4 */
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

#define Reax    TEax, %%r3, EMPTY
#define Recx    TEcx, %%r1, EMPTY
#define Redx    TEdx, %%r2, EMPTY
#define Rebx    TEbx, %%r4, EMPTY
#define Rebp    TEbp, %%r5, EMPTY
#define Resi    TEsi, %%r6, EMPTY
#define Redi    TEdi, %%r7, EMPTY
#define Reg8    TEg8, %%r8, EMPTY
#define Reg9    TEg9, %%r9, EMPTY
#define RegA    TEgA, %%r10,EMPTY
#define RegB    TEgB, %%r11,EMPTY
#define RegC    TEgC, %%r12,EMPTY

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
        EMIT6(MIM(0xC0, REG(RD),0x0F,VAL(IS)))

#define movwx_mi(MD, DD, IS)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    REG(MD), VAL(DD), A1(DD), EMPTY2)   \
        EMIT6(MIM(0xC0, TMxx,   0x01,VAL(IS)))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MD),REG(MD), VAL(DD), B1(DD), P1(DD)))

#define movwx_rr(RD, RS)                                                    \
        EMITW(MGM(0x16, REG(RD),REG(RS),0x00))

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
        EMIT6(MDM(0x16, REG(RD),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

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

/* and (G = G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define andwx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x0B,VAL(IS)))                              \
        EMIT6(MIM(0xC0, REG(RG),0x0A,   0x00))

#define andwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x0B,VAL(IS)))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define andwx_rr(RG, RS)                                                    \
        EMITH(MRM(0x14, REG(RG),REG(RS),0x00))                              \
        EMIT6(MIM(0xC0, REG(RG),0x0A,   0x00))

#define andwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x54, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MIM(0xC0, REG(RG),0x0A,   0x00))

#define andwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x14, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define andwx_mr(MG, DG, RS)                                                \
        andwx_st(W(RS), W(MG), W(DG))


#define andwxZri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x0A,   0x00))                              \
        EMIT6(MIM(0xC0, REG(RG),0x0B,VAL(IS)))

#define andwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x0B,VAL(IS)))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define andwxZrr(RG, RS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x0A,   0x00))                              \
        EMITH(MRM(0x14, REG(RG),REG(RS),0x00))

#define andwxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x54, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define andwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x14, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define andwxZmr(MG, DG, RS)                                                \
        andwxZst(W(RS), W(MG), W(DG))

/* ann (G = ~G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define annwx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x0B,VAL(IS)))                              \
        EMIT6(MIM(0xC0, REG(RG),0x0A,   0x00))

#define annwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x0B,VAL(IS)))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define annwx_rr(RG, RS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMITH(MRM(0x14, REG(RG),REG(RS),0x00))                              \
        EMIT6(MIM(0xC0, REG(RG),0x0A,   0x00))

#define annwx_ld(RG, MS, DS)                                                \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x54, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MIM(0xC0, REG(RG),0x0A,   0x00))

#define annwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMITH(MRM(0x14, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define annwx_mr(MG, DG, RS)                                                \
        annwx_st(W(RS), W(MG), W(DG))


#define annwxZri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x0A,   0x00))                              \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x0B,VAL(IS)))

#define annwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x0B,VAL(IS)))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define annwxZrr(RG, RS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x0A,   0x00))                              \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMITH(MRM(0x14, REG(RG),REG(RS),0x00))

#define annwxZld(RG, MS, DS)                                                \
        EMIT6(MIM(0xC0, REG(RG),0x0A,   0x00))                              \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x54, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define annwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMITH(MRM(0x14, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define annwxZmr(MG, DG, RS)                                                \
        annwxZst(W(RS), W(MG), W(DG))

/* orr (G = G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define orrwx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x0D,VAL(IS)))

#define orrwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x0D,VAL(IS)))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define orrwx_rr(RG, RS)                                                    \
        EMITH(MRM(0x16, REG(RG),REG(RS),0x00))

#define orrwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x56, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define orrwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x16, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define orrwx_mr(MG, DG, RS)                                                \
        orrwx_st(W(RS), W(MG), W(DG))


#define orrwxZri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x0D,VAL(IS)))

#define orrwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x0D,VAL(IS)))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define orrwxZrr(RG, RS)                                                    \
        EMITH(MRM(0x16, REG(RG),REG(RS),0x00))

#define orrwxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x56, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define orrwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x16, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define orrwxZmr(MG, DG, RS)                                                \
        orrwxZst(W(RS), W(MG), W(DG))

/* orn (G = ~G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define ornwx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x0D,VAL(IS)))

#define ornwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x0D,VAL(IS)))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define ornwx_rr(RG, RS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMITH(MRM(0x16, REG(RG),REG(RS),0x00))

#define ornwx_ld(RG, MS, DS)                                                \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x56, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define ornwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMITH(MRM(0x16, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define ornwx_mr(MG, DG, RS)                                                \
        ornwx_st(W(RS), W(MG), W(DG))


#define ornwxZri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x0D,VAL(IS)))

#define ornwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x0D,VAL(IS)))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define ornwxZrr(RG, RS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMITH(MRM(0x16, REG(RG),REG(RS),0x00))

#define ornwxZld(RG, MS, DS)                                                \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x56, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define ornwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMITH(MRM(0x16, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define ornwxZmr(MG, DG, RS)                                                \
        ornwxZst(W(RS), W(MG), W(DG))

/* xor (G = G ^ S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define xorwx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,VAL(IS)))

#define xorwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,VAL(IS)))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define xorwx_rr(RG, RS)                                                    \
        EMITH(MRM(0x17, REG(RG),REG(RS),0x00))

#define xorwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x57, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define xorwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x17, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define xorwx_mr(MG, DG, RS)                                                \
        xorwx_st(W(RS), W(MG), W(DG))


#define xorwxZri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,VAL(IS)))

#define xorwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,VAL(IS)))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define xorwxZrr(RG, RS)                                                    \
        EMITH(MRM(0x17, REG(RG),REG(RS),0x00))

#define xorwxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x57, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define xorwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x17, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define xorwxZmr(MG, DG, RS)                                                \
        xorwxZst(W(RS), W(MG), W(DG))

/* not (G = ~G)
 * set-flags: no */

#define notwx_rx(RG)                                                        \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))

#define notwx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

/* neg (G = -G)
 * set-flags: undefined (*_*), yes (*Z*) */

#define negwx_rx(RG)                                                        \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC2, REG(RG),0x09,0x01))

#define negwx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC2, TMxx,   0x09,0x01))                                 \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))


#define negwxZrx(RG)                                                        \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC2, REG(RG),0x09,0x01))

#define negwxZmx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC2, TMxx,   0x09,0x01))                                 \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

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

#define addwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x1A, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define addwx_mr(MG, DG, RS)                                                \
        addwx_st(W(RS), W(MG), W(DG))


#define addwxZri(RG, IS)                                                    \
        EMIT6(MIM(0xC2, REG(RG),0x09,VAL(IS)))

#define addwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC2, TMxx,   0x09,VAL(IS)))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define addwxZrr(RG, RS)                                                    \
        EMITH(MRM(0x1A, REG(RG),REG(RS),0x00))

#define addwxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x5A, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define addhnZld(RG, MS, DS)    /* add 16-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x7A, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define addhzZld(RG, MS, DS)    /* add 16-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x91, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x1A, REG(RG),TMxx,   0x00))

#define addbnZld(RG, MS, DS)    /* add  8-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x77, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x1A, REG(RG),TMxx,   0x00))

#define addbzZld(RG, MS, DS)    /* add  8-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x90, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x1A, REG(RG),TMxx,   0x00))

#define addwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x1A, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define addwxZmr(MG, DG, RS)                                                \
        addwxZst(W(RS), W(MG), W(DG))

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

#define subwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x1B, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define subwx_mr(MG, DG, RS)                                                \
        subwx_st(W(RS), W(MG), W(DG))


#define subwxZri(RG, IS)                                                    \
        EMIT6(MIM(0xC2, REG(RG),0x09, (-VAL(IS) & 0xFFFFFFFF)))

#define subwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC2, TMxx,   0x09, (-VAL(IS) & 0xFFFFFFFF)))             \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define subwxZrr(RG, RS)                                                    \
        EMITH(MRM(0x1B, REG(RG),REG(RS),0x00))

#define subwxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x5B, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define subhnZld(RG, MS, DS)    /* add 16-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x7B, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define subhzZld(RG, MS, DS)    /* add 16-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x91, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x1B, REG(RG),TMxx,   0x00))

#define subbnZld(RG, MS, DS)    /* add  8-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x77, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x1B, REG(RG),TMxx,   0x00))

#define subbzZld(RG, MS, DS)    /* add  8-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x90, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x1B, REG(RG),TMxx,   0x00))

#define subwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x1B, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define subwxZmr(MG, DG, RS)                                                \
        subwxZst(W(RS), W(MG), W(DG))

/* shl (G = G << S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlwx_rx(RG)                     /* reads Recx for shift count */   \
        EMIT6(MTM(0xDF, REG(RG),TEcx,   0x00))

#define shlwx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDF, TMxx,   TEcx,   0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shlwx_ri(RG, IS)                                                    \
        EMIT6(MTM(0xDF, REG(RG),0x00,VAL(IS)))

#define shlwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDF, TMxx,   0x00,VAL(IS)))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shlwx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMIT6(MTM(0xDF, REG(RG),REG(RS),0x00))

#define shlwx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x58, TDxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MTM(0xDF, REG(RG),TDxx,   0x00))

#define shlwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDF, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shlwx_mr(MG, DG, RS)                                                \
        shlwx_st(W(RS), W(MG), W(DG))

/* shr (G = G >> S), unsigned (logical)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrwx_rx(RG)                     /* reads Recx for shift count */   \
        EMIT6(MTM(0xDE, REG(RG),TEcx,   0x00))

#define shrwx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDE, TMxx,   TEcx,   0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrwx_ri(RG, IS)                                                    \
        EMIT6(MTM(0xDE, REG(RG),0x00,VAL(IS)))

#define shrwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDE, TMxx,   0x00,VAL(IS)))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrwx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMIT6(MTM(0xDE, REG(RG),REG(RS),0x00))

#define shrwx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x58, TDxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MTM(0xDE, REG(RG),TDxx,   0x00))

#define shrwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDE, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrwx_mr(MG, DG, RS)                                                \
        shrwx_st(W(RS), W(MG), W(DG))

/* shr (G = G >> S), signed (arithmetic)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrwn_rx(RG)                     /* reads Recx for shift count */   \
        EMIT6(MTM(0xDC, REG(RG),TEcx,   0x00))

#define shrwn_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDC, TMxx,   TEcx,   0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrwn_ri(RG, IS)                                                    \
        EMIT6(MTM(0xDC, REG(RG),0x00,VAL(IS)))

#define shrwn_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDC, TMxx,   0x00,VAL(IS)))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrwn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMIT6(MTM(0xDC, REG(RG),REG(RS),0x00))

#define shrwn_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x58, TDxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MTM(0xDC, REG(RG),TDxx,   0x00))

#define shrwn_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDC, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrwn_mr(MG, DG, RS)                                                \
        shrwn_st(W(RS), W(MG), W(DG))

/* ror (G = G >> S | G << 32 - S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define rorwx_rx(RG)                     /* reads Recx for shift count */   \
        EMITH(MRM(0x17, TDxx,   TDxx,   0x00))                              \
        EMITH(MRM(0x1B, TDxx,   TEcx,   0x00))                              \
        EMIT6(MTM(0x1D, REG(RG),TDxx,   0x00))

#define rorwx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        negwx_rx(Recx)                                                      \
        EMIT6(MTM(0x1D, TMxx,   TEcx,   0x00))                              \
        negwx_rx(Recx)                                                      \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define rorwx_ri(RG, IS)                                                    \
        EMIT6(MTM(0x1D, REG(RG),0x00,   (32-VAL(IS)) & 0x1F))

#define rorwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x1D, TMxx,   0x00,   (32-VAL(IS)) & 0x1F))               \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define rorwx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITH(MRM(0x17, TDxx,   TDxx,   0x00))                              \
        EMITH(MRM(0x1B, TDxx,   REG(RS),0x00))                              \
        EMIT6(MTM(0x1D, REG(RG),TDxx,   0x00))

#define rorwx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x17, TDxx,   TDxx,   0x00))                              \
        EMITH(MRM(0x1B, TDxx,   TMxx,   0x00))                              \
        EMIT6(MTM(0x1D, REG(RG),TDxx,   0x00))

#define rorwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        negwx_rx(W(RS))                                                     \
        EMIT6(MTM(0x1D, TMxx,   REG(RS),0x00))                              \
        negwx_rx(W(RS))                                                     \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define rorwx_mr(MG, DG, RS)                                                \
        rorwx_st(W(RS), W(MG), W(DG))

/* mul (G = G * S)
 * set-flags: undefined */

#define mulwx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC2, REG(RG),0x01,VAL(IS)))

#define mulwx_rr(RG, RS)                                                    \
        EMITW(MGM(0xFD, REG(RG),REG(RS),REG(RG)))

#define mulwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x51, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define mulhn_ld(RG, MS, DS)  /* mul 32-bit with 16-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x7C, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define mulhz_ld(RG, MS, DS)  /* mul 32-bit with 16-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x91, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0xFD, REG(RG),TMxx,   REG(RG)))

#define mulbn_ld(RG, MS, DS)  /* mul 32-bit with  8-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x77, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0xFD, REG(RG),TMxx,   REG(RG)))

#define mulbz_ld(RG, MS, DS)  /* mul 32-bit with  8-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x90, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0xFD, REG(RG),TMxx,   REG(RG)))


#define mulwx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(MGM(0x96, TEdx,REG(RS),   0x00))

#define mulwx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x96, TEdx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))


#define mulwn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITH(MRM(0x1C, TEdx,REG(RS),   0x00))

#define mulwn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x5C, TEdx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))


#define mulwp_xr(RS)     /* Reax is in/out, prepares Redx for divwn_x* */   \
        mulwx_rr(Reax, W(RS)) /* product must not exceed operands size */

#define mulwp_xm(MS, DS) /* Reax is in/out, prepares Redx for divwn_x* */   \
        mulwx_ld(Reax, W(MS), W(DS))  /* must not exceed operands size */

/* div (G = G / S)
 * set-flags: undefined */

#define divwx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movwx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movwx_rr(Reax, W(RG))                                               \
        prewx_xx()                                                          \
        divwx_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Redx)                                                      \
        movwx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divwx_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movwx_rr(Reax, W(RG))                                               \
        prewx_xx()                                                          \
        divwx_xr(W(RS))                                                     \
        stack_ld(Redx)                                                      \
        movwx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divwx_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movwx_rr(Reax, W(RG))                                               \
        prewx_xx()                                                          \
        divwx_xm(W(MS), W(DS))                                              \
        stack_ld(Redx)                                                      \
        movwx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)


#define divwn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movwx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movwx_rr(Reax, W(RG))                                               \
        prewn_xx()                                                          \
        divwn_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Redx)                                                      \
        movwx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divwn_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movwx_rr(Reax, W(RG))                                               \
        prewn_xx()                                                          \
        divwn_xr(W(RS))                                                     \
        stack_ld(Redx)                                                      \
        movwx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divwn_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movwx_rr(Reax, W(RG))                                               \
        prewn_xx()                                                          \
        divwn_xm(W(MS), W(DS))                                              \
        stack_ld(Redx)                                                      \
        movwx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)


#define prewx_xx()   /* to be placed right before divwx_x* or remwx_xx */   \
        movwx_ri(Redx, IC(0))        /* to prepare Redx for int-divide */

#define prewn_xx()   /* to be placed right before divwn_x* or remwn_xx */   \
        movwx_rr(Redx, Reax)         /* to prepare Redx for int-divide */   \
        shrwn_ri(Redx, IC(31))


#define divwx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(MGM(0x97, TEdx,REG(RS),   0x00))

#define divwx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x97, TEdx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))


#define divwn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITH(MRM(0x1D, TEdx,REG(RS),   0x00))

#define divwn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x1D, TEdx,TMxx,      0x00))


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
        stack_st(Reax)                                                      \
        movwx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movwx_rr(Reax, W(RG))                                               \
        prewx_xx()                                                          \
        divwx_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Reax)                                                      \
        movwx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remwx_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movwx_rr(Reax, W(RG))                                               \
        prewx_xx()                                                          \
        divwx_xr(W(RS))                                                     \
        stack_ld(Reax)                                                      \
        movwx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remwx_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movwx_rr(Reax, W(RG))                                               \
        prewx_xx()                                                          \
        divwx_xm(W(MS), W(DS))                                              \
        stack_ld(Reax)                                                      \
        movwx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)


#define remwn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movwx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movwx_rr(Reax, W(RG))                                               \
        prewn_xx()                                                          \
        divwn_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Reax)                                                      \
        movwx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remwn_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movwx_rr(Reax, W(RG))                                               \
        prewn_xx()                                                          \
        divwn_xr(W(RS))                                                     \
        stack_ld(Reax)                                                      \
        movwx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remwn_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movwx_rr(Reax, W(RG))                                               \
        prewn_xx()                                                          \
        divwn_xm(W(MS), W(DS))                                              \
        stack_ld(Reax)                                                      \
        movwx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)


#define remwx_xx() /* to be placed before divwx_x*, but after prewx_xx */   \
                                     /* to prepare for rem calculation */

#define remwx_xr(RS)        /* to be placed immediately after divwx_xr */   \
                                     /* to produce remainder Redx<-rem */

#define remwx_xm(MS, DS)    /* to be placed immediately after divwx_xm */   \
                                     /* to produce remainder Redx<-rem */


#define remwn_xx() /* to be placed before divwn_x*, but after prewn_xx */   \
                                     /* to prepare for rem calculation */

#define remwn_xr(RS)        /* to be placed immediately after divwn_xr */   \
                                     /* to produce remainder Redx<-rem */

#define remwn_xm(MS, DS)    /* to be placed immediately after divwn_xm */   \
                                     /* to produce remainder Redx<-rem */

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
        EMITW(MGM(0x14, TMxx,   REG(RS),0x00))                              \
        EMIT6(MIM(0xC0, TDxx,   0x01,VAL(IT)))                              \
        CWR(cc, %%r0,   %%r13,   lb)

#define cmjwx_mi(MS, DS, IT, cc, lb)                                        \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x14, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MIM(0xC0, TDxx,   0x01,VAL(IT)))                              \
        CWR(cc, %%r0,   %%r13,   lb)

#define cmjwx_rr(RS, RT, cc, lb)                                            \
        CWR(cc, MOD(RS), MOD(RT), lb)

#define cmjwx_rm(RS, MT, DT, cc, lb)                                        \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMIT6(MDM(0x14, TMxx,   MOD(MT),REG(MT), VAL(DT), B1(DT), P1(DT)))  \
        CWR(cc, MOD(RS), %%r0,   lb)

#define cmjhn_rm(RS, MT, DT, cc, lb)   /* cmj 32/16-bit with sign-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MT),REG(MT), VAL(DT), B1(DT), P1(DT)))  \
        CWR(cc, MOD(RS), %%r0,   lb)

#define cmjhz_rm(RS, MT, DT, cc, lb)   /* cmj 32/16-bit with zero-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMIT6(MDM(0x91, TMxx,   MOD(MT),REG(MT), VAL(DT), B1(DT), P1(DT)))  \
        CWR(cc, MOD(RS), %%r0,   lb)

#define cmjbn_rm(RS, MT, DT, cc, lb)   /* cmj 32/8-bit  with sign-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMIT6(MDM(0x77, TMxx,   MOD(MT),REG(MT), VAL(DT), B1(DT), P1(DT)))  \
        CWR(cc, MOD(RS), %%r0,   lb)

#define cmjbz_rm(RS, MT, DT, cc, lb)   /* cmj 32/8-bit  with zero-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMIT6(MDM(0x90, TMxx,   MOD(MT),REG(MT), VAL(DT), B1(DT), P1(DT)))  \
        CWR(cc, MOD(RS), %%r0,   lb)

#define cmjwx_mr(MS, DS, RT, cc, lb)                                        \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x14, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        CWR(cc, %%r0,   MOD(RT), lb)

#define cmjhn_mr(MS, DS, RT, cc, lb)   /* cmj 16/32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        CWR(cc, %%r0,   MOD(RT), lb)

#define cmjhz_mr(MS, DS, RT, cc, lb)   /* cmj 16/32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x91, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        CWR(cc, %%r0,   MOD(RT), lb)

#define cmjbn_mr(MS, DS, RT, cc, lb)   /* cmj  8/32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x77, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        CWR(cc, %%r0,   MOD(RT), lb)

#define cmjbz_mr(MS, DS, RT, cc, lb)   /* cmj  8/32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x90, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        CWR(cc, %%r0,   MOD(RT), lb)

/* cmp (flags = S ? T)
 * set-flags: yes */

#define cmpwx_ri(RS, IT)                                                    \
        EMITW(MGM(0x14, TLxx,   REG(RS),0x00))                              \
        EMIT6(MIM(0xC0, TRxx,   0x01,VAL(IT)))

#define cmpwx_mi(MS, DS, IT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x14, TLxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MIM(0xC0, TRxx,   0x01,VAL(IT)))

#define cmpwx_rr(RS, RT)                                                    \
        EMITW(MGM(0x14, TRxx,   REG(RT),0x00))                              \
        EMITW(MGM(0x14, TLxx,   REG(RS),0x00))

#define cmpwx_rm(RS, MT, DT)                                                \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMIT6(MDM(0x14, TRxx,   MOD(MT),REG(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(MGM(0x14, TLxx,   REG(RS),0x00))

#define cmphn_rm(RS, MT, DT)    /* cmp 32-bit to 16-bit with sign-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMIT6(MDM(0x15, TRxx,   MOD(MT),REG(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(MGM(0x14, TLxx,   REG(RS),0x00))

#define cmphz_rm(RS, MT, DT)    /* cmp 32-bit to 16-bit with zero-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMIT6(MDM(0x91, TRxx,   MOD(MT),REG(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(MGM(0x14, TLxx,   REG(RS),0x00))

#define cmpbn_rm(RS, MT, DT)    /* cmp 32-bit to  8-bit with sign-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMIT6(MDM(0x77, TRxx,   MOD(MT),REG(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(MGM(0x14, TLxx,   REG(RS),0x00))

#define cmpbz_rm(RS, MT, DT)    /* cmp 32-bit to  8-bit with zero-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMIT6(MDM(0x90, TRxx,   MOD(MT),REG(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(MGM(0x14, TLxx,   REG(RS),0x00))

#define cmpwx_mr(MS, DS, RT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x14, TLxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0x14, TRxx,   REG(RT),0x00))

#define cmphn_mr(MS, DS, RT)    /* cmp 16-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x15, TLxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0x14, TRxx,   REG(RT),0x00))

#define cmphz_mr(MS, DS, RT)    /* cmp 16-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x91, TLxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0x14, TRxx,   REG(RT),0x00))

#define cmpbn_mr(MS, DS, RT)    /* cmp  8-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x77, TLxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0x14, TRxx,   REG(RT),0x00))

#define cmpbz_mr(MS, DS, RT)    /* cmp  8-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x90, TLxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0x14, TRxx,   REG(RT),0x00))

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

/* and (G = G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define andzx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x0B,VAL(IS)))                              \
        EMIT6(MIM(0xC0, REG(RG),0x0A,   0x00))

#define andzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x0B,VAL(IS)))                              \
        EMIT6(MIM(0xC0, TMxx,   0x0A,   0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define andzx_rr(RG, RS)                                                    \
        EMITW(MGM(0x80, REG(RG),REG(RS),0x00))

#define andzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x80, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define andzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0x80, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define andzx_mr(MG, DG, RS)                                                \
        andzx_st(W(RS), W(MG), W(DG))


#define andzxZri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x0A,   0x00))                              \
        EMIT6(MIM(0xC0, REG(RG),0x0B,VAL(IS)))

#define andzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x0A,   0x00))                              \
        EMIT6(MIM(0xC0, TMxx,   0x0B,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define andzxZrr(RG, RS)                                                    \
        EMITW(MGM(0x80, REG(RG),REG(RS),0x00))

#define andzxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x80, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define andzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0x80, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define andzxZmr(MG, DG, RS)                                                \
        andzxZst(W(RS), W(MG), W(DG))

/* ann (G = ~G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define annzx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x06,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x0B,VAL(IS)))                              \
        EMIT6(MIM(0xC0, REG(RG),0x0A,   0x00))

#define annzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x06,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x0B,VAL(IS)))                              \
        EMIT6(MIM(0xC0, TMxx,   0x0A,   0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define annzx_rr(RG, RS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x06,0xFFFFFFFF))                           \
        EMITW(MGM(0x80, REG(RG),REG(RS),0x00))

#define annzx_ld(RG, MS, DS)                                                \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x06,0xFFFFFFFF))                           \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x80, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define annzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x06,0xFFFFFFFF))                           \
        EMITW(MGM(0x80, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define annzx_mr(MG, DG, RS)                                                \
        annzx_st(W(RS), W(MG), W(DG))


#define annzxZri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x0A,   0x00))                              \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x0B,VAL(IS)))                              \

#define annzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x0A,   0x00))                              \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x0B,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define annzxZrr(RG, RS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x06,0xFFFFFFFF))                           \
        EMITW(MGM(0x80, REG(RG),REG(RS),0x00))

#define annzxZld(RG, MS, DS)                                                \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x06,0xFFFFFFFF))                           \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x80, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define annzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x06,0xFFFFFFFF))                           \
        EMITW(MGM(0x80, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define annzxZmr(MG, DG, RS)                                                \
        annzxZst(W(RS), W(MG), W(DG))

/* orr (G = G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define orrzx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x0D,VAL(IS)))

#define orrzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x0D,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define orrzx_rr(RG, RS)                                                    \
        EMITW(MGM(0x81, REG(RG),REG(RS),0x00))

#define orrzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x81, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define orrzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0x81, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define orrzx_mr(MG, DG, RS)                                                \
        orrzx_st(W(RS), W(MG), W(DG))


#define orrzxZri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x0D,VAL(IS)))

#define orrzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x0D,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define orrzxZrr(RG, RS)                                                    \
        EMITW(MGM(0x81, REG(RG),REG(RS),0x00))

#define orrzxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x81, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define orrzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0x81, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define orrzxZmr(MG, DG, RS)                                                \
        orrzxZst(W(RS), W(MG), W(DG))

/* orn (G = ~G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define ornzx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x06,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x0D,VAL(IS)))

#define ornzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x06,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x0D,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define ornzx_rr(RG, RS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x06,0xFFFFFFFF))                           \
        EMITW(MGM(0x81, REG(RG),REG(RS),0x00))

#define ornzx_ld(RG, MS, DS)                                                \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x06,0xFFFFFFFF))                           \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x81, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define ornzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x06,0xFFFFFFFF))                           \
        EMITW(MGM(0x81, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define ornzx_mr(MG, DG, RS)                                                \
        ornzx_st(W(RS), W(MG), W(DG))


#define ornzxZri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x06,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x0D,VAL(IS)))

#define ornzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x06,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x0D,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define ornzxZrr(RG, RS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x06,0xFFFFFFFF))                           \
        EMITW(MGM(0x81, REG(RG),REG(RS),0x00))

#define ornzxZld(RG, MS, DS)                                                \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x06,0xFFFFFFFF))                           \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x81, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define ornzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x06,0xFFFFFFFF))                           \
        EMITW(MGM(0x81, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define ornzxZmr(MG, DG, RS)                                                \
        ornzxZst(W(RS), W(MG), W(DG))

/* xor (G = G ^ S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define xorzx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,VAL(IS)))

#define xorzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define xorzx_rr(RG, RS)                                                    \
        EMITW(MGM(0x82, REG(RG),REG(RS),0x00))

#define xorzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x82, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define xorzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0x82, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define xorzx_mr(MG, DG, RS)                                                \
        xorzx_st(W(RS), W(MG), W(DG))


#define xorzxZri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,VAL(IS)))

#define xorzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define xorzxZrr(RG, RS)                                                    \
        EMITW(MGM(0x82, REG(RG),REG(RS),0x00))

#define xorzxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x82, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define xorzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0x82, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define xorzxZmr(MG, DG, RS)                                                \
        xorzxZst(W(RS), W(MG), W(DG))

/* not (G = ~G)
 * set-flags: no */

#define notzx_rx(RG)                                                        \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x06,0xFFFFFFFF))

#define notzx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x06,0xFFFFFFFF))                           \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

/* neg (G = -G)
 * set-flags: undefined (*_*), yes (*Z*) */

#define negzx_rx(RG)                                                        \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x06,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC2, REG(RG),0x08,0x01))

#define negzx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x06,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC2, TMxx,   0x08,0x01))                                 \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))


#define negzxZrx(RG)                                                        \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x06,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC2, REG(RG),0x08,0x01))

#define negzxZmx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x06,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC2, TMxx,   0x08,0x01))                                 \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

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

#define addzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0x08, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define addzx_mr(MG, DG, RS)                                                \
        addzx_st(W(RS), W(MG), W(DG))


#define addzxZri(RG, IS)                                                    \
        EMIT6(MIM(0xC2, REG(RG),0x08,VAL(IS)))

#define addzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC2, TMxx,   0x08,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define addzxZrr(RG, RS)                                                    \
        EMITW(MGM(0x08, REG(RG),REG(RS),0x00))

#define addzxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x08, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define addwnZld(RG, MS, DS)    /* add 32-bit to 64-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x18, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define addwzZld(RG, MS, DS)    /* add 32-bit to 64-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x1A, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define addzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0x08, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define addzxZmr(MG, DG, RS)                                                \
        addzxZst(W(RS), W(MG), W(DG))

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

#define subzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0x09, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define subzx_mr(MG, DG, RS)                                                \
        subzx_st(W(RS), W(MG), W(DG))


#define subzxZri(RG, IS)                                                    \
        EMIT6(MIM(0xC2, REG(RG),0x04,VAL(IS)))

#define subzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC2, TMxx,   0x04,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define subzxZrr(RG, RS)                                                    \
        EMITW(MGM(0x09, REG(RG),REG(RS),0x00))

#define subzxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x09, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define subwnZld(RG, MS, DS)  /* sub 32-bit from 64-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x19, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define subwzZld(RG, MS, DS)  /* sub 32-bit from 64-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x1B, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define subzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0x09, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define subzxZmr(MG, DG, RS)                                                \
        subzxZst(W(RS), W(MG), W(DG))

/* shl (G = G << S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlzx_rx(RG)                     /* reads Recx for shift count */   \
        EMIT6(MTM(0x0D, REG(RG),TEcx,   0x00))

#define shlzx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x0D, TMxx,   TEcx,   0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shlzx_ri(RG, IS)                                                    \
        EMIT6(MTM(0x0D, REG(RG),0x00,VAL(IS)))

#define shlzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x0D, TMxx,   0x00,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shlzx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMIT6(MTM(0x0D, REG(RG),REG(RS),0x00))

#define shlzx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x04, TDxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MTM(0x0D, REG(RG),TDxx,   0x00))

#define shlzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x0D, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shlzx_mr(MG, DG, RS)                                                \
        shlzx_st(W(RS), W(MG), W(DG))

/* shr (G = G >> S), unsigned (logical)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrzx_rx(RG)                     /* reads Recx for shift count */   \
        EMIT6(MTM(0x0C, REG(RG),TEcx,   0x00))

#define shrzx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x0C, TMxx,   TEcx,   0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrzx_ri(RG, IS)                                                    \
        EMIT6(MTM(0x0C, REG(RG),0x00,VAL(IS)))

#define shrzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x0C, TMxx,   0x00,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrzx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMIT6(MTM(0x0C, REG(RG),REG(RS),0x00))

#define shrzx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x04, TDxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MTM(0x0C, REG(RG),TDxx,   0x00))

#define shrzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x0C, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrzx_mr(MG, DG, RS)                                                \
        shrzx_st(W(RS), W(MG), W(DG))

/* shr (G = G >> S), signed (arithmetic)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrzn_rx(RG)                     /* reads Recx for shift count */   \
        EMIT6(MTM(0x0A, REG(RG),TEcx,   0x00))

#define shrzn_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x0A, TMxx,   TEcx,   0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrzn_ri(RG, IS)                                                    \
        EMIT6(MTM(0x0A, REG(RG),0x00,VAL(IS)))

#define shrzn_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x0A, TMxx,   0x00,VAL(IS)))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrzn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMIT6(MTM(0x0A, REG(RG),REG(RS),0x00))

#define shrzn_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x04, TDxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MTM(0x0A, REG(RG),TDxx,   0x00))

#define shrzn_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x0A, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrzn_mr(MG, DG, RS)                                                \
        shrzn_st(W(RS), W(MG), W(DG))

/* ror (G = G >> S | G << 32 - S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define rorzx_rx(RG)                     /* reads Recx for shift count */   \
        EMITH(MRM(0x17, TDxx,   TDxx,   0x00))                              \
        EMITH(MRM(0x1B, TDxx,   TEcx,   0x00))                              \
        EMIT6(MTM(0x1C, REG(RG),TDxx,   0x00))

#define rorzx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        negwx_rx(Recx)                                                      \
        EMIT6(MTM(0x1C, TMxx,   TEcx,   0x00))                              \
        negwx_rx(Recx)                                                      \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define rorzx_ri(RG, IS)                                                    \
        EMIT6(MTM(0x1C, REG(RG),0x00,   (64-VAL(IS)) & 0x3F))

#define rorzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x1C, TMxx,   0x00,   (64-VAL(IS)) & 0x3F))               \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define rorzx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITH(MRM(0x17, TDxx,   TDxx,   0x00))                              \
        EMITH(MRM(0x1B, TDxx,   REG(RS),0x00))                              \
        EMIT6(MTM(0x1C, REG(RG),TDxx,   0x00))

#define rorzx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x17, TDxx,   TDxx,   0x00))                              \
        EMITH(MRM(0x1B, TDxx,   TMxx,   0x00))                              \
        EMIT6(MTM(0x1C, REG(RG),TDxx,   0x00))

#define rorzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        negwx_rx(W(RS))                                                     \
        EMIT6(MTM(0x1C, TMxx,   REG(RS),0x00))                              \
        negwx_rx(W(RS))                                                     \
        EMIT6(MDM(0x24, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define rorzx_mr(MG, DG, RS)                                                \
        rorzx_st(W(RS), W(MG), W(DG))

/* mul (G = G * S)
 * set-flags: undefined */

#define mulzx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC2, REG(RG),0x00,VAL(IS)))

#define mulzx_rr(RG, RS)                                                    \
        EMITW(MGM(0xED, REG(RG),REG(RS),REG(RG)))

#define mulzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x0C, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define mulwn_ld(RG, MS, DS)  /* mul 64-bit with 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x1C, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define mulwz_ld(RG, MS, DS)  /* mul 64-bit with 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x51, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0xED, REG(RG),TMxx,   REG(RG)))


#define mulzx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(MGM(0x86, TEdx,REG(RS),   0x00))

#define mulzx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x86, TEdx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))


#define mulzn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(MGM(0xEC, TEdx,REG(RS),   TEdx))

#define mulzn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x84, TEdx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))


#define mulzp_xr(RS)     /* Reax is in/out, prepares Redx for divzn_x* */   \
        mulzn_xr(W(RS))       /* product must not exceed operands size */

#define mulzp_xm(MS, DS) /* Reax is in/out, prepares Redx for divzn_x* */   \
        mulzn_xm(W(MS), W(DS))/* product must not exceed operands size */

/* div (G = G / S)
 * set-flags: undefined */

#define divzx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movzx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movzx_rr(Reax, W(RG))                                               \
        prezx_xx()                                                          \
        divzx_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Redx)                                                      \
        movzx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divzx_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezx_xx()                                                          \
        divzx_xr(W(RS))                                                     \
        stack_ld(Redx)                                                      \
        movzx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divzx_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezx_xx()                                                          \
        divzx_xm(W(MS), W(DS))                                              \
        stack_ld(Redx)                                                      \
        movzx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)


#define divzn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movzx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movzx_rr(Reax, W(RG))                                               \
        prezn_xx()                                                          \
        divzn_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Redx)                                                      \
        movzx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divzn_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezn_xx()                                                          \
        divzn_xr(W(RS))                                                     \
        stack_ld(Redx)                                                      \
        movzx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divzn_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezn_xx()                                                          \
        divzn_xm(W(MS), W(DS))                                              \
        stack_ld(Redx)                                                      \
        movzx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)


#define prezx_xx()   /* to be placed right before divzx_x* or remzx_xx */   \
        movzx_ri(Redx, IC(0))        /* to prepare Redx for int-divide */

#define prezn_xx()   /* to be placed right before divzn_x* or remzn_xx */   \
                                     /* to prepare Redx for int-divide */


#define divzx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(MGM(0x87, TEdx,REG(RS),   0x00))

#define divzx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x87, TEdx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))


#define divzn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(MGM(0x0D, TEdx,REG(RS),   0x00))

#define divzn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x0D, TEdx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))


#define divzp_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divzn_xr(W(RS))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divzp_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divzn_xm(W(MS), W(DS))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem (G = G % S)
 * set-flags: undefined */

#define remzx_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movzx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movzx_rr(Reax, W(RG))                                               \
        prezx_xx()                                                          \
        divzx_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Reax)                                                      \
        movzx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remzx_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezx_xx()                                                          \
        divzx_xr(W(RS))                                                     \
        stack_ld(Reax)                                                      \
        movzx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remzx_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezx_xx()                                                          \
        divzx_xm(W(MS), W(DS))                                              \
        stack_ld(Reax)                                                      \
        movzx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)


#define remzn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movzx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movzx_rr(Reax, W(RG))                                               \
        prezn_xx()                                                          \
        divzn_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Reax)                                                      \
        movzx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remzn_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezn_xx()                                                          \
        divzn_xr(W(RS))                                                     \
        stack_ld(Reax)                                                      \
        movzx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remzn_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezn_xx()                                                          \
        divzn_xm(W(MS), W(DS))                                              \
        stack_ld(Reax)                                                      \
        movzx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)


#define remzx_xx() /* to be placed before divzx_x*, but after prezx_xx */   \
                                     /* to prepare for rem calculation */

#define remzx_xr(RS)        /* to be placed immediately after divzx_xr */   \
                                     /* to produce remainder Redx<-rem */

#define remzx_xm(MS, DS)    /* to be placed immediately after divzx_xm */   \
                                     /* to produce remainder Redx<-rem */


#define remzn_xx() /* to be placed before divzn_x*, but after prezn_xx */   \
                                     /* to prepare for rem calculation */

#define remzn_xr(RS)        /* to be placed immediately after divzn_xr */   \
                                     /* to produce remainder Redx<-rem */

#define remzn_xm(MS, DS)    /* to be placed immediately after divzn_xm */   \
                                     /* to produce remainder Redx<-rem */

/* arj (G = G op S, if cc G then jump lb)
 * set-flags: undefined
 * refer to individual instruction descriptions
 * to stay within special register limitations */

     /* Definitions for arj's "op" and "cc" parameters
      * are provided in 32-bit rtarch_***.h files. */

#define arjzx_rx(RG, op, cc, lb)                                            \
        AR1(W(RG), op, z, Zrx)                                              \
        CMJ(cc, lb)

#define arjzx_mx(MG, DG, op, cc, lb)                                        \
        AR2(W(MG), W(DG), op, z, Zmx)                                       \
        CMJ(cc, lb)

#define arjzx_ri(RG, IS, op, cc, lb)                                        \
        AR2(W(RG), W(IS), op, z, Zri)                                       \
        CMJ(cc, lb)

#define arjzx_mi(MG, DG, IS, op, cc, lb)                                    \
        AR3(W(MG), W(DG), W(IS), op, z, Zmi)                                \
        CMJ(cc, lb)

#define arjzx_rr(RG, RS, op, cc, lb)                                        \
        AR2(W(RG), W(RS), op, z, Zrr)                                       \
        CMJ(cc, lb)

#define arjzx_ld(RG, MS, DS, op, cc, lb)                                    \
        AR3(W(RG), W(MS), W(DS), op, z, Zld)                                \
        CMJ(cc, lb)

#define arjzx_st(RS, MG, DG, op, cc, lb)                                    \
        AR3(W(RS), W(MG), W(DG), op, z, Zst)                                \
        CMJ(cc, lb)

#define arjzx_mr(MG, DG, RS, op, cc, lb)                                    \
        arjzx_st(W(RS), W(MG), W(DG), op, cc, lb)

/* cmj (flags = S ? T, if cc flags then jump lb)
 * set-flags: undefined */

     /* Definitions for cmj's "cc" parameter
      * are provided in 32-bit rtarch_***.h files. */

#define cmjzx_rz(RS, cc, lb)                                                \
        cmjzx_ri(W(RS), IC(0), cc, lb)

#define cmjzx_mz(MS, DS, cc, lb)                                            \
        cmjzx_mi(W(MS), W(DS), IC(0), cc, lb)

#define cmjzx_ri(RS, IT, cc, lb)                                            \
        EMIT6(MIM(0xC0, TDxx,   0x01,VAL(IT)))                              \
        CXR(cc, MOD(RS), TDxx,   lb)

#define cmjzx_mi(MS, DS, IT, cc, lb)                                        \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MIM(0xC0, TDxx,   0x01,VAL(IT)))                              \
        CXR(cc, %%r0,   TDxx,    lb)

#define cmjzx_rr(RS, RT, cc, lb)                                            \
        CXR(cc, MOD(RS), MOD(RT), lb)

#define cmjzx_rm(RS, MT, DT, cc, lb)                                        \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MT),REG(MT), VAL(DT), B1(DT), P1(DT)))  \
        CXR(cc, MOD(RS), %%r0,   lb)

#define cmjwn_rm(RS, MT, DT, cc, lb)   /* cmj 64/32-bit with sign-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMIT6(MDM(0x14, TMxx,   MOD(MT),REG(MT), VAL(DT), B1(DT), P1(DT)))  \
        CXR(cc, MOD(RS), %%r0,   lb)

#define cmjwz_rm(RS, MT, DT, cc, lb)   /* cmj 64/32-bit with zero-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMIT6(MDM(0x16, TMxx,   MOD(MT),REG(MT), VAL(DT), B1(DT), P1(DT)))  \
        CXR(cc, MOD(RS), %%r0,   lb)

#define cmjzx_mr(MS, DS, RT, cc, lb)                                        \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x04, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        CXR(cc, %%r0,   MOD(RT), lb)

#define cmjwn_mr(MS, DS, RT, cc, lb)   /* cmj 32/64-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x14, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        CXR(cc, %%r0,   MOD(RT), lb)

#define cmjwz_mr(MS, DS, RT, cc, lb)   /* cmj 32/64-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x16, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        CXR(cc, %%r0,   MOD(RT), lb)

/* cmp (flags = S ? T)
 * set-flags: yes */

#define cmpzx_ri(RS, IT)                                                    \
        EMITW(MGM(0x04, TLxx,   REG(RS),0x00))                              \
        EMIT6(MIM(0xC0, TRxx,   0x01,VAL(IT)))

#define cmpzx_mi(MS, DS, IT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x04, TLxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MIM(0xC0, TRxx,   0x01,VAL(IT)))

#define cmpzx_rr(RS, RT)                                                    \
        EMITW(MGM(0x04, TRxx,   REG(RT),0x00))                              \
        EMITW(MGM(0x04, TLxx,   REG(RS),0x00))

#define cmpzx_rm(RS, MT, DT)                                                \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMIT6(MDM(0x04, TRxx,   MOD(MT),REG(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(MGM(0x04, TLxx,   REG(RS),0x00))

#define cmpwn_rm(RS, MT, DT)    /* cmp 64-bit to 32-bit with sign-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMIT6(MDM(0x14, TRxx,   MOD(MT),REG(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(MGM(0x04, TLxx,   REG(RS),0x00))

#define cmpwz_rm(RS, MT, DT)    /* cmp 64-bit to 32-bit with zero-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMIT6(MDM(0x16, TRxx,   MOD(MT),REG(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(MGM(0x04, TLxx,   REG(RS),0x00))

#define cmpzx_mr(MS, DS, RT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x04, TLxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0x04, TRxx,   REG(RT),0x00))

#define cmpwn_mr(MS, DS, RT)    /* cmp 32-bit to 64-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x14, TLxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0x04, TRxx,   REG(RT),0x00))

#define cmpwz_mr(MS, DS, RT)    /* cmp 32-bit to 64-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x16, TLxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0x04, TRxx,   REG(RT),0x00))

/************************* pointer-sized instructions *************************/

/* jmp (if unconditional jump S/lb, else if cc flags then jump lb)
 * set-flags: no
 * maximum byte-address-range for un/conditional jumps is signed 18/16-bit
 * based on minimum natively-encoded offset across supported targets (u/c)
 * MIPS:18-bit, POWER:26-bit, AArch32:26-bit, AArch64:28-bit, x86:32-bit /
 * MIPS:18-bit, POWER:16-bit, AArch32:26-bit, AArch64:21-bit, x86:32-bit */

#define jmpxx_lb(lb)              /* label-targeted unconditional jump */   \
        ASM_BEG ASM_OP1(j,     lb) ASM_END

#define jezxx_lb(lb)               /* setting-flags-arithmetic -> jump */   \
        ASM_BEG ASM_OP1(je,    lb) ASM_END

#define jnzxx_lb(lb)               /* setting-flags-arithmetic -> jump */   \
        ASM_BEG ASM_OP1(jne,   lb) ASM_END

#define jeqxx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(clgrje,  %%r0, %%r13, lb) ASM_END

#define jnexx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(clgrjne, %%r0, %%r13, lb) ASM_END

#define jltxx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(clgrjl,  %%r0, %%r13, lb) ASM_END

#define jlexx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(clgrjle, %%r0, %%r13, lb) ASM_END

#define jgtxx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(clgrjh,  %%r0, %%r13, lb) ASM_END

#define jgexx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(clgrjhe, %%r0, %%r13, lb) ASM_END

#define jltxn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(cgrjl,   %%r0, %%r13, lb) ASM_END

#define jlexn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(cgrjle,  %%r0, %%r13, lb) ASM_END

#define jgtxn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(cgrjh,   %%r0, %%r13, lb) ASM_END

#define jgexn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP3(cgrjhe,  %%r0, %%r13, lb) ASM_END

#define LBL(lb)                                          /* code label */   \
        ASM_BEG ASM_OP0(lb:) ASM_END

/************************ internal definitions for arj ************************/

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

/************************ internal definitions for cmj ************************/

#define RWJ0(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(crje,    r1, r2, lb) ASM_END

#define RWJ1(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(crjne,   r1, r2, lb) ASM_END

#define RWJ2(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(clrjl,   r1, r2, lb) ASM_END

#define RWJ3(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(clrjle,  r1, r2, lb) ASM_END

#define RWJ4(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(clrjh,   r1, r2, lb) ASM_END

#define RWJ5(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(clrjhe,  r1, r2, lb) ASM_END

#define RWJ6(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(crjl,    r1, r2, lb) ASM_END

#define RWJ7(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(crjle,   r1, r2, lb) ASM_END

#define RWJ8(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(crjh,    r1, r2, lb) ASM_END

#define RWJ9(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(crjhe,   r1, r2, lb) ASM_END

#define CWR(cc, r1, r2, lb)                                                 \
        RW##cc(r1, r2, lb)


#define RXJ0(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(cgrje,   r1, r2, lb) ASM_END

#define RXJ1(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(cgrjne,  r1, r2, lb) ASM_END

#define RXJ2(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(clgrjl,  r1, r2, lb) ASM_END

#define RXJ3(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(clgrjle, r1, r2, lb) ASM_END

#define RXJ4(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(clgrjh,  r1, r2, lb) ASM_END

#define RXJ5(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(clgrjhe, r1, r2, lb) ASM_END

#define RXJ6(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(cgrjl,   r1, r2, lb) ASM_END

#define RXJ7(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(cgrjle,  r1, r2, lb) ASM_END

#define RXJ8(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(cgrjh,   r1, r2, lb) ASM_END

#define RXJ9(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP3(cgrjhe,  r1, r2, lb) ASM_END

#define CXR(cc, r1, r2, lb)                                                 \
        RX##cc(r1, r2, lb)

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

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmix_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        movrs_st(W(XS), W(MD), W(DD))

/***************   packed single-precision generic move/logic   ***************/

/* mov (D = S) */

#define movix_rr(XD, XS)                                                    \
        EMIT6(MXM(0x56, REG(XD),REG(XS),   0x00, 0x00))

#define movix_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, REG(XD),MOD(MS),REG(MS), VAL(DS), B2(DS), P2(DS)))

#define movix_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    REG(MD), VAL(DD), A2(DD), EMPTY2)   \
        EMIT6(MPM(0x0E, REG(XS),MOD(MD),REG(MD), VAL(DD), B2(DD), P2(DD)))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvix_rr(XG, XS)                                                    \
        andix_rr(W(XS), Xmm0)                                               \
        annix_rr(Xmm0, W(XG))                                               \
        orrix_rr(Xmm0, W(XS))                                               \
        movix_rr(W(XG), Xmm0)

#define mmvix_ld(XG, MS, DS)                                                \
        notix_rx(Xmm0)                                                      \
        andix_rr(W(XG), Xmm0)                                               \
        annix_ld(Xmm0, W(MS), W(DS))                                        \
        orrix_rr(W(XG), Xmm0)

#define mmvix_st(XS, MG, DG)                                                \
        andix_rr(W(XS), Xmm0)                                               \
        annix_ld(Xmm0, W(MG), W(DG))                                        \
        orrix_rr(Xmm0, W(XS))                                               \
        movix_st(Xmm0, W(MG), W(DG))

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

/************   packed single-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negis_rx(XG)                                                        \
        xorix_ld(W(XG), Mebp, inf_GPC06_32)

#define negis_rr(XD, XS)                                                    \
        movix_rr(W(XD), W(XS))                                              \
        negis_rx(W(XD))

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

/* sqr (D = sqrt S) */

#define sqris_rr(XD, XS)                                                    \
        EMIT6(MXM(0xCE, REG(XD),REG(XS),   0x00, 0x02))

#define sqris_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MXM(0xCE, REG(XD),TmmM,      0x00, 0x02))

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmais_rr(XG, XS, XT)                                                \
        EMIT6(MVM(0x8F, REG(XG),REG(XS),REG(XT), 0x02))

#define fmais_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MVM(0x8F, REG(XG),REG(XS),   TmmM, 0x02))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsis_rr(XG, XS, XT)                                                \
        EMIT6(MVM(0x9E, REG(XG),REG(XS),REG(XT), 0x02))

#define fmsis_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MVM(0x9E, REG(XG),REG(XS),   TmmM, 0x02))

#endif /* RT_SIMD_COMPAT_FMS */

/*************   packed single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minis_rr(XG, XS)                                                    \
        minis3rr(W(XG), W(XG), W(XS))

#define minis_ld(XG, MS, DS)                                                \
        minis3ld(W(XG), W(XG), W(MS), W(DS))

#define minis3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEE, REG(XD),REG(XS),REG(XT), 0x02))

#define minis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xEE, REG(XD),REG(XS),   TmmM, 0x02))

        /* mnp, mnh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxis_rr(XG, XS)                                                    \
        maxis3rr(W(XG), W(XG), W(XS))

#define maxis_ld(XG, MS, DS)                                                \
        maxis3ld(W(XG), W(XG), W(MS), W(DS))

#define maxis3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEF, REG(XD),REG(XS),REG(XT), 0x02))

#define maxis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xEF, REG(XD),REG(XS),   TmmM, 0x02))

        /* mxp, mxh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

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

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE32_128  MN32_128   /* none satisfy the condition */
#define RT_SIMD_MASK_FULL32_128  MF32_128   /*  all satisfy the condition */

#define S0(mask)    S1(mask)
#define S1(mask)    S##mask

#define SMN32_128(xs, lb) /* not portable, do not use outside */            \
        ASM_BEG ASM_OP3(cgije, %%r0,  0, lb) ASM_END

#define SMF32_128(xs, lb) /* not portable, do not use outside */            \
        ASM_BEG ASM_OP3(cgije, %%r0, -1, lb) ASM_END

#define mkjix_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        EMIT6(MXM(0x94, TmmM,   REG(XS),REG(XS), 0x02))                     \
        EMIT6(MXM(0x21, TMxx,   TmmM,   0x00,    0x03))                     \
        AUW(EMPTY, EMPTY, EMPTY, EMPTY, lb,                                 \
        S0(RT_SIMD_MASK_##mask##32_128), EMPTY2)

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

/************   packed single-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addix_rr(XG, XS)                                                    \
        addix3rr(W(XG), W(XG), W(XS))

#define addix_ld(XG, MS, DS)                                                \
        addix3ld(W(XG), W(XG), W(MS), W(DS))

#define addix3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF3, REG(XD),REG(XS),REG(XT), 0x02))

#define addix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xF3, REG(XD),REG(XS),   TmmM, 0x02))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subix_rr(XG, XS)                                                    \
        subix3rr(W(XG), W(XG), W(XS))

#define subix_ld(XG, MS, DS)                                                \
        subix3ld(W(XG), W(XG), W(MS), W(DS))

#define subix3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF7, REG(XD),REG(XS),REG(XT), 0x02))

#define subix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xF7, REG(XD),REG(XS),   TmmM, 0x02))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulix_rr(XG, XS)                                                    \
        mulix3rr(W(XG), W(XG), W(XS))

#define mulix_ld(XG, MS, DS)                                                \
        mulix3ld(W(XG), W(XG), W(MS), W(DS))

#define mulix3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xA2, REG(XD),REG(XS),REG(XT), 0x02))

#define mulix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xA2, REG(XD),REG(XS),   TmmM, 0x02))

        /* div, rem are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlix_ri(XG, IS)                                                    \
        shlix3ri(W(XG), W(XG), W(IS))

#define shlix_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlix3ld(W(XG), W(XG), W(MS), W(DS))

#define shlix3ri(XD, XS, IT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shlwx_mi(Mebp,  inf_SCR01(0x00), W(IT))                             \
        shlwx_mi(Mebp,  inf_SCR01(0x04), W(IT))                             \
        shlwx_mi(Mebp,  inf_SCR01(0x08), W(IT))                             \
        shlwx_mi(Mebp,  inf_SCR01(0x0C), W(IT))                             \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#define shlix3ld(XD, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  W(MT), W(DT))                                       \
        shlwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shlwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        shlwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shlwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        stack_ld(Recx)                                                      \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrix_ri(XG, IS)                                                    \
        shrix3ri(W(XG), W(XG), W(IS))

#define shrix_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrix3ld(W(XG), W(XG), W(MS), W(DS))

#define shrix3ri(XD, XS, IT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrwx_mi(Mebp,  inf_SCR01(0x00), W(IT))                             \
        shrwx_mi(Mebp,  inf_SCR01(0x04), W(IT))                             \
        shrwx_mi(Mebp,  inf_SCR01(0x08), W(IT))                             \
        shrwx_mi(Mebp,  inf_SCR01(0x0C), W(IT))                             \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#define shrix3ld(XD, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  W(MT), W(DT))                                       \
        shrwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        shrwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        stack_ld(Recx)                                                      \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrin_ri(XG, IS)                                                    \
        shrin3ri(W(XG), W(XG), W(IS))

#define shrin_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrin3ld(W(XG), W(XG), W(MS), W(DS))

#define shrin3ri(XD, XS, IT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrwn_mi(Mebp,  inf_SCR01(0x00), W(IT))                             \
        shrwn_mi(Mebp,  inf_SCR01(0x04), W(IT))                             \
        shrwn_mi(Mebp,  inf_SCR01(0x08), W(IT))                             \
        shrwn_mi(Mebp,  inf_SCR01(0x0C), W(IT))                             \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#define shrin3ld(XD, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  W(MT), W(DT))                                       \
        shrwn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrwn_mx(Mebp,  inf_SCR01(0x04))                                    \
        shrwn_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrwn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        stack_ld(Recx)                                                      \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlix_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlix3rr(W(XG), W(XG), W(XS))

#define svlix_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlix3ld(W(XG), W(XG), W(MS), W(DS))

#define svlix3rr(XD, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svlix_rx(W(XD))

#define svlix3ld(XD, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svlix_rx(W(XD))

#define svlix_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        stack_ld(Recx)                                                      \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrix_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrix3rr(W(XG), W(XG), W(XS))

#define svrix_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrix3ld(W(XG), W(XG), W(MS), W(DS))

#define svrix3rr(XD, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrix_rx(W(XD))

#define svrix3ld(XD, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrix_rx(W(XD))

#define svrix_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        stack_ld(Recx)                                                      \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrin_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrin3rr(W(XG), W(XG), W(XS))

#define svrin_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrin3ld(W(XG), W(XG), W(MS), W(DS))

#define svrin3rr(XD, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrin_rx(W(XD))

#define svrin3ld(XD, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrin_rx(W(XD))

#define svrin_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        stack_ld(Recx)                                                      \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

/****************   packed single-precision integer compare   *****************/

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqix_rr(XG, XS)                                                    \
        ceqix3rr(W(XG), W(XG), W(XS))

#define ceqix_ld(XG, MS, DS)                                                \
        ceqix3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqix3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),REG(XT), 0x02))

#define ceqix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),   TmmM, 0x02))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneix_rr(XG, XS)                                                    \
        cneix3rr(W(XG), W(XG), W(XS))

#define cneix_ld(XG, MS, DS)                                                \
        cneix3ld(W(XG), W(XG), W(MS), W(DS))

#define cneix3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),REG(XT), 0x02))                     \
        notix_rx(W(XD))

#define cneix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),   TmmM, 0x02))                     \
        notix_rx(W(XD))

/*--------------------------------   64-bit   -------------------- 128-bit ---*/

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmjx_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        movts_st(W(XS), W(MD), W(DD))

/***************   packed double-precision generic move/logic   ***************/

/* mov (D = S) */

#define movjx_rr(XD, XS)                                                    \
        EMIT6(MXM(0x56, REG(XD),REG(XS),   0x00, 0x00))

#define movjx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, REG(XD),MOD(MS),REG(MS), VAL(DS), B2(DS), P2(DS)))

#define movjx_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    REG(MD), VAL(DD), A2(DD), EMPTY2)   \
        EMIT6(MPM(0x0E, REG(XS),MOD(MD),REG(MD), VAL(DD), B2(DD), P2(DD)))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvjx_rr(XG, XS)                                                    \
        andjx_rr(W(XS), Xmm0)                                               \
        annjx_rr(Xmm0, W(XG))                                               \
        orrjx_rr(Xmm0, W(XS))                                               \
        movjx_rr(W(XG), Xmm0)

#define mmvjx_ld(XG, MS, DS)                                                \
        notjx_rx(Xmm0)                                                      \
        andjx_rr(W(XG), Xmm0)                                               \
        annjx_ld(Xmm0, W(MS), W(DS))                                        \
        orrjx_rr(W(XG), Xmm0)

#define mmvjx_st(XS, MG, DG)                                                \
        andjx_rr(W(XS), Xmm0)                                               \
        annjx_ld(Xmm0, W(MG), W(DG))                                        \
        orrjx_rr(Xmm0, W(XS))                                               \
        movjx_st(Xmm0, W(MG), W(DG))

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

/************   packed double-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negjs_rx(XG)                                                        \
        xorjx_ld(W(XG), Mebp, inf_GPC06_64)

#define negjs_rr(XD, XS)                                                    \
        movjx_rr(W(XD), W(XS))                                              \
        negjs_rx(W(XD))

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

/* sqr (D = sqrt S) */

#define sqrjs_rr(XD, XS)                                                    \
        EMIT6(MXM(0xCE, REG(XD),REG(XS),   0x00, 0x03))

#define sqrjs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MXM(0xCE, REG(XD),TmmM,      0x00, 0x03))

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmajs_rr(XG, XS, XT)                                                \
        EMIT6(MVM(0x8F, REG(XG),REG(XS),REG(XT), 0x03))

#define fmajs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MVM(0x8F, REG(XG),REG(XS),   TmmM, 0x03))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsjs_rr(XG, XS, XT)                                                \
        EMIT6(MVM(0x9E, REG(XG),REG(XS),REG(XT), 0x03))

#define fmsjs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MVM(0x9E, REG(XG),REG(XS),   TmmM, 0x03))

#endif /* RT_SIMD_COMPAT_FMS */

/*************   packed double-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minjs_rr(XG, XS)                                                    \
        minjs3rr(W(XG), W(XG), W(XS))

#define minjs_ld(XG, MS, DS)                                                \
        minjs3ld(W(XG), W(XG), W(MS), W(DS))

#define minjs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEE, REG(XD),REG(XS),REG(XT), 0x03))

#define minjs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xEE, REG(XD),REG(XS),   TmmM, 0x03))

        /* mnp, mnh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxjs_rr(XG, XS)                                                    \
        maxjs3rr(W(XG), W(XG), W(XS))

#define maxjs_ld(XG, MS, DS)                                                \
        maxjs3ld(W(XG), W(XG), W(MS), W(DS))

#define maxjs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEF, REG(XD),REG(XS),REG(XT), 0x03))

#define maxjs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xEF, REG(XD),REG(XS),   TmmM, 0x03))

        /* mxp, mxh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

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

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE64_128  MN64_128   /* none satisfy the condition */
#define RT_SIMD_MASK_FULL64_128  MF64_128   /*  all satisfy the condition */

#define S0(mask)    S1(mask)
#define S1(mask)    S##mask

#define SMN64_128(xs, lb) /* not portable, do not use outside */            \
        ASM_BEG ASM_OP3(cgije, %%r0,  0, lb) ASM_END

#define SMF64_128(xs, lb) /* not portable, do not use outside */            \
        ASM_BEG ASM_OP3(cgije, %%r0, -1, lb) ASM_END

#define mkjjx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        EMIT6(MXM(0x94, TmmM,   REG(XS),REG(XS), 0x03))                     \
        EMIT6(MXM(0x21, TMxx,   TmmM,   0x00,    0x03))                     \
        AUW(EMPTY, EMPTY, EMPTY, EMPTY, lb,                                 \
        S0(RT_SIMD_MASK_##mask##64_128), EMPTY2)

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

/************   packed double-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addjx_rr(XG, XS)                                                    \
        addjx3rr(W(XG), W(XG), W(XS))

#define addjx_ld(XG, MS, DS)                                                \
        addjx3ld(W(XG), W(XG), W(MS), W(DS))

#define addjx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF3, REG(XD),REG(XS),REG(XT), 0x03))

#define addjx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xF3, REG(XD),REG(XS),   TmmM, 0x03))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subjx_rr(XG, XS)                                                    \
        subjx3rr(W(XG), W(XG), W(XS))

#define subjx_ld(XG, MS, DS)                                                \
        subjx3ld(W(XG), W(XG), W(MS), W(DS))

#define subjx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF7, REG(XD),REG(XS),REG(XT), 0x03))

#define subjx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xF7, REG(XD),REG(XS),   TmmM, 0x03))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define muljx_rr(XG, XS)                                                    \
        muljx3rr(W(XG), W(XG), W(XS))

#define muljx_ld(XG, MS, DS)                                                \
        muljx3ld(W(XG), W(XG), W(MS), W(DS))

#define muljx3rr(XD, XS, XT)                                                \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        muljx_rx(W(XD))

#define muljx3ld(XD, XS, MT, DT)                                            \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_ld(W(XD), W(MT), W(DT))                                       \
        movjx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        muljx_rx(W(XD))

#define muljx_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR01(0x00))                              \
        mulzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        movzx_st(Recx,  Mebp, inf_SCR01(0x00))                              \
        movzx_ld(Recx,  Mebp, inf_SCR01(0x08))                              \
        mulzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        movzx_st(Recx,  Mebp, inf_SCR01(0x08))                              \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

        /* div, rem are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shljx_ri(XG, IS)                                                    \
        shljx3ri(W(XG), W(XG), W(IS))

#define shljx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shljx3ld(W(XG), W(XG), W(MS), W(DS))

#define shljx3ri(XD, XS, IT)                                                \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shlzx_mi(Mebp,  inf_SCR01(0x00), W(IT))                             \
        shlzx_mi(Mebp,  inf_SCR01(0x08), W(IT))                             \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

#define shljx3ld(XD, XS, MT, DT)                                            \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  W(MT), W(DT))                                       \
        shlzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shlzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrjx_ri(XG, IS)                                                    \
        shrjx3ri(W(XG), W(XG), W(IS))

#define shrjx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrjx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrjx3ri(XD, XS, IT)                                                \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrzx_mi(Mebp,  inf_SCR01(0x00), W(IT))                             \
        shrzx_mi(Mebp,  inf_SCR01(0x08), W(IT))                             \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrjx3ld(XD, XS, MT, DT)                                            \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  W(MT), W(DT))                                       \
        shrzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrjn_ri(XG, IS)                                                    \
        shrjn3ri(W(XG), W(XG), W(IS))

#define shrjn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrjn3ld(W(XG), W(XG), W(MS), W(DS))

#define shrjn3ri(XD, XS, IT)                                                \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrzn_mi(Mebp,  inf_SCR01(0x00), W(IT))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x08), W(IT))                             \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrjn3ld(XD, XS, MT, DT)                                            \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  W(MT), W(DT))                                       \
        shrzn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svljx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svljx3rr(W(XG), W(XG), W(XS))

#define svljx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svljx3ld(W(XG), W(XG), W(MS), W(DS))

#define svljx3rr(XD, XS, XT)                                                \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svljx_rx(W(XD))

#define svljx3ld(XD, XS, MT, DT)                                            \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_ld(W(XD), W(MT), W(DT))                                       \
        movjx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svljx_rx(W(XD))

#define svljx_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrjx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrjx3rr(W(XG), W(XG), W(XS))

#define svrjx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrjx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrjx3rr(XD, XS, XT)                                                \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrjx_rx(W(XD))

#define svrjx3ld(XD, XS, MT, DT)                                            \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_ld(W(XD), W(MT), W(DT))                                       \
        movjx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrjx_rx(W(XD))

#define svrjx_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrjn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrjn3rr(W(XG), W(XG), W(XS))

#define svrjn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrjn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrjn3rr(XD, XS, XT)                                                \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrjn_rx(W(XD))

#define svrjn3ld(XD, XS, MT, DT)                                            \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_ld(W(XD), W(MT), W(DT))                                       \
        movjx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrjn_rx(W(XD))

#define svrjn_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

/****************   packed double-precision integer compare   *****************/

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqjx_rr(XG, XS)                                                    \
        ceqjx3rr(W(XG), W(XG), W(XS))

#define ceqjx_ld(XG, MS, DS)                                                \
        ceqjx3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqjx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),REG(XT), 0x03))

#define ceqjx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),   TmmM, 0x03))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnejx_rr(XG, XS)                                                    \
        cnejx3rr(W(XG), W(XG), W(XS))

#define cnejx_ld(XG, MS, DS)                                                \
        cnejx3ld(W(XG), W(XG), W(MS), W(DS))

#define cnejx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),REG(XT), 0x03))                     \
        notjx_rx(W(XD))

#define cnejx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),   TmmM, 0x03))                     \
        notjx_rx(W(XD))

/*--------------------------------   32-bit   -------------------- 256-bit ---*/

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmcx_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        elmix_st(W(XS), W(MD), W(DD))

/***************   packed single-precision generic move/logic   ***************/

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

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvcx_rr(XG, XS)                                                    \
        andcx_rr(W(XS), Xmm0)                                               \
        anncx_rr(Xmm0, W(XG))                                               \
        orrcx_rr(Xmm0, W(XS))                                               \
        movcx_rr(W(XG), Xmm0)

#define mmvcx_ld(XG, MS, DS)                                                \
        notcx_rx(Xmm0)                                                      \
        andcx_rr(W(XG), Xmm0)                                               \
        anncx_ld(Xmm0, W(MS), W(DS))                                        \
        orrcx_rr(W(XG), Xmm0)

#define mmvcx_st(XS, MG, DG)                                                \
        andcx_rr(W(XS), Xmm0)                                               \
        anncx_ld(Xmm0, W(MG), W(DG))                                        \
        orrcx_rr(Xmm0, W(XS))                                               \
        movcx_st(Xmm0, W(MG), W(DG))

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

/************   packed single-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negcs_rx(XG)                                                        \
        xorcx_ld(W(XG), Mebp, inf_GPC06_32)

#define negcs_rr(XD, XS)                                                    \
        movcx_rr(W(XD), W(XS))                                              \
        negcs_rx(W(XD))

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

/* sqr (D = sqrt S) */

#define sqrcs_rr(XD, XS)                                                    \
        EMIT6(MXM(0xCE, REG(XD),REG(XS),   0x00, 0x02))                     \
        EMIT6(MXM(0xCE, RYG(XD),RYG(XS),   0x00, 0x02))

#define sqrcs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MXM(0xCE, REG(XD),TmmM,      0x00, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MXM(0xCE, RYG(XD),TmmM,      0x00, 0x02))

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmacs_rr(XG, XS, XT)                                                \
        EMIT6(MVM(0x8F, REG(XG),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MVM(0x8F, RYG(XG),RYG(XS),RYG(XT), 0x02))

#define fmacs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MVM(0x8F, REG(XG),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MVM(0x8F, RYG(XG),RYG(XS),   TmmM, 0x02))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmscs_rr(XG, XS, XT)                                                \
        EMIT6(MVM(0x9E, REG(XG),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MVM(0x9E, RYG(XG),RYG(XS),RYG(XT), 0x02))

#define fmscs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MVM(0x9E, REG(XG),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MVM(0x9E, RYG(XG),RYG(XS),   TmmM, 0x02))

#endif /* RT_SIMD_COMPAT_FMS */

/*************   packed single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define mincs_rr(XG, XS)                                                    \
        mincs3rr(W(XG), W(XG), W(XS))

#define mincs_ld(XG, MS, DS)                                                \
        mincs3ld(W(XG), W(XG), W(MS), W(DS))

#define mincs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEE, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xEE, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define mincs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEE, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEE, RYG(XD),RYG(XS),   TmmM, 0x02))

        /* mnp, mnh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxcs_rr(XG, XS)                                                    \
        maxcs3rr(W(XG), W(XG), W(XS))

#define maxcs_ld(XG, MS, DS)                                                \
        maxcs3ld(W(XG), W(XG), W(MS), W(DS))

#define maxcs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEF, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xEF, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define maxcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEF, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEF, RYG(XD),RYG(XS),   TmmM, 0x02))

        /* mxp, mxh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

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

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE32_256  MN32_256   /* none satisfy the condition */
#define RT_SIMD_MASK_FULL32_256  MF32_256   /*  all satisfy the condition */

#define S0(mask)    S1(mask)
#define S1(mask)    S##mask

#define SMN32_256(xs, lb) /* not portable, do not use outside */            \
        ASM_BEG ASM_OP3(cgije, %%r0,  0, lb) ASM_END

#define SMF32_256(xs, lb) /* not portable, do not use outside */            \
        ASM_BEG ASM_OP3(cgije, %%r0, -1, lb) ASM_END

#define mkjcx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        EMIT6(MXM(0x94, TmmM,   REG(XS),RYG(XS), 0x02))                     \
        EMIT6(MXM(0x94, TmmM,   TmmM,   TmmM,    0x02))                     \
        EMIT6(MXM(0x21, TMxx,   TmmM,   0x00,    0x03))                     \
        AUW(EMPTY, EMPTY, EMPTY, EMPTY, lb,                                 \
        S0(RT_SIMD_MASK_##mask##32_256), EMPTY2)

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

/************   packed single-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addcx_rr(XG, XS)                                                    \
        addcx3rr(W(XG), W(XG), W(XS))

#define addcx_ld(XG, MS, DS)                                                \
        addcx3ld(W(XG), W(XG), W(MS), W(DS))

#define addcx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF3, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xF3, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define addcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF3, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF3, RYG(XD),RYG(XS),   TmmM, 0x02))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subcx_rr(XG, XS)                                                    \
        subcx3rr(W(XG), W(XG), W(XS))

#define subcx_ld(XG, MS, DS)                                                \
        subcx3ld(W(XG), W(XG), W(MS), W(DS))

#define subcx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF7, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xF7, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define subcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF7, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF7, RYG(XD),RYG(XS),   TmmM, 0x02))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulcx_rr(XG, XS)                                                    \
        mulcx3rr(W(XG), W(XG), W(XS))

#define mulcx_ld(XG, MS, DS)                                                \
        mulcx3ld(W(XG), W(XG), W(MS), W(DS))

#define mulcx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xA2, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xA2, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define mulcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xA2, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xA2, RYG(XD),RYG(XS),   TmmM, 0x02))

        /* div, rem are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlcx_ri(XG, IS)                                                    \
        shlcx3ri(W(XG), W(XG), W(IS))

#define shlcx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlcx3ld(W(XG), W(XG), W(MS), W(DS))

#define shlcx3ri(XD, XS, IT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shlwx_mi(Mebp,  inf_SCR01(0x00), W(IT))                             \
        shlwx_mi(Mebp,  inf_SCR01(0x04), W(IT))                             \
        shlwx_mi(Mebp,  inf_SCR01(0x08), W(IT))                             \
        shlwx_mi(Mebp,  inf_SCR01(0x0C), W(IT))                             \
        shlwx_mi(Mebp,  inf_SCR01(0x10), W(IT))                             \
        shlwx_mi(Mebp,  inf_SCR01(0x14), W(IT))                             \
        shlwx_mi(Mebp,  inf_SCR01(0x18), W(IT))                             \
        shlwx_mi(Mebp,  inf_SCR01(0x1C), W(IT))                             \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

#define shlcx3ld(XD, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  W(MT), W(DT))                                       \
        shlwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shlwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        shlwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shlwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        shlwx_mx(Mebp,  inf_SCR01(0x10))                                    \
        shlwx_mx(Mebp,  inf_SCR01(0x14))                                    \
        shlwx_mx(Mebp,  inf_SCR01(0x18))                                    \
        shlwx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        stack_ld(Recx)                                                      \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrcx_ri(XG, IS)                                                    \
        shrcx3ri(W(XG), W(XG), W(IS))

#define shrcx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrcx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrcx3ri(XD, XS, IT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrwx_mi(Mebp,  inf_SCR01(0x00), W(IT))                             \
        shrwx_mi(Mebp,  inf_SCR01(0x04), W(IT))                             \
        shrwx_mi(Mebp,  inf_SCR01(0x08), W(IT))                             \
        shrwx_mi(Mebp,  inf_SCR01(0x0C), W(IT))                             \
        shrwx_mi(Mebp,  inf_SCR01(0x10), W(IT))                             \
        shrwx_mi(Mebp,  inf_SCR01(0x14), W(IT))                             \
        shrwx_mi(Mebp,  inf_SCR01(0x18), W(IT))                             \
        shrwx_mi(Mebp,  inf_SCR01(0x1C), W(IT))                             \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrcx3ld(XD, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  W(MT), W(DT))                                       \
        shrwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        shrwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        shrwx_mx(Mebp,  inf_SCR01(0x10))                                    \
        shrwx_mx(Mebp,  inf_SCR01(0x14))                                    \
        shrwx_mx(Mebp,  inf_SCR01(0x18))                                    \
        shrwx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        stack_ld(Recx)                                                      \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrcn_ri(XG, IS)                                                    \
        shrcn3ri(W(XG), W(XG), W(IS))

#define shrcn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrcn3ld(W(XG), W(XG), W(MS), W(DS))

#define shrcn3ri(XD, XS, IT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrwn_mi(Mebp,  inf_SCR01(0x00), W(IT))                             \
        shrwn_mi(Mebp,  inf_SCR01(0x04), W(IT))                             \
        shrwn_mi(Mebp,  inf_SCR01(0x08), W(IT))                             \
        shrwn_mi(Mebp,  inf_SCR01(0x0C), W(IT))                             \
        shrwn_mi(Mebp,  inf_SCR01(0x10), W(IT))                             \
        shrwn_mi(Mebp,  inf_SCR01(0x14), W(IT))                             \
        shrwn_mi(Mebp,  inf_SCR01(0x18), W(IT))                             \
        shrwn_mi(Mebp,  inf_SCR01(0x1C), W(IT))                             \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrcn3ld(XD, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  W(MT), W(DT))                                       \
        shrwn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrwn_mx(Mebp,  inf_SCR01(0x04))                                    \
        shrwn_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrwn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        shrwn_mx(Mebp,  inf_SCR01(0x10))                                    \
        shrwn_mx(Mebp,  inf_SCR01(0x14))                                    \
        shrwn_mx(Mebp,  inf_SCR01(0x18))                                    \
        shrwn_mx(Mebp,  inf_SCR01(0x1C))                                    \
        stack_ld(Recx)                                                      \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlcx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlcx3rr(W(XG), W(XG), W(XS))

#define svlcx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlcx3ld(W(XG), W(XG), W(MS), W(DS))

#define svlcx3rr(XD, XS, XT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svlcx_rx(W(XD))

#define svlcx3ld(XD, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_ld(W(XD), W(MT), W(DT))                                       \
        movcx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svlcx_rx(W(XD))

#define svlcx_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        stack_ld(Recx)                                                      \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrcx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrcx3rr(W(XG), W(XG), W(XS))

#define svrcx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrcx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrcx3rr(XD, XS, XT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrcx_rx(W(XD))

#define svrcx3ld(XD, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_ld(W(XD), W(MT), W(DT))                                       \
        movcx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrcx_rx(W(XD))

#define svrcx_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        stack_ld(Recx)                                                      \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrcn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrcn3rr(W(XG), W(XG), W(XS))

#define svrcn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrcn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrcn3rr(XD, XS, XT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrcn_rx(W(XD))

#define svrcn3ld(XD, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_ld(W(XD), W(MT), W(DT))                                       \
        movcx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrcn_rx(W(XD))

#define svrcn_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x10))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x14))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x18))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x1C))                                    \
        stack_ld(Recx)                                                      \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

/****************   packed single-precision integer compare   *****************/

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqcx_rr(XG, XS)                                                    \
        ceqcx3rr(W(XG), W(XG), W(XS))

#define ceqcx_ld(XG, MS, DS)                                                \
        ceqcx3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqcx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xF8, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define ceqcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF8, RYG(XD),RYG(XS),   TmmM, 0x02))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnecx_rr(XG, XS)                                                    \
        cnecx3rr(W(XG), W(XG), W(XS))

#define cnecx_ld(XG, MS, DS)                                                \
        cnecx3ld(W(XG), W(XG), W(MS), W(DS))

#define cnecx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xF8, RYG(XD),RYG(XS),RYG(XT), 0x02))                     \
        notcx_rx(W(XD))

#define cnecx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF8, RYG(XD),RYG(XS),   TmmM, 0x02))                     \
        notcx_rx(W(XD))

/*--------------------------------   64-bit   -------------------- 256-bit ---*/

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmdx_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        elmjx_st(W(XS), W(MD), W(DD))

/***************   packed double-precision generic move/logic   ***************/

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

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvdx_rr(XG, XS)                                                    \
        anddx_rr(W(XS), Xmm0)                                               \
        anndx_rr(Xmm0, W(XG))                                               \
        orrdx_rr(Xmm0, W(XS))                                               \
        movdx_rr(W(XG), Xmm0)

#define mmvdx_ld(XG, MS, DS)                                                \
        notdx_rx(Xmm0)                                                      \
        anddx_rr(W(XG), Xmm0)                                               \
        anndx_ld(Xmm0, W(MS), W(DS))                                        \
        orrdx_rr(W(XG), Xmm0)

#define mmvdx_st(XS, MG, DG)                                                \
        anddx_rr(W(XS), Xmm0)                                               \
        anndx_ld(Xmm0, W(MG), W(DG))                                        \
        orrdx_rr(Xmm0, W(XS))                                               \
        movdx_st(Xmm0, W(MG), W(DG))

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

/************   packed double-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negds_rx(XG)                                                        \
        xordx_ld(W(XG), Mebp, inf_GPC06_64)

#define negds_rr(XD, XS)                                                    \
        movdx_rr(W(XD), W(XS))                                              \
        negds_rx(W(XD))

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

/* sqr (D = sqrt S) */

#define sqrds_rr(XD, XS)                                                    \
        EMIT6(MXM(0xCE, REG(XD),REG(XS),   0x00, 0x03))                     \
        EMIT6(MXM(0xCE, RYG(XD),RYG(XS),   0x00, 0x03))

#define sqrds_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MXM(0xCE, REG(XD),TmmM,      0x00, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MXM(0xCE, RYG(XD),TmmM,      0x00, 0x03))

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmads_rr(XG, XS, XT)                                                \
        EMIT6(MVM(0x8F, REG(XG),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MVM(0x8F, RYG(XG),RYG(XS),RYG(XT), 0x03))

#define fmads_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MVM(0x8F, REG(XG),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MVM(0x8F, RYG(XG),RYG(XS),   TmmM, 0x03))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsds_rr(XG, XS, XT)                                                \
        EMIT6(MVM(0x9E, REG(XG),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MVM(0x9E, RYG(XG),RYG(XS),RYG(XT), 0x03))

#define fmsds_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MVM(0x9E, REG(XG),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MVM(0x9E, RYG(XG),RYG(XS),   TmmM, 0x03))

#endif /* RT_SIMD_COMPAT_FMS */

/*************   packed double-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minds_rr(XG, XS)                                                    \
        minds3rr(W(XG), W(XG), W(XS))

#define minds_ld(XG, MS, DS)                                                \
        minds3ld(W(XG), W(XG), W(MS), W(DS))

#define minds3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEE, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xEE, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define minds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEE, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEE, RYG(XD),RYG(XS),   TmmM, 0x03))

        /* mnp, mnh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxds_rr(XG, XS)                                                    \
        maxds3rr(W(XG), W(XG), W(XS))

#define maxds_ld(XG, MS, DS)                                                \
        maxds3ld(W(XG), W(XG), W(MS), W(DS))

#define maxds3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEF, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xEF, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define maxds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEF, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEF, RYG(XD),RYG(XS),   TmmM, 0x03))

        /* mxp, mxh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

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

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE64_256  MN64_256   /* none satisfy the condition */
#define RT_SIMD_MASK_FULL64_256  MF64_256   /*  all satisfy the condition */

#define S0(mask)    S1(mask)
#define S1(mask)    S##mask

#define SMN64_256(xs, lb) /* not portable, do not use outside */            \
        ASM_BEG ASM_OP3(cgije, %%r0,  0, lb) ASM_END

#define SMF64_256(xs, lb) /* not portable, do not use outside */            \
        ASM_BEG ASM_OP3(cgije, %%r0, -1, lb) ASM_END

#define mkjdx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        EMIT6(MXM(0x94, TmmM,   REG(XS),RYG(XS), 0x03))                     \
        EMIT6(MXM(0x94, TmmM,   TmmM,   TmmM,    0x03))                     \
        EMIT6(MXM(0x21, TMxx,   TmmM,   0x00,    0x03))                     \
        AUW(EMPTY, EMPTY, EMPTY, EMPTY, lb,                                 \
        S0(RT_SIMD_MASK_##mask##64_256), EMPTY2)

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

/************   packed double-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define adddx_rr(XG, XS)                                                    \
        adddx3rr(W(XG), W(XG), W(XS))

#define adddx_ld(XG, MS, DS)                                                \
        adddx3ld(W(XG), W(XG), W(MS), W(DS))

#define adddx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF3, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xF3, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define adddx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF3, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF3, RYG(XD),RYG(XS),   TmmM, 0x03))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subdx_rr(XG, XS)                                                    \
        subdx3rr(W(XG), W(XG), W(XS))

#define subdx_ld(XG, MS, DS)                                                \
        subdx3ld(W(XG), W(XG), W(MS), W(DS))

#define subdx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF7, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xF7, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define subdx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF7, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF7, RYG(XD),RYG(XS),   TmmM, 0x03))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define muldx_rr(XG, XS)                                                    \
        muldx3rr(W(XG), W(XG), W(XS))

#define muldx_ld(XG, MS, DS)                                                \
        muldx3ld(W(XG), W(XG), W(MS), W(DS))

#define muldx3rr(XD, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        muldx_rx(W(XD))

#define muldx3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), W(MT), W(DT))                                       \
        movdx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        muldx_rx(W(XD))

#define muldx_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR01(0x00))                              \
        mulzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        movzx_st(Recx,  Mebp, inf_SCR01(0x00))                              \
        movzx_ld(Recx,  Mebp, inf_SCR01(0x08))                              \
        mulzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        movzx_st(Recx,  Mebp, inf_SCR01(0x08))                              \
        movzx_ld(Recx,  Mebp, inf_SCR01(0x10))                              \
        mulzx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        movzx_st(Recx,  Mebp, inf_SCR01(0x10))                              \
        movzx_ld(Recx,  Mebp, inf_SCR01(0x18))                              \
        mulzx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        movzx_st(Recx,  Mebp, inf_SCR01(0x18))                              \
        stack_ld(Recx)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

        /* div, rem are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shldx_ri(XG, IS)                                                    \
        shldx3ri(W(XG), W(XG), W(IS))

#define shldx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shldx3ld(W(XG), W(XG), W(MS), W(DS))

#define shldx3ri(XD, XS, IT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shlzx_mi(Mebp,  inf_SCR01(0x00), W(IT))                             \
        shlzx_mi(Mebp,  inf_SCR01(0x08), W(IT))                             \
        shlzx_mi(Mebp,  inf_SCR01(0x10), W(IT))                             \
        shlzx_mi(Mebp,  inf_SCR01(0x18), W(IT))                             \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

#define shldx3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  W(MT), W(DT))                                       \
        shlzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shlzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shlzx_mx(Mebp,  inf_SCR01(0x10))                                    \
        shlzx_mx(Mebp,  inf_SCR01(0x18))                                    \
        stack_ld(Recx)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrdx_ri(XG, IS)                                                    \
        shrdx3ri(W(XG), W(XG), W(IS))

#define shrdx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrdx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrdx3ri(XD, XS, IT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrzx_mi(Mebp,  inf_SCR01(0x00), W(IT))                             \
        shrzx_mi(Mebp,  inf_SCR01(0x08), W(IT))                             \
        shrzx_mi(Mebp,  inf_SCR01(0x10), W(IT))                             \
        shrzx_mi(Mebp,  inf_SCR01(0x18), W(IT))                             \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrdx3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  W(MT), W(DT))                                       \
        shrzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrzx_mx(Mebp,  inf_SCR01(0x10))                                    \
        shrzx_mx(Mebp,  inf_SCR01(0x18))                                    \
        stack_ld(Recx)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrdn_ri(XG, IS)                                                    \
        shrdn3ri(W(XG), W(XG), W(IS))

#define shrdn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrdn3ld(W(XG), W(XG), W(MS), W(DS))

#define shrdn3ri(XD, XS, IT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrzn_mi(Mebp,  inf_SCR01(0x00), W(IT))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x08), W(IT))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x10), W(IT))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x18), W(IT))                             \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrdn3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  W(MT), W(DT))                                       \
        shrzn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x10))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x18))                                    \
        stack_ld(Recx)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svldx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svldx3rr(W(XG), W(XG), W(XS))

#define svldx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svldx3ld(W(XG), W(XG), W(MS), W(DS))

#define svldx3rr(XD, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svldx_rx(W(XD))

#define svldx3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), W(MT), W(DT))                                       \
        movdx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svldx_rx(W(XD))

#define svldx_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x18))                                    \
        stack_ld(Recx)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrdx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrdx3rr(W(XG), W(XG), W(XS))

#define svrdx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrdx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrdx3rr(XD, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrdx_rx(W(XD))

#define svrdx3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), W(MT), W(DT))                                       \
        movdx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrdx_rx(W(XD))

#define svrdx_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x18))                                    \
        stack_ld(Recx)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrdn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrdn3rr(W(XG), W(XG), W(XS))

#define svrdn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrdn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrdn3rr(XD, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrdn_rx(W(XD))

#define svrdn3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), W(MT), W(DT))                                       \
        movdx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrdn_rx(W(XD))

#define svrdn_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x08))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x10))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x18))                                    \
        stack_ld(Recx)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

/****************   packed double-precision integer compare   *****************/

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqdx_rr(XG, XS)                                                    \
        ceqdx3rr(W(XG), W(XG), W(XS))

#define ceqdx_ld(XG, MS, DS)                                                \
        ceqdx3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqdx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xF8, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define ceqdx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF8, RYG(XD),RYG(XS),   TmmM, 0x03))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnedx_rr(XG, XS)                                                    \
        cnedx3rr(W(XG), W(XG), W(XS))

#define cnedx_ld(XG, MS, DS)                                                \
        cnedx3ld(W(XG), W(XG), W(MS), W(DS))

#define cnedx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xF8, RYG(XD),RYG(XS),RYG(XT), 0x03))                     \
        notdx_rx(W(XD))

#define cnedx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF8, RYG(XD),RYG(XS),   TmmM, 0x03))                     \
        notdx_rx(W(XD))

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

/* sqr (D = sqrt S) */

#define sqrrs_rr(XD, XS)                                                    \
        EMIT6(MWM(0xCE, REG(XD),REG(XS),   0x00, 0x02))

#define sqrrs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MWM(0xCE, REG(XD),TmmM,      0x00, 0x02))

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmars_rr(XG, XS, XT)                                                \
        EMIT6(MUM(0x8F, REG(XG),REG(XS),REG(XT), 0x02))

#define fmars_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MUM(0x8F, REG(XG),REG(XS),   TmmM, 0x02))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsrs_rr(XG, XS, XT)                                                \
        EMIT6(MUM(0x9E, REG(XG),REG(XS),REG(XT), 0x02))

#define fmsrs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MUM(0x9E, REG(XG),REG(XS),   TmmM, 0x02))

#endif /* RT_SIMD_COMPAT_FMS */

/*************   scalar single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minrs_rr(XG, XS)                                                    \
        minrs3rr(W(XG), W(XG), W(XS))

#define minrs_ld(XG, MS, DS)                                                \
        minrs3ld(W(XG), W(XG), W(MS), W(DS))

#define minrs3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xEE, REG(XD),REG(XS),REG(XT), 0x02))

#define minrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xEE, REG(XD),REG(XS),   TmmM, 0x02))

        /* mnp, mnh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxrs_rr(XG, XS)                                                    \
        maxrs3rr(W(XG), W(XG), W(XS))

#define maxrs_ld(XG, MS, DS)                                                \
        maxrs3ld(W(XG), W(XG), W(MS), W(DS))

#define maxrs3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xEF, REG(XD),REG(XS),REG(XT), 0x02))

#define maxrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x03, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xEF, REG(XD),REG(XS),   TmmM, 0x02))

        /* mxp, mxh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

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

/* sqr (D = sqrt S) */

#define sqrts_rr(XD, XS)                                                    \
        EMIT6(MWM(0xCE, REG(XD),REG(XS),   0x00, 0x03))

#define sqrts_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MWM(0xCE, REG(XD),TmmM,      0x00, 0x03))

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmats_rr(XG, XS, XT)                                                \
        EMIT6(MUM(0x8F, REG(XG),REG(XS),REG(XT), 0x03))

#define fmats_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MUM(0x8F, REG(XG),REG(XS),   TmmM, 0x03))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsts_rr(XG, XS, XT)                                                \
        EMIT6(MUM(0x9E, REG(XG),REG(XS),REG(XT), 0x03))

#define fmsts_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MUM(0x9E, REG(XG),REG(XS),   TmmM, 0x03))

#endif /* RT_SIMD_COMPAT_FMS */

/*************   scalar double-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define mints_rr(XG, XS)                                                    \
        mints3rr(W(XG), W(XG), W(XS))

#define mints_ld(XG, MS, DS)                                                \
        mints3ld(W(XG), W(XG), W(MS), W(DS))

#define mints3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xEE, REG(XD),REG(XS),REG(XT), 0x03))

#define mints3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xEE, REG(XD),REG(XS),   TmmM, 0x03))

        /* mnp, mnh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxts_rr(XG, XS)                                                    \
        maxts3rr(W(XG), W(XG), W(XS))

#define maxts_ld(XG, MS, DS)                                                \
        maxts3ld(W(XG), W(XG), W(MS), W(DS))

#define maxts3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xEF, REG(XD),REG(XS),REG(XT), 0x03))

#define maxts3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x02, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xEF, REG(XD),REG(XS),   TmmM, 0x03))

        /* mxp, mxh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

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

#define fpscr_ld(RS) /* not portable, do not use outside */                 \
        EMITW(0xB3840000 | MRM(0x00, REG(RS),0x00,0x00))

#define fpscr_st(RD) /* not portable, do not use outside */                 \
        EMITW(0xB38C0000 | MRM(0x00, REG(RD),0x00,0x00))

#define FCTRL_SET(mode)   /* sets given mode into fp control register */    \
        EMITW(0xB2990000 | (RT_SIMD_MODE_##mode & 3))

#define FCTRL_RESET()     /* resumes default mode (ROUNDN) upon leave */    \
        EMITW(0xB2990000)

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

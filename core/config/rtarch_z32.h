/******************************************************************************/
/* Copyright (c) 2013-2026 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_Z32_H
#define RT_RTARCH_Z32_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_z32.h: Implementation of s390x BASE+SIMD instructions.
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

/* enable 14 BASE regs, dropping scaled-indexed and far-reach addressing modes
 * along with some rare instructions like: not_rx/mx, shl/shr_ld, ror_rx/rr/ld,
 * cmj_ri/mi, cmp/jeq/jne/jlt/jle/jgt/jge (jmp and arj with jez/jnz are allowed
 * except restricted forms), above 20-bit BASE and 12-bit SIMD are far-reach */
#define RT_FULL_BASE  0

/* structural */

#define MRM(cod, reg, ren, rem) /* arith32 */                               \
        ((reg) << 4 | (ren) | (rem) << 12 | (cod) << 8)

#define MGM(cod, reg, ren, rem) /* arith64 */                               \
        ((reg) << 4 | (ren) | (rem) << 12 | (0xB900 | (cod)) << 16)

#define MIM(cod, reg, ren, imm) /* arithIM */                               \
        ((reg) << 36 | (ren) << 32 | (imm) | (cod) << 40)

#define MHM(cod, reg, ren, imm) /* arithIH */                               \
        ((reg) << 20 | (ren) << 16 | (imm & 0xFFFF) | (cod) << 24)

#define MBM(cod, reg, ren, imm) /* arithIB */                               \
        ((reg) << 20 | (ren) << 16 | (imm & 0x00FF) | (cod) << 24)

#define MCM(cod, reg, ren, imm) /* arithIC */                               \
        ((reg) << 20 | (ren) << 16 | (imm & 0x00FF) | (cod) << 24 |         \
        (((imm & 0x0080) != 0) & 0xFF00))

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

#define MQM(cod, reg, ren, brm, vdp, bxx, pxx)                              \
        (pxx(vdp) | bxx(brm) << 28 | (reg & 0x0F) << 36 |                   \
         (ren & 0x0F) << 32 | (ren & 0x10) <<  6 |                          \
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
#if RT_FULL_BASE
#define TEgD    0x0D  /* r13 */
#define TEgE    0x0E  /* r14 */
#endif /* RT_FULL_BASE */

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

#define TmmF    0x1E  /* v30 */
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
#define TmmV    0x1A  /* v26 */
#define TmmR    0x1B  /* v27 */
#define TmmS    0x1C  /* v28 */
#define TmmT    0x1D  /* v29 */

#define TmmQ    0x0F  /* v15, all ones */
#define TmmM    0x1F  /* v31 */

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

#if (defined RT_Z64)

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
#if RT_FULL_BASE
#define RegD    TEgD, %%r13,EMPTY
#define RegE    TEgE, %%r14,EMPTY
#endif /* RT_FULL_BASE */

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
#if RT_FULL_BASE
#define MegD    TEgD, 0x00, EMPTY
#define MegE    TEgE, 0x00, EMPTY
#endif /* RT_FULL_BASE */

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
#if RT_FULL_BASE
#define SegD(R) TEgD,REG(R),EMPTY
#define SegE(R) TEgE,REG(R),EMPTY
#endif /* RT_FULL_BASE */

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

/* triplet pass-through wrapper */

#define W(p1, p2, p3)       p1,  p2,  p3

/******************************************************************************/
/**********************************   BASE   **********************************/
/******************************************************************************/

/* mov (D = S)
 * set-flags: no */

#define movwx_ri(RD, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RD),0x0F,VAL(IS)))

#define movwx_mi(MD, DD, IS)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    REG(MD), VAL(DD), A1(DD), EMPTY2)   \
        EMIT6(MIM(0xC2, SPxx,0x04,0x08)) /* SP -= 8 */                      \
        EMIT6(0xE30000000024 | MSM(TMxx,   SPxx,0x00))                      \
        EMIT6(MIM(0xC0, TMxx,   0x01,VAL(IS)))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MD),REG(MD), VAL(DD), B1(DD), P1(DD)))  \
        EMIT6(0xE30000000004 | MSM(TMxx,   SPxx,0x00))                      \
        EMIT6(MIM(0xC2, SPxx,0x08,0x08)) /* SP += 8 */

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
        EMIT6(MIM(0xC0, REG(RG),0x0B,VAL(IS)))

#define andwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x0B,VAL(IS)))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define andwx_rr(RG, RS)                                                    \
        EMITH(MRM(0x14, REG(RG),REG(RS),0x00))

#define andwx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x54, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define andwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x14, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define andwx_mr(MG, DG, RS)                                                \
        andwx_st(W(RS), W(MG), W(DG))


#define andwxZri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x0B,VAL(IS)))

#define andwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x0B,VAL(IS)))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define andwxZrr(RG, RS)                                                    \
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
        EMIT6(MIM(0xC0, REG(RG),0x0B,VAL(IS)))

#define annwx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x0B,VAL(IS)))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define annwx_rr(RG, RS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMITH(MRM(0x14, REG(RG),REG(RS),0x00))

#define annwx_ld(RG, MS, DS)                                                \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x54, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define annwx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMITH(MRM(0x14, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define annwx_mr(MG, DG, RS)                                                \
        annwx_st(W(RS), W(MG), W(DG))


#define annwxZri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, REG(RG),0x0B,VAL(IS)))

#define annwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MIM(0xC0, TMxx,   0x0B,VAL(IS)))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define annwxZrr(RG, RS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMITH(MRM(0x14, REG(RG),REG(RS),0x00))

#define annwxZld(RG, MS, DS)                                                \
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
        EMITW(0xB2220000 | TPxx << 4)                                       \
        EMIT6(MIM(0xC0, REG(RG),0x07,0xFFFFFFFF))                           \
        EMITH(0x0400 | TPxx << 4)

#define notwx_mx(MG, DG)                                                    \
        EMITW(0xB2220000 | TPxx << 4)                                       \
        EMIT6(MIM(0xC2, SPxx,0x04,0x08)) /* SP -= 8 */                      \
        EMIT6(0xE30000000024 | MSM(TPxx,   SPxx,0x00))                      \
        EMIT6(MIM(0xC2, SPxx,0x04,0x08)) /* SP -= 8 */                      \
        EMIT6(0xE30000000024 | MSM(TMxx,   SPxx,0x00))                      \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0xFFFFFFFF))                           \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(0xE30000000004 | MSM(TMxx,   SPxx,0x00))                      \
        EMIT6(MIM(0xC2, SPxx,0x08,0x08)) /* SP += 8 */                      \
        EMIT6(0xE30000000004 | MSM(TPxx,   SPxx,0x00))                      \
        EMIT6(MIM(0xC2, SPxx,0x08,0x08)) /* SP += 8 */                      \
        EMITH(0x0400 | TPxx << 4)

/* neg (G = -G)
 * set-flags: undefined (*_*), yes (*Z*) */

#define negwx_rx(RG)                                                        \
        EMITH(MRM(0x13, REG(RG),REG(RG),0x00))

#define negwx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x13, TMxx,   TMxx,   0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))


#define negwxZrx(RG)                                                        \
        EMITH(MRM(0x13, REG(RG),REG(RG),0x00))

#define negwxZmx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x13, TMxx,   TMxx,   0x00))                              \
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
        EMIT6(MIM(0xC2, REG(RG),0x09,VAL(IS)))                              \
        EMITH(MRM(0x12, REG(RG),REG(RG),0x00))         /* <- set flags (Z) */

#define addwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC2, TMxx,   0x09,VAL(IS)))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x12, TMxx,   TMxx,   0x00))         /* <- set flags (Z) */

#define addwxZrr(RG, RS)                                                    \
        EMITH(MRM(0x1A, REG(RG),REG(RS),0x00))                              \
        EMITH(MRM(0x12, REG(RG),REG(RG),0x00))         /* <- set flags (Z) */

#define addwxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x5A, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x12, REG(RG),REG(RG),0x00))         /* <- set flags (Z) */

#define addhnZld(RG, MS, DS)    /* add 16-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x7A, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x12, REG(RG),REG(RG),0x00))         /* <- set flags (Z) */

#define addhzZld(RG, MS, DS)    /* add 16-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x91, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x1A, REG(RG),TMxx,   0x00))                              \
        EMITH(MRM(0x12, REG(RG),REG(RG),0x00))         /* <- set flags (Z) */

#define addbnZld(RG, MS, DS)    /* add  8-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x77, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x1A, REG(RG),TMxx,   0x00))                              \
        EMITH(MRM(0x12, REG(RG),REG(RG),0x00))         /* <- set flags (Z) */

#define addbzZld(RG, MS, DS)    /* add  8-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x90, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x1A, REG(RG),TMxx,   0x00))                              \
        EMITH(MRM(0x12, REG(RG),REG(RG),0x00))         /* <- set flags (Z) */

#define addwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x1A, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x12, TMxx,   TMxx,   0x00))         /* <- set flags (Z) */

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
        EMIT6(MIM(0xC2, REG(RG),0x09, (-VAL(IS) & 0xFFFFFFFF)))             \
        EMITH(MRM(0x12, REG(RG),REG(RG),0x00))         /* <- set flags (Z) */

#define subwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC2, TMxx,   0x09, (-VAL(IS) & 0xFFFFFFFF)))             \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x12, TMxx,   TMxx,   0x00))         /* <- set flags (Z) */

#define subwxZrr(RG, RS)                                                    \
        EMITH(MRM(0x1B, REG(RG),REG(RS),0x00))                              \
        EMITH(MRM(0x12, REG(RG),REG(RG),0x00))         /* <- set flags (Z) */

#define subwxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x5B, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x12, REG(RG),REG(RG),0x00))         /* <- set flags (Z) */

#define subhnZld(RG, MS, DS)    /* add 16-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x7B, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x12, REG(RG),REG(RG),0x00))         /* <- set flags (Z) */

#define subhzZld(RG, MS, DS)    /* add 16-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x91, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x1B, REG(RG),TMxx,   0x00))                              \
        EMITH(MRM(0x12, REG(RG),REG(RG),0x00))         /* <- set flags (Z) */

#define subbnZld(RG, MS, DS)    /* add  8-bit to 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x77, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x1B, REG(RG),TMxx,   0x00))                              \
        EMITH(MRM(0x12, REG(RG),REG(RG),0x00))         /* <- set flags (Z) */

#define subbzZld(RG, MS, DS)    /* add  8-bit to 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x90, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x1B, REG(RG),TMxx,   0x00))                              \
        EMITH(MRM(0x12, REG(RG),REG(RG),0x00))         /* <- set flags (Z) */

#define subwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x1B, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x12, TMxx,   TMxx,   0x00))         /* <- set flags (Z) */

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


#define shlwxZrx(RG)                     /* reads Recx for shift count */   \
        EMIT6(MTM(0xDF, REG(RG),TEcx,   0x00))                              \
        EMITW(0xA70E0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define shlwxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDF, TMxx,   TEcx,   0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA70E0000 | TMxx << 20)                 /* <- set flags (Z) */

#define shlwxZri(RG, IS)                                                    \
        EMIT6(MTM(0xDF, REG(RG),0x00,VAL(IS)))                              \
        EMITW(0xA70E0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define shlwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDF, TMxx,   0x00,VAL(IS)))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA70E0000 | TMxx << 20)                 /* <- set flags (Z) */

#define shlwxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMIT6(MTM(0xDF, REG(RG),REG(RS),0x00))                              \
        EMITW(0xA70E0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define shlwxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x58, TDxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MTM(0xDF, REG(RG),TDxx,   0x00))                              \
        EMITW(0xA70E0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define shlwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDF, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA70E0000 | TMxx << 20)                 /* <- set flags (Z) */

#define shlwxZmr(MG, DG, RS)                                                \
        shlwxZst(W(RS), W(MG), W(DG))

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


#define shrwxZrx(RG)                     /* reads Recx for shift count */   \
        EMIT6(MTM(0xDE, REG(RG),TEcx,   0x00))                              \
        EMITW(0xA70E0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define shrwxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDE, TMxx,   TEcx,   0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA70E0000 | TMxx << 20)                 /* <- set flags (Z) */

#define shrwxZri(RG, IS)                                                    \
        EMIT6(MTM(0xDE, REG(RG),0x00,VAL(IS)))                              \
        EMITW(0xA70E0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define shrwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDE, TMxx,   0x00,VAL(IS)))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA70E0000 | TMxx << 20)                 /* <- set flags (Z) */

#define shrwxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMIT6(MTM(0xDE, REG(RG),REG(RS),0x00))                              \
        EMITW(0xA70E0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define shrwxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x58, TDxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MTM(0xDE, REG(RG),TDxx,   0x00))                              \
        EMITW(0xA70E0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define shrwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDE, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA70E0000 | TMxx << 20)                 /* <- set flags (Z) */

#define shrwxZmr(MG, DG, RS)                                                \
        shrwxZst(W(RS), W(MG), W(DG))

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


#define shrwnZrx(RG)                     /* reads Recx for shift count */   \
        EMIT6(MTM(0xDC, REG(RG),TEcx,   0x00))                              \
        EMITW(0xA70E0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define shrwnZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDC, TMxx,   TEcx,   0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA70E0000 | TMxx << 20)                 /* <- set flags (Z) */

#define shrwnZri(RG, IS)                                                    \
        EMIT6(MTM(0xDC, REG(RG),0x00,VAL(IS)))                              \
        EMITW(0xA70E0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define shrwnZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDC, TMxx,   0x00,VAL(IS)))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA70E0000 | TMxx << 20)                 /* <- set flags (Z) */

#define shrwnZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMIT6(MTM(0xDC, REG(RG),REG(RS),0x00))                              \
        EMITW(0xA70E0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define shrwnZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x58, TDxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MTM(0xDC, REG(RG),TDxx,   0x00))                              \
        EMITW(0xA70E0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define shrwnZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDC, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA70E0000 | TMxx << 20)                 /* <- set flags (Z) */

#define shrwnZmr(MG, DG, RS)                                                \
        shrwnZst(W(RS), W(MG), W(DG))

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


#define rorwxZrx(RG)                     /* reads Recx for shift count */   \
        EMITH(MRM(0x17, TDxx,   TDxx,   0x00))                              \
        EMITH(MRM(0x1B, TDxx,   TEcx,   0x00))                              \
        EMIT6(MTM(0x1D, REG(RG),TDxx,   0x00))                              \
        EMITW(0xA70E0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define rorwxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        negwx_rx(Recx)                                                      \
        EMIT6(MTM(0x1D, TMxx,   TEcx,   0x00))                              \
        negwx_rx(Recx)                                                      \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA70E0000 | TMxx << 20)                 /* <- set flags (Z) */

#define rorwxZri(RG, IS)                                                    \
        EMIT6(MTM(0x1D, REG(RG),0x00,   (32-VAL(IS)) & 0x1F))               \
        EMITW(0xA70E0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define rorwxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0x1D, TMxx,   0x00,   (32-VAL(IS)) & 0x1F))               \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA70E0000 | TMxx << 20)                 /* <- set flags (Z) */

#define rorwxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITH(MRM(0x17, TDxx,   TDxx,   0x00))                              \
        EMITH(MRM(0x1B, TDxx,   REG(RS),0x00))                              \
        EMIT6(MTM(0x1D, REG(RG),TDxx,   0x00))                              \
        EMITW(0xA70E0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define rorwxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x17, TDxx,   TDxx,   0x00))                              \
        EMITH(MRM(0x1B, TDxx,   TMxx,   0x00))                              \
        EMIT6(MTM(0x1D, REG(RG),TDxx,   0x00))                              \
        EMITW(0xA70E0000 | REG(RG) << 20)              /* <- set flags (Z) */

#define rorwxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x58, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        negwx_rx(W(RS))                                                     \
        EMIT6(MTM(0x1D, TMxx,   REG(RS),0x00))                              \
        negwx_rx(W(RS))                                                     \
        EMIT6(MDM(0x50, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA70E0000 | TMxx << 20)                 /* <- set flags (Z) */

#define rorwxZmr(MG, DG, RS)                                                \
        rorwxZst(W(RS), W(MG), W(DG))

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

#endif /* (defined RT_Z64) */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

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

#endif /* RT_RTARCH_Z32_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

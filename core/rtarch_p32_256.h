/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_P32_256_H
#define RT_RTARCH_P32_256_H

#include "rtarch_p64.h"

#define RT_SIMD_REGS        16
#define RT_SIMD_ALIGN       32
#define RT_SIMD_WIDTH32     8
#define RT_SIMD_SET32(s, v) s[0]=s[1]=s[2]=s[3]=s[4]=s[5]=s[6]=s[7]=v
#define RT_SIMD_WIDTH64     4
#define RT_SIMD_SET64(s, v) s[0]=s[1]=s[2]=s[3]=v

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_p32_256.h: Implementation of Power fp32 VMX/VSX instructions (pairs).
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
 * cmdpx_** - applies [cmd] to [p]acked unsigned integer args, [x] - default
 * cmdpn_** - applies [cmd] to [p]acked   signed integer args, [n] - negatable
 * cmdps_** - applies [cmd] to [p]acked floating point   args, [s] - scalable
 *
 * cmdo*_** - applies [cmd] to 32-bit SIMD register/memory/immediate args
 * cmdp*_** - applies [cmd] to L-size SIMD register/memory/immediate args
 * cmdq*_** - applies [cmd] to 64-bit SIMD register/memory/immediate args
 *
 * The cmdp*_** (rtbase.h) instructions are intended for SPMD programming model
 * and can be configured to work with 32/64-bit data-elements (int, fp).
 * In this model data-paths are fixed-width, BASE and SIMD data-elements are
 * width-compatible, code-path divergence is handled via CHECK_MASK macro.
 * Matching element-sized BASE subset cmdy*_** is defined in rtbase.h.
 *
 * Interpretation of instruction parameters:
 *
 * upper-case params have triplet structure and require W to pass-forward
 * lower-case params are singular and can be used/passed as such directly
 *
 * XD - SIMD register serving as destination only, if present
 * XG - SIMD register serving as destination and fisrt source
 * XS - SIMD register serving as second source (first if any)
 * XT - SIMD register serving as third source (second if any)
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
 */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#if defined (RT_SIMD_CODE)

#if defined (RT_256) && (RT_256 != 0) && (RT_SIMD_COMPAT_XMM > 0)

#undef  sregs_sa
#undef  sregs_la
#undef  movox_ld
#undef  movqx_ld
#define movqx_ld(XD, MS, DS)
#undef  EMITS
#define EMITS(w) EMITW(w)

/* structural */

#define MXM(reg, ren, rem)                                                  \
        ((rem) << 11 | (ren) << 16 | (reg) << 21)

#define MPM(reg, brm, vdp, bxx, pxx)                                        \
        (pxx(vdp) | bxx(brm) << 16 | (reg) << 21)

/* selectors  */

#define  B2(val, tp1, tp2)  B2##tp2
#define  P2(val, tp1, tp2)  P2##tp2
#define  C2(val, tp1, tp2)  C2##tp2

/* displacement encoding SIMD(TP2) */

#define B20(br) (br)
#define P20(dp) (0x00000000 | ((dp) & 0x7FF0))
#define C20(br, dp) EMPTY

#define B21(br) (br)
#define P21(dp) (0x44000214 | TDxx << 11)
#define C21(br, dp) EMITW(0x60000000 | TDxx << 16 | (0xFFF0 & (dp)))

#define B22(br) (br)
#define P22(dp) (0x44000214 | TDxx << 11)
#define C22(br, dp) EMITW(0x64000000 | TDxx << 16 | (0x7FFF & (dp) >> 16))  \
                    EMITW(0x60000000 | TDxx << 16 | TDxx << 21 |            \
                                                    (0xFFF0 & (dp)))

/* registers    REG   (check mapping with ASM_ENTER/ASM_LEAVE in rtarch.h) */

#define TmmQ    0x0F  /* v15, optional, full-mask all 1s */
#define TmmR    0x17  /* v23, VMX only, Rounding Mode */
#define TmmS    0x18  /* v24, VMX only, sign-mask 32-bit */
#define TmmT    0x1E  /* v30, VMX only */
#define TmmU    0x1A  /* v26, +1.0 32-bit */
#define TmmV    0x1B  /* v27, -0.5 32-bit */
#define TmmW    0x1C  /* v28 */
#define TmmX    0x15  /* v21, +1.0 64-bit */
#define TmmY    0x16  /* v22, -0.5 64-bit */

#define Tmm0    0x00  /* v0,  internal name for Xmm0 (in mmv) */
#define TmmE    0x0E  /* v14, internal name for XmmE (in sregs) */
#define TmmF    0x0F  /* v15, internal name for XmmF (in sregs) */
#define TmmM    0x1F  /* v31, temp-reg name for mem-args */

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/* registers    REG,  MOD,  SIB */

#define Xmm0    0x00, 0x00, EMPTY       /* v0 */
#define Xmm1    0x01, 0x00, EMPTY       /* v1 */
#define Xmm2    0x02, 0x00, EMPTY       /* v2 */
#define Xmm3    0x03, 0x00, EMPTY       /* v3 */
#define Xmm4    0x04, 0x00, EMPTY       /* v4 */
#define Xmm5    0x05, 0x00, EMPTY       /* v5 */
#define Xmm6    0x06, 0x00, EMPTY       /* v6 */
#define Xmm7    0x07, 0x00, EMPTY       /* v7 */
#define Xmm8    0x08, 0x00, EMPTY       /* v8 */
#define Xmm9    0x09, 0x00, EMPTY       /* v9 */
#define XmmA    0x0A, 0x00, EMPTY       /* v10 */
#define XmmB    0x0B, 0x00, EMPTY       /* v11 */
#define XmmC    0x0C, 0x00, EMPTY       /* v12 */
#define XmmD    0x0D, 0x00, EMPTY       /* v13 */
#if     RT_SIMD_COMPAT_XMM < 2
#define XmmE    0x0E, 0x00, EMPTY       /* v14, may be reserved in some cases */
#if     RT_SIMD_COMPAT_XMM < 1
#define XmmF    0x0F, 0x00, EMPTY       /* v15, may be reserved in some cases */
#endif/*RT_SIMD_COMPAT_XMM < 1*/
#endif/*RT_SIMD_COMPAT_XMM < 2*/

/* The last two SIMD registers can be reserved by the assembler when building
 * RISC targets with SIMD wider than natively supported 128-bit, in which case
 * they will be occupied by temporary data. Two hidden registers may also come
 * in handy when implementing elaborate register-spill techniques in the future
 * for current targets with less native registers than architecturally exposed.
 *
 * It should be possible to reserve only 1 SIMD register (XmmF) to achieve the
 * goals above (totalling 15 regs) at the cost of extra loads in certain ops. */

/******************************************************************************/
/**********************************   VSX   ***********************************/
/******************************************************************************/

/* adr (D = adr S) */

#define adrpx_ld(RD, MS, DS) /* RD is a BASE reg, MS/DS is SIMD-aligned */  \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(REG(RD), MOD(MS), VAL(DS), B2(DS), P2(DS)))

/**************************   packed generic (SIMD)   *************************/

/* mov (D = S) */

#define movox_rr(XD, XS)                                                    \
        EMITW(0xF0000497 | MXM(REG(XD), REG(XS), REG(XS)))                  \
        EMITW(0xF0000497 | MXM(RYG(XD), RYG(XS), RYG(XS)))

#define movox_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(REG(XD), Teax & (MOD(MS) == TPxx), TPxx))    \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(RYG(XD), Teax & (MOD(MS) == TPxx), TPxx))    \
                                                       /* ^ == -1 if true */

#define movox_st(XS, MD, DD)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MD), VAL(DD), B2(DD), P2(DD)))  \
        EMITW(0x7C000719 | MXM(REG(XS), Teax & (MOD(MD) == TPxx), TPxx))    \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MD), VYL(DD), C2(DD), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MD), VYL(DD), B2(DD), P2(DD)))  \
        EMITW(0x7C000719 | MXM(RYG(XS), Teax & (MOD(MD) == TPxx), TPxx))    \
                                                       /* ^ == -1 if true */

/* mmv (G = G mask-merge S, mask: 0 - keeps G, 1 - picks S with elem-size frag)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, XS unmasked frags */

#define mmvox_rr(XG, XS)                                                    \
        EMITW(0xF000003F | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF000043F | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define mmvox_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000003F | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000043F | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

#define mmvox_st(XS, MG, DG)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MG), VAL(DG), C2(DG), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MG), VAL(DG), B2(DG), P2(DG)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MG) == TPxx), TPxx))    \
        EMITW(0xF000003F | MXM(TmmM,    TmmM,    REG(XS)))                  \
        EMITW(0x7C000719 | MXM(TmmM,    Teax & (MOD(MG) == TPxx), TPxx))    \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MG), VYL(DG), C2(DG), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MG), VYL(DG), B2(DG), P2(DG)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MG) == TPxx), TPxx))    \
        EMITW(0xF000043F | MXM(TmmM,    TmmM,    RYG(XS)))                  \
        EMITW(0x7C000719 | MXM(TmmM,    Teax & (MOD(MG) == TPxx), TPxx))    \
                                                       /* ^ == -1 if true */

/* and (G = G & S) */

#define andox_rr(XG, XS)                                                    \
        EMITW(0xF0000417 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF0000417 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define andox_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000417 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000417 | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

/* ann (G = ~G & S) */

#define annox_rr(XG, XS)                                                    \
        EMITW(0xF0000457 | MXM(REG(XG), REG(XS), REG(XG)))                  \
        EMITW(0xF0000457 | MXM(RYG(XG), RYG(XS), RYG(XG)))

#define annox_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000457 | MXM(REG(XG), TmmM,    REG(XG)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000457 | MXM(RYG(XG), TmmM,    RYG(XG)))

/* orr (G = G | S) */

#define orrox_rr(XG, XS)                                                    \
        EMITW(0xF0000497 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF0000497 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define orrox_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000497 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000497 | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

/* orn (G = ~G | S) */

#if (RT_256 < 2)

#define ornox_rr(XG, XS)                                                    \
        notox_rx(W(XG))                                                     \
        orrox_rr(W(XG), W(XS))

#define ornox_ld(XG, MS, DS)                                                \
        notox_rx(W(XG))                                                     \
        orrox_ld(W(XG), W(MS), W(DS))

#else /* RT_256 >= 2 */

#define ornox_rr(XG, XS)                                                    \
        EMITW(0xF0000557 | MXM(REG(XG), REG(XS), REG(XG)))                  \
        EMITW(0xF0000557 | MXM(RYG(XG), RYG(XS), RYG(XG)))

#define ornox_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000557 | MXM(REG(XG), TmmM,    REG(XG)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000557 | MXM(RYG(XG), TmmM,    RYG(XG)))

#endif /* RT_256 >= 2 */

/* xor (G = G ^ S) */

#define xorox_rr(XG, XS)                                                    \
        EMITW(0xF00004D7 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF00004D7 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define xorox_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00004D7 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00004D7 | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

/* not (G = ~G) */

#define notox_rx(XG)                                                        \
        EMITW(0xF0000517 | MXM(REG(XG), REG(XG), REG(XG)))                  \
        EMITW(0xF0000517 | MXM(RYG(XG), RYG(XG), RYG(XG)))

/**************   packed single precision floating point (SIMD)   *************/

/* neg (G = -G) */

#define negos_rx(XG)                                                        \
        EMITW(0xF00006E7 | MXM(REG(XG), 0x00,    REG(XG)))                  \
        EMITW(0xF00006E7 | MXM(RYG(XG), 0x00,    RYG(XG)))

/* add (G = G + S) */

#define addos_rr(XG, XS)                                                    \
        EMITW(0xF0000207 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF0000207 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define addos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000207 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000207 | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

/* sub (G = G - S) */

#define subos_rr(XG, XS)                                                    \
        EMITW(0xF0000247 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF0000247 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define subos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000247 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000247 | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

/* mul (G = G * S) */

#define mulos_rr(XG, XS)                                                    \
        EMITW(0xF0000287 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF0000287 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define mulos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000287 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000287 | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

/* div (G = G / S) */

#define divos_rr(XG, XS)                                                    \
        EMITW(0xF00002C7 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF00002C7 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define divos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002C7 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002C7 | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

/* sqr (D = sqrt S) */

#define sqros_rr(XD, XS)                                                    \
        EMITW(0xF000022F | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF000022F | MXM(RYG(XD), 0x00,    RYG(XS)))

#define sqros_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000022F | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000022F | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rceos_rr(XD, XS)                                                    \
        EMITW(0xF000026B | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF000026B | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rcsos_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0xF00006CD | MXM(REG(XS), REG(XG), TmmU))                     \
        EMITW(0xF000020F | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF00006CD | MXM(RYG(XS), RYG(XG), TmmU))                     \
        EMITW(0xF000020F | MXM(RYG(XG), RYG(XG), RYG(XS)))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rseos_rr(XD, XS)                                                    \
        EMITW(0xF000022B | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF000022B | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rssos_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0xF0000287 | MXM(TmmM,    REG(XG), REG(XG)))                  \
        EMITW(0xF0000284 | MXM(TmmW,    REG(XG), TmmV))                     \
        EMITW(0xF00006CD | MXM(TmmM,    REG(XS), TmmU))                     \
        EMITW(0xF000068D | MXM(REG(XG), TmmM,    TmmW))                     \
        EMITW(0xF0000287 | MXM(TmmM,    RYG(XG), RYG(XG)))                  \
        EMITW(0xF0000284 | MXM(TmmW,    RYG(XG), TmmV))                     \
        EMITW(0xF00006CD | MXM(TmmM,    RYG(XS), TmmU))                     \
        EMITW(0xF000068D | MXM(RYG(XG), TmmM,    TmmW))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmaos_rr(XG, XS, XT)                                                \
        EMITW(0xF000020F | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0xF000020F | MXM(RYG(XG), RYG(XS), RYG(XT)))

#define fmaos_ld(XG, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000020F | MXM(REG(XG), REG(XS), TmmM))                     \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000020F | MXM(RYG(XG), RYG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsos_rr(XG, XS, XT)                                                \
        EMITW(0xF000068F | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0xF000068F | MXM(RYG(XG), RYG(XS), RYG(XT)))

#define fmsos_ld(XG, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000068F | MXM(REG(XG), REG(XS), TmmM))                     \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000068F | MXM(RYG(XG), RYG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMS */

/* min (G = G < S ? G : S) */

#define minos_rr(XG, XS)                                                    \
        EMITW(0xF0000647 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF0000647 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define minos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000647 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000647 | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

/* max (G = G > S ? G : S) */

#define maxos_rr(XG, XS)                                                    \
        EMITW(0xF0000607 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF0000607 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define maxos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000607 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000607 | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

/* cmp (G = G ? S) */

#define ceqos_rr(XG, XS)                                                    \
        EMITW(0xF000021F | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF000021F | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define ceqos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000021F | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000021F | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

#define cneos_rr(XG, XS)                                                    \
        EMITW(0xF000021F | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF0000517 | MXM(REG(XG), REG(XG), REG(XG)))                  \
        EMITW(0xF000021F | MXM(RYG(XG), RYG(XG), RYG(XS)))                  \
        EMITW(0xF0000517 | MXM(RYG(XG), RYG(XG), RYG(XG)))

#define cneos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000021F | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        EMITW(0xF0000517 | MXM(REG(XG), REG(XG), REG(XG)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000021F | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */\
        EMITW(0xF0000517 | MXM(RYG(XG), RYG(XG), RYG(XG)))

#define cltos_rr(XG, XS)                                                    \
        EMITW(0xF000025F | MXM(REG(XG), REG(XS), REG(XG)))                  \
        EMITW(0xF000025F | MXM(RYG(XG), RYG(XS), RYG(XG)))

#define cltos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000025F | MXM(REG(XG), TmmM,    REG(XG)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000025F | MXM(RYG(XG), TmmM,    RYG(XG)))

#define cleos_rr(XG, XS)                                                    \
        EMITW(0xF000029F | MXM(REG(XG), REG(XS), REG(XG)))                  \
        EMITW(0xF000029F | MXM(RYG(XG), RYG(XS), RYG(XG)))

#define cleos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000029F | MXM(REG(XG), TmmM,    REG(XG)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000029F | MXM(RYG(XG), TmmM,    RYG(XG)))

#define cgtos_rr(XG, XS)                                                    \
        EMITW(0xF000025F | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF000025F | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define cgtos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000025F | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000025F | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

#define cgeos_rr(XG, XS)                                                    \
        EMITW(0xF000029F | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF000029F | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define cgeos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000029F | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000029F | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzos_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0xF0000267 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF0000267 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rnzos_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000267 | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000267 | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */

#define cvzos_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0xF0000263 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF0000263 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define cvzos_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000263 | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000263 | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpos_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0xF00002A7 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002A7 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rnpos_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002A7 | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002A7 | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */

#define cvpos_rr(XD, XS)     /* round towards +inf */                       \
        rnpos_rr(W(XD), W(XS))                                              \
        cvzos_rr(W(XD), W(XD))

#define cvpos_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpos_ld(W(XD), W(MS), W(DS))                                       \
        cvzos_rr(W(XD), W(XD))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmos_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0xF00002E7 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002E7 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rnmos_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002E7 | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002E7 | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */

#define cvmos_rr(XD, XS)     /* round towards -inf */                       \
        rnmos_rr(W(XD), W(XS))                                              \
        cvzos_rr(W(XD), W(XD))

#define cvmos_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmos_ld(W(XD), W(MS), W(DS))                                       \
        cvzos_rr(W(XD), W(XD))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnos_rr(XD, XS)     /* round towards near */                       \
        EMITW(0xF00002AF | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002AF | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rnnos_ld(XD, MS, DS) /* round towards near */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002AF | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002AF | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */

#define cvnos_rr(XD, XS)     /* round towards near */                       \
        rnnos_rr(W(XD), W(XS))                                              \
        cvzos_rr(W(XD), W(XD))

#define cvnos_ld(XD, MS, DS) /* round towards near */                       \
        rnnos_ld(W(XD), W(MS), W(DS))                                       \
        cvzos_rr(W(XD), W(XD))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnon_rr(XD, XS)     /* round towards near */                       \
        cvton_rr(W(XD), W(XS))

#define cvnon_ld(XD, MS, DS) /* round towards near */                       \
        cvton_ld(W(XD), W(MS), W(DS))

/**************************   packed integer (SIMD)   *************************/

/* add (G = G + S) */

#define addox_rr(XG, XS)                                                    \
        EMITW(0x10000080 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x10000080 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define addox_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000080 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000080 | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

/* sub (G = G - S) */

#define subox_rr(XG, XS)                                                    \
        EMITW(0x10000480 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x10000480 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define subox_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000480 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000480 | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

/* shl (G = G << S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shlox_ri(XG, IS)                                                    \
        EMITW(0x1000038C | MXM(TmmM,    (0x1F & VAL(IS)), 0x00))            \
        EMITW(0x10000184 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x10000184 | MXM(RYG(XG), RYG(XG), TmmM))

#if RT_ENDIAN == 0

#define shlox_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C00008E | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1003028C | MXM(TmmM,    0x00,    TmmM))/* ^ == -1 if true */\
        EMITW(0x10000184 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x10000184 | MXM(RYG(XG), RYG(XG), TmmM))

#else /* RT_ENDIAN == 1 */

#define shlox_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C00008E | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000028C | MXM(TmmM,    0x00,    TmmM))/* ^ == -1 if true */\
        EMITW(0x10000184 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x10000184 | MXM(RYG(XG), RYG(XG), TmmM))

#endif /* RT_ENDIAN == 1 */

#define svlox_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x10000184 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x10000184 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define svlox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000184 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000184 | MXM(RYG(XG), RYG(XG), TmmM))

/* shr (G = G >> S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrox_ri(XG, IS)                                                    \
        EMITW(0x1000038C | MXM(TmmM,    (0x1F & VAL(IS)), 0x00))            \
        EMITW(0x10000284 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x10000284 | MXM(RYG(XG), RYG(XG), TmmM))

#if RT_ENDIAN == 0

#define shrox_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C00008E | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1003028C | MXM(TmmM,    0x00,    TmmM))/* ^ == -1 if true */\
        EMITW(0x10000284 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x10000284 | MXM(RYG(XG), RYG(XG), TmmM))

#else /* RT_ENDIAN == 1 */

#define shrox_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C00008E | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000028C | MXM(TmmM,    0x00,    TmmM))/* ^ == -1 if true */\
        EMITW(0x10000284 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x10000284 | MXM(RYG(XG), RYG(XG), TmmM))

#endif /* RT_ENDIAN == 1 */

#define svrox_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x10000284 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x10000284 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define svrox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000284 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000284 | MXM(RYG(XG), RYG(XG), TmmM))


#define shron_ri(XG, IS)                                                    \
        EMITW(0x1000038C | MXM(TmmM,    (0x1F & VAL(IS)), 0x00))            \
        EMITW(0x10000384 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x10000384 | MXM(RYG(XG), RYG(XG), TmmM))

#if RT_ENDIAN == 0

#define shron_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C00008E | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1003028C | MXM(TmmM,    0x00,    TmmM))/* ^ == -1 if true */\
        EMITW(0x10000384 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x10000384 | MXM(RYG(XG), RYG(XG), TmmM))

#else /* RT_ENDIAN == 1 */

#define shron_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C00008E | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000028C | MXM(TmmM,    0x00,    TmmM))/* ^ == -1 if true */\
        EMITW(0x10000384 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x10000384 | MXM(RYG(XG), RYG(XG), TmmM))

#endif /* RT_ENDIAN == 1 */

#define svron_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x10000384 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x10000384 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define svron_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000384 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000384 | MXM(RYG(XG), RYG(XG), TmmM))

/**************************   helper macros (SIMD)   **************************/

/* simd mask
 * compatibility with AVX-512 and ARM-SVE can be achieved by always keeping
 * one hidden SIMD register holding all 1s and using one hidden mask register
 * first in cmp (c**ps) to produce compatible result in target SIMD register
 * then in CHECK_MASK to facilitate branching on a given condition value */

#define RT_SIMD_MASK_NONE       MN      /* none satisfy the condition */
#define RT_SIMD_MASK_FULL       MF      /*  all satisfy the condition */

#define S0(mask)    S1(mask)
#define S1(mask)    S##mask

#define SMN(xs, lb)                                                         \
        EMITW(0xF0000497 | MXM(TmmM, xs,  xs+16))                           \
        EMITW(0x10000486 | MXM(TmmM, TmmM, TmmQ))                           \
        ASM_BEG ASM_OP2(beq, cr6, lb) ASM_END

#define SMF(xs, lb)                                                         \
        EMITW(0xF0000417 | MXM(TmmM, xs,  xs+16))                           \
        EMITW(0x10000486 | MXM(TmmM, TmmM, TmmQ))                           \
        ASM_BEG ASM_OP2(blt, cr6, lb) ASM_END

#define CHECK_MASK(lb, mask, XS) /* destroys Reax, jump lb if mask == S */  \
        AUW(EMPTY, EMPTY, EMPTY, REG(XS), lb, S0(RT_SIMD_MASK_##mask), EMPTY2)

/* simd mode
 * set via FCTRL macros, *_F for faster non-IEEE mode (optional on MIPS/Power),
 * original FCTRL blocks (FCTRL_ENTER/FCTRL_LEAVE) are defined in rtbase.h
 * NOTE: ARMv7 always uses ROUNDN non-IEEE mode for SIMD fp-arithmetic,
 * while fp<->int conversion takes ROUND* into account via VFP fallback */

#if RT_SIMD_FLUSH_ZERO == 0

#define RT_SIMD_MODE_ROUNDN     0x00    /* round towards near */
#define RT_SIMD_MODE_ROUNDM     0x03    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP     0x02    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ     0x01    /* round towards zero */

#else /* RT_SIMD_FLUSH_ZERO */

#define RT_SIMD_MODE_ROUNDN     0x04    /* round towards near */
#define RT_SIMD_MODE_ROUNDM     0x07    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP     0x06    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ     0x05    /* round towards zero */

#endif /* RT_SIMD_FLUSH_ZERO */

#define RT_SIMD_MODE_ROUNDN_F   0x04    /* round towards near */
#define RT_SIMD_MODE_ROUNDM_F   0x07    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP_F   0x06    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ_F   0x05    /* round towards zero */

#define fpscr_ld(RS) /* not portable, do not use outside */                 \
        EMITW(0xFE00058E | MRM(0x00,    REG(RS), 0x00))

#define fpscr_st(RD) /* not portable, do not use outside */                 \
        EMITW(0xFC00048E | MRM(REG(RD), 0x00,    0x00))

#define FCTRL_SET(mode)   /* sets given mode into fp control register */    \
        EMITW(0xFF80010C | RT_SIMD_MODE_##mode << 12)

#define FCTRL_RESET()     /* resumes default mode (ROUNDN) upon leave */    \
        EMITW(0xFF80010C)

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndos_rr(XD, XS)                                                    \
        EMITW(0xF00002AF | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002AF | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rndos_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002AF | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002AF | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */

#define cvtos_rr(XD, XS)                                                    \
        rndos_rr(W(XD), W(XS))                                              \
        cvzos_rr(W(XD), W(XD))

#define cvtos_ld(XD, MS, DS)                                                \
        rndos_ld(W(XD), W(MS), W(DS))                                       \
        cvzos_rr(W(XD), W(XD))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvton_rr(XD, XS)                                                    \
        EMITW(0xF00002E3 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002E3 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define cvton_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002E3 | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002E3 | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnros_rr(XD, XS, mode)                                              \
        EMITW(0x1000020A | MXM(REG(XD), 0x00,    REG(XS)) |                 \
        (RT_SIMD_MODE_##mode&3) << 6)                                       \
        EMITW(0x1000020A | MXM(RYG(XD), 0x00,    RYG(XS)) |                 \
        (RT_SIMD_MODE_##mode&3) << 6)

#define cvros_rr(XD, XS, mode)                                              \
        rnros_rr(W(XD), W(XS), mode)                                        \
        cvzos_rr(W(XD), W(XD))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/* sregs */

#define sregs_sa() /* save all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        movox_st(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(Xmm8, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(Xmm9, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(XmmA, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(XmmB, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(XmmC, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(XmmD, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000718 | MXM(TmmU,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000718 | MXM(TmmV,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000718 | MXM(TmmW,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000718 | MXM(TmmX,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000718 | MXM(TmmY,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000719 | MXM(TmmE,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000719 | MXM(TmmE+16, 0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000719 | MXM(TmmF,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000719 | MXM(TmmM,    0x00,    Teax))

#define sregs_la() /* load all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        movox_ld(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(Xmm8, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(Xmm9, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(XmmA, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(XmmB, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(XmmC, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(XmmD, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000618 | MXM(TmmU,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000618 | MXM(TmmV,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000618 | MXM(TmmW,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000618 | MXM(TmmX,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000618 | MXM(TmmY,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000619 | MXM(TmmE,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000619 | MXM(TmmE+16, 0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000619 | MXM(TmmF,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000619 | MXM(TmmM,    0x00,    Teax))

#endif /* RT_256 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_P32_256_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_A32_256_H
#define RT_RTARCH_A32_256_H

#include "rtarch_a64.h"

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
 * rtarch_a32_256.h: Implementation of AArch64 fp32 NEON instructions (pairs).
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

/* structural */

#define MXM(reg, ren, rem)                                                  \
        ((rem) << 16 | (ren) << 5 | (reg))

#define MPM(reg, brm, vdp, bxx, pxx)                                        \
        (pxx(vdp) | bxx(brm) << 5 | (reg))

/* selectors  */

#define  B2(val, tp1, tp2)  B2##tp2
#define  P2(val, tp1, tp2)  P2##tp2
#define  C2(val, tp1, tp2)  C2##tp2

/* displacement encoding SIMD(TP2) */

#define B20(br) (br)
#define P20(dp) (0x00000000 | ((dp) & 0xFFF0) << 6)
#define C20(br, dp) EMPTY

#define B22(br) TPxx
#define P22(dp) (0x00000000)
#define C22(br, dp) EMITW(0x52800000 | MXM(TDxx,    0x00,    0x00) |        \
                             (0xFFF0 & (dp)) << 5)                          \
                    EMITW(0x72A00000 | MXM(TDxx,    0x00,    0x00) |        \
                             (0x7FFF & (dp) >> 16) << 5)                    \
                    EMITW(0x0B000000 | MXM(TPxx,    (br),    TDxx) | ADR)

/* registers    REG   (check mapping with ASM_ENTER/ASM_LEAVE in rtarch.h) */

#define Tmm0    0x00  /* v0,  internal name for Xmm0 (in sregs, mmv) */
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
 * Neither of the above is currently supported by the assembler, while both are
 * considered as a potential optimization/compatibility option going forward. */

/* It should be possible to reserve only 1 SIMD register (XmmF) to achieve the
 * goals above (totalling 15 regs) at the cost of extra loads in certain ops. */

/******************************************************************************/
/**********************************   MPE   ***********************************/
/******************************************************************************/

/* adr (D = adr S) */

#define adrpx_ld(RD, MS, DS) /* RD is a BASE reg, MS/DS is SIMD-aligned */  \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C3(DS), EMPTY2)   \
        EMITW(0x8B000000 | MRM(REG(RD), MOD(MS), TDxx))

/**************************   packed generic (NEON)   *************************/

/* mov (D = S) */

#define movox_rr(XD, XS)                                                    \
        EMITW(0x4EA01C00 | MXM(REG(XD), REG(XS), REG(XS)))                  \
        EMITW(0x4EA01C00 | MXM(RYG(XD), RYG(XS), RYG(XS)))

#define movox_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(REG(XD), MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(RYG(XD), MOD(MS), VYL(DS), B2(DS), P2(DS)))

#define movox_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x3D800000 | MPM(REG(XS), MOD(MD), VAL(DD), B2(DD), P2(DD)))  \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VYL(DD), C2(DD), EMPTY2)   \
        EMITW(0x3D800000 | MPM(RYG(XS), MOD(MD), VYL(DD), B2(DD), P2(DD)))

/* mmv (G = G mask-merge S, mask: 0 - keeps G, 1 - picks S with elem-size frag)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, XS unmasked frags */

#define mmvox_rr(XG, XS)                                                    \
        EMITW(0x6EA01C00 | MXM(REG(XG), REG(XS), Tmm0))                     \
        EMITW(0x6EA01C00 | MXM(RYG(XG), RYG(XS), Tmm0+16))

#define mmvox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA01C00 | MXM(REG(XG), TmmM,    Tmm0))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA01C00 | MXM(RYG(XG), TmmM,    Tmm0+16))

#define mmvox_st(XS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C2(DG), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MG), VAL(DG), B2(DG), P2(DG)))  \
        EMITW(0x6EA01C00 | MXM(TmmM,    REG(XS), Tmm0))                     \
        EMITW(0x3D800000 | MPM(TmmM,    MOD(MG), VAL(DG), B2(DG), P2(DG)))  \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VYL(DG), C2(DG), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MG), VYL(DG), B2(DG), P2(DG)))  \
        EMITW(0x6EA01C00 | MXM(TmmM,    RYG(XS), Tmm0+16))                  \
        EMITW(0x3D800000 | MPM(TmmM,    MOD(MG), VYL(DG), B2(DG), P2(DG)))

/* and (G = G & S) */

#define andox_rr(XG, XS)                                                    \
        EMITW(0x4E201C00 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x4E201C00 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define andox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E201C00 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E201C00 | MXM(RYG(XG), RYG(XG), TmmM))

/* ann (G = ~G & S) */

#define annox_rr(XG, XS)                                                    \
        EMITW(0x4E601C00 | MXM(REG(XG), REG(XS), REG(XG)))                  \
        EMITW(0x4E601C00 | MXM(RYG(XG), RYG(XS), RYG(XG)))

#define annox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E601C00 | MXM(REG(XG), TmmM,    REG(XG)))                  \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E601C00 | MXM(RYG(XG), TmmM,    RYG(XG)))

/* orr (G = G | S) */

#define orrox_rr(XG, XS)                                                    \
        EMITW(0x4EA01C00 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x4EA01C00 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define orrox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA01C00 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA01C00 | MXM(RYG(XG), RYG(XG), TmmM))

/* orn (G = ~G | S) */

#define ornox_rr(XG, XS)                                                    \
        EMITW(0x4EE01C00 | MXM(REG(XG), REG(XS), REG(XG)))                  \
        EMITW(0x4EE01C00 | MXM(RYG(XG), RYG(XS), RYG(XG)))

#define ornox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE01C00 | MXM(REG(XG), TmmM,    REG(XG)))                  \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE01C00 | MXM(RYG(XG), TmmM,    RYG(XG)))

/* xor (G = G ^ S) */

#define xorox_rr(XG, XS)                                                    \
        EMITW(0x6E201C00 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x6E201C00 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define xorox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6E201C00 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6E201C00 | MXM(RYG(XG), RYG(XG), TmmM))

/* not (G = ~G) */

#define notox_rx(XG)                                                        \
        EMITW(0x6E205800 | MXM(REG(XG), REG(XG), 0x00))                     \
        EMITW(0x6E205800 | MXM(RYG(XG), RYG(XG), 0x00))

/**************   packed single precision floating point (NEON)   *************/

/* neg (G = -G) */

#define negos_rx(XG)                                                        \
        EMITW(0x6EA0F800 | MXM(REG(XG), REG(XG), 0x00))                     \
        EMITW(0x6EA0F800 | MXM(RYG(XG), RYG(XG), 0x00))

/* add (G = G + S) */

#define addos_rr(XG, XS)                                                    \
        EMITW(0x4E20D400 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x4E20D400 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define addos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E20D400 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E20D400 | MXM(RYG(XG), RYG(XG), TmmM))

/* sub (G = G - S) */

#define subos_rr(XG, XS)                                                    \
        EMITW(0x4EA0D400 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x4EA0D400 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define subos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA0D400 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA0D400 | MXM(RYG(XG), RYG(XG), TmmM))

/* mul (G = G * S) */

#define mulos_rr(XG, XS)                                                    \
        EMITW(0x6E20DC00 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x6E20DC00 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define mulos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6E20DC00 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6E20DC00 | MXM(RYG(XG), RYG(XG), TmmM))

/* div (G = G / S) */

#define divos_rr(XG, XS)                                                    \
        EMITW(0x6E20FC00 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x6E20FC00 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define divos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6E20FC00 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6E20FC00 | MXM(RYG(XG), RYG(XG), TmmM))

/* sqr (D = sqrt S) */

#define sqros_rr(XD, XS)                                                    \
        EMITW(0x6EA1F800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x6EA1F800 | MXM(RYG(XD), RYG(XS), 0x00))

#define sqros_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA1F800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA1F800 | MXM(RYG(XD), TmmM,    0x00))

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rceos_rr(XD, XS)                                                    \
        EMITW(0x4EA1D800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4EA1D800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rcsos_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x4E20FC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x6E20DC00 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x4E20FC00 | MXM(RYG(XS), RYG(XS), RYG(XG)))                  \
        EMITW(0x6E20DC00 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rseos_rr(XD, XS)                                                    \
        EMITW(0x6EA1D800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x6EA1D800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rssos_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x6E20DC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x4EA0FC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x6E20DC00 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x6E20DC00 | MXM(RYG(XS), RYG(XS), RYG(XG)))                  \
        EMITW(0x4EA0FC00 | MXM(RYG(XS), RYG(XS), RYG(XG)))                  \
        EMITW(0x6E20DC00 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmaos_rr(XG, XS, XT)                                                \
        EMITW(0x4E20CC00 | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0x4E20CC00 | MXM(RYG(XG), RYG(XS), RYG(XT)))

#define fmaos_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E20CC00 | MXM(REG(XG), REG(XS), TmmM))                     \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E20CC00 | MXM(RYG(XG), RYG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsos_rr(XG, XS, XT)                                                \
        EMITW(0x4EA0CC00 | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0x4EA0CC00 | MXM(RYG(XG), RYG(XS), RYG(XT)))

#define fmsos_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EA0CC00 | MXM(REG(XG), REG(XS), TmmM))                     \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EA0CC00 | MXM(RYG(XG), RYG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMS */

/* min (G = G < S ? G : S) */

#define minos_rr(XG, XS)                                                    \
        EMITW(0x4EA0F400 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x4EA0F400 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define minos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA0F400 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA0F400 | MXM(RYG(XG), RYG(XG), TmmM))

/* max (G = G > S ? G : S) */

#define maxos_rr(XG, XS)                                                    \
        EMITW(0x4E20F400 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x4E20F400 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define maxos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E20F400 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E20F400 | MXM(RYG(XG), RYG(XG), TmmM))

/* cmp (G = G ? S) */

#define ceqos_rr(XG, XS)                                                    \
        EMITW(0x4E20E400 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x4E20E400 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define ceqos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E20E400 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E20E400 | MXM(RYG(XG), RYG(XG), TmmM))

#define cneos_rr(XG, XS)                                                    \
        EMITW(0x4E20E400 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x6E205800 | MXM(REG(XG), REG(XG), 0x00))                     \
        EMITW(0x4E20E400 | MXM(RYG(XG), RYG(XG), RYG(XS)))                  \
        EMITW(0x6E205800 | MXM(RYG(XG), RYG(XG), 0x00))

#define cneos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E20E400 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x6E205800 | MXM(REG(XG), REG(XG), 0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E20E400 | MXM(RYG(XG), RYG(XG), TmmM))                     \
        EMITW(0x6E205800 | MXM(RYG(XG), RYG(XG), 0x00))

#define cltos_rr(XG, XS)                                                    \
        EMITW(0x6EA0E400 | MXM(REG(XG), REG(XS), REG(XG)))                  \
        EMITW(0x6EA0E400 | MXM(RYG(XG), RYG(XS), RYG(XG)))

#define cltos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA0E400 | MXM(REG(XG), TmmM,    REG(XG)))                  \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA0E400 | MXM(RYG(XG), TmmM,    RYG(XG)))

#define cleos_rr(XG, XS)                                                    \
        EMITW(0x6E20E400 | MXM(REG(XG), REG(XS), REG(XG)))                  \
        EMITW(0x6E20E400 | MXM(RYG(XG), RYG(XS), RYG(XG)))

#define cleos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6E20E400 | MXM(REG(XG), TmmM,    REG(XG)))                  \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6E20E400 | MXM(RYG(XG), TmmM,    RYG(XG)))

#define cgtos_rr(XG, XS)                                                    \
        EMITW(0x6EA0E400 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x6EA0E400 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define cgtos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA0E400 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA0E400 | MXM(RYG(XG), RYG(XG), TmmM))

#define cgeos_rr(XG, XS)                                                    \
        EMITW(0x6E20E400 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x6E20E400 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define cgeos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6E20E400 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6E20E400 | MXM(RYG(XG), RYG(XG), TmmM))

/**************************   packed integer (NEON)   *************************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzos_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x4EA19800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4EA19800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rnzos_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA19800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA19800 | MXM(RYG(XD), TmmM,    0x00))

#define cvzos_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x4EA1B800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4EA1B800 | MXM(RYG(XD), RYG(XS), 0x00))

#define cvzos_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA1B800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA1B800 | MXM(RYG(XD), TmmM,    0x00))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpos_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0x4EA18800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4EA18800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rnpos_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA18800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA18800 | MXM(RYG(XD), TmmM,    0x00))

#define cvpos_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0x4EA1A800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4EA1A800 | MXM(RYG(XD), RYG(XS), 0x00))

#define cvpos_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA1A800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA1A800 | MXM(RYG(XD), TmmM,    0x00))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmos_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0x4E219800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4E219800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rnmos_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E219800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E219800 | MXM(RYG(XD), TmmM,    0x00))

#define cvmos_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0x4E21B800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4E21B800 | MXM(RYG(XD), RYG(XS), 0x00))

#define cvmos_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E21B800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E21B800 | MXM(RYG(XD), TmmM,    0x00))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnos_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x4E218800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4E218800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rnnos_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E218800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E218800 | MXM(RYG(XD), TmmM,    0x00))

#define cvnos_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x4E21A800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4E21A800 | MXM(RYG(XD), RYG(XS), 0x00))

#define cvnos_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E21A800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E21A800 | MXM(RYG(XD), TmmM,    0x00))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnon_rr(XD, XS)     /* round towards near */                       \
        cvton_rr(W(XD), W(XS))

#define cvnon_ld(XD, MS, DS) /* round towards near */                       \
        cvton_ld(W(XD), W(MS), W(DS))

/* add (G = G + S) */

#define addox_rr(XG, XS)                                                    \
        EMITW(0x4EA08400 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x4EA08400 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define addox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA08400 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA08400 | MXM(RYG(XG), RYG(XG), TmmM))

/* sub (G = G - S) */

#define subox_rr(XG, XS)                                                    \
        EMITW(0x6EA08400 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x6EA08400 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define subox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA08400 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA08400 | MXM(RYG(XG), RYG(XG), TmmM))

/* shl (G = G << S) */

#define shlox_ri(XG, IS)                                                    \
        EMITW(0x4F205400 | MXM(REG(XG), REG(XG), 0x00) |                    \
                                                 (0x1F & VAL(IS)) << 16)    \
        EMITW(0x4F205400 | MXM(RYG(XG), RYG(XG), 0x00) |                    \
                                                 (0x1F & VAL(IS)) << 16)

#define shlox_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E040400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EA04400 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x6EA04400 | MXM(RYG(XG), RYG(XG), TmmM))

/* shr (G = G >> S) */

#define shrox_ri(XG, IS) /* emits shift-left for zero-immediate args */     \
        EMITW(0x4F200400 | MXM(REG(XG), REG(XG), 0x00) |                    \
        (+(VAL(IS) == 0) & 0x00005000) | (+(VAL(IS) != 0) & 0x20000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x1F &-VAL(IS)) << 16)    \
        EMITW(0x4F200400 | MXM(RYG(XG), RYG(XG), 0x00) |                    \
        (+(VAL(IS) == 0) & 0x00005000) | (+(VAL(IS) != 0) & 0x20000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x1F &-VAL(IS)) << 16)

#define shrox_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E040400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EA0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EA04400 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x6EA04400 | MXM(RYG(XG), RYG(XG), TmmM))

#define shron_ri(XG, IS) /* emits shift-left for zero-immediate args */     \
        EMITW(0x4F200400 | MXM(REG(XG), REG(XG), 0x00) |                    \
        (+(VAL(IS) == 0) & 0x00005000) | (+(VAL(IS) != 0) & 0x00000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x1F &-VAL(IS)) << 16)    \
        EMITW(0x4F200400 | MXM(RYG(XG), RYG(XG), 0x00) |                    \
        (+(VAL(IS) == 0) & 0x00005000) | (+(VAL(IS) != 0) & 0x00000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x1F &-VAL(IS)) << 16)

#define shron_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E040400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EA0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x4EA04400 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x4EA04400 | MXM(RYG(XG), RYG(XG), TmmM))

/**************************   helper macros (NEON)   **************************/

/* simd mask
 * compatibility with AVX-512 and ARM-SVE can be achieved by always keeping
 * one hidden SIMD register holding all 1s and using one hidden mask register
 * first in cmp (c**ps) to produce compatible result in target SIMD register
 * then in CHECK_MASK to facilitate branching on a given condition value */

#define RT_SIMD_MASK_NONE       0x00    /* none satisfy the condition */
#define RT_SIMD_MASK_FULL       0x04    /*  all satisfy the condition */

#define CHECK_MASK(lb, mask, XS) /* destroys Reax, jump lb if mask == S */  \
        EMITW(0x4E201C00 | MXM(TmmM,    REG(XS), RYG(XS)) |                 \
                                        (0x04 - RT_SIMD_MASK_##mask) << 21) \
        EMITW(0x4EB1B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x0E043C00 | MXM(Teax,    TmmM,    0x00))                     \
        addwz_ri(Reax, IB(RT_SIMD_MASK_##mask))                             \
        jezxx_lb(lb)

/* simd mode
 * set via FCTRL macros, *_F for faster non-IEEE mode (optional on MIPS/Power),
 * original FCTRL blocks (FCTRL_ENTER/FCTRL_LEAVE) are defined in rtbase.h
 * NOTE: ARMv7 always uses ROUNDN non-IEEE mode for SIMD fp-arithmetic,
 * while fp<->int conversion takes ROUND* into account via VFP fallback */

#if RT_SIMD_FLUSH_ZERO == 0

#define RT_SIMD_MODE_ROUNDN     0x00    /* round towards near */
#define RT_SIMD_MODE_ROUNDM     0x02    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP     0x01    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ     0x03    /* round towards zero */

#else /* RT_SIMD_FLUSH_ZERO */

#define RT_SIMD_MODE_ROUNDN     0x04    /* round towards near */
#define RT_SIMD_MODE_ROUNDM     0x06    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP     0x05    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ     0x07    /* round towards zero */

#endif /* RT_SIMD_FLUSH_ZERO */

#define RT_SIMD_MODE_ROUNDN_F   0x04    /* round towards near */
#define RT_SIMD_MODE_ROUNDM_F   0x06    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP_F   0x05    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ_F   0x07    /* round towards zero */

#define fpscr_ld(RS) /* not portable, do not use outside */                 \
        EMITW(0xD51B4400 | MRM(REG(RS), 0x00,    0x00))

#define fpscr_st(RD) /* not portable, do not use outside */                 \
        EMITW(0xD53B4400 | MRM(REG(RD), 0x00,    0x00))

#if RT_SIMD_FAST_FCTRL == 0

#define FCTRL_SET(mode)   /* sets given mode into fp control register */    \
        EMITW(0x52A00000 | MRM(TIxx,    0x00,    0x00) |                    \
                           RT_SIMD_MODE_##mode << 11)                       \
        EMITW(0xD51B4400 | MRM(TIxx,    0x00,    0x00))

#define FCTRL_RESET()     /* resumes default mode (ROUNDN) upon leave */    \
        EMITW(0xD51B4400 | MRM(TNxx,    0x00,    0x00))

#else /* RT_SIMD_FAST_FCTRL */

#define FCTRL_SET(mode)   /* sets given mode into fp control register */    \
        EMITW(0xD51B4400 | MRM(TNxx+(RT_SIMD_MODE_##mode&3), 0x00, 0x00))

#define FCTRL_RESET()     /* resumes default mode (ROUNDN) upon leave */    \
        EMITW(0xD51B4400 | MRM(TNxx,    0x00,    0x00))

#endif /* RT_SIMD_FAST_FCTRL */

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndos_rr(XD, XS)                                                    \
        EMITW(0x6EA19800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x6EA19800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rndos_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA19800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA19800 | MXM(RYG(XD), TmmM,    0x00))

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
        EMITW(0x4E21D800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4E21D800 | MXM(RYG(XD), RYG(XS), 0x00))

#define cvton_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E21D800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E21D800 | MXM(RYG(XD), TmmM,    0x00))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnros_rr(XD, XS, mode)                                              \
        EMITW(0x4E218800 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)      \
        EMITW(0x4E218800 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)

#define cvros_rr(XD, XS, mode)                                              \
        EMITW(0x4E21A800 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)      \
        EMITW(0x4E21A800 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)

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
        EMITW(0x3D800000 | MXM(TmmE,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x3D800000 | MXM(TmmF,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x3D800000 | MXM(TmmE+16, Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x3D800000 | MXM(TmmM,    Teax,    0x00))

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
        EMITW(0x3DC00000 | MXM(TmmE,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x3DC00000 | MXM(TmmF,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x3DC00000 | MXM(TmmE+16, Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x3DC00000 | MXM(TmmM,    Teax,    0x00))

#endif /* RT_256 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_A32_256_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

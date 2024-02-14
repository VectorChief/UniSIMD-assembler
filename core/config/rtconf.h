/******************************************************************************/
/* Copyright (c) 2013-2025 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTCONF_H
#define RT_RTCONF_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtconf.h: Configuration file for instruction subset mapping.
 * Table of contents is provided below.
 */

/*----------------------------------------------------------------------------*/

/************************   COMMON SIMD INSTRUCTIONS   ************************/

/**** var-len **** (rcp/rsq/fma/fms) with fixed-32-bit element ****************/
/**** 256-bit **** (rcp/rsq/fma/fms) with fixed-32-bit element ****************/
/**** 128-bit **** (rcp/rsq/fma/fms) with fixed-32-bit element ****************/
/**** scalar ***** (rcp/rsq/fma/fms) with fixed-32-bit element ****************/

/**** var-len **** (rcp/rsq/fma/fms) with fixed-64-bit element ****************/
/**** 256-bit **** (rcp/rsq/fma/fms) with fixed-64-bit element ****************/
/**** 128-bit **** (rcp/rsq/fma/fms) with fixed-64-bit element ****************/
/**** scalar ***** (rcp/rsq/fma/fms) with fixed-64-bit element ****************/

/**** var-len **** SIMD instructions with fixed-16-bit element **** 256-bit ***/
/**** var-len **** SIMD instructions with fixed-16-bit element **** 128-bit ***/
/**** var-len **** SIMD instructions with fixed-32-bit element **** 256-bit ***/
/**** var-len **** SIMD instructions with fixed-32-bit element **** 128-bit ***/
/**** var-len **** SIMD instructions with fixed-64-bit element **** 256-bit ***/
/**** var-len **** SIMD instructions with fixed-64-bit element **** 128-bit ***/

/**** var-len **** SIMD instructions with configurable element **** 32-bit ****/
/**** 256-bit **** SIMD instructions with configurable element **** 32-bit ****/
/**** 128-bit **** SIMD instructions with configurable element **** 32-bit ****/
/**** scalar ***** SIMD instructions with configurable element **** 32-bit ****/

/**** var-len **** SIMD instructions with configurable element **** 64-bit ****/
/**** 256-bit **** SIMD instructions with configurable element **** 64-bit ****/
/**** 128-bit **** SIMD instructions with configurable element **** 64-bit ****/
/**** scalar ***** SIMD instructions with configurable element **** 64-bit ****/

/************************   COMMON BASE INSTRUCTIONS   ************************/

/***************** address-sized instructions for 32-bit mode *****************/
/***************** address-sized instructions for 64-bit mode *****************/
/***************** element-sized instructions for 32-bit mode *****************/
/***************** element-sized instructions for 64-bit mode *****************/

/*----------------------------------------------------------------------------*/

/******************************************************************************/
/************************   COMMON SIMD INSTRUCTIONS   ************************/
/******************************************************************************/

/*
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
 * cmd*x_** - applies [cmd] to [p]acked unsigned integer args, [x] - default
 * cmd*n_** - applies [cmd] to [p]acked   signed integer args, [n] - negatable
 * cmd*s_** - applies [cmd] to [p]acked floating point   args, [s] - scalable
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

#if (defined RT_SIMD_CODE)

/******************************************************************************/
/**** var-len **** (rcp/rsq/fma/fms) with fixed-32-bit element ****************/
/******************************************************************************/

#if   (RT_SIMD >= 512) || (RT_SIMD == 256 && defined RT_SVEX1)

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if   RT_SIMD_COMPAT_RCP == 0 || RT_SIMD_COMPAT_RCP == 2

#define rcpos_rr(XD, XS) /* destroys XS */                                  \
        rceos_rr(W(XD), W(XS))                                              \
        rcsos_rr(W(XD), W(XS)) /* <- not reusable without extra temp reg */

#elif RT_SIMD_COMPAT_RCP == 1

#define rcpos_rr(XD, XS) /* destroys XS */                                  \
        movox_ld(W(XD), Mebp, inf_GPC01_32)                                 \
        divos_rr(W(XD), W(XS))

#define rceos_rr(XD, XS)                                                    \
        movox_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movox_ld(W(XD), Mebp, inf_GPC01_32)                                 \
        divos_ld(W(XD), Mebp, inf_SCR02(0))

#define rcsos_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if   RT_SIMD_COMPAT_RSQ == 0 || RT_SIMD_COMPAT_RSQ == 2

#define rsqos_rr(XD, XS) /* destroys XS */                                  \
        rseos_rr(W(XD), W(XS))                                              \
        rssos_rr(W(XD), W(XS)) /* <- not reusable without extra temp reg */

#elif RT_SIMD_COMPAT_RSQ == 1

#define rsqos_rr(XD, XS) /* destroys XS */                                  \
        sqros_rr(W(XS), W(XS))                                              \
        movox_ld(W(XD), Mebp, inf_GPC01_32)                                 \
        divos_rr(W(XD), W(XS))

#define rseos_rr(XD, XS)                                                    \
        sqros_rr(W(XD), W(XS))                                              \
        movox_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movox_ld(W(XD), Mebp, inf_GPC01_32)                                 \
        divos_ld(W(XD), Mebp, inf_SCR02(0))

#define rssos_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA == 2

#define fmaos_rr(XG, XS, XT)                                                \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulos_rr(W(XS), W(XT))                                              \
        addos_rr(W(XG), W(XS))                                              \
        movox_ld(W(XS), Mebp, inf_SCR01(0))

#define fmaos_ld(XG, XS, MT, DT)                                            \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulos_ld(W(XS), W(MT), W(DT))                                       \
        addos_rr(W(XG), W(XS))                                              \
        movox_ld(W(XS), Mebp, inf_SCR01(0))

#endif /* RT_SIMD_COMPAT_FMA */

#define fmaos3rr(XG, XS, XT)                                                \
        fmaos_rr(W(XG), W(XS), W(XT))

#define fmaos3ld(XG, XS, MT, DT)                                            \
        fmaos_ld(W(XG), W(XS), W(MT), W(DT))

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS == 2

#define fmsos_rr(XG, XS, XT)                                                \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulos_rr(W(XS), W(XT))                                              \
        subos_rr(W(XG), W(XS))                                              \
        movox_ld(W(XS), Mebp, inf_SCR01(0))

#define fmsos_ld(XG, XS, MT, DT)                                            \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulos_ld(W(XS), W(MT), W(DT))                                       \
        subos_rr(W(XG), W(XS))                                              \
        movox_ld(W(XS), Mebp, inf_SCR01(0))

#endif /* RT_SIMD_COMPAT_FMS */

#define fmsos3rr(XG, XS, XT)                                                \
        fmsos_rr(W(XG), W(XS), W(XT))

#define fmsos3ld(XG, XS, MT, DT)                                            \
        fmsos_ld(W(XG), W(XS), W(MT), W(DT))

#endif /* RT_SIMD: 2K8, 1K4, 512 */

/******************************************************************************/
/**** 256-bit **** (rcp/rsq/fma/fms) with fixed-32-bit element ****************/
/******************************************************************************/

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if   RT_SIMD_COMPAT_RCP == 0 || RT_SIMD_COMPAT_RCP == 2

#define rcpcs_rr(XD, XS) /* destroys XS */                                  \
        rcecs_rr(W(XD), W(XS))                                              \
        rcscs_rr(W(XD), W(XS)) /* <- not reusable without extra temp reg */

#elif RT_SIMD_COMPAT_RCP == 1

#define rcpcs_rr(XD, XS) /* destroys XS */                                  \
        movcx_ld(W(XD), Mebp, inf_GPC01_32)                                 \
        divcs_rr(W(XD), W(XS))

#define rcecs_rr(XD, XS)                                                    \
        movcx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movcx_ld(W(XD), Mebp, inf_GPC01_32)                                 \
        divcs_ld(W(XD), Mebp, inf_SCR02(0))

#define rcscs_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if   RT_SIMD_COMPAT_RSQ == 0 || RT_SIMD_COMPAT_RSQ == 2

#define rsqcs_rr(XD, XS) /* destroys XS */                                  \
        rsecs_rr(W(XD), W(XS))                                              \
        rsscs_rr(W(XD), W(XS)) /* <- not reusable without extra temp reg */

#elif RT_SIMD_COMPAT_RSQ == 1

#define rsqcs_rr(XD, XS) /* destroys XS */                                  \
        sqrcs_rr(W(XS), W(XS))                                              \
        movcx_ld(W(XD), Mebp, inf_GPC01_32)                                 \
        divcs_rr(W(XD), W(XS))

#define rsecs_rr(XD, XS)                                                    \
        sqrcs_rr(W(XD), W(XS))                                              \
        movcx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movcx_ld(W(XD), Mebp, inf_GPC01_32)                                 \
        divcs_ld(W(XD), Mebp, inf_SCR02(0))

#define rsscs_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA == 2

#define fmacs_rr(XG, XS, XT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulcs_rr(W(XS), W(XT))                                              \
        addcs_rr(W(XG), W(XS))                                              \
        movcx_ld(W(XS), Mebp, inf_SCR01(0))

#define fmacs_ld(XG, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulcs_ld(W(XS), W(MT), W(DT))                                       \
        addcs_rr(W(XG), W(XS))                                              \
        movcx_ld(W(XS), Mebp, inf_SCR01(0))

#endif /* RT_SIMD_COMPAT_FMA */

#define fmacs3rr(XG, XS, XT)                                                \
        fmacs_rr(W(XG), W(XS), W(XT))

#define fmacs3ld(XG, XS, MT, DT)                                            \
        fmacs_ld(W(XG), W(XS), W(MT), W(DT))

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS == 2

#define fmscs_rr(XG, XS, XT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulcs_rr(W(XS), W(XT))                                              \
        subcs_rr(W(XG), W(XS))                                              \
        movcx_ld(W(XS), Mebp, inf_SCR01(0))

#define fmscs_ld(XG, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulcs_ld(W(XS), W(MT), W(DT))                                       \
        subcs_rr(W(XG), W(XS))                                              \
        movcx_ld(W(XS), Mebp, inf_SCR01(0))

#endif /* RT_SIMD_COMPAT_FMS */

#define fmscs3rr(XG, XS, XT)                                                \
        fmscs_rr(W(XG), W(XS), W(XT))

#define fmscs3ld(XG, XS, MT, DT)                                            \
        fmscs_ld(W(XG), W(XS), W(MT), W(DT))

/******************************************************************************/
/**** 128-bit **** (rcp/rsq/fma/fms) with fixed-32-bit element ****************/
/******************************************************************************/

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if   RT_SIMD_COMPAT_RCP == 0 || RT_SIMD_COMPAT_RCP == 2

#define rcpis_rr(XD, XS) /* destroys XS */                                  \
        rceis_rr(W(XD), W(XS))                                              \
        rcsis_rr(W(XD), W(XS)) /* <- not reusable without extra temp reg */

#elif RT_SIMD_COMPAT_RCP == 1

#define rcpis_rr(XD, XS) /* destroys XS */                                  \
        movix_ld(W(XD), Mebp, inf_GPC01_32)                                 \
        divis_rr(W(XD), W(XS))

#define rceis_rr(XD, XS)                                                    \
        movix_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XD), Mebp, inf_GPC01_32)                                 \
        divis_ld(W(XD), Mebp, inf_SCR02(0))

#define rcsis_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if   RT_SIMD_COMPAT_RSQ == 0 || RT_SIMD_COMPAT_RSQ == 2

#define rsqis_rr(XD, XS) /* destroys XS */                                  \
        rseis_rr(W(XD), W(XS))                                              \
        rssis_rr(W(XD), W(XS)) /* <- not reusable without extra temp reg */

#elif RT_SIMD_COMPAT_RSQ == 1

#define rsqis_rr(XD, XS) /* destroys XS */                                  \
        sqris_rr(W(XS), W(XS))                                              \
        movix_ld(W(XD), Mebp, inf_GPC01_32)                                 \
        divis_rr(W(XD), W(XS))

#define rseis_rr(XD, XS)                                                    \
        sqris_rr(W(XD), W(XS))                                              \
        movix_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XD), Mebp, inf_GPC01_32)                                 \
        divis_ld(W(XD), Mebp, inf_SCR02(0))

#define rssis_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA == 2

#define fmais_rr(XG, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulis_rr(W(XS), W(XT))                                              \
        addis_rr(W(XG), W(XS))                                              \
        movix_ld(W(XS), Mebp, inf_SCR01(0))

#define fmais_ld(XG, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulis_ld(W(XS), W(MT), W(DT))                                       \
        addis_rr(W(XG), W(XS))                                              \
        movix_ld(W(XS), Mebp, inf_SCR01(0))

#endif /* RT_SIMD_COMPAT_FMA */

#define fmais3rr(XG, XS, XT)                                                \
        fmais_rr(W(XG), W(XS), W(XT))

#define fmais3ld(XG, XS, MT, DT)                                            \
        fmais_ld(W(XG), W(XS), W(MT), W(DT))

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS == 2

#define fmsis_rr(XG, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulis_rr(W(XS), W(XT))                                              \
        subis_rr(W(XG), W(XS))                                              \
        movix_ld(W(XS), Mebp, inf_SCR01(0))

#define fmsis_ld(XG, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulis_ld(W(XS), W(MT), W(DT))                                       \
        subis_rr(W(XG), W(XS))                                              \
        movix_ld(W(XS), Mebp, inf_SCR01(0))

#endif /* RT_SIMD_COMPAT_FMS */

#define fmsis3rr(XG, XS, XT)                                                \
        fmsis_rr(W(XG), W(XS), W(XT))

#define fmsis3ld(XG, XS, MT, DT)                                            \
        fmsis_ld(W(XG), W(XS), W(MT), W(DT))

/******************************************************************************/
/**** scalar ***** (rcp/rsq/fma/fms) with fixed-32-bit element ****************/
/******************************************************************************/

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if   RT_SIMD_COMPAT_RCP == 0 || RT_SIMD_COMPAT_RCP == 2

#define rcprs_rr(XD, XS) /* destroys XS */                                  \
        rcers_rr(W(XD), W(XS))                                              \
        rcsrs_rr(W(XD), W(XS)) /* <- not reusable without extra temp reg */

#elif RT_SIMD_COMPAT_RCP == 1

#define rcprs_rr(XD, XS) /* destroys XS */                                  \
        movrs_ld(W(XD), Mebp, inf_GPC01_32)                                 \
        divrs_rr(W(XD), W(XS))

#define rcers_rr(XD, XS)                                                    \
        movrs_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movrs_ld(W(XD), Mebp, inf_GPC01_32)                                 \
        divrs_ld(W(XD), Mebp, inf_SCR02(0))

#define rcsrs_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if   RT_SIMD_COMPAT_RSQ == 0 || RT_SIMD_COMPAT_RSQ == 2

#define rsqrs_rr(XD, XS) /* destroys XS */                                  \
        rsers_rr(W(XD), W(XS))                                              \
        rssrs_rr(W(XD), W(XS)) /* <- not reusable without extra temp reg */

#elif RT_SIMD_COMPAT_RSQ == 1

#define rsqrs_rr(XD, XS) /* destroys XS */                                  \
        sqrrs_rr(W(XS), W(XS))                                              \
        movrs_ld(W(XD), Mebp, inf_GPC01_32)                                 \
        divrs_rr(W(XD), W(XS))

#define rsers_rr(XD, XS)                                                    \
        sqrrs_rr(W(XD), W(XS))                                              \
        movrs_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movrs_ld(W(XD), Mebp, inf_GPC01_32)                                 \
        divrs_ld(W(XD), Mebp, inf_SCR02(0))

#define rssrs_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA == 2

#define fmars_rr(XG, XS, XT)                                                \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulrs_rr(W(XS), W(XT))                                              \
        addrs_rr(W(XG), W(XS))                                              \
        movrs_ld(W(XS), Mebp, inf_SCR01(0))

#define fmars_ld(XG, XS, MT, DT)                                            \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulrs_ld(W(XS), W(MT), W(DT))                                       \
        addrs_rr(W(XG), W(XS))                                              \
        movrs_ld(W(XS), Mebp, inf_SCR01(0))

#endif /* RT_SIMD_COMPAT_FMA */

#define fmars3rr(XG, XS, XT)                                                \
        fmars_rr(W(XG), W(XS), W(XT))

#define fmars3ld(XG, XS, MT, DT)                                            \
        fmars_ld(W(XG), W(XS), W(MT), W(DT))

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS == 2

#define fmsrs_rr(XG, XS, XT)                                                \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulrs_rr(W(XS), W(XT))                                              \
        subrs_rr(W(XG), W(XS))                                              \
        movrs_ld(W(XS), Mebp, inf_SCR01(0))

#define fmsrs_ld(XG, XS, MT, DT)                                            \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulrs_ld(W(XS), W(MT), W(DT))                                       \
        subrs_rr(W(XG), W(XS))                                              \
        movrs_ld(W(XS), Mebp, inf_SCR01(0))

#endif /* RT_SIMD_COMPAT_FMS */

#define fmsrs3rr(XG, XS, XT)                                                \
        fmsrs_rr(W(XG), W(XS), W(XT))

#define fmsrs3ld(XG, XS, MT, DT)                                            \
        fmsrs_ld(W(XG), W(XS), W(MT), W(DT))

/******************************************************************************/
/**** var-len **** (rcp/rsq/fma/fms) with fixed-64-bit element ****************/
/******************************************************************************/

#if   (RT_SIMD >= 512) || (RT_SIMD == 256 && defined RT_SVEX1)

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if   RT_SIMD_COMPAT_RCP == 0 || RT_SIMD_COMPAT_RCP == 2

#define rcpqs_rr(XD, XS) /* destroys XS */                                  \
        rceqs_rr(W(XD), W(XS))                                              \
        rcsqs_rr(W(XD), W(XS)) /* <- not reusable without extra temp reg */

#elif RT_SIMD_COMPAT_RCP == 1

#define rcpqs_rr(XD, XS) /* destroys XS */                                  \
        movqx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divqs_rr(W(XD), W(XS))

#define rceqs_rr(XD, XS)                                                    \
        movqx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movqx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divqs_ld(W(XD), Mebp, inf_SCR02(0))

#define rcsqs_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if   RT_SIMD_COMPAT_RSQ == 0 || RT_SIMD_COMPAT_RSQ == 2

#define rsqqs_rr(XD, XS) /* destroys XS */                                  \
        rseqs_rr(W(XD), W(XS))                                              \
        rssqs_rr(W(XD), W(XS)) /* <- not reusable without extra temp reg */

#elif RT_SIMD_COMPAT_RSQ == 1

#define rsqqs_rr(XD, XS) /* destroys XS */                                  \
        sqrqs_rr(W(XS), W(XS))                                              \
        movqx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divqs_rr(W(XD), W(XS))

#define rseqs_rr(XD, XS)                                                    \
        sqrqs_rr(W(XD), W(XS))                                              \
        movqx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movqx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divqs_ld(W(XD), Mebp, inf_SCR02(0))

#define rssqs_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA == 2

#define fmaqs_rr(XG, XS, XT)                                                \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulqs_rr(W(XS), W(XT))                                              \
        addqs_rr(W(XG), W(XS))                                              \
        movqx_ld(W(XS), Mebp, inf_SCR01(0))

#define fmaqs_ld(XG, XS, MT, DT)                                            \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulqs_ld(W(XS), W(MT), W(DT))                                       \
        addqs_rr(W(XG), W(XS))                                              \
        movqx_ld(W(XS), Mebp, inf_SCR01(0))

#endif /* RT_SIMD_COMPAT_FMA */

#define fmaqs3rr(XG, XS, XT)                                                \
        fmaqs_rr(W(XG), W(XS), W(XT))

#define fmaqs3ld(XG, XS, MT, DT)                                            \
        fmaqs_ld(W(XG), W(XS), W(MT), W(DT))

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS == 2

#define fmsqs_rr(XG, XS, XT)                                                \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulqs_rr(W(XS), W(XT))                                              \
        subqs_rr(W(XG), W(XS))                                              \
        movqx_ld(W(XS), Mebp, inf_SCR01(0))

#define fmsqs_ld(XG, XS, MT, DT)                                            \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulqs_ld(W(XS), W(MT), W(DT))                                       \
        subqs_rr(W(XG), W(XS))                                              \
        movqx_ld(W(XS), Mebp, inf_SCR01(0))

#endif /* RT_SIMD_COMPAT_FMS */

#define fmsqs3rr(XG, XS, XT)                                                \
        fmsqs_rr(W(XG), W(XS), W(XT))

#define fmsqs3ld(XG, XS, MT, DT)                                            \
        fmsqs_ld(W(XG), W(XS), W(MT), W(DT))

#endif /* RT_SIMD: 2K8, 1K4, 512 */

/******************************************************************************/
/**** 256-bit **** (rcp/rsq/fma/fms) with fixed-64-bit element ****************/
/******************************************************************************/

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if   RT_SIMD_COMPAT_RCP == 0 || RT_SIMD_COMPAT_RCP == 2

#define rcpds_rr(XD, XS) /* destroys XS */                                  \
        rceds_rr(W(XD), W(XS))                                              \
        rcsds_rr(W(XD), W(XS)) /* <- not reusable without extra temp reg */

#elif RT_SIMD_COMPAT_RCP == 1

#define rcpds_rr(XD, XS) /* destroys XS */                                  \
        movdx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divds_rr(W(XD), W(XS))

#define rceds_rr(XD, XS)                                                    \
        movdx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movdx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divds_ld(W(XD), Mebp, inf_SCR02(0))

#define rcsds_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if   RT_SIMD_COMPAT_RSQ == 0 || RT_SIMD_COMPAT_RSQ == 2

#define rsqds_rr(XD, XS) /* destroys XS */                                  \
        rseds_rr(W(XD), W(XS))                                              \
        rssds_rr(W(XD), W(XS)) /* <- not reusable without extra temp reg */

#elif RT_SIMD_COMPAT_RSQ == 1

#define rsqds_rr(XD, XS) /* destroys XS */                                  \
        sqrds_rr(W(XS), W(XS))                                              \
        movdx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divds_rr(W(XD), W(XS))

#define rseds_rr(XD, XS)                                                    \
        sqrds_rr(W(XD), W(XS))                                              \
        movdx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movdx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divds_ld(W(XD), Mebp, inf_SCR02(0))

#define rssds_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA == 2

#define fmads_rr(XG, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulds_rr(W(XS), W(XT))                                              \
        addds_rr(W(XG), W(XS))                                              \
        movdx_ld(W(XS), Mebp, inf_SCR01(0))

#define fmads_ld(XG, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulds_ld(W(XS), W(MT), W(DT))                                       \
        addds_rr(W(XG), W(XS))                                              \
        movdx_ld(W(XS), Mebp, inf_SCR01(0))

#endif /* RT_SIMD_COMPAT_FMA */

#define fmads3rr(XG, XS, XT)                                                \
        fmads_rr(W(XG), W(XS), W(XT))

#define fmads3ld(XG, XS, MT, DT)                                            \
        fmads_ld(W(XG), W(XS), W(MT), W(DT))

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS == 2

#define fmsds_rr(XG, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulds_rr(W(XS), W(XT))                                              \
        subds_rr(W(XG), W(XS))                                              \
        movdx_ld(W(XS), Mebp, inf_SCR01(0))

#define fmsds_ld(XG, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulds_ld(W(XS), W(MT), W(DT))                                       \
        subds_rr(W(XG), W(XS))                                              \
        movdx_ld(W(XS), Mebp, inf_SCR01(0))

#endif /* RT_SIMD_COMPAT_FMS */

#define fmsds3rr(XG, XS, XT)                                                \
        fmsds_rr(W(XG), W(XS), W(XT))

#define fmsds3ld(XG, XS, MT, DT)                                            \
        fmsds_ld(W(XG), W(XS), W(MT), W(DT))

/******************************************************************************/
/**** 128-bit **** (rcp/rsq/fma/fms) with fixed-64-bit element ****************/
/******************************************************************************/

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if   RT_SIMD_COMPAT_RCP == 0 || RT_SIMD_COMPAT_RCP == 2

#define rcpjs_rr(XD, XS) /* destroys XS */                                  \
        rcejs_rr(W(XD), W(XS))                                              \
        rcsjs_rr(W(XD), W(XS)) /* <- not reusable without extra temp reg */

#elif RT_SIMD_COMPAT_RCP == 1

#define rcpjs_rr(XD, XS) /* destroys XS */                                  \
        movjx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divjs_rr(W(XD), W(XS))

#define rcejs_rr(XD, XS)                                                    \
        movjx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movjx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divjs_ld(W(XD), Mebp, inf_SCR02(0))

#define rcsjs_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if   RT_SIMD_COMPAT_RSQ == 0 || RT_SIMD_COMPAT_RSQ == 2

#define rsqjs_rr(XD, XS) /* destroys XS */                                  \
        rsejs_rr(W(XD), W(XS))                                              \
        rssjs_rr(W(XD), W(XS)) /* <- not reusable without extra temp reg */

#elif RT_SIMD_COMPAT_RSQ == 1

#define rsqjs_rr(XD, XS) /* destroys XS */                                  \
        sqrjs_rr(W(XS), W(XS))                                              \
        movjx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divjs_rr(W(XD), W(XS))

#define rsejs_rr(XD, XS)                                                    \
        sqrjs_rr(W(XD), W(XS))                                              \
        movjx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movjx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divjs_ld(W(XD), Mebp, inf_SCR02(0))

#define rssjs_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA == 2

#define fmajs_rr(XG, XS, XT)                                                \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        muljs_rr(W(XS), W(XT))                                              \
        addjs_rr(W(XG), W(XS))                                              \
        movjx_ld(W(XS), Mebp, inf_SCR01(0))

#define fmajs_ld(XG, XS, MT, DT)                                            \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        muljs_ld(W(XS), W(MT), W(DT))                                       \
        addjs_rr(W(XG), W(XS))                                              \
        movjx_ld(W(XS), Mebp, inf_SCR01(0))

#endif /* RT_SIMD_COMPAT_FMA */

#define fmajs3rr(XG, XS, XT)                                                \
        fmajs_rr(W(XG), W(XS), W(XT))

#define fmajs3ld(XG, XS, MT, DT)                                            \
        fmajs_ld(W(XG), W(XS), W(MT), W(DT))

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS == 2

#define fmsjs_rr(XG, XS, XT)                                                \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        muljs_rr(W(XS), W(XT))                                              \
        subjs_rr(W(XG), W(XS))                                              \
        movjx_ld(W(XS), Mebp, inf_SCR01(0))

#define fmsjs_ld(XG, XS, MT, DT)                                            \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        muljs_ld(W(XS), W(MT), W(DT))                                       \
        subjs_rr(W(XG), W(XS))                                              \
        movjx_ld(W(XS), Mebp, inf_SCR01(0))

#endif /* RT_SIMD_COMPAT_FMS */

#define fmsjs3rr(XG, XS, XT)                                                \
        fmsjs_rr(W(XG), W(XS), W(XT))

#define fmsjs3ld(XG, XS, MT, DT)                                            \
        fmsjs_ld(W(XG), W(XS), W(MT), W(DT))

/******************************************************************************/
/**** scalar ***** (rcp/rsq/fma/fms) with fixed-64-bit element ****************/
/******************************************************************************/

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if   RT_SIMD_COMPAT_RCP == 0 || RT_SIMD_COMPAT_RCP == 2

#define rcpts_rr(XD, XS) /* destroys XS */                                  \
        rcets_rr(W(XD), W(XS))                                              \
        rcsts_rr(W(XD), W(XS)) /* <- not reusable without extra temp reg */

#elif RT_SIMD_COMPAT_RCP == 1

#define rcpts_rr(XD, XS) /* destroys XS */                                  \
        movts_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divts_rr(W(XD), W(XS))

#define rcets_rr(XD, XS)                                                    \
        movts_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movts_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divts_ld(W(XD), Mebp, inf_SCR02(0))

#define rcsts_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if   RT_SIMD_COMPAT_RSQ == 0 || RT_SIMD_COMPAT_RSQ == 2

#define rsqts_rr(XD, XS) /* destroys XS */                                  \
        rsets_rr(W(XD), W(XS))                                              \
        rssts_rr(W(XD), W(XS)) /* <- not reusable without extra temp reg */

#elif RT_SIMD_COMPAT_RSQ == 1

#define rsqts_rr(XD, XS) /* destroys XS */                                  \
        sqrts_rr(W(XS), W(XS))                                              \
        movts_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divts_rr(W(XD), W(XS))

#define rsets_rr(XD, XS)                                                    \
        sqrts_rr(W(XD), W(XS))                                              \
        movts_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movts_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divts_ld(W(XD), Mebp, inf_SCR02(0))

#define rssts_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA == 2

#define fmats_rr(XG, XS, XT)                                                \
        movts_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mults_rr(W(XS), W(XT))                                              \
        addts_rr(W(XG), W(XS))                                              \
        movts_ld(W(XS), Mebp, inf_SCR01(0))

#define fmats_ld(XG, XS, MT, DT)                                            \
        movts_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mults_ld(W(XS), W(MT), W(DT))                                       \
        addts_rr(W(XG), W(XS))                                              \
        movts_ld(W(XS), Mebp, inf_SCR01(0))

#endif /* RT_SIMD_COMPAT_FMA */

#define fmats3rr(XG, XS, XT)                                                \
        fmats_rr(W(XG), W(XS), W(XT))

#define fmats3ld(XG, XS, MT, DT)                                            \
        fmats_ld(W(XG), W(XS), W(MT), W(DT))

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS == 2

#define fmsts_rr(XG, XS, XT)                                                \
        movts_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mults_rr(W(XS), W(XT))                                              \
        subts_rr(W(XG), W(XS))                                              \
        movts_ld(W(XS), Mebp, inf_SCR01(0))

#define fmsts_ld(XG, XS, MT, DT)                                            \
        movts_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mults_ld(W(XS), W(MT), W(DT))                                       \
        subts_rr(W(XG), W(XS))                                              \
        movts_ld(W(XS), Mebp, inf_SCR01(0))

#endif /* RT_SIMD_COMPAT_FMS */

#define fmsts3rr(XG, XS, XT)                                                \
        fmsts_rr(W(XG), W(XS), W(XT))

#define fmsts3ld(XG, XS, MT, DT)                                            \
        fmsts_ld(W(XG), W(XS), W(MT), W(DT))

/******************************************************************************/
/**** var-len **** SIMD instructions with fixed-16-bit element **** 256-bit ***/
/******************************************************************************/

#if   (RT_SIMD == 256) && !(defined RT_SVEX1)

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmmx_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        elmax_st(W(XS), W(MD), W(DD))

/****************   packed half-precision generic move/logic   ****************/

/* mov (D = S) */

#define movmx_rr(XD, XS)                                                    \
        movax_rr(W(XD), W(XS))

#define movmx_ld(XD, MS, DS)                                                \
        movax_ld(W(XD), W(MS), W(DS))

#define movmx_st(XS, MD, DD)                                                \
        movax_st(W(XS), W(MD), W(DD))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvmx_rr(XD, XS)                                                    \
        mmvax_rr(W(XD), W(XS))

#define mmvmx_ld(XG, MS, DS)                                                \
        mmvax_ld(W(XG), W(MS), W(DS))

#define mmvmx_st(XS, MG, DG)                                                \
        mmvax_st(W(XS), W(MG), W(DG))


#define mmvmb_rr(XD, XS)                                                    \
        mmvab_rr(W(XD), W(XS))

#define mmvmb_ld(XG, MS, DS)                                                \
        mmvab_ld(W(XG), W(MS), W(DS))

#define mmvmb_st(XS, MG, DG)                                                \
        mmvab_st(W(XS), W(MG), W(DG))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andmx_rr(XG, XS)                                                    \
        andax_rr(W(XG), W(XS))

#define andmx_ld(XG, MS, DS)                                                \
        andax_ld(W(XG), W(MS), W(DS))

#define andmx3rr(XD, XS, XT)                                                \
        andax3rr(W(XD), W(XS), W(XT))

#define andmx3ld(XD, XS, MT, DT)                                            \
        andax3ld(W(XD), W(XS), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annmx_rr(XG, XS)                                                    \
        annax_rr(W(XG), W(XS))

#define annmx_ld(XG, MS, DS)                                                \
        annax_ld(W(XG), W(MS), W(DS))

#define annmx3rr(XD, XS, XT)                                                \
        annax3rr(W(XD), W(XS), W(XT))

#define annmx3ld(XD, XS, MT, DT)                                            \
        annax3ld(W(XD), W(XS), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrmx_rr(XG, XS)                                                    \
        orrax_rr(W(XG), W(XS))

#define orrmx_ld(XG, MS, DS)                                                \
        orrax_ld(W(XG), W(MS), W(DS))

#define orrmx3rr(XD, XS, XT)                                                \
        orrax3rr(W(XD), W(XS), W(XT))

#define orrmx3ld(XD, XS, MT, DT)                                            \
        orrax3ld(W(XD), W(XS), W(MT), W(DT))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define ornmx_rr(XG, XS)                                                    \
        ornax_rr(W(XG), W(XS))

#define ornmx_ld(XG, MS, DS)                                                \
        ornax_ld(W(XG), W(MS), W(DS))

#define ornmx3rr(XD, XS, XT)                                                \
        ornax3rr(W(XD), W(XS), W(XT))

#define ornmx3ld(XD, XS, MT, DT)                                            \
        ornax3ld(W(XD), W(XS), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xormx_rr(XG, XS)                                                    \
        xorax_rr(W(XG), W(XS))

#define xormx_ld(XG, MS, DS)                                                \
        xorax_ld(W(XG), W(MS), W(DS))

#define xormx3rr(XD, XS, XT)                                                \
        xorax3rr(W(XD), W(XS), W(XT))

#define xormx3ld(XD, XS, MT, DT)                                            \
        xorax3ld(W(XD), W(XS), W(MT), W(DT))

/* not (G = ~G), (D = ~S) */

#define notmx_rx(XG)                                                        \
        notax_rx(W(XG))

#define notmx_rr(XD, XS)                                                    \
        notax_rr(W(XD), W(XS))

/*************   packed half-precision floating-point arithmetic   ************/

/* neg (G = -G), (D = -S) */

#define negms_rx(XG)                                                        \
        negas_rx(W(XG))

#define negms_rr(XD, XS)                                                    \
        negas_rr(W(XD), W(XS))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addms_rr(XG, XS)                                                    \
        addas_rr(W(XG), W(XS))

#define addms_ld(XG, MS, DS)                                                \
        addas_ld(W(XG), W(MS), W(DS))

#define addms3rr(XD, XS, XT)                                                \
        addas3rr(W(XD), W(XS), W(XT))

#define addms3ld(XD, XS, MT, DT)                                            \
        addas3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subms_rr(XG, XS)                                                    \
        subas_rr(W(XG), W(XS))

#define subms_ld(XG, MS, DS)                                                \
        subas_ld(W(XG), W(MS), W(DS))

#define subms3rr(XD, XS, XT)                                                \
        subas3rr(W(XD), W(XS), W(XT))

#define subms3ld(XD, XS, MT, DT)                                            \
        subas3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulms_rr(XG, XS)                                                    \
        mulas_rr(W(XG), W(XS))

#define mulms_ld(XG, MS, DS)                                                \
        mulas_ld(W(XG), W(MS), W(DS))

#define mulms3rr(XD, XS, XT)                                                \
        mulas3rr(W(XD), W(XS), W(XT))

#define mulms3ld(XD, XS, MT, DT)                                            \
        mulas3ld(W(XD), W(XS), W(MT), W(DT))

/* div (G = G / S), (D = S / T) if (#D != #T) */

#define divms_rr(XG, XS)                                                    \
        divas_rr(W(XG), W(XS))

#define divms_ld(XG, MS, DS)                                                \
        divas_ld(W(XG), W(MS), W(DS))

#define divms3rr(XD, XS, XT)                                                \
        divas3rr(W(XD), W(XS), W(XT))

#define divms3ld(XD, XS, MT, DT)                                            \
        divas3ld(W(XD), W(XS), W(MT), W(DT))

/* sqr (D = sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define sqrms_rr(XD, XS)                                                    \
        sqras_rr(W(XD), W(XS))

#define sqrms_ld(XD, MS, DS)                                                \
        sqras_ld(W(XD), W(MS), W(DS))

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rcems_rr(XD, XS)                                                    \
        rceas_rr(W(XD), W(XS))

#define rcsms_rr(XG, XS) /* destroys XS */                                  \
        rcsas_rr(W(XG), W(XS))

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rsems_rr(XD, XS)                                                    \
        rseas_rr(W(XD), W(XS))

#define rssms_rr(XG, XS) /* destroys XS */                                  \
        rssas_rr(W(XG), W(XS))

/* fma (G = G + S * T) if (#G != #S && #G != #T) */

#define fmams_rr(XG, XS, XT)                                                \
        fmaas_rr(W(XG), W(XS), W(XT))

#define fmams_ld(XG, XS, MT, DT)                                            \
        fmaas_ld(W(XG), W(XS), W(MT), W(DT))

#define fmams3rr(XG, XS, XT)                                                \
        fmams_rr(W(XG), W(XS), W(XT))

#define fmams3ld(XG, XS, MT, DT)                                            \
        fmams_ld(W(XG), W(XS), W(MT), W(DT))

#define fmaas3rr(XG, XS, XT)                                                \
        fmaas_rr(W(XG), W(XS), W(XT))

#define fmaas3ld(XG, XS, MT, DT)                                            \
        fmaas_ld(W(XG), W(XS), W(MT), W(DT))

#define fmags3rr(XG, XS, XT)                                                \
        fmags_rr(W(XG), W(XS), W(XT))

#define fmags3ld(XG, XS, MT, DT)                                            \
        fmags_ld(W(XG), W(XS), W(MT), W(DT))

/* fms (G = G - S * T) if (#G != #S && #G != #T) */

#define fmsms_rr(XG, XS, XT)                                                \
        fmsas_rr(W(XG), W(XS), W(XT))

#define fmsms_ld(XG, XS, MT, DT)                                            \
        fmsas_ld(W(XG), W(XS), W(MT), W(DT))

#define fmsms3rr(XG, XS, XT)                                                \
        fmsms_rr(W(XG), W(XS), W(XT))

#define fmsms3ld(XG, XS, MT, DT)                                            \
        fmsms_ld(W(XG), W(XS), W(MT), W(DT))

#define fmsas3rr(XG, XS, XT)                                                \
        fmsas_rr(W(XG), W(XS), W(XT))

#define fmsas3ld(XG, XS, MT, DT)                                            \
        fmsas_ld(W(XG), W(XS), W(MT), W(DT))

#define fmsgs3rr(XG, XS, XT)                                                \
        fmsgs_rr(W(XG), W(XS), W(XT))

#define fmsgs3ld(XG, XS, MT, DT)                                            \
        fmsgs_ld(W(XG), W(XS), W(MT), W(DT))

/**************   packed half-precision floating-point compare   **************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minms_rr(XG, XS)                                                    \
        minas_rr(W(XG), W(XS))

#define minms_ld(XG, MS, DS)                                                \
        minas_ld(W(XG), W(MS), W(DS))

#define minms3rr(XD, XS, XT)                                                \
        minas3rr(W(XD), W(XS), W(XT))

#define minms3ld(XD, XS, MT, DT)                                            \
        minas3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxms_rr(XG, XS)                                                    \
        maxas_rr(W(XG), W(XS))

#define maxms_ld(XG, MS, DS)                                                \
        maxas_ld(W(XG), W(MS), W(DS))

#define maxms3rr(XD, XS, XT)                                                \
        maxas3rr(W(XD), W(XS), W(XT))

#define maxms3ld(XD, XS, MT, DT)                                            \
        maxas3ld(W(XD), W(XS), W(MT), W(DT))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqms_rr(XG, XS)                                                    \
        ceqas_rr(W(XG), W(XS))

#define ceqms_ld(XG, MS, DS)                                                \
        ceqas_ld(W(XG), W(MS), W(DS))

#define ceqms3rr(XD, XS, XT)                                                \
        ceqas3rr(W(XD), W(XS), W(XT))

#define ceqms3ld(XD, XS, MT, DT)                                            \
        ceqas3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnems_rr(XG, XS)                                                    \
        cneas_rr(W(XG), W(XS))

#define cnems_ld(XG, MS, DS)                                                \
        cneas_ld(W(XG), W(MS), W(DS))

#define cnems3rr(XD, XS, XT)                                                \
        cneas3rr(W(XD), W(XS), W(XT))

#define cnems3ld(XD, XS, MT, DT)                                            \
        cneas3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltms_rr(XG, XS)                                                    \
        cltas_rr(W(XG), W(XS))

#define cltms_ld(XG, MS, DS)                                                \
        cltas_ld(W(XG), W(MS), W(DS))

#define cltms3rr(XD, XS, XT)                                                \
        cltas3rr(W(XD), W(XS), W(XT))

#define cltms3ld(XD, XS, MT, DT)                                            \
        cltas3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define clems_rr(XG, XS)                                                    \
        cleas_rr(W(XG), W(XS))

#define clems_ld(XG, MS, DS)                                                \
        cleas_ld(W(XG), W(MS), W(DS))

#define clems3rr(XD, XS, XT)                                                \
        cleas3rr(W(XD), W(XS), W(XT))

#define clems3ld(XD, XS, MT, DT)                                            \
        cleas3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtms_rr(XG, XS)                                                    \
        cgtas_rr(W(XG), W(XS))

#define cgtms_ld(XG, MS, DS)                                                \
        cgtas_ld(W(XG), W(MS), W(DS))

#define cgtms3rr(XD, XS, XT)                                                \
        cgtas3rr(W(XD), W(XS), W(XT))

#define cgtms3ld(XD, XS, MT, DT)                                            \
        cgtas3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgems_rr(XG, XS)                                                    \
        cgeas_rr(W(XG), W(XS))

#define cgems_ld(XG, MS, DS)                                                \
        cgeas_ld(W(XG), W(MS), W(DS))

#define cgems3rr(XD, XS, XT)                                                \
        cgeas3rr(W(XD), W(XS), W(XT))

#define cgems3ld(XD, XS, MT, DT)                                            \
        cgeas3ld(W(XD), W(XS), W(MT), W(DT))

/**************   packed half-precision floating-point convert   **************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnzms_rr(XD, XS)     /* round towards zero */                       \
        rnzas_rr(W(XD), W(XS))

#define rnzms_ld(XD, MS, DS) /* round towards zero */                       \
        rnzas_ld(W(XD), W(MS), W(DS))

#define cvzms_rr(XD, XS)     /* round towards zero */                       \
        cvzas_rr(W(XD), W(XS))

#define cvzms_ld(XD, MS, DS) /* round towards zero */                       \
        cvzas_ld(W(XD), W(MS), W(DS))

/* cvp (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnpms_rr(XD, XS)     /* round towards +inf */                       \
        rnpas_rr(W(XD), W(XS))

#define rnpms_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpas_ld(W(XD), W(MS), W(DS))

#define cvpms_rr(XD, XS)     /* round towards +inf */                       \
        cvpas_rr(W(XD), W(XS))

#define cvpms_ld(XD, MS, DS) /* round towards +inf */                       \
        cvpas_ld(W(XD), W(MS), W(DS))

/* cvm (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnmms_rr(XD, XS)     /* round towards -inf */                       \
        rnmas_rr(W(XD), W(XS))

#define rnmms_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmas_ld(W(XD), W(MS), W(DS))

#define cvmms_rr(XD, XS)     /* round towards -inf */                       \
        cvmas_rr(W(XD), W(XS))

#define cvmms_ld(XD, MS, DS) /* round towards -inf */                       \
        cvmas_ld(W(XD), W(MS), W(DS))

/* cvn (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnnms_rr(XD, XS)     /* round towards near */                       \
        rnnas_rr(W(XD), W(XS))

#define rnnms_ld(XD, MS, DS) /* round towards near */                       \
        rnnas_ld(W(XD), W(MS), W(DS))

#define cvnms_rr(XD, XS)     /* round towards near */                       \
        cvnas_rr(W(XD), W(XS))

#define cvnms_ld(XD, MS, DS) /* round towards near */                       \
        cvnas_ld(W(XD), W(MS), W(DS))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnmn_rr(XD, XS)     /* round towards near */                       \
        cvnan_rr(W(XD), W(XS))

#define cvnmn_ld(XD, MS, DS) /* round towards near */                       \
        cvnan_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from control register (set in FCTRL blocks) */

#define rndms_rr(XD, XS)                                                    \
        rndas_rr(W(XD), W(XS))

#define rndms_ld(XD, MS, DS)                                                \
        rndas_ld(W(XD), W(MS), W(DS))

#define cvtms_rr(XD, XS)                                                    \
        cvtas_rr(W(XD), W(XS))

#define cvtms_ld(XD, MS, DS)                                                \
        cvtas_ld(W(XD), W(MS), W(DS))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from control register (set in FCTRL blocks) */

#define cvtmn_rr(XD, XS)                                                    \
        cvtan_rr(W(XD), W(XS))

#define cvtmn_ld(XD, MS, DS)                                                \
        cvtan_ld(W(XD), W(MS), W(DS))

/* cvn (D = unsigned-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnmx_rr(XD, XS)     /* round towards near */                       \
        cvnax_rr(W(XD), W(XS))

#define cvnmx_ld(XD, MS, DS) /* round towards near */                       \
        cvnax_ld(W(XD), W(MS), W(DS))

/* cvt (D = unsigned-int-to-fp S)
 * rounding mode comes from control register (set in FCTRL blocks) */

#define cvtmx_rr(XD, XS)                                                    \
        cvtax_rr(W(XD), W(XS))

#define cvtmx_ld(XD, MS, DS)                                                \
        cvtax_ld(W(XD), W(MS), W(DS))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnrms_rr(XD, XS, mode)                                              \
        rnras_rr(W(XD), W(XS), mode)

#define cvrms_rr(XD, XS, mode)                                              \
        cvras_rr(W(XD), W(XS), mode)

/*************   packed half-precision integer arithmetic/shifts   ************/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addmx_rr(XG, XS)                                                    \
        addax_rr(W(XG), W(XS))

#define addmx_ld(XG, MS, DS)                                                \
        addax_ld(W(XG), W(MS), W(DS))

#define addmx3rr(XD, XS, XT)                                                \
        addax3rr(W(XD), W(XS), W(XT))

#define addmx3ld(XD, XS, MT, DT)                                            \
        addax3ld(W(XD), W(XS), W(MT), W(DT))


#define addmb_rr(XG, XS)                                                    \
        addab_rr(W(XG), W(XS))

#define addmb_ld(XG, MS, DS)                                                \
        addab_ld(W(XG), W(MS), W(DS))

#define addmb3rr(XD, XS, XT)                                                \
        addab3rr(W(XD), W(XS), W(XT))

#define addmb3ld(XD, XS, MT, DT)                                            \
        addab3ld(W(XD), W(XS), W(MT), W(DT))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsmx_rr(XG, XS)                                                    \
        adsax_rr(W(XG), W(XS))

#define adsmx_ld(XG, MS, DS)                                                \
        adsax_ld(W(XG), W(MS), W(DS))

#define adsmx3rr(XD, XS, XT)                                                \
        adsax3rr(W(XD), W(XS), W(XT))

#define adsmx3ld(XD, XS, MT, DT)                                            \
        adsax3ld(W(XD), W(XS), W(MT), W(DT))


#define adsmb_rr(XG, XS)                                                    \
        adsab_rr(W(XG), W(XS))

#define adsmb_ld(XG, MS, DS)                                                \
        adsab_ld(W(XG), W(MS), W(DS))

#define adsmb3rr(XD, XS, XT)                                                \
        adsab3rr(W(XD), W(XS), W(XT))

#define adsmb3ld(XD, XS, MT, DT)                                            \
        adsab3ld(W(XD), W(XS), W(MT), W(DT))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsmn_rr(XG, XS)                                                    \
        adsan_rr(W(XG), W(XS))

#define adsmn_ld(XG, MS, DS)                                                \
        adsan_ld(W(XG), W(MS), W(DS))

#define adsmn3rr(XD, XS, XT)                                                \
        adsan3rr(W(XD), W(XS), W(XT))

#define adsmn3ld(XD, XS, MT, DT)                                            \
        adsan3ld(W(XD), W(XS), W(MT), W(DT))


#define adsmc_rr(XG, XS)                                                    \
        adsac_rr(W(XG), W(XS))

#define adsmc_ld(XG, MS, DS)                                                \
        adsac_ld(W(XG), W(MS), W(DS))

#define adsmc3rr(XD, XS, XT)                                                \
        adsac3rr(W(XD), W(XS), W(XT))

#define adsmc3ld(XD, XS, MT, DT)                                            \
        adsac3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define submx_rr(XG, XS)                                                    \
        subax_rr(W(XG), W(XS))

#define submx_ld(XG, MS, DS)                                                \
        subax_ld(W(XG), W(MS), W(DS))

#define submx3rr(XD, XS, XT)                                                \
        subax3rr(W(XD), W(XS), W(XT))

#define submx3ld(XD, XS, MT, DT)                                            \
        subax3ld(W(XD), W(XS), W(MT), W(DT))


#define submb_rr(XG, XS)                                                    \
        subab_rr(W(XG), W(XS))

#define submb_ld(XG, MS, DS)                                                \
        subab_ld(W(XG), W(MS), W(DS))

#define submb3rr(XD, XS, XT)                                                \
        subab3rr(W(XD), W(XS), W(XT))

#define submb3ld(XD, XS, MT, DT)                                            \
        subab3ld(W(XD), W(XS), W(MT), W(DT))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsmx_rr(XG, XS)                                                    \
        sbsax_rr(W(XG), W(XS))

#define sbsmx_ld(XG, MS, DS)                                                \
        sbsax_ld(W(XG), W(MS), W(DS))

#define sbsmx3rr(XD, XS, XT)                                                \
        sbsax3rr(W(XD), W(XS), W(XT))

#define sbsmx3ld(XD, XS, MT, DT)                                            \
        sbsax3ld(W(XD), W(XS), W(MT), W(DT))


#define sbsmb_rr(XG, XS)                                                    \
        sbsab_rr(W(XG), W(XS))

#define sbsmb_ld(XG, MS, DS)                                                \
        sbsab_ld(W(XG), W(MS), W(DS))

#define sbsmb3rr(XD, XS, XT)                                                \
        sbsab3rr(W(XD), W(XS), W(XT))

#define sbsmb3ld(XD, XS, MT, DT)                                            \
        sbsab3ld(W(XD), W(XS), W(MT), W(DT))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsmn_rr(XG, XS)                                                    \
        sbsan_rr(W(XG), W(XS))

#define sbsmn_ld(XG, MS, DS)                                                \
        sbsan_ld(W(XG), W(MS), W(DS))

#define sbsmn3rr(XD, XS, XT)                                                \
        sbsan3rr(W(XD), W(XS), W(XT))

#define sbsmn3ld(XD, XS, MT, DT)                                            \
        sbsan3ld(W(XD), W(XS), W(MT), W(DT))


#define sbsmc_rr(XG, XS)                                                    \
        sbsac_rr(W(XG), W(XS))

#define sbsmc_ld(XG, MS, DS)                                                \
        sbsac_ld(W(XG), W(MS), W(DS))

#define sbsmc3rr(XD, XS, XT)                                                \
        sbsac3rr(W(XD), W(XS), W(XT))

#define sbsmc3ld(XD, XS, MT, DT)                                            \
        sbsac3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulmx_rr(XG, XS)                                                    \
        mulax_rr(W(XG), W(XS))

#define mulmx_ld(XG, MS, DS)                                                \
        mulax_ld(W(XG), W(MS), W(DS))

#define mulmx3rr(XD, XS, XT)                                                \
        mulax3rr(W(XD), W(XS), W(XT))

#define mulmx3ld(XD, XS, MT, DT)                                            \
        mulax3ld(W(XD), W(XS), W(MT), W(DT))


#define mulmb_rr(XG, XS)                                                    \
        mulab_rr(W(XG), W(XS))

#define mulmb_ld(XG, MS, DS)                                                \
        mulab_ld(W(XG), W(MS), W(DS))

#define mulmb3rr(XD, XS, XT)                                                \
        mulab3rr(W(XD), W(XS), W(XT))

#define mulmb3ld(XD, XS, MT, DT)                                            \
        mulab3ld(W(XD), W(XS), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlmx_ri(XG, IS)                                                    \
        shlax_ri(W(XG), W(IS))

#define shlmx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlax_ld(W(XG), W(MS), W(DS))

#define shlmx3ri(XD, XS, IT)                                                \
        shlax3ri(W(XD), W(XS), W(IT))

#define shlmx3ld(XD, XS, MT, DT)                                            \
        shlax3ld(W(XD), W(XS), W(MT), W(DT))


#define shlmb_ri(XG, IS)                                                    \
        shlab_ri(W(XG), W(IS))

#define shlmb_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlab_ld(W(XG), W(MS), W(DS))

#define shlmb3ri(XD, XS, IT)                                                \
        shlab3ri(W(XD), W(XS), W(IT))

#define shlmb3ld(XD, XS, MT, DT)                                            \
        shlab3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmx_ri(XG, IS)                                                    \
        shrax_ri(W(XG), W(IS))

#define shrmx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrax_ld(W(XG), W(MS), W(DS))

#define shrmx3ri(XD, XS, IT)                                                \
        shrax3ri(W(XD), W(XS), W(IT))

#define shrmx3ld(XD, XS, MT, DT)                                            \
        shrax3ld(W(XD), W(XS), W(MT), W(DT))


#define shrmb_ri(XG, IS)                                                    \
        shrab_ri(W(XG), W(IS))

#define shrmb_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrab_ld(W(XG), W(MS), W(DS))

#define shrmb3ri(XD, XS, IT)                                                \
        shrab3ri(W(XD), W(XS), W(IT))

#define shrmb3ld(XD, XS, MT, DT)                                            \
        shrab3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmn_ri(XG, IS)                                                    \
        shran_ri(W(XG), W(IS))

#define shrmn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shran_ld(W(XG), W(MS), W(DS))

#define shrmn3ri(XD, XS, IT)                                                \
        shran3ri(W(XD), W(XS), W(IT))

#define shrmn3ld(XD, XS, MT, DT)                                            \
        shran3ld(W(XD), W(XS), W(MT), W(DT))


#define shrmc_ri(XG, IS)                                                    \
        shrac_ri(W(XG), W(IS))

#define shrmc_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrac_ld(W(XG), W(MS), W(DS))

#define shrmc3ri(XD, XS, IT)                                                \
        shrac3ri(W(XD), W(XS), W(IT))

#define shrmc3ld(XD, XS, MT, DT)                                            \
        shrac3ld(W(XD), W(XS), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlmx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlax_rr(W(XG), W(XS))

#define svlmx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlax_ld(W(XG), W(MS), W(DS))

#define svlmx3rr(XD, XS, XT)                                                \
        svlax3rr(W(XD), W(XS), W(XT))

#define svlmx3ld(XD, XS, MT, DT)                                            \
        svlax3ld(W(XD), W(XS), W(MT), W(DT))


#define svlmb_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlab_rr(W(XG), W(XS))

#define svlmb_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlab_ld(W(XG), W(MS), W(DS))

#define svlmb3rr(XD, XS, XT)                                                \
        svlab3rr(W(XD), W(XS), W(XT))

#define svlmb3ld(XD, XS, MT, DT)                                            \
        svlab3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrax_rr(W(XG), W(XS))

#define svrmx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrax_ld(W(XG), W(MS), W(DS))

#define svrmx3rr(XD, XS, XT)                                                \
        svrax3rr(W(XD), W(XS), W(XT))

#define svrmx3ld(XD, XS, MT, DT)                                            \
        svrax3ld(W(XD), W(XS), W(MT), W(DT))


#define svrmb_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrab_rr(W(XG), W(XS))

#define svrmb_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrab_ld(W(XG), W(MS), W(DS))

#define svrmb3rr(XD, XS, XT)                                                \
        svrab3rr(W(XD), W(XS), W(XT))

#define svrmb3ld(XD, XS, MT, DT)                                            \
        svrab3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svran_rr(W(XG), W(XS))

#define svrmn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svran_ld(W(XG), W(MS), W(DS))

#define svrmn3rr(XD, XS, XT)                                                \
        svran3rr(W(XD), W(XS), W(XT))

#define svrmn3ld(XD, XS, MT, DT)                                            \
        svran3ld(W(XD), W(XS), W(MT), W(DT))


#define svrmc_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrac_rr(W(XG), W(XS))

#define svrmc_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrac_ld(W(XG), W(MS), W(DS))

#define svrmc3rr(XD, XS, XT)                                                \
        svrac3rr(W(XD), W(XS), W(XT))

#define svrmc3ld(XD, XS, MT, DT)                                            \
        svrac3ld(W(XD), W(XS), W(MT), W(DT))

/*****************   packed half-precision integer compare   ******************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minmx_rr(XG, XS)                                                    \
        minax_rr(W(XG), W(XS))

#define minmx_ld(XG, MS, DS)                                                \
        minax_ld(W(XG), W(MS), W(DS))

#define minmx3rr(XD, XS, XT)                                                \
        minax3rr(W(XD), W(XS), W(XT))

#define minmx3ld(XD, XS, MT, DT)                                            \
        minax3ld(W(XD), W(XS), W(MT), W(DT))


#define minmb_rr(XG, XS)                                                    \
        minab_rr(W(XG), W(XS))

#define minmb_ld(XG, MS, DS)                                                \
        minab_ld(W(XG), W(MS), W(DS))

#define minmb3rr(XD, XS, XT)                                                \
        minab3rr(W(XD), W(XS), W(XT))

#define minmb3ld(XD, XS, MT, DT)                                            \
        minab3ld(W(XD), W(XS), W(MT), W(DT))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minmn_rr(XG, XS)                                                    \
        minan_rr(W(XG), W(XS))

#define minmn_ld(XG, MS, DS)                                                \
        minan_ld(W(XG), W(MS), W(DS))

#define minmn3rr(XD, XS, XT)                                                \
        minan3rr(W(XD), W(XS), W(XT))

#define minmn3ld(XD, XS, MT, DT)                                            \
        minan3ld(W(XD), W(XS), W(MT), W(DT))


#define minmc_rr(XG, XS)                                                    \
        minac_rr(W(XG), W(XS))

#define minmc_ld(XG, MS, DS)                                                \
        minac_ld(W(XG), W(MS), W(DS))

#define minmc3rr(XD, XS, XT)                                                \
        minac3rr(W(XD), W(XS), W(XT))

#define minmc3ld(XD, XS, MT, DT)                                            \
        minac3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxmx_rr(XG, XS)                                                    \
        maxax_rr(W(XG), W(XS))

#define maxmx_ld(XG, MS, DS)                                                \
        maxax_ld(W(XG), W(MS), W(DS))

#define maxmx3rr(XD, XS, XT)                                                \
        maxax3rr(W(XD), W(XS), W(XT))

#define maxmx3ld(XD, XS, MT, DT)                                            \
        maxax3ld(W(XD), W(XS), W(MT), W(DT))


#define maxmb_rr(XG, XS)                                                    \
        maxab_rr(W(XG), W(XS))

#define maxmb_ld(XG, MS, DS)                                                \
        maxab_ld(W(XG), W(MS), W(DS))

#define maxmb3rr(XD, XS, XT)                                                \
        maxab3rr(W(XD), W(XS), W(XT))

#define maxmb3ld(XD, XS, MT, DT)                                            \
        maxab3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxmn_rr(XG, XS)                                                    \
        maxan_rr(W(XG), W(XS))

#define maxmn_ld(XG, MS, DS)                                                \
        maxan_ld(W(XG), W(MS), W(DS))

#define maxmn3rr(XD, XS, XT)                                                \
        maxan3rr(W(XD), W(XS), W(XT))

#define maxmn3ld(XD, XS, MT, DT)                                            \
        maxan3ld(W(XD), W(XS), W(MT), W(DT))


#define maxmc_rr(XG, XS)                                                    \
        maxac_rr(W(XG), W(XS))

#define maxmc_ld(XG, MS, DS)                                                \
        maxac_ld(W(XG), W(MS), W(DS))

#define maxmc3rr(XD, XS, XT)                                                \
        maxac3rr(W(XD), W(XS), W(XT))

#define maxmc3ld(XD, XS, MT, DT)                                            \
        maxac3ld(W(XD), W(XS), W(MT), W(DT))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqmx_rr(XG, XS)                                                    \
        ceqax_rr(W(XG), W(XS))

#define ceqmx_ld(XG, MS, DS)                                                \
        ceqax_ld(W(XG), W(MS), W(DS))

#define ceqmx3rr(XD, XS, XT)                                                \
        ceqax3rr(W(XD), W(XS), W(XT))

#define ceqmx3ld(XD, XS, MT, DT)                                            \
        ceqax3ld(W(XD), W(XS), W(MT), W(DT))


#define ceqmb_rr(XG, XS)                                                    \
        ceqab_rr(W(XG), W(XS))

#define ceqmb_ld(XG, MS, DS)                                                \
        ceqab_ld(W(XG), W(MS), W(DS))

#define ceqmb3rr(XD, XS, XT)                                                \
        ceqab3rr(W(XD), W(XS), W(XT))

#define ceqmb3ld(XD, XS, MT, DT)                                            \
        ceqab3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnemx_rr(XG, XS)                                                    \
        cneax_rr(W(XG), W(XS))

#define cnemx_ld(XG, MS, DS)                                                \
        cneax_ld(W(XG), W(MS), W(DS))

#define cnemx3rr(XD, XS, XT)                                                \
        cneax3rr(W(XD), W(XS), W(XT))

#define cnemx3ld(XD, XS, MT, DT)                                            \
        cneax3ld(W(XD), W(XS), W(MT), W(DT))


#define cnemb_rr(XG, XS)                                                    \
        cneab_rr(W(XG), W(XS))

#define cnemb_ld(XG, MS, DS)                                                \
        cneab_ld(W(XG), W(MS), W(DS))

#define cnemb3rr(XD, XS, XT)                                                \
        cneab3rr(W(XD), W(XS), W(XT))

#define cnemb3ld(XD, XS, MT, DT)                                            \
        cneab3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltmx_rr(XG, XS)                                                    \
        cltax_rr(W(XG), W(XS))

#define cltmx_ld(XG, MS, DS)                                                \
        cltax_ld(W(XG), W(MS), W(DS))

#define cltmx3rr(XD, XS, XT)                                                \
        cltax3rr(W(XD), W(XS), W(XT))

#define cltmx3ld(XD, XS, MT, DT)                                            \
        cltax3ld(W(XD), W(XS), W(MT), W(DT))


#define cltmb_rr(XG, XS)                                                    \
        cltab_rr(W(XG), W(XS))

#define cltmb_ld(XG, MS, DS)                                                \
        cltab_ld(W(XG), W(MS), W(DS))

#define cltmb3rr(XD, XS, XT)                                                \
        cltab3rr(W(XD), W(XS), W(XT))

#define cltmb3ld(XD, XS, MT, DT)                                            \
        cltab3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltmn_rr(XG, XS)                                                    \
        cltan_rr(W(XG), W(XS))

#define cltmn_ld(XG, MS, DS)                                                \
        cltan_ld(W(XG), W(MS), W(DS))

#define cltmn3rr(XD, XS, XT)                                                \
        cltan3rr(W(XD), W(XS), W(XT))

#define cltmn3ld(XD, XS, MT, DT)                                            \
        cltan3ld(W(XD), W(XS), W(MT), W(DT))


#define cltmc_rr(XG, XS)                                                    \
        cltac_rr(W(XG), W(XS))

#define cltmc_ld(XG, MS, DS)                                                \
        cltac_ld(W(XG), W(MS), W(DS))

#define cltmc3rr(XD, XS, XT)                                                \
        cltac3rr(W(XD), W(XS), W(XT))

#define cltmc3ld(XD, XS, MT, DT)                                            \
        cltac3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clemx_rr(XG, XS)                                                    \
        cleax_rr(W(XG), W(XS))

#define clemx_ld(XG, MS, DS)                                                \
        cleax_ld(W(XG), W(MS), W(DS))

#define clemx3rr(XD, XS, XT)                                                \
        cleax3rr(W(XD), W(XS), W(XT))

#define clemx3ld(XD, XS, MT, DT)                                            \
        cleax3ld(W(XD), W(XS), W(MT), W(DT))


#define clemb_rr(XG, XS)                                                    \
        cleab_rr(W(XG), W(XS))

#define clemb_ld(XG, MS, DS)                                                \
        cleab_ld(W(XG), W(MS), W(DS))

#define clemb3rr(XD, XS, XT)                                                \
        cleab3rr(W(XD), W(XS), W(XT))

#define clemb3ld(XD, XS, MT, DT)                                            \
        cleab3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clemn_rr(XG, XS)                                                    \
        clean_rr(W(XG), W(XS))

#define clemn_ld(XG, MS, DS)                                                \
        clean_ld(W(XG), W(MS), W(DS))

#define clemn3rr(XD, XS, XT)                                                \
        clean3rr(W(XD), W(XS), W(XT))

#define clemn3ld(XD, XS, MT, DT)                                            \
        clean3ld(W(XD), W(XS), W(MT), W(DT))


#define clemc_rr(XG, XS)                                                    \
        cleac_rr(W(XG), W(XS))

#define clemc_ld(XG, MS, DS)                                                \
        cleac_ld(W(XG), W(MS), W(DS))

#define clemc3rr(XD, XS, XT)                                                \
        cleac3rr(W(XD), W(XS), W(XT))

#define clemc3ld(XD, XS, MT, DT)                                            \
        cleac3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtmx_rr(XG, XS)                                                    \
        cgtax_rr(W(XG), W(XS))

#define cgtmx_ld(XG, MS, DS)                                                \
        cgtax_ld(W(XG), W(MS), W(DS))

#define cgtmx3rr(XD, XS, XT)                                                \
        cgtax3rr(W(XD), W(XS), W(XT))

#define cgtmx3ld(XD, XS, MT, DT)                                            \
        cgtax3ld(W(XD), W(XS), W(MT), W(DT))


#define cgtmb_rr(XG, XS)                                                    \
        cgtab_rr(W(XG), W(XS))

#define cgtmb_ld(XG, MS, DS)                                                \
        cgtab_ld(W(XG), W(MS), W(DS))

#define cgtmb3rr(XD, XS, XT)                                                \
        cgtab3rr(W(XD), W(XS), W(XT))

#define cgtmb3ld(XD, XS, MT, DT)                                            \
        cgtab3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtmn_rr(XG, XS)                                                    \
        cgtan_rr(W(XG), W(XS))

#define cgtmn_ld(XG, MS, DS)                                                \
        cgtan_ld(W(XG), W(MS), W(DS))

#define cgtmn3rr(XD, XS, XT)                                                \
        cgtan3rr(W(XD), W(XS), W(XT))

#define cgtmn3ld(XD, XS, MT, DT)                                            \
        cgtan3ld(W(XD), W(XS), W(MT), W(DT))


#define cgtmc_rr(XG, XS)                                                    \
        cgtac_rr(W(XG), W(XS))

#define cgtmc_ld(XG, MS, DS)                                                \
        cgtac_ld(W(XG), W(MS), W(DS))

#define cgtmc3rr(XD, XS, XT)                                                \
        cgtac3rr(W(XD), W(XS), W(XT))

#define cgtmc3ld(XD, XS, MT, DT)                                            \
        cgtac3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgemx_rr(XG, XS)                                                    \
        cgeax_rr(W(XG), W(XS))

#define cgemx_ld(XG, MS, DS)                                                \
        cgeax_ld(W(XG), W(MS), W(DS))

#define cgemx3rr(XD, XS, XT)                                                \
        cgeax3rr(W(XD), W(XS), W(XT))

#define cgemx3ld(XD, XS, MT, DT)                                            \
        cgeax3ld(W(XD), W(XS), W(MT), W(DT))


#define cgemb_rr(XG, XS)                                                    \
        cgeab_rr(W(XG), W(XS))

#define cgemb_ld(XG, MS, DS)                                                \
        cgeab_ld(W(XG), W(MS), W(DS))

#define cgemb3rr(XD, XS, XT)                                                \
        cgeab3rr(W(XD), W(XS), W(XT))

#define cgemb3ld(XD, XS, MT, DT)                                            \
        cgeab3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgemn_rr(XG, XS)                                                    \
        cgean_rr(W(XG), W(XS))

#define cgemn_ld(XG, MS, DS)                                                \
        cgean_ld(W(XG), W(MS), W(DS))

#define cgemn3rr(XD, XS, XT)                                                \
        cgean3rr(W(XD), W(XS), W(XT))

#define cgemn3ld(XD, XS, MT, DT)                                            \
        cgean3ld(W(XD), W(XS), W(MT), W(DT))


#define cgemc_rr(XG, XS)                                                    \
        cgeac_rr(W(XG), W(XS))

#define cgemc_ld(XG, MS, DS)                                                \
        cgeac_ld(W(XG), W(MS), W(DS))

#define cgemc3rr(XD, XS, XT)                                                \
        cgeac3rr(W(XD), W(XS), W(XT))

#define cgemc3ld(XD, XS, MT, DT)                                            \
        cgeac3ld(W(XD), W(XS), W(MT), W(DT))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define mkjmx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        mkjax_rx(W(XS), mask, lb)

#define mkjmb_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        mkjab_rx(W(XS), mask, lb)

/******************************************************************************/
/**** var-len **** SIMD instructions with fixed-16-bit element **** 128-bit ***/
/******************************************************************************/

#elif (RT_SIMD == 128)

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmmx_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        elmgx_st(W(XS), W(MD), W(DD))

/****************   packed half-precision generic move/logic   ****************/

/* mov (D = S) */

#define movmx_rr(XD, XS)                                                    \
        movgx_rr(W(XD), W(XS))

#define movmx_ld(XD, MS, DS)                                                \
        movgx_ld(W(XD), W(MS), W(DS))

#define movmx_st(XS, MD, DD)                                                \
        movgx_st(W(XS), W(MD), W(DD))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvmx_rr(XD, XS)                                                    \
        mmvgx_rr(W(XD), W(XS))

#define mmvmx_ld(XG, MS, DS)                                                \
        mmvgx_ld(W(XG), W(MS), W(DS))

#define mmvmx_st(XS, MG, DG)                                                \
        mmvgx_st(W(XS), W(MG), W(DG))


#define mmvmb_rr(XD, XS)                                                    \
        mmvgb_rr(W(XD), W(XS))

#define mmvmb_ld(XG, MS, DS)                                                \
        mmvgb_ld(W(XG), W(MS), W(DS))

#define mmvmb_st(XS, MG, DG)                                                \
        mmvgb_st(W(XS), W(MG), W(DG))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andmx_rr(XG, XS)                                                    \
        andgx_rr(W(XG), W(XS))

#define andmx_ld(XG, MS, DS)                                                \
        andgx_ld(W(XG), W(MS), W(DS))

#define andmx3rr(XD, XS, XT)                                                \
        andgx3rr(W(XD), W(XS), W(XT))

#define andmx3ld(XD, XS, MT, DT)                                            \
        andgx3ld(W(XD), W(XS), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annmx_rr(XG, XS)                                                    \
        anngx_rr(W(XG), W(XS))

#define annmx_ld(XG, MS, DS)                                                \
        anngx_ld(W(XG), W(MS), W(DS))

#define annmx3rr(XD, XS, XT)                                                \
        anngx3rr(W(XD), W(XS), W(XT))

#define annmx3ld(XD, XS, MT, DT)                                            \
        anngx3ld(W(XD), W(XS), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrmx_rr(XG, XS)                                                    \
        orrgx_rr(W(XG), W(XS))

#define orrmx_ld(XG, MS, DS)                                                \
        orrgx_ld(W(XG), W(MS), W(DS))

#define orrmx3rr(XD, XS, XT)                                                \
        orrgx3rr(W(XD), W(XS), W(XT))

#define orrmx3ld(XD, XS, MT, DT)                                            \
        orrgx3ld(W(XD), W(XS), W(MT), W(DT))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define ornmx_rr(XG, XS)                                                    \
        orngx_rr(W(XG), W(XS))

#define ornmx_ld(XG, MS, DS)                                                \
        orngx_ld(W(XG), W(MS), W(DS))

#define ornmx3rr(XD, XS, XT)                                                \
        orngx3rr(W(XD), W(XS), W(XT))

#define ornmx3ld(XD, XS, MT, DT)                                            \
        orngx3ld(W(XD), W(XS), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xormx_rr(XG, XS)                                                    \
        xorgx_rr(W(XG), W(XS))

#define xormx_ld(XG, MS, DS)                                                \
        xorgx_ld(W(XG), W(MS), W(DS))

#define xormx3rr(XD, XS, XT)                                                \
        xorgx3rr(W(XD), W(XS), W(XT))

#define xormx3ld(XD, XS, MT, DT)                                            \
        xorgx3ld(W(XD), W(XS), W(MT), W(DT))

/* not (G = ~G), (D = ~S) */

#define notmx_rx(XG)                                                        \
        notgx_rx(W(XG))

#define notmx_rr(XD, XS)                                                    \
        notgx_rr(W(XD), W(XS))

/*************   packed half-precision floating-point arithmetic   ************/

/* neg (G = -G), (D = -S) */

#define negms_rx(XG)                                                        \
        neggs_rx(W(XG))

#define negms_rr(XD, XS)                                                    \
        neggs_rr(W(XD), W(XS))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addms_rr(XG, XS)                                                    \
        addgs_rr(W(XG), W(XS))

#define addms_ld(XG, MS, DS)                                                \
        addgs_ld(W(XG), W(MS), W(DS))

#define addms3rr(XD, XS, XT)                                                \
        addgs3rr(W(XD), W(XS), W(XT))

#define addms3ld(XD, XS, MT, DT)                                            \
        addgs3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subms_rr(XG, XS)                                                    \
        subgs_rr(W(XG), W(XS))

#define subms_ld(XG, MS, DS)                                                \
        subgs_ld(W(XG), W(MS), W(DS))

#define subms3rr(XD, XS, XT)                                                \
        subgs3rr(W(XD), W(XS), W(XT))

#define subms3ld(XD, XS, MT, DT)                                            \
        subgs3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulms_rr(XG, XS)                                                    \
        mulgs_rr(W(XG), W(XS))

#define mulms_ld(XG, MS, DS)                                                \
        mulgs_ld(W(XG), W(MS), W(DS))

#define mulms3rr(XD, XS, XT)                                                \
        mulgs3rr(W(XD), W(XS), W(XT))

#define mulms3ld(XD, XS, MT, DT)                                            \
        mulgs3ld(W(XD), W(XS), W(MT), W(DT))

/* div (G = G / S), (D = S / T) if (#D != #T) */

#define divms_rr(XG, XS)                                                    \
        divgs_rr(W(XG), W(XS))

#define divms_ld(XG, MS, DS)                                                \
        divgs_ld(W(XG), W(MS), W(DS))

#define divms3rr(XD, XS, XT)                                                \
        divgs3rr(W(XD), W(XS), W(XT))

#define divms3ld(XD, XS, MT, DT)                                            \
        divgs3ld(W(XD), W(XS), W(MT), W(DT))

/* sqr (D = sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define sqrms_rr(XD, XS)                                                    \
        sqrgs_rr(W(XD), W(XS))

#define sqrms_ld(XD, MS, DS)                                                \
        sqrgs_ld(W(XD), W(MS), W(DS))

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rcems_rr(XD, XS)                                                    \
        rcegs_rr(W(XD), W(XS))

#define rcsms_rr(XG, XS) /* destroys XS */                                  \
        rcsgs_rr(W(XG), W(XS))

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rsems_rr(XD, XS)                                                    \
        rsegs_rr(W(XD), W(XS))

#define rssms_rr(XG, XS) /* destroys XS */                                  \
        rssgs_rr(W(XG), W(XS))

/* fma (G = G + S * T) if (#G != #S && #G != #T) */

#define fmams_rr(XG, XS, XT)                                                \
        fmags_rr(W(XG), W(XS), W(XT))

#define fmams_ld(XG, XS, MT, DT)                                            \
        fmags_ld(W(XG), W(XS), W(MT), W(DT))

#define fmams3rr(XG, XS, XT)                                                \
        fmams_rr(W(XG), W(XS), W(XT))

#define fmams3ld(XG, XS, MT, DT)                                            \
        fmams_ld(W(XG), W(XS), W(MT), W(DT))

#define fmaas3rr(XG, XS, XT)                                                \
        fmaas_rr(W(XG), W(XS), W(XT))

#define fmaas3ld(XG, XS, MT, DT)                                            \
        fmaas_ld(W(XG), W(XS), W(MT), W(DT))

#define fmags3rr(XG, XS, XT)                                                \
        fmags_rr(W(XG), W(XS), W(XT))

#define fmags3ld(XG, XS, MT, DT)                                            \
        fmags_ld(W(XG), W(XS), W(MT), W(DT))

/* fms (G = G - S * T) if (#G != #S && #G != #T) */

#define fmsms_rr(XG, XS, XT)                                                \
        fmsgs_rr(W(XG), W(XS), W(XT))

#define fmsms_ld(XG, XS, MT, DT)                                            \
        fmsgs_ld(W(XG), W(XS), W(MT), W(DT))

#define fmsms3rr(XG, XS, XT)                                                \
        fmsms_rr(W(XG), W(XS), W(XT))

#define fmsms3ld(XG, XS, MT, DT)                                            \
        fmsms_ld(W(XG), W(XS), W(MT), W(DT))

#define fmsas3rr(XG, XS, XT)                                                \
        fmsas_rr(W(XG), W(XS), W(XT))

#define fmsas3ld(XG, XS, MT, DT)                                            \
        fmsas_ld(W(XG), W(XS), W(MT), W(DT))

#define fmsgs3rr(XG, XS, XT)                                                \
        fmsgs_rr(W(XG), W(XS), W(XT))

#define fmsgs3ld(XG, XS, MT, DT)                                            \
        fmsgs_ld(W(XG), W(XS), W(MT), W(DT))

/**************   packed half-precision floating-point compare   **************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minms_rr(XG, XS)                                                    \
        mings_rr(W(XG), W(XS))

#define minms_ld(XG, MS, DS)                                                \
        mings_ld(W(XG), W(MS), W(DS))

#define minms3rr(XD, XS, XT)                                                \
        mings3rr(W(XD), W(XS), W(XT))

#define minms3ld(XD, XS, MT, DT)                                            \
        mings3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxms_rr(XG, XS)                                                    \
        maxgs_rr(W(XG), W(XS))

#define maxms_ld(XG, MS, DS)                                                \
        maxgs_ld(W(XG), W(MS), W(DS))

#define maxms3rr(XD, XS, XT)                                                \
        maxgs3rr(W(XD), W(XS), W(XT))

#define maxms3ld(XD, XS, MT, DT)                                            \
        maxgs3ld(W(XD), W(XS), W(MT), W(DT))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqms_rr(XG, XS)                                                    \
        ceqgs_rr(W(XG), W(XS))

#define ceqms_ld(XG, MS, DS)                                                \
        ceqgs_ld(W(XG), W(MS), W(DS))

#define ceqms3rr(XD, XS, XT)                                                \
        ceqgs3rr(W(XD), W(XS), W(XT))

#define ceqms3ld(XD, XS, MT, DT)                                            \
        ceqgs3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnems_rr(XG, XS)                                                    \
        cnegs_rr(W(XG), W(XS))

#define cnems_ld(XG, MS, DS)                                                \
        cnegs_ld(W(XG), W(MS), W(DS))

#define cnems3rr(XD, XS, XT)                                                \
        cnegs3rr(W(XD), W(XS), W(XT))

#define cnems3ld(XD, XS, MT, DT)                                            \
        cnegs3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltms_rr(XG, XS)                                                    \
        cltgs_rr(W(XG), W(XS))

#define cltms_ld(XG, MS, DS)                                                \
        cltgs_ld(W(XG), W(MS), W(DS))

#define cltms3rr(XD, XS, XT)                                                \
        cltgs3rr(W(XD), W(XS), W(XT))

#define cltms3ld(XD, XS, MT, DT)                                            \
        cltgs3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define clems_rr(XG, XS)                                                    \
        clegs_rr(W(XG), W(XS))

#define clems_ld(XG, MS, DS)                                                \
        clegs_ld(W(XG), W(MS), W(DS))

#define clems3rr(XD, XS, XT)                                                \
        clegs3rr(W(XD), W(XS), W(XT))

#define clems3ld(XD, XS, MT, DT)                                            \
        clegs3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtms_rr(XG, XS)                                                    \
        cgtgs_rr(W(XG), W(XS))

#define cgtms_ld(XG, MS, DS)                                                \
        cgtgs_ld(W(XG), W(MS), W(DS))

#define cgtms3rr(XD, XS, XT)                                                \
        cgtgs3rr(W(XD), W(XS), W(XT))

#define cgtms3ld(XD, XS, MT, DT)                                            \
        cgtgs3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgems_rr(XG, XS)                                                    \
        cgegs_rr(W(XG), W(XS))

#define cgems_ld(XG, MS, DS)                                                \
        cgegs_ld(W(XG), W(MS), W(DS))

#define cgems3rr(XD, XS, XT)                                                \
        cgegs3rr(W(XD), W(XS), W(XT))

#define cgems3ld(XD, XS, MT, DT)                                            \
        cgegs3ld(W(XD), W(XS), W(MT), W(DT))

/**************   packed half-precision floating-point convert   **************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnzms_rr(XD, XS)     /* round towards zero */                       \
        rnzgs_rr(W(XD), W(XS))

#define rnzms_ld(XD, MS, DS) /* round towards zero */                       \
        rnzgs_ld(W(XD), W(MS), W(DS))

#define cvzms_rr(XD, XS)     /* round towards zero */                       \
        cvzgs_rr(W(XD), W(XS))

#define cvzms_ld(XD, MS, DS) /* round towards zero */                       \
        cvzgs_ld(W(XD), W(MS), W(DS))

/* cvp (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnpms_rr(XD, XS)     /* round towards +inf */                       \
        rnpgs_rr(W(XD), W(XS))

#define rnpms_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpgs_ld(W(XD), W(MS), W(DS))

#define cvpms_rr(XD, XS)     /* round towards +inf */                       \
        cvpgs_rr(W(XD), W(XS))

#define cvpms_ld(XD, MS, DS) /* round towards +inf */                       \
        cvpgs_ld(W(XD), W(MS), W(DS))

/* cvm (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnmms_rr(XD, XS)     /* round towards -inf */                       \
        rnmgs_rr(W(XD), W(XS))

#define rnmms_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmgs_ld(W(XD), W(MS), W(DS))

#define cvmms_rr(XD, XS)     /* round towards -inf */                       \
        cvmgs_rr(W(XD), W(XS))

#define cvmms_ld(XD, MS, DS) /* round towards -inf */                       \
        cvmgs_ld(W(XD), W(MS), W(DS))

/* cvn (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnnms_rr(XD, XS)     /* round towards near */                       \
        rnngs_rr(W(XD), W(XS))

#define rnnms_ld(XD, MS, DS) /* round towards near */                       \
        rnngs_ld(W(XD), W(MS), W(DS))

#define cvnms_rr(XD, XS)     /* round towards near */                       \
        cvngs_rr(W(XD), W(XS))

#define cvnms_ld(XD, MS, DS) /* round towards near */                       \
        cvngs_ld(W(XD), W(MS), W(DS))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnmn_rr(XD, XS)     /* round towards near */                       \
        cvngn_rr(W(XD), W(XS))

#define cvnmn_ld(XD, MS, DS) /* round towards near */                       \
        cvngn_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from control register (set in FCTRL blocks) */

#define rndms_rr(XD, XS)                                                    \
        rndgs_rr(W(XD), W(XS))

#define rndms_ld(XD, MS, DS)                                                \
        rndgs_ld(W(XD), W(MS), W(DS))

#define cvtms_rr(XD, XS)                                                    \
        cvtgs_rr(W(XD), W(XS))

#define cvtms_ld(XD, MS, DS)                                                \
        cvtgs_ld(W(XD), W(MS), W(DS))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from control register (set in FCTRL blocks) */

#define cvtmn_rr(XD, XS)                                                    \
        cvtgn_rr(W(XD), W(XS))

#define cvtmn_ld(XD, MS, DS)                                                \
        cvtgn_ld(W(XD), W(MS), W(DS))

/* cvn (D = unsigned-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnmx_rr(XD, XS)     /* round towards near */                       \
        cvngx_rr(W(XD), W(XS))

#define cvnmx_ld(XD, MS, DS) /* round towards near */                       \
        cvngx_ld(W(XD), W(MS), W(DS))

/* cvt (D = unsigned-int-to-fp S)
 * rounding mode comes from control register (set in FCTRL blocks) */

#define cvtmx_rr(XD, XS)                                                    \
        cvtgx_rr(W(XD), W(XS))

#define cvtmx_ld(XD, MS, DS)                                                \
        cvtgx_ld(W(XD), W(MS), W(DS))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnrms_rr(XD, XS, mode)                                              \
        rnrgs_rr(W(XD), W(XS), mode)

#define cvrms_rr(XD, XS, mode)                                              \
        cvrgs_rr(W(XD), W(XS), mode)

/*************   packed half-precision integer arithmetic/shifts   ************/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addmx_rr(XG, XS)                                                    \
        addgx_rr(W(XG), W(XS))

#define addmx_ld(XG, MS, DS)                                                \
        addgx_ld(W(XG), W(MS), W(DS))

#define addmx3rr(XD, XS, XT)                                                \
        addgx3rr(W(XD), W(XS), W(XT))

#define addmx3ld(XD, XS, MT, DT)                                            \
        addgx3ld(W(XD), W(XS), W(MT), W(DT))


#define addmb_rr(XG, XS)                                                    \
        addgb_rr(W(XG), W(XS))

#define addmb_ld(XG, MS, DS)                                                \
        addgb_ld(W(XG), W(MS), W(DS))

#define addmb3rr(XD, XS, XT)                                                \
        addgb3rr(W(XD), W(XS), W(XT))

#define addmb3ld(XD, XS, MT, DT)                                            \
        addgb3ld(W(XD), W(XS), W(MT), W(DT))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsmx_rr(XG, XS)                                                    \
        adsgx_rr(W(XG), W(XS))

#define adsmx_ld(XG, MS, DS)                                                \
        adsgx_ld(W(XG), W(MS), W(DS))

#define adsmx3rr(XD, XS, XT)                                                \
        adsgx3rr(W(XD), W(XS), W(XT))

#define adsmx3ld(XD, XS, MT, DT)                                            \
        adsgx3ld(W(XD), W(XS), W(MT), W(DT))


#define adsmb_rr(XG, XS)                                                    \
        adsgb_rr(W(XG), W(XS))

#define adsmb_ld(XG, MS, DS)                                                \
        adsgb_ld(W(XG), W(MS), W(DS))

#define adsmb3rr(XD, XS, XT)                                                \
        adsgb3rr(W(XD), W(XS), W(XT))

#define adsmb3ld(XD, XS, MT, DT)                                            \
        adsgb3ld(W(XD), W(XS), W(MT), W(DT))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsmn_rr(XG, XS)                                                    \
        adsgn_rr(W(XG), W(XS))

#define adsmn_ld(XG, MS, DS)                                                \
        adsgn_ld(W(XG), W(MS), W(DS))

#define adsmn3rr(XD, XS, XT)                                                \
        adsgn3rr(W(XD), W(XS), W(XT))

#define adsmn3ld(XD, XS, MT, DT)                                            \
        adsgn3ld(W(XD), W(XS), W(MT), W(DT))


#define adsmc_rr(XG, XS)                                                    \
        adsgc_rr(W(XG), W(XS))

#define adsmc_ld(XG, MS, DS)                                                \
        adsgc_ld(W(XG), W(MS), W(DS))

#define adsmc3rr(XD, XS, XT)                                                \
        adsgc3rr(W(XD), W(XS), W(XT))

#define adsmc3ld(XD, XS, MT, DT)                                            \
        adsgc3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define submx_rr(XG, XS)                                                    \
        subgx_rr(W(XG), W(XS))

#define submx_ld(XG, MS, DS)                                                \
        subgx_ld(W(XG), W(MS), W(DS))

#define submx3rr(XD, XS, XT)                                                \
        subgx3rr(W(XD), W(XS), W(XT))

#define submx3ld(XD, XS, MT, DT)                                            \
        subgx3ld(W(XD), W(XS), W(MT), W(DT))


#define submb_rr(XG, XS)                                                    \
        subgb_rr(W(XG), W(XS))

#define submb_ld(XG, MS, DS)                                                \
        subgb_ld(W(XG), W(MS), W(DS))

#define submb3rr(XD, XS, XT)                                                \
        subgb3rr(W(XD), W(XS), W(XT))

#define submb3ld(XD, XS, MT, DT)                                            \
        subgb3ld(W(XD), W(XS), W(MT), W(DT))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsmx_rr(XG, XS)                                                    \
        sbsgx_rr(W(XG), W(XS))

#define sbsmx_ld(XG, MS, DS)                                                \
        sbsgx_ld(W(XG), W(MS), W(DS))

#define sbsmx3rr(XD, XS, XT)                                                \
        sbsgx3rr(W(XD), W(XS), W(XT))

#define sbsmx3ld(XD, XS, MT, DT)                                            \
        sbsgx3ld(W(XD), W(XS), W(MT), W(DT))


#define sbsmb_rr(XG, XS)                                                    \
        sbsgb_rr(W(XG), W(XS))

#define sbsmb_ld(XG, MS, DS)                                                \
        sbsgb_ld(W(XG), W(MS), W(DS))

#define sbsmb3rr(XD, XS, XT)                                                \
        sbsgb3rr(W(XD), W(XS), W(XT))

#define sbsmb3ld(XD, XS, MT, DT)                                            \
        sbsgb3ld(W(XD), W(XS), W(MT), W(DT))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsmn_rr(XG, XS)                                                    \
        sbsgn_rr(W(XG), W(XS))

#define sbsmn_ld(XG, MS, DS)                                                \
        sbsgn_ld(W(XG), W(MS), W(DS))

#define sbsmn3rr(XD, XS, XT)                                                \
        sbsgn3rr(W(XD), W(XS), W(XT))

#define sbsmn3ld(XD, XS, MT, DT)                                            \
        sbsgn3ld(W(XD), W(XS), W(MT), W(DT))


#define sbsmc_rr(XG, XS)                                                    \
        sbsgc_rr(W(XG), W(XS))

#define sbsmc_ld(XG, MS, DS)                                                \
        sbsgc_ld(W(XG), W(MS), W(DS))

#define sbsmc3rr(XD, XS, XT)                                                \
        sbsgc3rr(W(XD), W(XS), W(XT))

#define sbsmc3ld(XD, XS, MT, DT)                                            \
        sbsgc3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulmx_rr(XG, XS)                                                    \
        mulgx_rr(W(XG), W(XS))

#define mulmx_ld(XG, MS, DS)                                                \
        mulgx_ld(W(XG), W(MS), W(DS))

#define mulmx3rr(XD, XS, XT)                                                \
        mulgx3rr(W(XD), W(XS), W(XT))

#define mulmx3ld(XD, XS, MT, DT)                                            \
        mulgx3ld(W(XD), W(XS), W(MT), W(DT))


#define mulmb_rr(XG, XS)                                                    \
        mulgb_rr(W(XG), W(XS))

#define mulmb_ld(XG, MS, DS)                                                \
        mulgb_ld(W(XG), W(MS), W(DS))

#define mulmb3rr(XD, XS, XT)                                                \
        mulgb3rr(W(XD), W(XS), W(XT))

#define mulmb3ld(XD, XS, MT, DT)                                            \
        mulgb3ld(W(XD), W(XS), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlmx_ri(XG, IS)                                                    \
        shlgx_ri(W(XG), W(IS))

#define shlmx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlgx_ld(W(XG), W(MS), W(DS))

#define shlmx3ri(XD, XS, IT)                                                \
        shlgx3ri(W(XD), W(XS), W(IT))

#define shlmx3ld(XD, XS, MT, DT)                                            \
        shlgx3ld(W(XD), W(XS), W(MT), W(DT))


#define shlmb_ri(XG, IS)                                                    \
        shlgb_ri(W(XG), W(IS))

#define shlmb_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlgb_ld(W(XG), W(MS), W(DS))

#define shlmb3ri(XD, XS, IT)                                                \
        shlgb3ri(W(XD), W(XS), W(IT))

#define shlmb3ld(XD, XS, MT, DT)                                            \
        shlgb3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmx_ri(XG, IS)                                                    \
        shrgx_ri(W(XG), W(IS))

#define shrmx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrgx_ld(W(XG), W(MS), W(DS))

#define shrmx3ri(XD, XS, IT)                                                \
        shrgx3ri(W(XD), W(XS), W(IT))

#define shrmx3ld(XD, XS, MT, DT)                                            \
        shrgx3ld(W(XD), W(XS), W(MT), W(DT))


#define shrmb_ri(XG, IS)                                                    \
        shrgb_ri(W(XG), W(IS))

#define shrmb_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrgb_ld(W(XG), W(MS), W(DS))

#define shrmb3ri(XD, XS, IT)                                                \
        shrgb3ri(W(XD), W(XS), W(IT))

#define shrmb3ld(XD, XS, MT, DT)                                            \
        shrgb3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmn_ri(XG, IS)                                                    \
        shrgn_ri(W(XG), W(IS))

#define shrmn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrgn_ld(W(XG), W(MS), W(DS))

#define shrmn3ri(XD, XS, IT)                                                \
        shrgn3ri(W(XD), W(XS), W(IT))

#define shrmn3ld(XD, XS, MT, DT)                                            \
        shrgn3ld(W(XD), W(XS), W(MT), W(DT))


#define shrmc_ri(XG, IS)                                                    \
        shrgc_ri(W(XG), W(IS))

#define shrmc_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrgc_ld(W(XG), W(MS), W(DS))

#define shrmc3ri(XD, XS, IT)                                                \
        shrgc3ri(W(XD), W(XS), W(IT))

#define shrmc3ld(XD, XS, MT, DT)                                            \
        shrgc3ld(W(XD), W(XS), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlmx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlgx_rr(W(XG), W(XS))

#define svlmx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlgx_ld(W(XG), W(MS), W(DS))

#define svlmx3rr(XD, XS, XT)                                                \
        svlgx3rr(W(XD), W(XS), W(XT))

#define svlmx3ld(XD, XS, MT, DT)                                            \
        svlgx3ld(W(XD), W(XS), W(MT), W(DT))


#define svlmb_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlgb_rr(W(XG), W(XS))

#define svlmb_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlgb_ld(W(XG), W(MS), W(DS))

#define svlmb3rr(XD, XS, XT)                                                \
        svlgb3rr(W(XD), W(XS), W(XT))

#define svlmb3ld(XD, XS, MT, DT)                                            \
        svlgb3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgx_rr(W(XG), W(XS))

#define svrmx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgx_ld(W(XG), W(MS), W(DS))

#define svrmx3rr(XD, XS, XT)                                                \
        svrgx3rr(W(XD), W(XS), W(XT))

#define svrmx3ld(XD, XS, MT, DT)                                            \
        svrgx3ld(W(XD), W(XS), W(MT), W(DT))


#define svrmb_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgb_rr(W(XG), W(XS))

#define svrmb_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgb_ld(W(XG), W(MS), W(DS))

#define svrmb3rr(XD, XS, XT)                                                \
        svrgb3rr(W(XD), W(XS), W(XT))

#define svrmb3ld(XD, XS, MT, DT)                                            \
        svrgb3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgn_rr(W(XG), W(XS))

#define svrmn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgn_ld(W(XG), W(MS), W(DS))

#define svrmn3rr(XD, XS, XT)                                                \
        svrgn3rr(W(XD), W(XS), W(XT))

#define svrmn3ld(XD, XS, MT, DT)                                            \
        svrgn3ld(W(XD), W(XS), W(MT), W(DT))


#define svrmc_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgc_rr(W(XG), W(XS))

#define svrmc_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgc_ld(W(XG), W(MS), W(DS))

#define svrmc3rr(XD, XS, XT)                                                \
        svrgc3rr(W(XD), W(XS), W(XT))

#define svrmc3ld(XD, XS, MT, DT)                                            \
        svrgc3ld(W(XD), W(XS), W(MT), W(DT))

/*****************   packed half-precision integer compare   ******************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minmx_rr(XG, XS)                                                    \
        mingx_rr(W(XG), W(XS))

#define minmx_ld(XG, MS, DS)                                                \
        mingx_ld(W(XG), W(MS), W(DS))

#define minmx3rr(XD, XS, XT)                                                \
        mingx3rr(W(XD), W(XS), W(XT))

#define minmx3ld(XD, XS, MT, DT)                                            \
        mingx3ld(W(XD), W(XS), W(MT), W(DT))


#define minmb_rr(XG, XS)                                                    \
        mingb_rr(W(XG), W(XS))

#define minmb_ld(XG, MS, DS)                                                \
        mingb_ld(W(XG), W(MS), W(DS))

#define minmb3rr(XD, XS, XT)                                                \
        mingb3rr(W(XD), W(XS), W(XT))

#define minmb3ld(XD, XS, MT, DT)                                            \
        mingb3ld(W(XD), W(XS), W(MT), W(DT))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minmn_rr(XG, XS)                                                    \
        mingn_rr(W(XG), W(XS))

#define minmn_ld(XG, MS, DS)                                                \
        mingn_ld(W(XG), W(MS), W(DS))

#define minmn3rr(XD, XS, XT)                                                \
        mingn3rr(W(XD), W(XS), W(XT))

#define minmn3ld(XD, XS, MT, DT)                                            \
        mingn3ld(W(XD), W(XS), W(MT), W(DT))


#define minmc_rr(XG, XS)                                                    \
        mingc_rr(W(XG), W(XS))

#define minmc_ld(XG, MS, DS)                                                \
        mingc_ld(W(XG), W(MS), W(DS))

#define minmc3rr(XD, XS, XT)                                                \
        mingc3rr(W(XD), W(XS), W(XT))

#define minmc3ld(XD, XS, MT, DT)                                            \
        mingc3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxmx_rr(XG, XS)                                                    \
        maxgx_rr(W(XG), W(XS))

#define maxmx_ld(XG, MS, DS)                                                \
        maxgx_ld(W(XG), W(MS), W(DS))

#define maxmx3rr(XD, XS, XT)                                                \
        maxgx3rr(W(XD), W(XS), W(XT))

#define maxmx3ld(XD, XS, MT, DT)                                            \
        maxgx3ld(W(XD), W(XS), W(MT), W(DT))


#define maxmb_rr(XG, XS)                                                    \
        maxgb_rr(W(XG), W(XS))

#define maxmb_ld(XG, MS, DS)                                                \
        maxgb_ld(W(XG), W(MS), W(DS))

#define maxmb3rr(XD, XS, XT)                                                \
        maxgb3rr(W(XD), W(XS), W(XT))

#define maxmb3ld(XD, XS, MT, DT)                                            \
        maxgb3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxmn_rr(XG, XS)                                                    \
        maxgn_rr(W(XG), W(XS))

#define maxmn_ld(XG, MS, DS)                                                \
        maxgn_ld(W(XG), W(MS), W(DS))

#define maxmn3rr(XD, XS, XT)                                                \
        maxgn3rr(W(XD), W(XS), W(XT))

#define maxmn3ld(XD, XS, MT, DT)                                            \
        maxgn3ld(W(XD), W(XS), W(MT), W(DT))


#define maxmc_rr(XG, XS)                                                    \
        maxgc_rr(W(XG), W(XS))

#define maxmc_ld(XG, MS, DS)                                                \
        maxgc_ld(W(XG), W(MS), W(DS))

#define maxmc3rr(XD, XS, XT)                                                \
        maxgc3rr(W(XD), W(XS), W(XT))

#define maxmc3ld(XD, XS, MT, DT)                                            \
        maxgc3ld(W(XD), W(XS), W(MT), W(DT))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqmx_rr(XG, XS)                                                    \
        ceqgx_rr(W(XG), W(XS))

#define ceqmx_ld(XG, MS, DS)                                                \
        ceqgx_ld(W(XG), W(MS), W(DS))

#define ceqmx3rr(XD, XS, XT)                                                \
        ceqgx3rr(W(XD), W(XS), W(XT))

#define ceqmx3ld(XD, XS, MT, DT)                                            \
        ceqgx3ld(W(XD), W(XS), W(MT), W(DT))


#define ceqmb_rr(XG, XS)                                                    \
        ceqgb_rr(W(XG), W(XS))

#define ceqmb_ld(XG, MS, DS)                                                \
        ceqgb_ld(W(XG), W(MS), W(DS))

#define ceqmb3rr(XD, XS, XT)                                                \
        ceqgb3rr(W(XD), W(XS), W(XT))

#define ceqmb3ld(XD, XS, MT, DT)                                            \
        ceqgb3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnemx_rr(XG, XS)                                                    \
        cnegx_rr(W(XG), W(XS))

#define cnemx_ld(XG, MS, DS)                                                \
        cnegx_ld(W(XG), W(MS), W(DS))

#define cnemx3rr(XD, XS, XT)                                                \
        cnegx3rr(W(XD), W(XS), W(XT))

#define cnemx3ld(XD, XS, MT, DT)                                            \
        cnegx3ld(W(XD), W(XS), W(MT), W(DT))


#define cnemb_rr(XG, XS)                                                    \
        cnegb_rr(W(XG), W(XS))

#define cnemb_ld(XG, MS, DS)                                                \
        cnegb_ld(W(XG), W(MS), W(DS))

#define cnemb3rr(XD, XS, XT)                                                \
        cnegb3rr(W(XD), W(XS), W(XT))

#define cnemb3ld(XD, XS, MT, DT)                                            \
        cnegb3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltmx_rr(XG, XS)                                                    \
        cltgx_rr(W(XG), W(XS))

#define cltmx_ld(XG, MS, DS)                                                \
        cltgx_ld(W(XG), W(MS), W(DS))

#define cltmx3rr(XD, XS, XT)                                                \
        cltgx3rr(W(XD), W(XS), W(XT))

#define cltmx3ld(XD, XS, MT, DT)                                            \
        cltgx3ld(W(XD), W(XS), W(MT), W(DT))


#define cltmb_rr(XG, XS)                                                    \
        cltgb_rr(W(XG), W(XS))

#define cltmb_ld(XG, MS, DS)                                                \
        cltgb_ld(W(XG), W(MS), W(DS))

#define cltmb3rr(XD, XS, XT)                                                \
        cltgb3rr(W(XD), W(XS), W(XT))

#define cltmb3ld(XD, XS, MT, DT)                                            \
        cltgb3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltmn_rr(XG, XS)                                                    \
        cltgn_rr(W(XG), W(XS))

#define cltmn_ld(XG, MS, DS)                                                \
        cltgn_ld(W(XG), W(MS), W(DS))

#define cltmn3rr(XD, XS, XT)                                                \
        cltgn3rr(W(XD), W(XS), W(XT))

#define cltmn3ld(XD, XS, MT, DT)                                            \
        cltgn3ld(W(XD), W(XS), W(MT), W(DT))


#define cltmc_rr(XG, XS)                                                    \
        cltgc_rr(W(XG), W(XS))

#define cltmc_ld(XG, MS, DS)                                                \
        cltgc_ld(W(XG), W(MS), W(DS))

#define cltmc3rr(XD, XS, XT)                                                \
        cltgc3rr(W(XD), W(XS), W(XT))

#define cltmc3ld(XD, XS, MT, DT)                                            \
        cltgc3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clemx_rr(XG, XS)                                                    \
        clegx_rr(W(XG), W(XS))

#define clemx_ld(XG, MS, DS)                                                \
        clegx_ld(W(XG), W(MS), W(DS))

#define clemx3rr(XD, XS, XT)                                                \
        clegx3rr(W(XD), W(XS), W(XT))

#define clemx3ld(XD, XS, MT, DT)                                            \
        clegx3ld(W(XD), W(XS), W(MT), W(DT))


#define clemb_rr(XG, XS)                                                    \
        clegb_rr(W(XG), W(XS))

#define clemb_ld(XG, MS, DS)                                                \
        clegb_ld(W(XG), W(MS), W(DS))

#define clemb3rr(XD, XS, XT)                                                \
        clegb3rr(W(XD), W(XS), W(XT))

#define clemb3ld(XD, XS, MT, DT)                                            \
        clegb3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clemn_rr(XG, XS)                                                    \
        clegn_rr(W(XG), W(XS))

#define clemn_ld(XG, MS, DS)                                                \
        clegn_ld(W(XG), W(MS), W(DS))

#define clemn3rr(XD, XS, XT)                                                \
        clegn3rr(W(XD), W(XS), W(XT))

#define clemn3ld(XD, XS, MT, DT)                                            \
        clegn3ld(W(XD), W(XS), W(MT), W(DT))


#define clemc_rr(XG, XS)                                                    \
        clegc_rr(W(XG), W(XS))

#define clemc_ld(XG, MS, DS)                                                \
        clegc_ld(W(XG), W(MS), W(DS))

#define clemc3rr(XD, XS, XT)                                                \
        clegc3rr(W(XD), W(XS), W(XT))

#define clemc3ld(XD, XS, MT, DT)                                            \
        clegc3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtmx_rr(XG, XS)                                                    \
        cgtgx_rr(W(XG), W(XS))

#define cgtmx_ld(XG, MS, DS)                                                \
        cgtgx_ld(W(XG), W(MS), W(DS))

#define cgtmx3rr(XD, XS, XT)                                                \
        cgtgx3rr(W(XD), W(XS), W(XT))

#define cgtmx3ld(XD, XS, MT, DT)                                            \
        cgtgx3ld(W(XD), W(XS), W(MT), W(DT))


#define cgtmb_rr(XG, XS)                                                    \
        cgtgb_rr(W(XG), W(XS))

#define cgtmb_ld(XG, MS, DS)                                                \
        cgtgb_ld(W(XG), W(MS), W(DS))

#define cgtmb3rr(XD, XS, XT)                                                \
        cgtgb3rr(W(XD), W(XS), W(XT))

#define cgtmb3ld(XD, XS, MT, DT)                                            \
        cgtgb3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtmn_rr(XG, XS)                                                    \
        cgtgn_rr(W(XG), W(XS))

#define cgtmn_ld(XG, MS, DS)                                                \
        cgtgn_ld(W(XG), W(MS), W(DS))

#define cgtmn3rr(XD, XS, XT)                                                \
        cgtgn3rr(W(XD), W(XS), W(XT))

#define cgtmn3ld(XD, XS, MT, DT)                                            \
        cgtgn3ld(W(XD), W(XS), W(MT), W(DT))


#define cgtmc_rr(XG, XS)                                                    \
        cgtgc_rr(W(XG), W(XS))

#define cgtmc_ld(XG, MS, DS)                                                \
        cgtgc_ld(W(XG), W(MS), W(DS))

#define cgtmc3rr(XD, XS, XT)                                                \
        cgtgc3rr(W(XD), W(XS), W(XT))

#define cgtmc3ld(XD, XS, MT, DT)                                            \
        cgtgc3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgemx_rr(XG, XS)                                                    \
        cgegx_rr(W(XG), W(XS))

#define cgemx_ld(XG, MS, DS)                                                \
        cgegx_ld(W(XG), W(MS), W(DS))

#define cgemx3rr(XD, XS, XT)                                                \
        cgegx3rr(W(XD), W(XS), W(XT))

#define cgemx3ld(XD, XS, MT, DT)                                            \
        cgegx3ld(W(XD), W(XS), W(MT), W(DT))


#define cgemb_rr(XG, XS)                                                    \
        cgegb_rr(W(XG), W(XS))

#define cgemb_ld(XG, MS, DS)                                                \
        cgegb_ld(W(XG), W(MS), W(DS))

#define cgemb3rr(XD, XS, XT)                                                \
        cgegb3rr(W(XD), W(XS), W(XT))

#define cgemb3ld(XD, XS, MT, DT)                                            \
        cgegb3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgemn_rr(XG, XS)                                                    \
        cgegn_rr(W(XG), W(XS))

#define cgemn_ld(XG, MS, DS)                                                \
        cgegn_ld(W(XG), W(MS), W(DS))

#define cgemn3rr(XD, XS, XT)                                                \
        cgegn3rr(W(XD), W(XS), W(XT))

#define cgemn3ld(XD, XS, MT, DT)                                            \
        cgegn3ld(W(XD), W(XS), W(MT), W(DT))


#define cgemc_rr(XG, XS)                                                    \
        cgegc_rr(W(XG), W(XS))

#define cgemc_ld(XG, MS, DS)                                                \
        cgegc_ld(W(XG), W(MS), W(DS))

#define cgemc3rr(XD, XS, XT)                                                \
        cgegc3rr(W(XD), W(XS), W(XT))

#define cgemc3ld(XD, XS, MT, DT)                                            \
        cgegc3ld(W(XD), W(XS), W(MT), W(DT))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define mkjmx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        mkjgx_rx(W(XS), mask, lb)

#define mkjmb_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        mkjgb_rx(W(XS), mask, lb)

#endif /* RT_SIMD: 256, 128 */

/******************************************************************************/
/**** var-len **** SIMD instructions with fixed-32-bit element **** 256-bit ***/
/******************************************************************************/

#if   (RT_SIMD == 256) && !(defined RT_SVEX1)

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmox_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        elmcx_st(W(XS), W(MD), W(DD))

/***************   packed single-precision generic move/logic   ***************/

/* mov (D = S) */

#define movox_rr(XD, XS)                                                    \
        movcx_rr(W(XD), W(XS))

#define movox_ld(XD, MS, DS)                                                \
        movcx_ld(W(XD), W(MS), W(DS))

#define movox_st(XS, MD, DD)                                                \
        movcx_st(W(XS), W(MD), W(DD))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvox_rr(XD, XS)                                                    \
        mmvcx_rr(W(XD), W(XS))

#define mmvox_ld(XG, MS, DS)                                                \
        mmvcx_ld(W(XG), W(MS), W(DS))

#define mmvox_st(XS, MG, DG)                                                \
        mmvcx_st(W(XS), W(MG), W(DG))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andox_rr(XG, XS)                                                    \
        andcx_rr(W(XG), W(XS))

#define andox_ld(XG, MS, DS)                                                \
        andcx_ld(W(XG), W(MS), W(DS))

#define andox3rr(XD, XS, XT)                                                \
        andcx3rr(W(XD), W(XS), W(XT))

#define andox3ld(XD, XS, MT, DT)                                            \
        andcx3ld(W(XD), W(XS), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annox_rr(XG, XS)                                                    \
        anncx_rr(W(XG), W(XS))

#define annox_ld(XG, MS, DS)                                                \
        anncx_ld(W(XG), W(MS), W(DS))

#define annox3rr(XD, XS, XT)                                                \
        anncx3rr(W(XD), W(XS), W(XT))

#define annox3ld(XD, XS, MT, DT)                                            \
        anncx3ld(W(XD), W(XS), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrox_rr(XG, XS)                                                    \
        orrcx_rr(W(XG), W(XS))

#define orrox_ld(XG, MS, DS)                                                \
        orrcx_ld(W(XG), W(MS), W(DS))

#define orrox3rr(XD, XS, XT)                                                \
        orrcx3rr(W(XD), W(XS), W(XT))

#define orrox3ld(XD, XS, MT, DT)                                            \
        orrcx3ld(W(XD), W(XS), W(MT), W(DT))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define ornox_rr(XG, XS)                                                    \
        orncx_rr(W(XG), W(XS))

#define ornox_ld(XG, MS, DS)                                                \
        orncx_ld(W(XG), W(MS), W(DS))

#define ornox3rr(XD, XS, XT)                                                \
        orncx3rr(W(XD), W(XS), W(XT))

#define ornox3ld(XD, XS, MT, DT)                                            \
        orncx3ld(W(XD), W(XS), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorox_rr(XG, XS)                                                    \
        xorcx_rr(W(XG), W(XS))

#define xorox_ld(XG, MS, DS)                                                \
        xorcx_ld(W(XG), W(MS), W(DS))

#define xorox3rr(XD, XS, XT)                                                \
        xorcx3rr(W(XD), W(XS), W(XT))

#define xorox3ld(XD, XS, MT, DT)                                            \
        xorcx3ld(W(XD), W(XS), W(MT), W(DT))

/* not (G = ~G), (D = ~S) */

#define notox_rx(XG)                                                        \
        notcx_rx(W(XG))

#define notox_rr(XD, XS)                                                    \
        notcx_rr(W(XD), W(XS))

/************   packed single-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negos_rx(XG)                                                        \
        negcs_rx(W(XG))

#define negos_rr(XD, XS)                                                    \
        negcs_rr(W(XD), W(XS))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addos_rr(XG, XS)                                                    \
        addcs_rr(W(XG), W(XS))

#define addos_ld(XG, MS, DS)                                                \
        addcs_ld(W(XG), W(MS), W(DS))

#define addos3rr(XD, XS, XT)                                                \
        addcs3rr(W(XD), W(XS), W(XT))

#define addos3ld(XD, XS, MT, DT)                                            \
        addcs3ld(W(XD), W(XS), W(MT), W(DT))

#define adpos_rr(XG, XS) /* horizontal pairwise add, first 15-regs only */  \
        adpcs_rr(W(XG), W(XS))

#define adpos_ld(XG, MS, DS)                                                \
        adpcs_ld(W(XG), W(MS), W(DS))

#define adpos3rr(XD, XS, XT)                                                \
        adpcs3rr(W(XD), W(XS), W(XT))

#define adpos3ld(XD, XS, MT, DT)                                            \
        adpcs3ld(W(XD), W(XS), W(MT), W(DT))

#define adhos_rr(XG, XS) /* horizontal reductive add, first 15-regs only */ \
        adhcs_rr(W(XG), W(XS))

#define adhos_ld(XG, MS, DS)                                                \
        adhcs_ld(W(XG), W(MS), W(DS))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subos_rr(XG, XS)                                                    \
        subcs_rr(W(XG), W(XS))

#define subos_ld(XG, MS, DS)                                                \
        subcs_ld(W(XG), W(MS), W(DS))

#define subos3rr(XD, XS, XT)                                                \
        subcs3rr(W(XD), W(XS), W(XT))

#define subos3ld(XD, XS, MT, DT)                                            \
        subcs3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulos_rr(XG, XS)                                                    \
        mulcs_rr(W(XG), W(XS))

#define mulos_ld(XG, MS, DS)                                                \
        mulcs_ld(W(XG), W(MS), W(DS))

#define mulos3rr(XD, XS, XT)                                                \
        mulcs3rr(W(XD), W(XS), W(XT))

#define mulos3ld(XD, XS, MT, DT)                                            \
        mulcs3ld(W(XD), W(XS), W(MT), W(DT))

#define mlpos_rr(XG, XS) /* horizontal pairwise mul */                      \
        mlpcs_rr(W(XG), W(XS))

#define mlpos_ld(XG, MS, DS)                                                \
        mlpcs_ld(W(XG), W(MS), W(DS))

#define mlpos3rr(XD, XS, XT)                                                \
        mlpcs3rr(W(XD), W(XS), W(XT))

#define mlpos3ld(XD, XS, MT, DT)                                            \
        mlpcs3ld(W(XD), W(XS), W(MT), W(DT))

#define mlhos_rr(XD, XS) /* horizontal reductive mul */                     \
        mlhcs_rr(W(XD), W(XS))

#define mlhos_ld(XD, MS, DS)                                                \
        mlhcs_ld(W(XD), W(MS), W(DS))

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divos_rr(XG, XS)                                                    \
        divcs_rr(W(XG), W(XS))

#define divos_ld(XG, MS, DS)                                                \
        divcs_ld(W(XG), W(MS), W(DS))

#define divos3rr(XD, XS, XT)                                                \
        divcs3rr(W(XD), W(XS), W(XT))

#define divos3ld(XD, XS, MT, DT)                                            \
        divcs3ld(W(XD), W(XS), W(MT), W(DT))

/* sqr (D = sqrt S) */

#define sqros_rr(XD, XS)                                                    \
        sqrcs_rr(W(XD), W(XS))

#define sqros_ld(XD, MS, DS)                                                \
        sqrcs_ld(W(XD), W(MS), W(DS))

/* cbr (D = cbrt S) */

#define cbros_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbrcs_rr(W(XD), W(X1), W(X2), W(XS))

#define cbeos_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbecs_rr(W(XD), W(X1), W(X2), W(XS))

#define cbsos_rr(XG, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbscs_rr(W(XG), W(X1), W(X2), W(XS))

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rcpos_rr(XD, XS) /* destroys XS */                                  \
        rcpcs_rr(W(XD), W(XS))

#define rceos_rr(XD, XS)                                                    \
        rcecs_rr(W(XD), W(XS))

#define rcsos_rr(XG, XS) /* destroys XS */                                  \
        rcscs_rr(W(XG), W(XS))

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rsqos_rr(XD, XS) /* destroys XS */                                  \
        rsqcs_rr(W(XD), W(XS))

#define rseos_rr(XD, XS)                                                    \
        rsecs_rr(W(XD), W(XS))

#define rssos_rr(XG, XS) /* destroys XS */                                  \
        rsscs_rr(W(XG), W(XS))

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmaos_rr(XG, XS, XT)                                                \
        fmacs_rr(W(XG), W(XS), W(XT))

#define fmaos_ld(XG, XS, MT, DT)                                            \
        fmacs_ld(W(XG), W(XS), W(MT), W(DT))

#define fmaos3rr(XG, XS, XT)                                                \
        fmaos_rr(W(XG), W(XS), W(XT))

#define fmaos3ld(XG, XS, MT, DT)                                            \
        fmaos_ld(W(XG), W(XS), W(MT), W(DT))

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsos_rr(XG, XS, XT)                                                \
        fmscs_rr(W(XG), W(XS), W(XT))

#define fmsos_ld(XG, XS, MT, DT)                                            \
        fmscs_ld(W(XG), W(XS), W(MT), W(DT))

#define fmsos3rr(XG, XS, XT)                                                \
        fmsos_rr(W(XG), W(XS), W(XT))

#define fmsos3ld(XG, XS, MT, DT)                                            \
        fmsos_ld(W(XG), W(XS), W(MT), W(DT))

/*************   packed single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minos_rr(XG, XS)                                                    \
        mincs_rr(W(XG), W(XS))

#define minos_ld(XG, MS, DS)                                                \
        mincs_ld(W(XG), W(MS), W(DS))

#define minos3rr(XD, XS, XT)                                                \
        mincs3rr(W(XD), W(XS), W(XT))

#define minos3ld(XD, XS, MT, DT)                                            \
        mincs3ld(W(XD), W(XS), W(MT), W(DT))

#define mnpos_rr(XG, XS) /* horizontal pairwise min */                      \
        mnpcs_rr(W(XG), W(XS))

#define mnpos_ld(XG, MS, DS)                                                \
        mnpcs_ld(W(XG), W(MS), W(DS))

#define mnpos3rr(XD, XS, XT)                                                \
        mnpcs3rr(W(XD), W(XS), W(XT))

#define mnpos3ld(XD, XS, MT, DT)                                            \
        mnpcs3ld(W(XD), W(XS), W(MT), W(DT))

#define mnhos_rr(XD, XS) /* horizontal reductive min */                     \
        mnhcs_rr(W(XD), W(XS))

#define mnhos_ld(XD, MS, DS)                                                \
        mnhcs_ld(W(XD), W(MS), W(DS))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxos_rr(XG, XS)                                                    \
        maxcs_rr(W(XG), W(XS))

#define maxos_ld(XG, MS, DS)                                                \
        maxcs_ld(W(XG), W(MS), W(DS))

#define maxos3rr(XD, XS, XT)                                                \
        maxcs3rr(W(XD), W(XS), W(XT))

#define maxos3ld(XD, XS, MT, DT)                                            \
        maxcs3ld(W(XD), W(XS), W(MT), W(DT))

#define mxpos_rr(XG, XS) /* horizontal pairwise max */                      \
        mxpcs_rr(W(XG), W(XS))

#define mxpos_ld(XG, MS, DS)                                                \
        mxpcs_ld(W(XG), W(MS), W(DS))

#define mxpos3rr(XD, XS, XT)                                                \
        mxpcs3rr(W(XD), W(XS), W(XT))

#define mxpos3ld(XD, XS, MT, DT)                                            \
        mxpcs3ld(W(XD), W(XS), W(MT), W(DT))

#define mxhos_rr(XD, XS) /* horizontal reductive max */                     \
        mxhcs_rr(W(XD), W(XS))

#define mxhos_ld(XD, MS, DS)                                                \
        mxhcs_ld(W(XD), W(MS), W(DS))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqos_rr(XG, XS)                                                    \
        ceqcs_rr(W(XG), W(XS))

#define ceqos_ld(XG, MS, DS)                                                \
        ceqcs_ld(W(XG), W(MS), W(DS))

#define ceqos3rr(XD, XS, XT)                                                \
        ceqcs3rr(W(XD), W(XS), W(XT))

#define ceqos3ld(XD, XS, MT, DT)                                            \
        ceqcs3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneos_rr(XG, XS)                                                    \
        cnecs_rr(W(XG), W(XS))

#define cneos_ld(XG, MS, DS)                                                \
        cnecs_ld(W(XG), W(MS), W(DS))

#define cneos3rr(XD, XS, XT)                                                \
        cnecs3rr(W(XD), W(XS), W(XT))

#define cneos3ld(XD, XS, MT, DT)                                            \
        cnecs3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltos_rr(XG, XS)                                                    \
        cltcs_rr(W(XG), W(XS))

#define cltos_ld(XG, MS, DS)                                                \
        cltcs_ld(W(XG), W(MS), W(DS))

#define cltos3rr(XD, XS, XT)                                                \
        cltcs3rr(W(XD), W(XS), W(XT))

#define cltos3ld(XD, XS, MT, DT)                                            \
        cltcs3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define cleos_rr(XG, XS)                                                    \
        clecs_rr(W(XG), W(XS))

#define cleos_ld(XG, MS, DS)                                                \
        clecs_ld(W(XG), W(MS), W(DS))

#define cleos3rr(XD, XS, XT)                                                \
        clecs3rr(W(XD), W(XS), W(XT))

#define cleos3ld(XD, XS, MT, DT)                                            \
        clecs3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtos_rr(XG, XS)                                                    \
        cgtcs_rr(W(XG), W(XS))

#define cgtos_ld(XG, MS, DS)                                                \
        cgtcs_ld(W(XG), W(MS), W(DS))

#define cgtos3rr(XD, XS, XT)                                                \
        cgtcs3rr(W(XD), W(XS), W(XT))

#define cgtos3ld(XD, XS, MT, DT)                                            \
        cgtcs3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgeos_rr(XG, XS)                                                    \
        cgecs_rr(W(XG), W(XS))

#define cgeos_ld(XG, MS, DS)                                                \
        cgecs_ld(W(XG), W(MS), W(DS))

#define cgeos3rr(XD, XS, XT)                                                \
        cgecs3rr(W(XD), W(XS), W(XT))

#define cgeos3ld(XD, XS, MT, DT)                                            \
        cgecs3ld(W(XD), W(XS), W(MT), W(DT))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define mkjox_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        mkjcx_rx(W(XS), mask, lb)

/*************   packed single-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzos_rr(XD, XS)     /* round towards zero */                       \
        rnzcs_rr(W(XD), W(XS))

#define rnzos_ld(XD, MS, DS) /* round towards zero */                       \
        rnzcs_ld(W(XD), W(MS), W(DS))

#define cvzos_rr(XD, XS)     /* round towards zero */                       \
        cvzcs_rr(W(XD), W(XS))

#define cvzos_ld(XD, MS, DS) /* round towards zero */                       \
        cvzcs_ld(W(XD), W(MS), W(DS))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpos_rr(XD, XS)     /* round towards +inf */                       \
        rnpcs_rr(W(XD), W(XS))

#define rnpos_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpcs_ld(W(XD), W(MS), W(DS))

#define cvpos_rr(XD, XS)     /* round towards +inf */                       \
        cvpcs_rr(W(XD), W(XS))

#define cvpos_ld(XD, MS, DS) /* round towards +inf */                       \
        cvpcs_ld(W(XD), W(MS), W(DS))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmos_rr(XD, XS)     /* round towards -inf */                       \
        rnmcs_rr(W(XD), W(XS))

#define rnmos_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmcs_ld(W(XD), W(MS), W(DS))

#define cvmos_rr(XD, XS)     /* round towards -inf */                       \
        cvmcs_rr(W(XD), W(XS))

#define cvmos_ld(XD, MS, DS) /* round towards -inf */                       \
        cvmcs_ld(W(XD), W(MS), W(DS))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnos_rr(XD, XS)     /* round towards near */                       \
        rnncs_rr(W(XD), W(XS))

#define rnnos_ld(XD, MS, DS) /* round towards near */                       \
        rnncs_ld(W(XD), W(MS), W(DS))

#define cvnos_rr(XD, XS)     /* round towards near */                       \
        cvncs_rr(W(XD), W(XS))

#define cvnos_ld(XD, MS, DS) /* round towards near */                       \
        cvncs_ld(W(XD), W(MS), W(DS))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnon_rr(XD, XS)     /* round towards near */                       \
        cvncn_rr(W(XD), W(XS))

#define cvnon_ld(XD, MS, DS) /* round towards near */                       \
        cvncn_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndos_rr(XD, XS)                                                    \
        rndcs_rr(W(XD), W(XS))

#define rndos_ld(XD, MS, DS)                                                \
        rndcs_ld(W(XD), W(MS), W(DS))

#define cvtos_rr(XD, XS)                                                    \
        cvtcs_rr(W(XD), W(XS))

#define cvtos_ld(XD, MS, DS)                                                \
        cvtcs_ld(W(XD), W(MS), W(DS))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvton_rr(XD, XS)                                                    \
        cvtcn_rr(W(XD), W(XS))

#define cvton_ld(XD, MS, DS)                                                \
        cvtcn_ld(W(XD), W(MS), W(DS))

/* cvn (D = unsigned-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnox_rr(XD, XS)     /* round towards near */                       \
        cvncx_rr(W(XD), W(XS))

#define cvnox_ld(XD, MS, DS) /* round towards near */                       \
        cvncx_ld(W(XD), W(MS), W(DS))

/* cvt (D = unsigned-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtox_rr(XD, XS)                                                    \
        cvtcx_rr(W(XD), W(XS))

#define cvtox_ld(XD, MS, DS)                                                \
        cvtcx_ld(W(XD), W(MS), W(DS))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnros_rr(XD, XS, mode)                                              \
        rnrcs_rr(W(XD), W(XS), mode)

#define cvros_rr(XD, XS, mode)                                              \
        cvrcs_rr(W(XD), W(XS), mode)

/************   packed single-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addox_rr(XG, XS)                                                    \
        addcx_rr(W(XG), W(XS))

#define addox_ld(XG, MS, DS)                                                \
        addcx_ld(W(XG), W(MS), W(DS))

#define addox3rr(XD, XS, XT)                                                \
        addcx3rr(W(XD), W(XS), W(XT))

#define addox3ld(XD, XS, MT, DT)                                            \
        addcx3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subox_rr(XG, XS)                                                    \
        subcx_rr(W(XG), W(XS))

#define subox_ld(XG, MS, DS)                                                \
        subcx_ld(W(XG), W(MS), W(DS))

#define subox3rr(XD, XS, XT)                                                \
        subcx3rr(W(XD), W(XS), W(XT))

#define subox3ld(XD, XS, MT, DT)                                            \
        subcx3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulox_rr(XG, XS)                                                    \
        mulcx_rr(W(XG), W(XS))

#define mulox_ld(XG, MS, DS)                                                \
        mulcx_ld(W(XG), W(MS), W(DS))

#define mulox3rr(XD, XS, XT)                                                \
        mulcx3rr(W(XD), W(XS), W(XT))

#define mulox3ld(XD, XS, MT, DT)                                            \
        mulcx3ld(W(XD), W(XS), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlox_ri(XG, IS)                                                    \
        shlcx_ri(W(XG), W(IS))

#define shlox_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlcx_ld(W(XG), W(MS), W(DS))

#define shlox3ri(XD, XS, IT)                                                \
        shlcx3ri(W(XD), W(XS), W(IT))

#define shlox3ld(XD, XS, MT, DT)                                            \
        shlcx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrox_ri(XG, IS)                                                    \
        shrcx_ri(W(XG), W(IS))

#define shrox_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrcx_ld(W(XG), W(MS), W(DS))

#define shrox3ri(XD, XS, IT)                                                \
        shrcx3ri(W(XD), W(XS), W(IT))

#define shrox3ld(XD, XS, MT, DT)                                            \
        shrcx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shron_ri(XG, IS)                                                    \
        shrcn_ri(W(XG), W(IS))

#define shron_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrcn_ld(W(XG), W(MS), W(DS))

#define shron3ri(XD, XS, IT)                                                \
        shrcn3ri(W(XD), W(XS), W(IT))

#define shron3ld(XD, XS, MT, DT)                                            \
        shrcn3ld(W(XD), W(XS), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlox_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlcx_rr(W(XG), W(XS))

#define svlox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlcx_ld(W(XG), W(MS), W(DS))

#define svlox3rr(XD, XS, XT)                                                \
        svlcx3rr(W(XD), W(XS), W(XT))

#define svlox3ld(XD, XS, MT, DT)                                            \
        svlcx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrox_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrcx_rr(W(XG), W(XS))

#define svrox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrcx_ld(W(XG), W(MS), W(DS))

#define svrox3rr(XD, XS, XT)                                                \
        svrcx3rr(W(XD), W(XS), W(XT))

#define svrox3ld(XD, XS, MT, DT)                                            \
        svrcx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svron_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrcn_rr(W(XG), W(XS))

#define svron_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrcn_ld(W(XG), W(MS), W(DS))

#define svron3rr(XD, XS, XT)                                                \
        svrcn3rr(W(XD), W(XS), W(XT))

#define svron3ld(XD, XS, MT, DT)                                            \
        svrcn3ld(W(XD), W(XS), W(MT), W(DT))

/****************   packed single-precision integer compare   *****************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minox_rr(XG, XS)                                                    \
        mincx_rr(W(XG), W(XS))

#define minox_ld(XG, MS, DS)                                                \
        mincx_ld(W(XG), W(MS), W(DS))

#define minox3rr(XD, XS, XT)                                                \
        mincx3rr(W(XD), W(XS), W(XT))

#define minox3ld(XD, XS, MT, DT)                                            \
        mincx3ld(W(XD), W(XS), W(MT), W(DT))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minon_rr(XG, XS)                                                    \
        mincn_rr(W(XG), W(XS))

#define minon_ld(XG, MS, DS)                                                \
        mincn_ld(W(XG), W(MS), W(DS))

#define minon3rr(XD, XS, XT)                                                \
        mincn3rr(W(XD), W(XS), W(XT))

#define minon3ld(XD, XS, MT, DT)                                            \
        mincn3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxox_rr(XG, XS)                                                    \
        maxcx_rr(W(XG), W(XS))

#define maxox_ld(XG, MS, DS)                                                \
        maxcx_ld(W(XG), W(MS), W(DS))

#define maxox3rr(XD, XS, XT)                                                \
        maxcx3rr(W(XD), W(XS), W(XT))

#define maxox3ld(XD, XS, MT, DT)                                            \
        maxcx3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxon_rr(XG, XS)                                                    \
        maxcn_rr(W(XG), W(XS))

#define maxon_ld(XG, MS, DS)                                                \
        maxcn_ld(W(XG), W(MS), W(DS))

#define maxon3rr(XD, XS, XT)                                                \
        maxcn3rr(W(XD), W(XS), W(XT))

#define maxon3ld(XD, XS, MT, DT)                                            \
        maxcn3ld(W(XD), W(XS), W(MT), W(DT))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqox_rr(XG, XS)                                                    \
        ceqcx_rr(W(XG), W(XS))

#define ceqox_ld(XG, MS, DS)                                                \
        ceqcx_ld(W(XG), W(MS), W(DS))

#define ceqox3rr(XD, XS, XT)                                                \
        ceqcx3rr(W(XD), W(XS), W(XT))

#define ceqox3ld(XD, XS, MT, DT)                                            \
        ceqcx3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneox_rr(XG, XS)                                                    \
        cnecx_rr(W(XG), W(XS))

#define cneox_ld(XG, MS, DS)                                                \
        cnecx_ld(W(XG), W(MS), W(DS))

#define cneox3rr(XD, XS, XT)                                                \
        cnecx3rr(W(XD), W(XS), W(XT))

#define cneox3ld(XD, XS, MT, DT)                                            \
        cnecx3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltox_rr(XG, XS)                                                    \
        cltcx_rr(W(XG), W(XS))

#define cltox_ld(XG, MS, DS)                                                \
        cltcx_ld(W(XG), W(MS), W(DS))

#define cltox3rr(XD, XS, XT)                                                \
        cltcx3rr(W(XD), W(XS), W(XT))

#define cltox3ld(XD, XS, MT, DT)                                            \
        cltcx3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define clton_rr(XG, XS)                                                    \
        cltcn_rr(W(XG), W(XS))

#define clton_ld(XG, MS, DS)                                                \
        cltcn_ld(W(XG), W(MS), W(DS))

#define clton3rr(XD, XS, XT)                                                \
        cltcn3rr(W(XD), W(XS), W(XT))

#define clton3ld(XD, XS, MT, DT)                                            \
        cltcn3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define cleox_rr(XG, XS)                                                    \
        clecx_rr(W(XG), W(XS))

#define cleox_ld(XG, MS, DS)                                                \
        clecx_ld(W(XG), W(MS), W(DS))

#define cleox3rr(XD, XS, XT)                                                \
        clecx3rr(W(XD), W(XS), W(XT))

#define cleox3ld(XD, XS, MT, DT)                                            \
        clecx3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define cleon_rr(XG, XS)                                                    \
        clecn_rr(W(XG), W(XS))

#define cleon_ld(XG, MS, DS)                                                \
        clecn_ld(W(XG), W(MS), W(DS))

#define cleon3rr(XD, XS, XT)                                                \
        clecn3rr(W(XD), W(XS), W(XT))

#define cleon3ld(XD, XS, MT, DT)                                            \
        clecn3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtox_rr(XG, XS)                                                    \
        cgtcx_rr(W(XG), W(XS))

#define cgtox_ld(XG, MS, DS)                                                \
        cgtcx_ld(W(XG), W(MS), W(DS))

#define cgtox3rr(XD, XS, XT)                                                \
        cgtcx3rr(W(XD), W(XS), W(XT))

#define cgtox3ld(XD, XS, MT, DT)                                            \
        cgtcx3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgton_rr(XG, XS)                                                    \
        cgtcn_rr(W(XG), W(XS))

#define cgton_ld(XG, MS, DS)                                                \
        cgtcn_ld(W(XG), W(MS), W(DS))

#define cgton3rr(XD, XS, XT)                                                \
        cgtcn3rr(W(XD), W(XS), W(XT))

#define cgton3ld(XD, XS, MT, DT)                                            \
        cgtcn3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgeox_rr(XG, XS)                                                    \
        cgecx_rr(W(XG), W(XS))

#define cgeox_ld(XG, MS, DS)                                                \
        cgecx_ld(W(XG), W(MS), W(DS))

#define cgeox3rr(XD, XS, XT)                                                \
        cgecx3rr(W(XD), W(XS), W(XT))

#define cgeox3ld(XD, XS, MT, DT)                                            \
        cgecx3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgeon_rr(XG, XS)                                                    \
        cgecn_rr(W(XG), W(XS))

#define cgeon_ld(XG, MS, DS)                                                \
        cgecn_ld(W(XG), W(MS), W(DS))

#define cgeon3rr(XD, XS, XT)                                                \
        cgecn3rr(W(XD), W(XS), W(XT))

#define cgeon3ld(XD, XS, MT, DT)                                            \
        cgecn3ld(W(XD), W(XS), W(MT), W(DT))

/******************************************************************************/
/**** var-len **** SIMD instructions with fixed-32-bit element **** 128-bit ***/
/******************************************************************************/

#elif (RT_SIMD == 128)

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmox_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        elmix_st(W(XS), W(MD), W(DD))

/***************   packed single-precision generic move/logic   ***************/

/* mov (D = S) */

#define movox_rr(XD, XS)                                                    \
        movix_rr(W(XD), W(XS))

#define movox_ld(XD, MS, DS)                                                \
        movix_ld(W(XD), W(MS), W(DS))

#define movox_st(XS, MD, DD)                                                \
        movix_st(W(XS), W(MD), W(DD))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvox_rr(XD, XS)                                                    \
        mmvix_rr(W(XD), W(XS))

#define mmvox_ld(XG, MS, DS)                                                \
        mmvix_ld(W(XG), W(MS), W(DS))

#define mmvox_st(XS, MG, DG)                                                \
        mmvix_st(W(XS), W(MG), W(DG))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andox_rr(XG, XS)                                                    \
        andix_rr(W(XG), W(XS))

#define andox_ld(XG, MS, DS)                                                \
        andix_ld(W(XG), W(MS), W(DS))

#define andox3rr(XD, XS, XT)                                                \
        andix3rr(W(XD), W(XS), W(XT))

#define andox3ld(XD, XS, MT, DT)                                            \
        andix3ld(W(XD), W(XS), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annox_rr(XG, XS)                                                    \
        annix_rr(W(XG), W(XS))

#define annox_ld(XG, MS, DS)                                                \
        annix_ld(W(XG), W(MS), W(DS))

#define annox3rr(XD, XS, XT)                                                \
        annix3rr(W(XD), W(XS), W(XT))

#define annox3ld(XD, XS, MT, DT)                                            \
        annix3ld(W(XD), W(XS), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrox_rr(XG, XS)                                                    \
        orrix_rr(W(XG), W(XS))

#define orrox_ld(XG, MS, DS)                                                \
        orrix_ld(W(XG), W(MS), W(DS))

#define orrox3rr(XD, XS, XT)                                                \
        orrix3rr(W(XD), W(XS), W(XT))

#define orrox3ld(XD, XS, MT, DT)                                            \
        orrix3ld(W(XD), W(XS), W(MT), W(DT))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define ornox_rr(XG, XS)                                                    \
        ornix_rr(W(XG), W(XS))

#define ornox_ld(XG, MS, DS)                                                \
        ornix_ld(W(XG), W(MS), W(DS))

#define ornox3rr(XD, XS, XT)                                                \
        ornix3rr(W(XD), W(XS), W(XT))

#define ornox3ld(XD, XS, MT, DT)                                            \
        ornix3ld(W(XD), W(XS), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorox_rr(XG, XS)                                                    \
        xorix_rr(W(XG), W(XS))

#define xorox_ld(XG, MS, DS)                                                \
        xorix_ld(W(XG), W(MS), W(DS))

#define xorox3rr(XD, XS, XT)                                                \
        xorix3rr(W(XD), W(XS), W(XT))

#define xorox3ld(XD, XS, MT, DT)                                            \
        xorix3ld(W(XD), W(XS), W(MT), W(DT))

/* not (G = ~G), (D = ~S) */

#define notox_rx(XG)                                                        \
        notix_rx(W(XG))

#define notox_rr(XD, XS)                                                    \
        notix_rr(W(XD), W(XS))

/************   packed single-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negos_rx(XG)                                                        \
        negis_rx(W(XG))

#define negos_rr(XD, XS)                                                    \
        negis_rr(W(XD), W(XS))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addos_rr(XG, XS)                                                    \
        addis_rr(W(XG), W(XS))

#define addos_ld(XG, MS, DS)                                                \
        addis_ld(W(XG), W(MS), W(DS))

#define addos3rr(XD, XS, XT)                                                \
        addis3rr(W(XD), W(XS), W(XT))

#define addos3ld(XD, XS, MT, DT)                                            \
        addis3ld(W(XD), W(XS), W(MT), W(DT))

#define adpos_rr(XG, XS) /* horizontal pairwise add, first 15-regs only */  \
        adpis_rr(W(XG), W(XS))

#define adpos_ld(XG, MS, DS)                                                \
        adpis_ld(W(XG), W(MS), W(DS))

#define adpos3rr(XD, XS, XT)                                                \
        adpis3rr(W(XD), W(XS), W(XT))

#define adpos3ld(XD, XS, MT, DT)                                            \
        adpis3ld(W(XD), W(XS), W(MT), W(DT))

#define adhos_rr(XG, XS) /* horizontal reductive add, first 15-regs only */ \
        adhis_rr(W(XG), W(XS))

#define adhos_ld(XG, MS, DS)                                                \
        adhis_ld(W(XG), W(MS), W(DS))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subos_rr(XG, XS)                                                    \
        subis_rr(W(XG), W(XS))

#define subos_ld(XG, MS, DS)                                                \
        subis_ld(W(XG), W(MS), W(DS))

#define subos3rr(XD, XS, XT)                                                \
        subis3rr(W(XD), W(XS), W(XT))

#define subos3ld(XD, XS, MT, DT)                                            \
        subis3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulos_rr(XG, XS)                                                    \
        mulis_rr(W(XG), W(XS))

#define mulos_ld(XG, MS, DS)                                                \
        mulis_ld(W(XG), W(MS), W(DS))

#define mulos3rr(XD, XS, XT)                                                \
        mulis3rr(W(XD), W(XS), W(XT))

#define mulos3ld(XD, XS, MT, DT)                                            \
        mulis3ld(W(XD), W(XS), W(MT), W(DT))

#define mlpos_rr(XG, XS) /* horizontal pairwise mul */                      \
        mlpis_rr(W(XG), W(XS))

#define mlpos_ld(XG, MS, DS)                                                \
        mlpis_ld(W(XG), W(MS), W(DS))

#define mlpos3rr(XD, XS, XT)                                                \
        mlpis3rr(W(XD), W(XS), W(XT))

#define mlpos3ld(XD, XS, MT, DT)                                            \
        mlpis3ld(W(XD), W(XS), W(MT), W(DT))

#define mlhos_rr(XD, XS) /* horizontal reductive mul */                     \
        mlhis_rr(W(XD), W(XS))

#define mlhos_ld(XD, MS, DS)                                                \
        mlhis_ld(W(XD), W(MS), W(DS))

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divos_rr(XG, XS)                                                    \
        divis_rr(W(XG), W(XS))

#define divos_ld(XG, MS, DS)                                                \
        divis_ld(W(XG), W(MS), W(DS))

#define divos3rr(XD, XS, XT)                                                \
        divis3rr(W(XD), W(XS), W(XT))

#define divos3ld(XD, XS, MT, DT)                                            \
        divis3ld(W(XD), W(XS), W(MT), W(DT))

/* sqr (D = sqrt S) */

#define sqros_rr(XD, XS)                                                    \
        sqris_rr(W(XD), W(XS))

#define sqros_ld(XD, MS, DS)                                                \
        sqris_ld(W(XD), W(MS), W(DS))

/* cbr (D = cbrt S) */

#define cbros_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbris_rr(W(XD), W(X1), W(X2), W(XS))

#define cbeos_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbeis_rr(W(XD), W(X1), W(X2), W(XS))

#define cbsos_rr(XG, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbsis_rr(W(XG), W(X1), W(X2), W(XS))

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rcpos_rr(XD, XS) /* destroys XS */                                  \
        rcpis_rr(W(XD), W(XS))

#define rceos_rr(XD, XS)                                                    \
        rceis_rr(W(XD), W(XS))

#define rcsos_rr(XG, XS) /* destroys XS */                                  \
        rcsis_rr(W(XG), W(XS))

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rsqos_rr(XD, XS) /* destroys XS */                                  \
        rsqis_rr(W(XD), W(XS))

#define rseos_rr(XD, XS)                                                    \
        rseis_rr(W(XD), W(XS))

#define rssos_rr(XG, XS) /* destroys XS */                                  \
        rssis_rr(W(XG), W(XS))

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmaos_rr(XG, XS, XT)                                                \
        fmais_rr(W(XG), W(XS), W(XT))

#define fmaos_ld(XG, XS, MT, DT)                                            \
        fmais_ld(W(XG), W(XS), W(MT), W(DT))

#define fmaos3rr(XG, XS, XT)                                                \
        fmaos_rr(W(XG), W(XS), W(XT))

#define fmaos3ld(XG, XS, MT, DT)                                            \
        fmaos_ld(W(XG), W(XS), W(MT), W(DT))

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsos_rr(XG, XS, XT)                                                \
        fmsis_rr(W(XG), W(XS), W(XT))

#define fmsos_ld(XG, XS, MT, DT)                                            \
        fmsis_ld(W(XG), W(XS), W(MT), W(DT))

#define fmsos3rr(XG, XS, XT)                                                \
        fmsos_rr(W(XG), W(XS), W(XT))

#define fmsos3ld(XG, XS, MT, DT)                                            \
        fmsos_ld(W(XG), W(XS), W(MT), W(DT))

/*************   packed single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minos_rr(XG, XS)                                                    \
        minis_rr(W(XG), W(XS))

#define minos_ld(XG, MS, DS)                                                \
        minis_ld(W(XG), W(MS), W(DS))

#define minos3rr(XD, XS, XT)                                                \
        minis3rr(W(XD), W(XS), W(XT))

#define minos3ld(XD, XS, MT, DT)                                            \
        minis3ld(W(XD), W(XS), W(MT), W(DT))

#define mnpos_rr(XG, XS) /* horizontal pairwise min */                      \
        mnpis_rr(W(XG), W(XS))

#define mnpos_ld(XG, MS, DS)                                                \
        mnpis_ld(W(XG), W(MS), W(DS))

#define mnpos3rr(XD, XS, XT)                                                \
        mnpis3rr(W(XD), W(XS), W(XT))

#define mnpos3ld(XD, XS, MT, DT)                                            \
        mnpis3ld(W(XD), W(XS), W(MT), W(DT))

#define mnhos_rr(XD, XS) /* horizontal reductive min */                     \
        mnhis_rr(W(XD), W(XS))

#define mnhos_ld(XD, MS, DS)                                                \
        mnhis_ld(W(XD), W(MS), W(DS))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxos_rr(XG, XS)                                                    \
        maxis_rr(W(XG), W(XS))

#define maxos_ld(XG, MS, DS)                                                \
        maxis_ld(W(XG), W(MS), W(DS))

#define maxos3rr(XD, XS, XT)                                                \
        maxis3rr(W(XD), W(XS), W(XT))

#define maxos3ld(XD, XS, MT, DT)                                            \
        maxis3ld(W(XD), W(XS), W(MT), W(DT))

#define mxpos_rr(XG, XS) /* horizontal pairwise max */                      \
        mxpis_rr(W(XG), W(XS))

#define mxpos_ld(XG, MS, DS)                                                \
        mxpis_ld(W(XG), W(MS), W(DS))

#define mxpos3rr(XD, XS, XT)                                                \
        mxpis3rr(W(XD), W(XS), W(XT))

#define mxpos3ld(XD, XS, MT, DT)                                            \
        mxpis3ld(W(XD), W(XS), W(MT), W(DT))

#define mxhos_rr(XD, XS) /* horizontal reductive max */                     \
        mxhis_rr(W(XD), W(XS))

#define mxhos_ld(XD, MS, DS)                                                \
        mxhis_ld(W(XD), W(MS), W(DS))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqos_rr(XG, XS)                                                    \
        ceqis_rr(W(XG), W(XS))

#define ceqos_ld(XG, MS, DS)                                                \
        ceqis_ld(W(XG), W(MS), W(DS))

#define ceqos3rr(XD, XS, XT)                                                \
        ceqis3rr(W(XD), W(XS), W(XT))

#define ceqos3ld(XD, XS, MT, DT)                                            \
        ceqis3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneos_rr(XG, XS)                                                    \
        cneis_rr(W(XG), W(XS))

#define cneos_ld(XG, MS, DS)                                                \
        cneis_ld(W(XG), W(MS), W(DS))

#define cneos3rr(XD, XS, XT)                                                \
        cneis3rr(W(XD), W(XS), W(XT))

#define cneos3ld(XD, XS, MT, DT)                                            \
        cneis3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltos_rr(XG, XS)                                                    \
        cltis_rr(W(XG), W(XS))

#define cltos_ld(XG, MS, DS)                                                \
        cltis_ld(W(XG), W(MS), W(DS))

#define cltos3rr(XD, XS, XT)                                                \
        cltis3rr(W(XD), W(XS), W(XT))

#define cltos3ld(XD, XS, MT, DT)                                            \
        cltis3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define cleos_rr(XG, XS)                                                    \
        cleis_rr(W(XG), W(XS))

#define cleos_ld(XG, MS, DS)                                                \
        cleis_ld(W(XG), W(MS), W(DS))

#define cleos3rr(XD, XS, XT)                                                \
        cleis3rr(W(XD), W(XS), W(XT))

#define cleos3ld(XD, XS, MT, DT)                                            \
        cleis3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtos_rr(XG, XS)                                                    \
        cgtis_rr(W(XG), W(XS))

#define cgtos_ld(XG, MS, DS)                                                \
        cgtis_ld(W(XG), W(MS), W(DS))

#define cgtos3rr(XD, XS, XT)                                                \
        cgtis3rr(W(XD), W(XS), W(XT))

#define cgtos3ld(XD, XS, MT, DT)                                            \
        cgtis3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgeos_rr(XG, XS)                                                    \
        cgeis_rr(W(XG), W(XS))

#define cgeos_ld(XG, MS, DS)                                                \
        cgeis_ld(W(XG), W(MS), W(DS))

#define cgeos3rr(XD, XS, XT)                                                \
        cgeis3rr(W(XD), W(XS), W(XT))

#define cgeos3ld(XD, XS, MT, DT)                                            \
        cgeis3ld(W(XD), W(XS), W(MT), W(DT))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define mkjox_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        mkjix_rx(W(XS), mask, lb)

/*************   packed single-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzos_rr(XD, XS)     /* round towards zero */                       \
        rnzis_rr(W(XD), W(XS))

#define rnzos_ld(XD, MS, DS) /* round towards zero */                       \
        rnzis_ld(W(XD), W(MS), W(DS))

#define cvzos_rr(XD, XS)     /* round towards zero */                       \
        cvzis_rr(W(XD), W(XS))

#define cvzos_ld(XD, MS, DS) /* round towards zero */                       \
        cvzis_ld(W(XD), W(MS), W(DS))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpos_rr(XD, XS)     /* round towards +inf */                       \
        rnpis_rr(W(XD), W(XS))

#define rnpos_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpis_ld(W(XD), W(MS), W(DS))

#define cvpos_rr(XD, XS)     /* round towards +inf */                       \
        cvpis_rr(W(XD), W(XS))

#define cvpos_ld(XD, MS, DS) /* round towards +inf */                       \
        cvpis_ld(W(XD), W(MS), W(DS))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmos_rr(XD, XS)     /* round towards -inf */                       \
        rnmis_rr(W(XD), W(XS))

#define rnmos_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmis_ld(W(XD), W(MS), W(DS))

#define cvmos_rr(XD, XS)     /* round towards -inf */                       \
        cvmis_rr(W(XD), W(XS))

#define cvmos_ld(XD, MS, DS) /* round towards -inf */                       \
        cvmis_ld(W(XD), W(MS), W(DS))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnos_rr(XD, XS)     /* round towards near */                       \
        rnnis_rr(W(XD), W(XS))

#define rnnos_ld(XD, MS, DS) /* round towards near */                       \
        rnnis_ld(W(XD), W(MS), W(DS))

#define cvnos_rr(XD, XS)     /* round towards near */                       \
        cvnis_rr(W(XD), W(XS))

#define cvnos_ld(XD, MS, DS) /* round towards near */                       \
        cvnis_ld(W(XD), W(MS), W(DS))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnon_rr(XD, XS)     /* round towards near */                       \
        cvnin_rr(W(XD), W(XS))

#define cvnon_ld(XD, MS, DS) /* round towards near */                       \
        cvnin_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndos_rr(XD, XS)                                                    \
        rndis_rr(W(XD), W(XS))

#define rndos_ld(XD, MS, DS)                                                \
        rndis_ld(W(XD), W(MS), W(DS))

#define cvtos_rr(XD, XS)                                                    \
        cvtis_rr(W(XD), W(XS))

#define cvtos_ld(XD, MS, DS)                                                \
        cvtis_ld(W(XD), W(MS), W(DS))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvton_rr(XD, XS)                                                    \
        cvtin_rr(W(XD), W(XS))

#define cvton_ld(XD, MS, DS)                                                \
        cvtin_ld(W(XD), W(MS), W(DS))

/* cvn (D = unsigned-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnox_rr(XD, XS)     /* round towards near */                       \
        cvnix_rr(W(XD), W(XS))

#define cvnox_ld(XD, MS, DS) /* round towards near */                       \
        cvnix_ld(W(XD), W(MS), W(DS))

/* cvt (D = unsigned-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtox_rr(XD, XS)                                                    \
        cvtix_rr(W(XD), W(XS))

#define cvtox_ld(XD, MS, DS)                                                \
        cvtix_ld(W(XD), W(MS), W(DS))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnros_rr(XD, XS, mode)                                              \
        rnris_rr(W(XD), W(XS), mode)

#define cvros_rr(XD, XS, mode)                                              \
        cvris_rr(W(XD), W(XS), mode)

/************   packed single-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addox_rr(XG, XS)                                                    \
        addix_rr(W(XG), W(XS))

#define addox_ld(XG, MS, DS)                                                \
        addix_ld(W(XG), W(MS), W(DS))

#define addox3rr(XD, XS, XT)                                                \
        addix3rr(W(XD), W(XS), W(XT))

#define addox3ld(XD, XS, MT, DT)                                            \
        addix3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subox_rr(XG, XS)                                                    \
        subix_rr(W(XG), W(XS))

#define subox_ld(XG, MS, DS)                                                \
        subix_ld(W(XG), W(MS), W(DS))

#define subox3rr(XD, XS, XT)                                                \
        subix3rr(W(XD), W(XS), W(XT))

#define subox3ld(XD, XS, MT, DT)                                            \
        subix3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulox_rr(XG, XS)                                                    \
        mulix_rr(W(XG), W(XS))

#define mulox_ld(XG, MS, DS)                                                \
        mulix_ld(W(XG), W(MS), W(DS))

#define mulox3rr(XD, XS, XT)                                                \
        mulix3rr(W(XD), W(XS), W(XT))

#define mulox3ld(XD, XS, MT, DT)                                            \
        mulix3ld(W(XD), W(XS), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlox_ri(XG, IS)                                                    \
        shlix_ri(W(XG), W(IS))

#define shlox_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlix_ld(W(XG), W(MS), W(DS))

#define shlox3ri(XD, XS, IT)                                                \
        shlix3ri(W(XD), W(XS), W(IT))

#define shlox3ld(XD, XS, MT, DT)                                            \
        shlix3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrox_ri(XG, IS)                                                    \
        shrix_ri(W(XG), W(IS))

#define shrox_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrix_ld(W(XG), W(MS), W(DS))

#define shrox3ri(XD, XS, IT)                                                \
        shrix3ri(W(XD), W(XS), W(IT))

#define shrox3ld(XD, XS, MT, DT)                                            \
        shrix3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shron_ri(XG, IS)                                                    \
        shrin_ri(W(XG), W(IS))

#define shron_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrin_ld(W(XG), W(MS), W(DS))

#define shron3ri(XD, XS, IT)                                                \
        shrin3ri(W(XD), W(XS), W(IT))

#define shron3ld(XD, XS, MT, DT)                                            \
        shrin3ld(W(XD), W(XS), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlox_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlix_rr(W(XG), W(XS))

#define svlox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlix_ld(W(XG), W(MS), W(DS))

#define svlox3rr(XD, XS, XT)                                                \
        svlix3rr(W(XD), W(XS), W(XT))

#define svlox3ld(XD, XS, MT, DT)                                            \
        svlix3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrox_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrix_rr(W(XG), W(XS))

#define svrox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrix_ld(W(XG), W(MS), W(DS))

#define svrox3rr(XD, XS, XT)                                                \
        svrix3rr(W(XD), W(XS), W(XT))

#define svrox3ld(XD, XS, MT, DT)                                            \
        svrix3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svron_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrin_rr(W(XG), W(XS))

#define svron_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrin_ld(W(XG), W(MS), W(DS))

#define svron3rr(XD, XS, XT)                                                \
        svrin3rr(W(XD), W(XS), W(XT))

#define svron3ld(XD, XS, MT, DT)                                            \
        svrin3ld(W(XD), W(XS), W(MT), W(DT))

/****************   packed single-precision integer compare   *****************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minox_rr(XG, XS)                                                    \
        minix_rr(W(XG), W(XS))

#define minox_ld(XG, MS, DS)                                                \
        minix_ld(W(XG), W(MS), W(DS))

#define minox3rr(XD, XS, XT)                                                \
        minix3rr(W(XD), W(XS), W(XT))

#define minox3ld(XD, XS, MT, DT)                                            \
        minix3ld(W(XD), W(XS), W(MT), W(DT))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minon_rr(XG, XS)                                                    \
        minin_rr(W(XG), W(XS))

#define minon_ld(XG, MS, DS)                                                \
        minin_ld(W(XG), W(MS), W(DS))

#define minon3rr(XD, XS, XT)                                                \
        minin3rr(W(XD), W(XS), W(XT))

#define minon3ld(XD, XS, MT, DT)                                            \
        minin3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxox_rr(XG, XS)                                                    \
        maxix_rr(W(XG), W(XS))

#define maxox_ld(XG, MS, DS)                                                \
        maxix_ld(W(XG), W(MS), W(DS))

#define maxox3rr(XD, XS, XT)                                                \
        maxix3rr(W(XD), W(XS), W(XT))

#define maxox3ld(XD, XS, MT, DT)                                            \
        maxix3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxon_rr(XG, XS)                                                    \
        maxin_rr(W(XG), W(XS))

#define maxon_ld(XG, MS, DS)                                                \
        maxin_ld(W(XG), W(MS), W(DS))

#define maxon3rr(XD, XS, XT)                                                \
        maxin3rr(W(XD), W(XS), W(XT))

#define maxon3ld(XD, XS, MT, DT)                                            \
        maxin3ld(W(XD), W(XS), W(MT), W(DT))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqox_rr(XG, XS)                                                    \
        ceqix_rr(W(XG), W(XS))

#define ceqox_ld(XG, MS, DS)                                                \
        ceqix_ld(W(XG), W(MS), W(DS))

#define ceqox3rr(XD, XS, XT)                                                \
        ceqix3rr(W(XD), W(XS), W(XT))

#define ceqox3ld(XD, XS, MT, DT)                                            \
        ceqix3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneox_rr(XG, XS)                                                    \
        cneix_rr(W(XG), W(XS))

#define cneox_ld(XG, MS, DS)                                                \
        cneix_ld(W(XG), W(MS), W(DS))

#define cneox3rr(XD, XS, XT)                                                \
        cneix3rr(W(XD), W(XS), W(XT))

#define cneox3ld(XD, XS, MT, DT)                                            \
        cneix3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltox_rr(XG, XS)                                                    \
        cltix_rr(W(XG), W(XS))

#define cltox_ld(XG, MS, DS)                                                \
        cltix_ld(W(XG), W(MS), W(DS))

#define cltox3rr(XD, XS, XT)                                                \
        cltix3rr(W(XD), W(XS), W(XT))

#define cltox3ld(XD, XS, MT, DT)                                            \
        cltix3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define clton_rr(XG, XS)                                                    \
        cltin_rr(W(XG), W(XS))

#define clton_ld(XG, MS, DS)                                                \
        cltin_ld(W(XG), W(MS), W(DS))

#define clton3rr(XD, XS, XT)                                                \
        cltin3rr(W(XD), W(XS), W(XT))

#define clton3ld(XD, XS, MT, DT)                                            \
        cltin3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define cleox_rr(XG, XS)                                                    \
        cleix_rr(W(XG), W(XS))

#define cleox_ld(XG, MS, DS)                                                \
        cleix_ld(W(XG), W(MS), W(DS))

#define cleox3rr(XD, XS, XT)                                                \
        cleix3rr(W(XD), W(XS), W(XT))

#define cleox3ld(XD, XS, MT, DT)                                            \
        cleix3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define cleon_rr(XG, XS)                                                    \
        clein_rr(W(XG), W(XS))

#define cleon_ld(XG, MS, DS)                                                \
        clein_ld(W(XG), W(MS), W(DS))

#define cleon3rr(XD, XS, XT)                                                \
        clein3rr(W(XD), W(XS), W(XT))

#define cleon3ld(XD, XS, MT, DT)                                            \
        clein3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtox_rr(XG, XS)                                                    \
        cgtix_rr(W(XG), W(XS))

#define cgtox_ld(XG, MS, DS)                                                \
        cgtix_ld(W(XG), W(MS), W(DS))

#define cgtox3rr(XD, XS, XT)                                                \
        cgtix3rr(W(XD), W(XS), W(XT))

#define cgtox3ld(XD, XS, MT, DT)                                            \
        cgtix3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgton_rr(XG, XS)                                                    \
        cgtin_rr(W(XG), W(XS))

#define cgton_ld(XG, MS, DS)                                                \
        cgtin_ld(W(XG), W(MS), W(DS))

#define cgton3rr(XD, XS, XT)                                                \
        cgtin3rr(W(XD), W(XS), W(XT))

#define cgton3ld(XD, XS, MT, DT)                                            \
        cgtin3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgeox_rr(XG, XS)                                                    \
        cgeix_rr(W(XG), W(XS))

#define cgeox_ld(XG, MS, DS)                                                \
        cgeix_ld(W(XG), W(MS), W(DS))

#define cgeox3rr(XD, XS, XT)                                                \
        cgeix3rr(W(XD), W(XS), W(XT))

#define cgeox3ld(XD, XS, MT, DT)                                            \
        cgeix3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgeon_rr(XG, XS)                                                    \
        cgein_rr(W(XG), W(XS))

#define cgeon_ld(XG, MS, DS)                                                \
        cgein_ld(W(XG), W(MS), W(DS))

#define cgeon3rr(XD, XS, XT)                                                \
        cgein3rr(W(XD), W(XS), W(XT))

#define cgeon3ld(XD, XS, MT, DT)                                            \
        cgein3ld(W(XD), W(XS), W(MT), W(DT))

#endif /* RT_SIMD: 256, 128 */

/******************************************************************************/
/**** var-len **** SIMD instructions with fixed-64-bit element **** 256-bit ***/
/******************************************************************************/

#if   (RT_SIMD == 256) && !(defined RT_SVEX1)

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmqx_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        elmdx_st(W(XS), W(MD), W(DD))

/***************   packed double-precision generic move/logic   ***************/

/* mov (D = S) */

#define movqx_rr(XD, XS)                                                    \
        movdx_rr(W(XD), W(XS))

#define movqx_ld(XD, MS, DS)                                                \
        movdx_ld(W(XD), W(MS), W(DS))

#define movqx_st(XS, MD, DD)                                                \
        movdx_st(W(XS), W(MD), W(DD))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvqx_rr(XD, XS)                                                    \
        mmvdx_rr(W(XD), W(XS))

#define mmvqx_ld(XG, MS, DS)                                                \
        mmvdx_ld(W(XG), W(MS), W(DS))

#define mmvqx_st(XS, MG, DG)                                                \
        mmvdx_st(W(XS), W(MG), W(DG))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andqx_rr(XG, XS)                                                    \
        anddx_rr(W(XG), W(XS))

#define andqx_ld(XG, MS, DS)                                                \
        anddx_ld(W(XG), W(MS), W(DS))

#define andqx3rr(XD, XS, XT)                                                \
        anddx3rr(W(XD), W(XS), W(XT))

#define andqx3ld(XD, XS, MT, DT)                                            \
        anddx3ld(W(XD), W(XS), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annqx_rr(XG, XS)                                                    \
        anndx_rr(W(XG), W(XS))

#define annqx_ld(XG, MS, DS)                                                \
        anndx_ld(W(XG), W(MS), W(DS))

#define annqx3rr(XD, XS, XT)                                                \
        anndx3rr(W(XD), W(XS), W(XT))

#define annqx3ld(XD, XS, MT, DT)                                            \
        anndx3ld(W(XD), W(XS), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrqx_rr(XG, XS)                                                    \
        orrdx_rr(W(XG), W(XS))

#define orrqx_ld(XG, MS, DS)                                                \
        orrdx_ld(W(XG), W(MS), W(DS))

#define orrqx3rr(XD, XS, XT)                                                \
        orrdx3rr(W(XD), W(XS), W(XT))

#define orrqx3ld(XD, XS, MT, DT)                                            \
        orrdx3ld(W(XD), W(XS), W(MT), W(DT))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define ornqx_rr(XG, XS)                                                    \
        orndx_rr(W(XG), W(XS))

#define ornqx_ld(XG, MS, DS)                                                \
        orndx_ld(W(XG), W(MS), W(DS))

#define ornqx3rr(XD, XS, XT)                                                \
        orndx3rr(W(XD), W(XS), W(XT))

#define ornqx3ld(XD, XS, MT, DT)                                            \
        orndx3ld(W(XD), W(XS), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorqx_rr(XG, XS)                                                    \
        xordx_rr(W(XG), W(XS))

#define xorqx_ld(XG, MS, DS)                                                \
        xordx_ld(W(XG), W(MS), W(DS))

#define xorqx3rr(XD, XS, XT)                                                \
        xordx3rr(W(XD), W(XS), W(XT))

#define xorqx3ld(XD, XS, MT, DT)                                            \
        xordx3ld(W(XD), W(XS), W(MT), W(DT))

/* not (G = ~G), (D = ~S) */

#define notqx_rx(XG)                                                        \
        notdx_rx(W(XG))

#define notqx_rr(XD, XS)                                                    \
        notdx_rr(W(XD), W(XS))

/************   packed double-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negqs_rx(XG)                                                        \
        negds_rx(W(XG))

#define negqs_rr(XD, XS)                                                    \
        negds_rr(W(XD), W(XS))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addqs_rr(XG, XS)                                                    \
        addds_rr(W(XG), W(XS))

#define addqs_ld(XG, MS, DS)                                                \
        addds_ld(W(XG), W(MS), W(DS))

#define addqs3rr(XD, XS, XT)                                                \
        addds3rr(W(XD), W(XS), W(XT))

#define addqs3ld(XD, XS, MT, DT)                                            \
        addds3ld(W(XD), W(XS), W(MT), W(DT))

#define adpqs_rr(XG, XS) /* horizontal pairwise add, first 15-regs only */  \
        adpds_rr(W(XG), W(XS))

#define adpqs_ld(XG, MS, DS)                                                \
        adpds_ld(W(XG), W(MS), W(DS))

#define adpqs3rr(XD, XS, XT)                                                \
        adpds3rr(W(XD), W(XS), W(XT))

#define adpqs3ld(XD, XS, MT, DT)                                            \
        adpds3ld(W(XD), W(XS), W(MT), W(DT))

#define adhqs_rr(XG, XS) /* horizontal reductive add, first 15-regs only */ \
        adhds_rr(W(XG), W(XS))

#define adhqs_ld(XG, MS, DS)                                                \
        adhds_ld(W(XG), W(MS), W(DS))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subqs_rr(XG, XS)                                                    \
        subds_rr(W(XG), W(XS))

#define subqs_ld(XG, MS, DS)                                                \
        subds_ld(W(XG), W(MS), W(DS))

#define subqs3rr(XD, XS, XT)                                                \
        subds3rr(W(XD), W(XS), W(XT))

#define subqs3ld(XD, XS, MT, DT)                                            \
        subds3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulqs_rr(XG, XS)                                                    \
        mulds_rr(W(XG), W(XS))

#define mulqs_ld(XG, MS, DS)                                                \
        mulds_ld(W(XG), W(MS), W(DS))

#define mulqs3rr(XD, XS, XT)                                                \
        mulds3rr(W(XD), W(XS), W(XT))

#define mulqs3ld(XD, XS, MT, DT)                                            \
        mulds3ld(W(XD), W(XS), W(MT), W(DT))

#define mlpqs_rr(XG, XS) /* horizontal pairwise mul */                      \
        mlpds_rr(W(XG), W(XS))

#define mlpqs_ld(XG, MS, DS)                                                \
        mlpds_ld(W(XG), W(MS), W(DS))

#define mlpqs3rr(XD, XS, XT)                                                \
        mlpds3rr(W(XD), W(XS), W(XT))

#define mlpqs3ld(XD, XS, MT, DT)                                            \
        mlpds3ld(W(XD), W(XS), W(MT), W(DT))

#define mlhqs_rr(XD, XS) /* horizontal reductive mul */                     \
        mlhds_rr(W(XD), W(XS))

#define mlhqs_ld(XD, MS, DS)                                                \
        mlhds_ld(W(XD), W(MS), W(DS))

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divqs_rr(XG, XS)                                                    \
        divds_rr(W(XG), W(XS))

#define divqs_ld(XG, MS, DS)                                                \
        divds_ld(W(XG), W(MS), W(DS))

#define divqs3rr(XD, XS, XT)                                                \
        divds3rr(W(XD), W(XS), W(XT))

#define divqs3ld(XD, XS, MT, DT)                                            \
        divds3ld(W(XD), W(XS), W(MT), W(DT))

/* sqr (D = sqrt S) */

#define sqrqs_rr(XD, XS)                                                    \
        sqrds_rr(W(XD), W(XS))

#define sqrqs_ld(XD, MS, DS)                                                \
        sqrds_ld(W(XD), W(MS), W(DS))

/* cbr (D = cbrt S) */

#define cbrqs_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbrds_rr(W(XD), W(X1), W(X2), W(XS))

#define cbeqs_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbeds_rr(W(XD), W(X1), W(X2), W(XS))

#define cbsqs_rr(XG, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbsds_rr(W(XG), W(X1), W(X2), W(XS))

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rcpqs_rr(XD, XS) /* destroys XS */                                  \
        rcpds_rr(W(XD), W(XS))

#define rceqs_rr(XD, XS)                                                    \
        rceds_rr(W(XD), W(XS))

#define rcsqs_rr(XG, XS) /* destroys XS */                                  \
        rcsds_rr(W(XG), W(XS))

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rsqqs_rr(XD, XS) /* destroys XS */                                  \
        rsqds_rr(W(XD), W(XS))

#define rseqs_rr(XD, XS)                                                    \
        rseds_rr(W(XD), W(XS))

#define rssqs_rr(XG, XS) /* destroys XS */                                  \
        rssds_rr(W(XG), W(XS))

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmaqs_rr(XG, XS, XT)                                                \
        fmads_rr(W(XG), W(XS), W(XT))

#define fmaqs_ld(XG, XS, MT, DT)                                            \
        fmads_ld(W(XG), W(XS), W(MT), W(DT))

#define fmaqs3rr(XG, XS, XT)                                                \
        fmaqs_rr(W(XG), W(XS), W(XT))

#define fmaqs3ld(XG, XS, MT, DT)                                            \
        fmaqs_ld(W(XG), W(XS), W(MT), W(DT))

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsqs_rr(XG, XS, XT)                                                \
        fmsds_rr(W(XG), W(XS), W(XT))

#define fmsqs_ld(XG, XS, MT, DT)                                            \
        fmsds_ld(W(XG), W(XS), W(MT), W(DT))

#define fmsqs3rr(XG, XS, XT)                                                \
        fmsqs_rr(W(XG), W(XS), W(XT))

#define fmsqs3ld(XG, XS, MT, DT)                                            \
        fmsqs_ld(W(XG), W(XS), W(MT), W(DT))

/*************   packed double-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minqs_rr(XG, XS)                                                    \
        minds_rr(W(XG), W(XS))

#define minqs_ld(XG, MS, DS)                                                \
        minds_ld(W(XG), W(MS), W(DS))

#define minqs3rr(XD, XS, XT)                                                \
        minds3rr(W(XD), W(XS), W(XT))

#define minqs3ld(XD, XS, MT, DT)                                            \
        minds3ld(W(XD), W(XS), W(MT), W(DT))

#define mnpqs_rr(XG, XS) /* horizontal pairwise min */                      \
        mnpds_rr(W(XG), W(XS))

#define mnpqs_ld(XG, MS, DS)                                                \
        mnpds_ld(W(XG), W(MS), W(DS))

#define mnpqs3rr(XD, XS, XT)                                                \
        mnpds3rr(W(XD), W(XS), W(XT))

#define mnpqs3ld(XD, XS, MT, DT)                                            \
        mnpds3ld(W(XD), W(XS), W(MT), W(DT))

#define mnhqs_rr(XD, XS) /* horizontal reductive min */                     \
        mnhds_rr(W(XD), W(XS))

#define mnhqs_ld(XD, MS, DS)                                                \
        mnhds_ld(W(XD), W(MS), W(DS))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxqs_rr(XG, XS)                                                    \
        maxds_rr(W(XG), W(XS))

#define maxqs_ld(XG, MS, DS)                                                \
        maxds_ld(W(XG), W(MS), W(DS))

#define maxqs3rr(XD, XS, XT)                                                \
        maxds3rr(W(XD), W(XS), W(XT))

#define maxqs3ld(XD, XS, MT, DT)                                            \
        maxds3ld(W(XD), W(XS), W(MT), W(DT))

#define mxpqs_rr(XG, XS) /* horizontal pairwise max */                      \
        mxpds_rr(W(XG), W(XS))

#define mxpqs_ld(XG, MS, DS)                                                \
        mxpds_ld(W(XG), W(MS), W(DS))

#define mxpqs3rr(XD, XS, XT)                                                \
        mxpds3rr(W(XD), W(XS), W(XT))

#define mxpqs3ld(XD, XS, MT, DT)                                            \
        mxpds3ld(W(XD), W(XS), W(MT), W(DT))

#define mxhqs_rr(XD, XS) /* horizontal reductive max */                     \
        mxhds_rr(W(XD), W(XS))

#define mxhqs_ld(XD, MS, DS)                                                \
        mxhds_ld(W(XD), W(MS), W(DS))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqqs_rr(XG, XS)                                                    \
        ceqds_rr(W(XG), W(XS))

#define ceqqs_ld(XG, MS, DS)                                                \
        ceqds_ld(W(XG), W(MS), W(DS))

#define ceqqs3rr(XD, XS, XT)                                                \
        ceqds3rr(W(XD), W(XS), W(XT))

#define ceqqs3ld(XD, XS, MT, DT)                                            \
        ceqds3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneqs_rr(XG, XS)                                                    \
        cneds_rr(W(XG), W(XS))

#define cneqs_ld(XG, MS, DS)                                                \
        cneds_ld(W(XG), W(MS), W(DS))

#define cneqs3rr(XD, XS, XT)                                                \
        cneds3rr(W(XD), W(XS), W(XT))

#define cneqs3ld(XD, XS, MT, DT)                                            \
        cneds3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltqs_rr(XG, XS)                                                    \
        cltds_rr(W(XG), W(XS))

#define cltqs_ld(XG, MS, DS)                                                \
        cltds_ld(W(XG), W(MS), W(DS))

#define cltqs3rr(XD, XS, XT)                                                \
        cltds3rr(W(XD), W(XS), W(XT))

#define cltqs3ld(XD, XS, MT, DT)                                            \
        cltds3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define cleqs_rr(XG, XS)                                                    \
        cleds_rr(W(XG), W(XS))

#define cleqs_ld(XG, MS, DS)                                                \
        cleds_ld(W(XG), W(MS), W(DS))

#define cleqs3rr(XD, XS, XT)                                                \
        cleds3rr(W(XD), W(XS), W(XT))

#define cleqs3ld(XD, XS, MT, DT)                                            \
        cleds3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtqs_rr(XG, XS)                                                    \
        cgtds_rr(W(XG), W(XS))

#define cgtqs_ld(XG, MS, DS)                                                \
        cgtds_ld(W(XG), W(MS), W(DS))

#define cgtqs3rr(XD, XS, XT)                                                \
        cgtds3rr(W(XD), W(XS), W(XT))

#define cgtqs3ld(XD, XS, MT, DT)                                            \
        cgtds3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgeqs_rr(XG, XS)                                                    \
        cgeds_rr(W(XG), W(XS))

#define cgeqs_ld(XG, MS, DS)                                                \
        cgeds_ld(W(XG), W(MS), W(DS))

#define cgeqs3rr(XD, XS, XT)                                                \
        cgeds3rr(W(XD), W(XS), W(XT))

#define cgeqs3ld(XD, XS, MT, DT)                                            \
        cgeds3ld(W(XD), W(XS), W(MT), W(DT))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define mkjqx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        mkjdx_rx(W(XS), mask, lb)

/*************   packed double-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnzqs_rr(XD, XS)     /* round towards zero */                       \
        rnzds_rr(W(XD), W(XS))

#define rnzqs_ld(XD, MS, DS) /* round towards zero */                       \
        rnzds_ld(W(XD), W(MS), W(DS))

#define cvzqs_rr(XD, XS)     /* round towards zero */                       \
        cvzds_rr(W(XD), W(XS))

#define cvzqs_ld(XD, MS, DS) /* round towards zero */                       \
        cvzds_ld(W(XD), W(MS), W(DS))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnpqs_rr(XD, XS)     /* round towards +inf */                       \
        rnpds_rr(W(XD), W(XS))

#define rnpqs_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpds_ld(W(XD), W(MS), W(DS))

#define cvpqs_rr(XD, XS)     /* round towards +inf */                       \
        cvpds_rr(W(XD), W(XS))

#define cvpqs_ld(XD, MS, DS) /* round towards +inf */                       \
        cvpds_ld(W(XD), W(MS), W(DS))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnmqs_rr(XD, XS)     /* round towards -inf */                       \
        rnmds_rr(W(XD), W(XS))

#define rnmqs_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmds_ld(W(XD), W(MS), W(DS))

#define cvmqs_rr(XD, XS)     /* round towards -inf */                       \
        cvmds_rr(W(XD), W(XS))

#define cvmqs_ld(XD, MS, DS) /* round towards -inf */                       \
        cvmds_ld(W(XD), W(MS), W(DS))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnnqs_rr(XD, XS)     /* round towards near */                       \
        rnnds_rr(W(XD), W(XS))

#define rnnqs_ld(XD, MS, DS) /* round towards near */                       \
        rnnds_ld(W(XD), W(MS), W(DS))

#define cvnqs_rr(XD, XS)     /* round towards near */                       \
        cvnds_rr(W(XD), W(XS))

#define cvnqs_ld(XD, MS, DS) /* round towards near */                       \
        cvnds_ld(W(XD), W(MS), W(DS))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnqn_rr(XD, XS)     /* round towards near */                       \
        cvndn_rr(W(XD), W(XS))

#define cvnqn_ld(XD, MS, DS) /* round towards near */                       \
        cvndn_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndqs_rr(XD, XS)                                                    \
        rndds_rr(W(XD), W(XS))

#define rndqs_ld(XD, MS, DS)                                                \
        rndds_ld(W(XD), W(MS), W(DS))

#define cvtqs_rr(XD, XS)                                                    \
        cvtds_rr(W(XD), W(XS))

#define cvtqs_ld(XD, MS, DS)                                                \
        cvtds_ld(W(XD), W(MS), W(DS))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtqn_rr(XD, XS)                                                    \
        cvtdn_rr(W(XD), W(XS))

#define cvtqn_ld(XD, MS, DS)                                                \
        cvtdn_ld(W(XD), W(MS), W(DS))

/* cvn (D = unsigned-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnqx_rr(XD, XS)     /* round towards near */                       \
        cvndx_rr(W(XD), W(XS))

#define cvnqx_ld(XD, MS, DS) /* round towards near */                       \
        cvndx_ld(W(XD), W(MS), W(DS))

/* cvt (D = unsigned-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtqx_rr(XD, XS)                                                    \
        cvtdx_rr(W(XD), W(XS))

#define cvtqx_ld(XD, MS, DS)                                                \
        cvtdx_ld(W(XD), W(MS), W(DS))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnrqs_rr(XD, XS, mode)                                              \
        rnrds_rr(W(XD), W(XS), mode)

#define cvrqs_rr(XD, XS, mode)                                              \
        cvrds_rr(W(XD), W(XS), mode)

/************   packed double-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addqx_rr(XG, XS)                                                    \
        adddx_rr(W(XG), W(XS))

#define addqx_ld(XG, MS, DS)                                                \
        adddx_ld(W(XG), W(MS), W(DS))

#define addqx3rr(XD, XS, XT)                                                \
        adddx3rr(W(XD), W(XS), W(XT))

#define addqx3ld(XD, XS, MT, DT)                                            \
        adddx3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subqx_rr(XG, XS)                                                    \
        subdx_rr(W(XG), W(XS))

#define subqx_ld(XG, MS, DS)                                                \
        subdx_ld(W(XG), W(MS), W(DS))

#define subqx3rr(XD, XS, XT)                                                \
        subdx3rr(W(XD), W(XS), W(XT))

#define subqx3ld(XD, XS, MT, DT)                                            \
        subdx3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulqx_rr(XG, XS)                                                    \
        muldx_rr(W(XG), W(XS))

#define mulqx_ld(XG, MS, DS)                                                \
        muldx_ld(W(XG), W(MS), W(DS))

#define mulqx3rr(XD, XS, XT)                                                \
        muldx3rr(W(XD), W(XS), W(XT))

#define mulqx3ld(XD, XS, MT, DT)                                            \
        muldx3ld(W(XD), W(XS), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlqx_ri(XG, IS)                                                    \
        shldx_ri(W(XG), W(IS))

#define shlqx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shldx_ld(W(XG), W(MS), W(DS))

#define shlqx3ri(XD, XS, IT)                                                \
        shldx3ri(W(XD), W(XS), W(IT))

#define shlqx3ld(XD, XS, MT, DT)                                            \
        shldx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrqx_ri(XG, IS)                                                    \
        shrdx_ri(W(XG), W(IS))

#define shrqx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrdx_ld(W(XG), W(MS), W(DS))

#define shrqx3ri(XD, XS, IT)                                                \
        shrdx3ri(W(XD), W(XS), W(IT))

#define shrqx3ld(XD, XS, MT, DT)                                            \
        shrdx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrqn_ri(XG, IS)                                                    \
        shrdn_ri(W(XG), W(IS))

#define shrqn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrdn_ld(W(XG), W(MS), W(DS))

#define shrqn3ri(XD, XS, IT)                                                \
        shrdn3ri(W(XD), W(XS), W(IT))

#define shrqn3ld(XD, XS, MT, DT)                                            \
        shrdn3ld(W(XD), W(XS), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlqx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svldx_rr(W(XG), W(XS))

#define svlqx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svldx_ld(W(XG), W(MS), W(DS))

#define svlqx3rr(XD, XS, XT)                                                \
        svldx3rr(W(XD), W(XS), W(XT))

#define svlqx3ld(XD, XS, MT, DT)                                            \
        svldx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrqx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrdx_rr(W(XG), W(XS))

#define svrqx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrdx_ld(W(XG), W(MS), W(DS))

#define svrqx3rr(XD, XS, XT)                                                \
        svrdx3rr(W(XD), W(XS), W(XT))

#define svrqx3ld(XD, XS, MT, DT)                                            \
        svrdx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrqn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrdn_rr(W(XG), W(XS))

#define svrqn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrdn_ld(W(XG), W(MS), W(DS))

#define svrqn3rr(XD, XS, XT)                                                \
        svrdn3rr(W(XD), W(XS), W(XT))

#define svrqn3ld(XD, XS, MT, DT)                                            \
        svrdn3ld(W(XD), W(XS), W(MT), W(DT))

/****************   packed double-precision integer compare   *****************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minqx_rr(XG, XS)                                                    \
        mindx_rr(W(XG), W(XS))

#define minqx_ld(XG, MS, DS)                                                \
        mindx_ld(W(XG), W(MS), W(DS))

#define minqx3rr(XD, XS, XT)                                                \
        mindx3rr(W(XD), W(XS), W(XT))

#define minqx3ld(XD, XS, MT, DT)                                            \
        mindx3ld(W(XD), W(XS), W(MT), W(DT))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minqn_rr(XG, XS)                                                    \
        mindn_rr(W(XG), W(XS))

#define minqn_ld(XG, MS, DS)                                                \
        mindn_ld(W(XG), W(MS), W(DS))

#define minqn3rr(XD, XS, XT)                                                \
        mindn3rr(W(XD), W(XS), W(XT))

#define minqn3ld(XD, XS, MT, DT)                                            \
        mindn3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxqx_rr(XG, XS)                                                    \
        maxdx_rr(W(XG), W(XS))

#define maxqx_ld(XG, MS, DS)                                                \
        maxdx_ld(W(XG), W(MS), W(DS))

#define maxqx3rr(XD, XS, XT)                                                \
        maxdx3rr(W(XD), W(XS), W(XT))

#define maxqx3ld(XD, XS, MT, DT)                                            \
        maxdx3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxqn_rr(XG, XS)                                                    \
        maxdn_rr(W(XG), W(XS))

#define maxqn_ld(XG, MS, DS)                                                \
        maxdn_ld(W(XG), W(MS), W(DS))

#define maxqn3rr(XD, XS, XT)                                                \
        maxdn3rr(W(XD), W(XS), W(XT))

#define maxqn3ld(XD, XS, MT, DT)                                            \
        maxdn3ld(W(XD), W(XS), W(MT), W(DT))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqqx_rr(XG, XS)                                                    \
        ceqdx_rr(W(XG), W(XS))

#define ceqqx_ld(XG, MS, DS)                                                \
        ceqdx_ld(W(XG), W(MS), W(DS))

#define ceqqx3rr(XD, XS, XT)                                                \
        ceqdx3rr(W(XD), W(XS), W(XT))

#define ceqqx3ld(XD, XS, MT, DT)                                            \
        ceqdx3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneqx_rr(XG, XS)                                                    \
        cnedx_rr(W(XG), W(XS))

#define cneqx_ld(XG, MS, DS)                                                \
        cnedx_ld(W(XG), W(MS), W(DS))

#define cneqx3rr(XD, XS, XT)                                                \
        cnedx3rr(W(XD), W(XS), W(XT))

#define cneqx3ld(XD, XS, MT, DT)                                            \
        cnedx3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltqx_rr(XG, XS)                                                    \
        cltdx_rr(W(XG), W(XS))

#define cltqx_ld(XG, MS, DS)                                                \
        cltdx_ld(W(XG), W(MS), W(DS))

#define cltqx3rr(XD, XS, XT)                                                \
        cltdx3rr(W(XD), W(XS), W(XT))

#define cltqx3ld(XD, XS, MT, DT)                                            \
        cltdx3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltqn_rr(XG, XS)                                                    \
        cltdn_rr(W(XG), W(XS))

#define cltqn_ld(XG, MS, DS)                                                \
        cltdn_ld(W(XG), W(MS), W(DS))

#define cltqn3rr(XD, XS, XT)                                                \
        cltdn3rr(W(XD), W(XS), W(XT))

#define cltqn3ld(XD, XS, MT, DT)                                            \
        cltdn3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define cleqx_rr(XG, XS)                                                    \
        cledx_rr(W(XG), W(XS))

#define cleqx_ld(XG, MS, DS)                                                \
        cledx_ld(W(XG), W(MS), W(DS))

#define cleqx3rr(XD, XS, XT)                                                \
        cledx3rr(W(XD), W(XS), W(XT))

#define cleqx3ld(XD, XS, MT, DT)                                            \
        cledx3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define cleqn_rr(XG, XS)                                                    \
        cledn_rr(W(XG), W(XS))

#define cleqn_ld(XG, MS, DS)                                                \
        cledn_ld(W(XG), W(MS), W(DS))

#define cleqn3rr(XD, XS, XT)                                                \
        cledn3rr(W(XD), W(XS), W(XT))

#define cleqn3ld(XD, XS, MT, DT)                                            \
        cledn3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtqx_rr(XG, XS)                                                    \
        cgtdx_rr(W(XG), W(XS))

#define cgtqx_ld(XG, MS, DS)                                                \
        cgtdx_ld(W(XG), W(MS), W(DS))

#define cgtqx3rr(XD, XS, XT)                                                \
        cgtdx3rr(W(XD), W(XS), W(XT))

#define cgtqx3ld(XD, XS, MT, DT)                                            \
        cgtdx3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtqn_rr(XG, XS)                                                    \
        cgtdn_rr(W(XG), W(XS))

#define cgtqn_ld(XG, MS, DS)                                                \
        cgtdn_ld(W(XG), W(MS), W(DS))

#define cgtqn3rr(XD, XS, XT)                                                \
        cgtdn3rr(W(XD), W(XS), W(XT))

#define cgtqn3ld(XD, XS, MT, DT)                                            \
        cgtdn3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgeqx_rr(XG, XS)                                                    \
        cgedx_rr(W(XG), W(XS))

#define cgeqx_ld(XG, MS, DS)                                                \
        cgedx_ld(W(XG), W(MS), W(DS))

#define cgeqx3rr(XD, XS, XT)                                                \
        cgedx3rr(W(XD), W(XS), W(XT))

#define cgeqx3ld(XD, XS, MT, DT)                                            \
        cgedx3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgeqn_rr(XG, XS)                                                    \
        cgedn_rr(W(XG), W(XS))

#define cgeqn_ld(XG, MS, DS)                                                \
        cgedn_ld(W(XG), W(MS), W(DS))

#define cgeqn3rr(XD, XS, XT)                                                \
        cgedn3rr(W(XD), W(XS), W(XT))

#define cgeqn3ld(XD, XS, MT, DT)                                            \
        cgedn3ld(W(XD), W(XS), W(MT), W(DT))

/******************************************************************************/
/**** var-len **** SIMD instructions with fixed-64-bit element **** 128-bit ***/
/******************************************************************************/

#elif (RT_SIMD == 128)

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmqx_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        elmjx_st(W(XS), W(MD), W(DD))

/***************   packed double-precision generic move/logic   ***************/

/* mov (D = S) */

#define movqx_rr(XD, XS)                                                    \
        movjx_rr(W(XD), W(XS))

#define movqx_ld(XD, MS, DS)                                                \
        movjx_ld(W(XD), W(MS), W(DS))

#define movqx_st(XS, MD, DD)                                                \
        movjx_st(W(XS), W(MD), W(DD))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvqx_rr(XD, XS)                                                    \
        mmvjx_rr(W(XD), W(XS))

#define mmvqx_ld(XG, MS, DS)                                                \
        mmvjx_ld(W(XG), W(MS), W(DS))

#define mmvqx_st(XS, MG, DG)                                                \
        mmvjx_st(W(XS), W(MG), W(DG))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andqx_rr(XG, XS)                                                    \
        andjx_rr(W(XG), W(XS))

#define andqx_ld(XG, MS, DS)                                                \
        andjx_ld(W(XG), W(MS), W(DS))

#define andqx3rr(XD, XS, XT)                                                \
        andjx3rr(W(XD), W(XS), W(XT))

#define andqx3ld(XD, XS, MT, DT)                                            \
        andjx3ld(W(XD), W(XS), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annqx_rr(XG, XS)                                                    \
        annjx_rr(W(XG), W(XS))

#define annqx_ld(XG, MS, DS)                                                \
        annjx_ld(W(XG), W(MS), W(DS))

#define annqx3rr(XD, XS, XT)                                                \
        annjx3rr(W(XD), W(XS), W(XT))

#define annqx3ld(XD, XS, MT, DT)                                            \
        annjx3ld(W(XD), W(XS), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrqx_rr(XG, XS)                                                    \
        orrjx_rr(W(XG), W(XS))

#define orrqx_ld(XG, MS, DS)                                                \
        orrjx_ld(W(XG), W(MS), W(DS))

#define orrqx3rr(XD, XS, XT)                                                \
        orrjx3rr(W(XD), W(XS), W(XT))

#define orrqx3ld(XD, XS, MT, DT)                                            \
        orrjx3ld(W(XD), W(XS), W(MT), W(DT))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define ornqx_rr(XG, XS)                                                    \
        ornjx_rr(W(XG), W(XS))

#define ornqx_ld(XG, MS, DS)                                                \
        ornjx_ld(W(XG), W(MS), W(DS))

#define ornqx3rr(XD, XS, XT)                                                \
        ornjx3rr(W(XD), W(XS), W(XT))

#define ornqx3ld(XD, XS, MT, DT)                                            \
        ornjx3ld(W(XD), W(XS), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorqx_rr(XG, XS)                                                    \
        xorjx_rr(W(XG), W(XS))

#define xorqx_ld(XG, MS, DS)                                                \
        xorjx_ld(W(XG), W(MS), W(DS))

#define xorqx3rr(XD, XS, XT)                                                \
        xorjx3rr(W(XD), W(XS), W(XT))

#define xorqx3ld(XD, XS, MT, DT)                                            \
        xorjx3ld(W(XD), W(XS), W(MT), W(DT))

/* not (G = ~G), (D = ~S) */

#define notqx_rx(XG)                                                        \
        notjx_rx(W(XG))

#define notqx_rr(XD, XS)                                                    \
        notjx_rr(W(XD), W(XS))

/************   packed double-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negqs_rx(XG)                                                        \
        negjs_rx(W(XG))

#define negqs_rr(XD, XS)                                                    \
        negjs_rr(W(XD), W(XS))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addqs_rr(XG, XS)                                                    \
        addjs_rr(W(XG), W(XS))

#define addqs_ld(XG, MS, DS)                                                \
        addjs_ld(W(XG), W(MS), W(DS))

#define addqs3rr(XD, XS, XT)                                                \
        addjs3rr(W(XD), W(XS), W(XT))

#define addqs3ld(XD, XS, MT, DT)                                            \
        addjs3ld(W(XD), W(XS), W(MT), W(DT))

#define adpqs_rr(XG, XS) /* horizontal pairwise add, first 15-regs only */  \
        adpjs_rr(W(XG), W(XS))

#define adpqs_ld(XG, MS, DS)                                                \
        adpjs_ld(W(XG), W(MS), W(DS))

#define adpqs3rr(XD, XS, XT)                                                \
        adpjs3rr(W(XD), W(XS), W(XT))

#define adpqs3ld(XD, XS, MT, DT)                                            \
        adpjs3ld(W(XD), W(XS), W(MT), W(DT))

#define adhqs_rr(XG, XS) /* horizontal reductive add, first 15-regs only */ \
        adhjs_rr(W(XG), W(XS))

#define adhqs_ld(XG, MS, DS)                                                \
        adhjs_ld(W(XG), W(MS), W(DS))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subqs_rr(XG, XS)                                                    \
        subjs_rr(W(XG), W(XS))

#define subqs_ld(XG, MS, DS)                                                \
        subjs_ld(W(XG), W(MS), W(DS))

#define subqs3rr(XD, XS, XT)                                                \
        subjs3rr(W(XD), W(XS), W(XT))

#define subqs3ld(XD, XS, MT, DT)                                            \
        subjs3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulqs_rr(XG, XS)                                                    \
        muljs_rr(W(XG), W(XS))

#define mulqs_ld(XG, MS, DS)                                                \
        muljs_ld(W(XG), W(MS), W(DS))

#define mulqs3rr(XD, XS, XT)                                                \
        muljs3rr(W(XD), W(XS), W(XT))

#define mulqs3ld(XD, XS, MT, DT)                                            \
        muljs3ld(W(XD), W(XS), W(MT), W(DT))

#define mlpqs_rr(XG, XS) /* horizontal pairwise mul */                      \
        mlpjs_rr(W(XG), W(XS))

#define mlpqs_ld(XG, MS, DS)                                                \
        mlpjs_ld(W(XG), W(MS), W(DS))

#define mlpqs3rr(XD, XS, XT)                                                \
        mlpjs3rr(W(XD), W(XS), W(XT))

#define mlpqs3ld(XD, XS, MT, DT)                                            \
        mlpjs3ld(W(XD), W(XS), W(MT), W(DT))

#define mlhqs_rr(XD, XS) /* horizontal reductive mul */                     \
        mlhjs_rr(W(XD), W(XS))

#define mlhqs_ld(XD, MS, DS)                                                \
        mlhjs_ld(W(XD), W(MS), W(DS))

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divqs_rr(XG, XS)                                                    \
        divjs_rr(W(XG), W(XS))

#define divqs_ld(XG, MS, DS)                                                \
        divjs_ld(W(XG), W(MS), W(DS))

#define divqs3rr(XD, XS, XT)                                                \
        divjs3rr(W(XD), W(XS), W(XT))

#define divqs3ld(XD, XS, MT, DT)                                            \
        divjs3ld(W(XD), W(XS), W(MT), W(DT))

/* sqr (D = sqrt S) */

#define sqrqs_rr(XD, XS)                                                    \
        sqrjs_rr(W(XD), W(XS))

#define sqrqs_ld(XD, MS, DS)                                                \
        sqrjs_ld(W(XD), W(MS), W(DS))

/* cbr (D = cbrt S) */

#define cbrqs_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbrjs_rr(W(XD), W(X1), W(X2), W(XS))

#define cbeqs_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbejs_rr(W(XD), W(X1), W(X2), W(XS))

#define cbsqs_rr(XG, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbsjs_rr(W(XG), W(X1), W(X2), W(XS))

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rcpqs_rr(XD, XS) /* destroys XS */                                  \
        rcpjs_rr(W(XD), W(XS))

#define rceqs_rr(XD, XS)                                                    \
        rcejs_rr(W(XD), W(XS))

#define rcsqs_rr(XG, XS) /* destroys XS */                                  \
        rcsjs_rr(W(XG), W(XS))

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rsqqs_rr(XD, XS) /* destroys XS */                                  \
        rsqjs_rr(W(XD), W(XS))

#define rseqs_rr(XD, XS)                                                    \
        rsejs_rr(W(XD), W(XS))

#define rssqs_rr(XG, XS) /* destroys XS */                                  \
        rssjs_rr(W(XG), W(XS))

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmaqs_rr(XG, XS, XT)                                                \
        fmajs_rr(W(XG), W(XS), W(XT))

#define fmaqs_ld(XG, XS, MT, DT)                                            \
        fmajs_ld(W(XG), W(XS), W(MT), W(DT))

#define fmaqs3rr(XG, XS, XT)                                                \
        fmaqs_rr(W(XG), W(XS), W(XT))

#define fmaqs3ld(XG, XS, MT, DT)                                            \
        fmaqs_ld(W(XG), W(XS), W(MT), W(DT))

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsqs_rr(XG, XS, XT)                                                \
        fmsjs_rr(W(XG), W(XS), W(XT))

#define fmsqs_ld(XG, XS, MT, DT)                                            \
        fmsjs_ld(W(XG), W(XS), W(MT), W(DT))

#define fmsqs3rr(XG, XS, XT)                                                \
        fmsqs_rr(W(XG), W(XS), W(XT))

#define fmsqs3ld(XG, XS, MT, DT)                                            \
        fmsqs_ld(W(XG), W(XS), W(MT), W(DT))

/*************   packed double-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minqs_rr(XG, XS)                                                    \
        minjs_rr(W(XG), W(XS))

#define minqs_ld(XG, MS, DS)                                                \
        minjs_ld(W(XG), W(MS), W(DS))

#define minqs3rr(XD, XS, XT)                                                \
        minjs3rr(W(XD), W(XS), W(XT))

#define minqs3ld(XD, XS, MT, DT)                                            \
        minjs3ld(W(XD), W(XS), W(MT), W(DT))

#define mnpqs_rr(XG, XS) /* horizontal pairwise min */                      \
        mnpjs_rr(W(XG), W(XS))

#define mnpqs_ld(XG, MS, DS)                                                \
        mnpjs_ld(W(XG), W(MS), W(DS))

#define mnpqs3rr(XD, XS, XT)                                                \
        mnpjs3rr(W(XD), W(XS), W(XT))

#define mnpqs3ld(XD, XS, MT, DT)                                            \
        mnpjs3ld(W(XD), W(XS), W(MT), W(DT))

#define mnhqs_rr(XD, XS) /* horizontal reductive min */                     \
        mnhjs_rr(W(XD), W(XS))

#define mnhqs_ld(XD, MS, DS)                                                \
        mnhjs_ld(W(XD), W(MS), W(DS))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxqs_rr(XG, XS)                                                    \
        maxjs_rr(W(XG), W(XS))

#define maxqs_ld(XG, MS, DS)                                                \
        maxjs_ld(W(XG), W(MS), W(DS))

#define maxqs3rr(XD, XS, XT)                                                \
        maxjs3rr(W(XD), W(XS), W(XT))

#define maxqs3ld(XD, XS, MT, DT)                                            \
        maxjs3ld(W(XD), W(XS), W(MT), W(DT))

#define mxpqs_rr(XG, XS) /* horizontal pairwise max */                      \
        mxpjs_rr(W(XG), W(XS))

#define mxpqs_ld(XG, MS, DS)                                                \
        mxpjs_ld(W(XG), W(MS), W(DS))

#define mxpqs3rr(XD, XS, XT)                                                \
        mxpjs3rr(W(XD), W(XS), W(XT))

#define mxpqs3ld(XD, XS, MT, DT)                                            \
        mxpjs3ld(W(XD), W(XS), W(MT), W(DT))

#define mxhqs_rr(XD, XS) /* horizontal reductive max */                     \
        mxhjs_rr(W(XD), W(XS))

#define mxhqs_ld(XD, MS, DS)                                                \
        mxhjs_ld(W(XD), W(MS), W(DS))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqqs_rr(XG, XS)                                                    \
        ceqjs_rr(W(XG), W(XS))

#define ceqqs_ld(XG, MS, DS)                                                \
        ceqjs_ld(W(XG), W(MS), W(DS))

#define ceqqs3rr(XD, XS, XT)                                                \
        ceqjs3rr(W(XD), W(XS), W(XT))

#define ceqqs3ld(XD, XS, MT, DT)                                            \
        ceqjs3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneqs_rr(XG, XS)                                                    \
        cnejs_rr(W(XG), W(XS))

#define cneqs_ld(XG, MS, DS)                                                \
        cnejs_ld(W(XG), W(MS), W(DS))

#define cneqs3rr(XD, XS, XT)                                                \
        cnejs3rr(W(XD), W(XS), W(XT))

#define cneqs3ld(XD, XS, MT, DT)                                            \
        cnejs3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltqs_rr(XG, XS)                                                    \
        cltjs_rr(W(XG), W(XS))

#define cltqs_ld(XG, MS, DS)                                                \
        cltjs_ld(W(XG), W(MS), W(DS))

#define cltqs3rr(XD, XS, XT)                                                \
        cltjs3rr(W(XD), W(XS), W(XT))

#define cltqs3ld(XD, XS, MT, DT)                                            \
        cltjs3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define cleqs_rr(XG, XS)                                                    \
        clejs_rr(W(XG), W(XS))

#define cleqs_ld(XG, MS, DS)                                                \
        clejs_ld(W(XG), W(MS), W(DS))

#define cleqs3rr(XD, XS, XT)                                                \
        clejs3rr(W(XD), W(XS), W(XT))

#define cleqs3ld(XD, XS, MT, DT)                                            \
        clejs3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtqs_rr(XG, XS)                                                    \
        cgtjs_rr(W(XG), W(XS))

#define cgtqs_ld(XG, MS, DS)                                                \
        cgtjs_ld(W(XG), W(MS), W(DS))

#define cgtqs3rr(XD, XS, XT)                                                \
        cgtjs3rr(W(XD), W(XS), W(XT))

#define cgtqs3ld(XD, XS, MT, DT)                                            \
        cgtjs3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgeqs_rr(XG, XS)                                                    \
        cgejs_rr(W(XG), W(XS))

#define cgeqs_ld(XG, MS, DS)                                                \
        cgejs_ld(W(XG), W(MS), W(DS))

#define cgeqs3rr(XD, XS, XT)                                                \
        cgejs3rr(W(XD), W(XS), W(XT))

#define cgeqs3ld(XD, XS, MT, DT)                                            \
        cgejs3ld(W(XD), W(XS), W(MT), W(DT))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define mkjqx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        mkjjx_rx(W(XS), mask, lb)

/*************   packed double-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnzqs_rr(XD, XS)     /* round towards zero */                       \
        rnzjs_rr(W(XD), W(XS))

#define rnzqs_ld(XD, MS, DS) /* round towards zero */                       \
        rnzjs_ld(W(XD), W(MS), W(DS))

#define cvzqs_rr(XD, XS)     /* round towards zero */                       \
        cvzjs_rr(W(XD), W(XS))

#define cvzqs_ld(XD, MS, DS) /* round towards zero */                       \
        cvzjs_ld(W(XD), W(MS), W(DS))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnpqs_rr(XD, XS)     /* round towards +inf */                       \
        rnpjs_rr(W(XD), W(XS))

#define rnpqs_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpjs_ld(W(XD), W(MS), W(DS))

#define cvpqs_rr(XD, XS)     /* round towards +inf */                       \
        cvpjs_rr(W(XD), W(XS))

#define cvpqs_ld(XD, MS, DS) /* round towards +inf */                       \
        cvpjs_ld(W(XD), W(MS), W(DS))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnmqs_rr(XD, XS)     /* round towards -inf */                       \
        rnmjs_rr(W(XD), W(XS))

#define rnmqs_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmjs_ld(W(XD), W(MS), W(DS))

#define cvmqs_rr(XD, XS)     /* round towards -inf */                       \
        cvmjs_rr(W(XD), W(XS))

#define cvmqs_ld(XD, MS, DS) /* round towards -inf */                       \
        cvmjs_ld(W(XD), W(MS), W(DS))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnnqs_rr(XD, XS)     /* round towards near */                       \
        rnnjs_rr(W(XD), W(XS))

#define rnnqs_ld(XD, MS, DS) /* round towards near */                       \
        rnnjs_ld(W(XD), W(MS), W(DS))

#define cvnqs_rr(XD, XS)     /* round towards near */                       \
        cvnjs_rr(W(XD), W(XS))

#define cvnqs_ld(XD, MS, DS) /* round towards near */                       \
        cvnjs_ld(W(XD), W(MS), W(DS))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnqn_rr(XD, XS)     /* round towards near */                       \
        cvnjn_rr(W(XD), W(XS))

#define cvnqn_ld(XD, MS, DS) /* round towards near */                       \
        cvnjn_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndqs_rr(XD, XS)                                                    \
        rndjs_rr(W(XD), W(XS))

#define rndqs_ld(XD, MS, DS)                                                \
        rndjs_ld(W(XD), W(MS), W(DS))

#define cvtqs_rr(XD, XS)                                                    \
        cvtjs_rr(W(XD), W(XS))

#define cvtqs_ld(XD, MS, DS)                                                \
        cvtjs_ld(W(XD), W(MS), W(DS))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtqn_rr(XD, XS)                                                    \
        cvtjn_rr(W(XD), W(XS))

#define cvtqn_ld(XD, MS, DS)                                                \
        cvtjn_ld(W(XD), W(MS), W(DS))

/* cvn (D = unsigned-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnqx_rr(XD, XS)     /* round towards near */                       \
        cvnjx_rr(W(XD), W(XS))

#define cvnqx_ld(XD, MS, DS) /* round towards near */                       \
        cvnjx_ld(W(XD), W(MS), W(DS))

/* cvt (D = unsigned-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtqx_rr(XD, XS)                                                    \
        cvtjx_rr(W(XD), W(XS))

#define cvtqx_ld(XD, MS, DS)                                                \
        cvtjx_ld(W(XD), W(MS), W(DS))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnrqs_rr(XD, XS, mode)                                              \
        rnrjs_rr(W(XD), W(XS), mode)

#define cvrqs_rr(XD, XS, mode)                                              \
        cvrjs_rr(W(XD), W(XS), mode)

/************   packed double-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addqx_rr(XG, XS)                                                    \
        addjx_rr(W(XG), W(XS))

#define addqx_ld(XG, MS, DS)                                                \
        addjx_ld(W(XG), W(MS), W(DS))

#define addqx3rr(XD, XS, XT)                                                \
        addjx3rr(W(XD), W(XS), W(XT))

#define addqx3ld(XD, XS, MT, DT)                                            \
        addjx3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subqx_rr(XG, XS)                                                    \
        subjx_rr(W(XG), W(XS))

#define subqx_ld(XG, MS, DS)                                                \
        subjx_ld(W(XG), W(MS), W(DS))

#define subqx3rr(XD, XS, XT)                                                \
        subjx3rr(W(XD), W(XS), W(XT))

#define subqx3ld(XD, XS, MT, DT)                                            \
        subjx3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulqx_rr(XG, XS)                                                    \
        muljx_rr(W(XG), W(XS))

#define mulqx_ld(XG, MS, DS)                                                \
        muljx_ld(W(XG), W(MS), W(DS))

#define mulqx3rr(XD, XS, XT)                                                \
        muljx3rr(W(XD), W(XS), W(XT))

#define mulqx3ld(XD, XS, MT, DT)                                            \
        muljx3ld(W(XD), W(XS), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlqx_ri(XG, IS)                                                    \
        shljx_ri(W(XG), W(IS))

#define shlqx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shljx_ld(W(XG), W(MS), W(DS))

#define shlqx3ri(XD, XS, IT)                                                \
        shljx3ri(W(XD), W(XS), W(IT))

#define shlqx3ld(XD, XS, MT, DT)                                            \
        shljx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrqx_ri(XG, IS)                                                    \
        shrjx_ri(W(XG), W(IS))

#define shrqx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrjx_ld(W(XG), W(MS), W(DS))

#define shrqx3ri(XD, XS, IT)                                                \
        shrjx3ri(W(XD), W(XS), W(IT))

#define shrqx3ld(XD, XS, MT, DT)                                            \
        shrjx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrqn_ri(XG, IS)                                                    \
        shrjn_ri(W(XG), W(IS))

#define shrqn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrjn_ld(W(XG), W(MS), W(DS))

#define shrqn3ri(XD, XS, IT)                                                \
        shrjn3ri(W(XD), W(XS), W(IT))

#define shrqn3ld(XD, XS, MT, DT)                                            \
        shrjn3ld(W(XD), W(XS), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlqx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svljx_rr(W(XG), W(XS))

#define svlqx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svljx_ld(W(XG), W(MS), W(DS))

#define svlqx3rr(XD, XS, XT)                                                \
        svljx3rr(W(XD), W(XS), W(XT))

#define svlqx3ld(XD, XS, MT, DT)                                            \
        svljx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrqx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrjx_rr(W(XG), W(XS))

#define svrqx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrjx_ld(W(XG), W(MS), W(DS))

#define svrqx3rr(XD, XS, XT)                                                \
        svrjx3rr(W(XD), W(XS), W(XT))

#define svrqx3ld(XD, XS, MT, DT)                                            \
        svrjx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrqn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrjn_rr(W(XG), W(XS))

#define svrqn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrjn_ld(W(XG), W(MS), W(DS))

#define svrqn3rr(XD, XS, XT)                                                \
        svrjn3rr(W(XD), W(XS), W(XT))

#define svrqn3ld(XD, XS, MT, DT)                                            \
        svrjn3ld(W(XD), W(XS), W(MT), W(DT))

/****************   packed double-precision integer compare   *****************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minqx_rr(XG, XS)                                                    \
        minjx_rr(W(XG), W(XS))

#define minqx_ld(XG, MS, DS)                                                \
        minjx_ld(W(XG), W(MS), W(DS))

#define minqx3rr(XD, XS, XT)                                                \
        minjx3rr(W(XD), W(XS), W(XT))

#define minqx3ld(XD, XS, MT, DT)                                            \
        minjx3ld(W(XD), W(XS), W(MT), W(DT))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minqn_rr(XG, XS)                                                    \
        minjn_rr(W(XG), W(XS))

#define minqn_ld(XG, MS, DS)                                                \
        minjn_ld(W(XG), W(MS), W(DS))

#define minqn3rr(XD, XS, XT)                                                \
        minjn3rr(W(XD), W(XS), W(XT))

#define minqn3ld(XD, XS, MT, DT)                                            \
        minjn3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxqx_rr(XG, XS)                                                    \
        maxjx_rr(W(XG), W(XS))

#define maxqx_ld(XG, MS, DS)                                                \
        maxjx_ld(W(XG), W(MS), W(DS))

#define maxqx3rr(XD, XS, XT)                                                \
        maxjx3rr(W(XD), W(XS), W(XT))

#define maxqx3ld(XD, XS, MT, DT)                                            \
        maxjx3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxqn_rr(XG, XS)                                                    \
        maxjn_rr(W(XG), W(XS))

#define maxqn_ld(XG, MS, DS)                                                \
        maxjn_ld(W(XG), W(MS), W(DS))

#define maxqn3rr(XD, XS, XT)                                                \
        maxjn3rr(W(XD), W(XS), W(XT))

#define maxqn3ld(XD, XS, MT, DT)                                            \
        maxjn3ld(W(XD), W(XS), W(MT), W(DT))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqqx_rr(XG, XS)                                                    \
        ceqjx_rr(W(XG), W(XS))

#define ceqqx_ld(XG, MS, DS)                                                \
        ceqjx_ld(W(XG), W(MS), W(DS))

#define ceqqx3rr(XD, XS, XT)                                                \
        ceqjx3rr(W(XD), W(XS), W(XT))

#define ceqqx3ld(XD, XS, MT, DT)                                            \
        ceqjx3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneqx_rr(XG, XS)                                                    \
        cnejx_rr(W(XG), W(XS))

#define cneqx_ld(XG, MS, DS)                                                \
        cnejx_ld(W(XG), W(MS), W(DS))

#define cneqx3rr(XD, XS, XT)                                                \
        cnejx3rr(W(XD), W(XS), W(XT))

#define cneqx3ld(XD, XS, MT, DT)                                            \
        cnejx3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltqx_rr(XG, XS)                                                    \
        cltjx_rr(W(XG), W(XS))

#define cltqx_ld(XG, MS, DS)                                                \
        cltjx_ld(W(XG), W(MS), W(DS))

#define cltqx3rr(XD, XS, XT)                                                \
        cltjx3rr(W(XD), W(XS), W(XT))

#define cltqx3ld(XD, XS, MT, DT)                                            \
        cltjx3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltqn_rr(XG, XS)                                                    \
        cltjn_rr(W(XG), W(XS))

#define cltqn_ld(XG, MS, DS)                                                \
        cltjn_ld(W(XG), W(MS), W(DS))

#define cltqn3rr(XD, XS, XT)                                                \
        cltjn3rr(W(XD), W(XS), W(XT))

#define cltqn3ld(XD, XS, MT, DT)                                            \
        cltjn3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define cleqx_rr(XG, XS)                                                    \
        clejx_rr(W(XG), W(XS))

#define cleqx_ld(XG, MS, DS)                                                \
        clejx_ld(W(XG), W(MS), W(DS))

#define cleqx3rr(XD, XS, XT)                                                \
        clejx3rr(W(XD), W(XS), W(XT))

#define cleqx3ld(XD, XS, MT, DT)                                            \
        clejx3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define cleqn_rr(XG, XS)                                                    \
        clejn_rr(W(XG), W(XS))

#define cleqn_ld(XG, MS, DS)                                                \
        clejn_ld(W(XG), W(MS), W(DS))

#define cleqn3rr(XD, XS, XT)                                                \
        clejn3rr(W(XD), W(XS), W(XT))

#define cleqn3ld(XD, XS, MT, DT)                                            \
        clejn3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtqx_rr(XG, XS)                                                    \
        cgtjx_rr(W(XG), W(XS))

#define cgtqx_ld(XG, MS, DS)                                                \
        cgtjx_ld(W(XG), W(MS), W(DS))

#define cgtqx3rr(XD, XS, XT)                                                \
        cgtjx3rr(W(XD), W(XS), W(XT))

#define cgtqx3ld(XD, XS, MT, DT)                                            \
        cgtjx3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtqn_rr(XG, XS)                                                    \
        cgtjn_rr(W(XG), W(XS))

#define cgtqn_ld(XG, MS, DS)                                                \
        cgtjn_ld(W(XG), W(MS), W(DS))

#define cgtqn3rr(XD, XS, XT)                                                \
        cgtjn3rr(W(XD), W(XS), W(XT))

#define cgtqn3ld(XD, XS, MT, DT)                                            \
        cgtjn3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgeqx_rr(XG, XS)                                                    \
        cgejx_rr(W(XG), W(XS))

#define cgeqx_ld(XG, MS, DS)                                                \
        cgejx_ld(W(XG), W(MS), W(DS))

#define cgeqx3rr(XD, XS, XT)                                                \
        cgejx3rr(W(XD), W(XS), W(XT))

#define cgeqx3ld(XD, XS, MT, DT)                                            \
        cgejx3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgeqn_rr(XG, XS)                                                    \
        cgejn_rr(W(XG), W(XS))

#define cgeqn_ld(XG, MS, DS)                                                \
        cgejn_ld(W(XG), W(MS), W(DS))

#define cgeqn3rr(XD, XS, XT)                                                \
        cgejn3rr(W(XD), W(XS), W(XT))

#define cgeqn3ld(XD, XS, MT, DT)                                            \
        cgejn3ld(W(XD), W(XS), W(MT), W(DT))

#endif /* RT_SIMD: 256, 128 */

/******************************************************************************/
/**** var-len **** SIMD instructions with configurable element **** 32-bit ****/
/******************************************************************************/

#if   RT_ELEMENT == 32

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmpx_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        elmox_st(W(XS), W(MD), W(DD))

/***************   packed single-precision generic move/logic   ***************/

/* mov (D = S) */

#define movpx_rr(XD, XS)                                                    \
        movox_rr(W(XD), W(XS))

#define movpx_ld(XD, MS, DS)                                                \
        movox_ld(W(XD), W(MS), W(DS))

#define movpx_st(XS, MD, DD)                                                \
        movox_st(W(XS), W(MD), W(DD))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvpx_rr(XD, XS)                                                    \
        mmvox_rr(W(XD), W(XS))

#define mmvpx_ld(XG, MS, DS)                                                \
        mmvox_ld(W(XG), W(MS), W(DS))

#define mmvpx_st(XS, MG, DG)                                                \
        mmvox_st(W(XS), W(MG), W(DG))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andpx_rr(XG, XS)                                                    \
        andox_rr(W(XG), W(XS))

#define andpx_ld(XG, MS, DS)                                                \
        andox_ld(W(XG), W(MS), W(DS))

#define andpx3rr(XD, XS, XT)                                                \
        andox3rr(W(XD), W(XS), W(XT))

#define andpx3ld(XD, XS, MT, DT)                                            \
        andox3ld(W(XD), W(XS), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annpx_rr(XG, XS)                                                    \
        annox_rr(W(XG), W(XS))

#define annpx_ld(XG, MS, DS)                                                \
        annox_ld(W(XG), W(MS), W(DS))

#define annpx3rr(XD, XS, XT)                                                \
        annox3rr(W(XD), W(XS), W(XT))

#define annpx3ld(XD, XS, MT, DT)                                            \
        annox3ld(W(XD), W(XS), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrpx_rr(XG, XS)                                                    \
        orrox_rr(W(XG), W(XS))

#define orrpx_ld(XG, MS, DS)                                                \
        orrox_ld(W(XG), W(MS), W(DS))

#define orrpx3rr(XD, XS, XT)                                                \
        orrox3rr(W(XD), W(XS), W(XT))

#define orrpx3ld(XD, XS, MT, DT)                                            \
        orrox3ld(W(XD), W(XS), W(MT), W(DT))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define ornpx_rr(XG, XS)                                                    \
        ornox_rr(W(XG), W(XS))

#define ornpx_ld(XG, MS, DS)                                                \
        ornox_ld(W(XG), W(MS), W(DS))

#define ornpx3rr(XD, XS, XT)                                                \
        ornox3rr(W(XD), W(XS), W(XT))

#define ornpx3ld(XD, XS, MT, DT)                                            \
        ornox3ld(W(XD), W(XS), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorpx_rr(XG, XS)                                                    \
        xorox_rr(W(XG), W(XS))

#define xorpx_ld(XG, MS, DS)                                                \
        xorox_ld(W(XG), W(MS), W(DS))

#define xorpx3rr(XD, XS, XT)                                                \
        xorox3rr(W(XD), W(XS), W(XT))

#define xorpx3ld(XD, XS, MT, DT)                                            \
        xorox3ld(W(XD), W(XS), W(MT), W(DT))

/* not (G = ~G), (D = ~S) */

#define notpx_rx(XG)                                                        \
        notox_rx(W(XG))

#define notpx_rr(XD, XS)                                                    \
        notox_rr(W(XD), W(XS))

/************   packed single-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negps_rx(XG)                                                        \
        negos_rx(W(XG))

#define negps_rr(XD, XS)                                                    \
        negos_rr(W(XD), W(XS))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addps_rr(XG, XS)                                                    \
        addos_rr(W(XG), W(XS))

#define addps_ld(XG, MS, DS)                                                \
        addos_ld(W(XG), W(MS), W(DS))

#define addps3rr(XD, XS, XT)                                                \
        addos3rr(W(XD), W(XS), W(XT))

#define addps3ld(XD, XS, MT, DT)                                            \
        addos3ld(W(XD), W(XS), W(MT), W(DT))

#define adpps_rr(XG, XS) /* horizontal pairwise add, first 15-regs only */  \
        adpos_rr(W(XG), W(XS))

#define adpps_ld(XG, MS, DS)                                                \
        adpos_ld(W(XG), W(MS), W(DS))

#define adpps3rr(XD, XS, XT)                                                \
        adpos3rr(W(XD), W(XS), W(XT))

#define adpps3ld(XD, XS, MT, DT)                                            \
        adpos3ld(W(XD), W(XS), W(MT), W(DT))

#define adhps_rr(XG, XS) /* horizontal reductive add, first 15-regs only */ \
        adhos_rr(W(XG), W(XS))

#define adhps_ld(XG, MS, DS)                                                \
        adhos_ld(W(XG), W(MS), W(DS))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subps_rr(XG, XS)                                                    \
        subos_rr(W(XG), W(XS))

#define subps_ld(XG, MS, DS)                                                \
        subos_ld(W(XG), W(MS), W(DS))

#define subps3rr(XD, XS, XT)                                                \
        subos3rr(W(XD), W(XS), W(XT))

#define subps3ld(XD, XS, MT, DT)                                            \
        subos3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulps_rr(XG, XS)                                                    \
        mulos_rr(W(XG), W(XS))

#define mulps_ld(XG, MS, DS)                                                \
        mulos_ld(W(XG), W(MS), W(DS))

#define mulps3rr(XD, XS, XT)                                                \
        mulos3rr(W(XD), W(XS), W(XT))

#define mulps3ld(XD, XS, MT, DT)                                            \
        mulos3ld(W(XD), W(XS), W(MT), W(DT))

#define mlpps_rr(XG, XS) /* horizontal pairwise mul */                      \
        mlpos_rr(W(XG), W(XS))

#define mlpps_ld(XG, MS, DS)                                                \
        mlpos_ld(W(XG), W(MS), W(DS))

#define mlpps3rr(XD, XS, XT)                                                \
        mlpos3rr(W(XD), W(XS), W(XT))

#define mlpps3ld(XD, XS, MT, DT)                                            \
        mlpos3ld(W(XD), W(XS), W(MT), W(DT))

#define mlhps_rr(XD, XS) /* horizontal reductive mul */                     \
        mlhos_rr(W(XD), W(XS))

#define mlhps_ld(XD, MS, DS)                                                \
        mlhos_ld(W(XD), W(MS), W(DS))

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divps_rr(XG, XS)                                                    \
        divos_rr(W(XG), W(XS))

#define divps_ld(XG, MS, DS)                                                \
        divos_ld(W(XG), W(MS), W(DS))

#define divps3rr(XD, XS, XT)                                                \
        divos3rr(W(XD), W(XS), W(XT))

#define divps3ld(XD, XS, MT, DT)                                            \
        divos3ld(W(XD), W(XS), W(MT), W(DT))

/* sqr (D = sqrt S) */

#define sqrps_rr(XD, XS)                                                    \
        sqros_rr(W(XD), W(XS))

#define sqrps_ld(XD, MS, DS)                                                \
        sqros_ld(W(XD), W(MS), W(DS))

/* cbr (D = cbrt S) */

#define cbrps_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbros_rr(W(XD), W(X1), W(X2), W(XS))

#define cbeps_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbeos_rr(W(XD), W(X1), W(X2), W(XS))

#define cbsps_rr(XG, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbsos_rr(W(XG), W(X1), W(X2), W(XS))

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rcpps_rr(XD, XS) /* destroys XS */                                  \
        rcpos_rr(W(XD), W(XS))

#define rceps_rr(XD, XS)                                                    \
        rceos_rr(W(XD), W(XS))

#define rcsps_rr(XG, XS) /* destroys XS */                                  \
        rcsos_rr(W(XG), W(XS))

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rsqps_rr(XD, XS) /* destroys XS */                                  \
        rsqos_rr(W(XD), W(XS))

#define rseps_rr(XD, XS)                                                    \
        rseos_rr(W(XD), W(XS))

#define rssps_rr(XG, XS) /* destroys XS */                                  \
        rssos_rr(W(XG), W(XS))

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmaps_rr(XG, XS, XT)                                                \
        fmaos_rr(W(XG), W(XS), W(XT))

#define fmaps_ld(XG, XS, MT, DT)                                            \
        fmaos_ld(W(XG), W(XS), W(MT), W(DT))

#define fmaps3rr(XG, XS, XT)                                                \
        fmaps_rr(W(XG), W(XS), W(XT))

#define fmaps3ld(XG, XS, MT, DT)                                            \
        fmaps_ld(W(XG), W(XS), W(MT), W(DT))

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsps_rr(XG, XS, XT)                                                \
        fmsos_rr(W(XG), W(XS), W(XT))

#define fmsps_ld(XG, XS, MT, DT)                                            \
        fmsos_ld(W(XG), W(XS), W(MT), W(DT))

#define fmsps3rr(XG, XS, XT)                                                \
        fmsps_rr(W(XG), W(XS), W(XT))

#define fmsps3ld(XG, XS, MT, DT)                                            \
        fmsps_ld(W(XG), W(XS), W(MT), W(DT))

/*************   packed single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minps_rr(XG, XS)                                                    \
        minos_rr(W(XG), W(XS))

#define minps_ld(XG, MS, DS)                                                \
        minos_ld(W(XG), W(MS), W(DS))

#define minps3rr(XD, XS, XT)                                                \
        minos3rr(W(XD), W(XS), W(XT))

#define minps3ld(XD, XS, MT, DT)                                            \
        minos3ld(W(XD), W(XS), W(MT), W(DT))

#define mnpps_rr(XG, XS) /* horizontal pairwise min */                      \
        mnpos_rr(W(XG), W(XS))

#define mnpps_ld(XG, MS, DS)                                                \
        mnpos_ld(W(XG), W(MS), W(DS))

#define mnpps3rr(XD, XS, XT)                                                \
        mnpos3rr(W(XD), W(XS), W(XT))

#define mnpps3ld(XD, XS, MT, DT)                                            \
        mnpos3ld(W(XD), W(XS), W(MT), W(DT))

#define mnhps_rr(XD, XS) /* horizontal reductive min */                     \
        mnhos_rr(W(XD), W(XS))

#define mnhps_ld(XD, MS, DS)                                                \
        mnhos_ld(W(XD), W(MS), W(DS))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxps_rr(XG, XS)                                                    \
        maxos_rr(W(XG), W(XS))

#define maxps_ld(XG, MS, DS)                                                \
        maxos_ld(W(XG), W(MS), W(DS))

#define maxps3rr(XD, XS, XT)                                                \
        maxos3rr(W(XD), W(XS), W(XT))

#define maxps3ld(XD, XS, MT, DT)                                            \
        maxos3ld(W(XD), W(XS), W(MT), W(DT))

#define mxpps_rr(XG, XS) /* horizontal pairwise max */                      \
        mxpos_rr(W(XG), W(XS))

#define mxpps_ld(XG, MS, DS)                                                \
        mxpos_ld(W(XG), W(MS), W(DS))

#define mxpps3rr(XD, XS, XT)                                                \
        mxpos3rr(W(XD), W(XS), W(XT))

#define mxpps3ld(XD, XS, MT, DT)                                            \
        mxpos3ld(W(XD), W(XS), W(MT), W(DT))

#define mxhps_rr(XD, XS) /* horizontal reductive max */                     \
        mxhos_rr(W(XD), W(XS))

#define mxhps_ld(XD, MS, DS)                                                \
        mxhos_ld(W(XD), W(MS), W(DS))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqps_rr(XG, XS)                                                    \
        ceqos_rr(W(XG), W(XS))

#define ceqps_ld(XG, MS, DS)                                                \
        ceqos_ld(W(XG), W(MS), W(DS))

#define ceqps3rr(XD, XS, XT)                                                \
        ceqos3rr(W(XD), W(XS), W(XT))

#define ceqps3ld(XD, XS, MT, DT)                                            \
        ceqos3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneps_rr(XG, XS)                                                    \
        cneos_rr(W(XG), W(XS))

#define cneps_ld(XG, MS, DS)                                                \
        cneos_ld(W(XG), W(MS), W(DS))

#define cneps3rr(XD, XS, XT)                                                \
        cneos3rr(W(XD), W(XS), W(XT))

#define cneps3ld(XD, XS, MT, DT)                                            \
        cneos3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltps_rr(XG, XS)                                                    \
        cltos_rr(W(XG), W(XS))

#define cltps_ld(XG, MS, DS)                                                \
        cltos_ld(W(XG), W(MS), W(DS))

#define cltps3rr(XD, XS, XT)                                                \
        cltos3rr(W(XD), W(XS), W(XT))

#define cltps3ld(XD, XS, MT, DT)                                            \
        cltos3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define cleps_rr(XG, XS)                                                    \
        cleos_rr(W(XG), W(XS))

#define cleps_ld(XG, MS, DS)                                                \
        cleos_ld(W(XG), W(MS), W(DS))

#define cleps3rr(XD, XS, XT)                                                \
        cleos3rr(W(XD), W(XS), W(XT))

#define cleps3ld(XD, XS, MT, DT)                                            \
        cleos3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtps_rr(XG, XS)                                                    \
        cgtos_rr(W(XG), W(XS))

#define cgtps_ld(XG, MS, DS)                                                \
        cgtos_ld(W(XG), W(MS), W(DS))

#define cgtps3rr(XD, XS, XT)                                                \
        cgtos3rr(W(XD), W(XS), W(XT))

#define cgtps3ld(XD, XS, MT, DT)                                            \
        cgtos3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgeps_rr(XG, XS)                                                    \
        cgeos_rr(W(XG), W(XS))

#define cgeps_ld(XG, MS, DS)                                                \
        cgeos_ld(W(XG), W(MS), W(DS))

#define cgeps3rr(XD, XS, XT)                                                \
        cgeos3rr(W(XD), W(XS), W(XT))

#define cgeps3ld(XD, XS, MT, DT)                                            \
        cgeos3ld(W(XD), W(XS), W(MT), W(DT))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define mkjpx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        mkjox_rx(W(XS), mask, lb)

/*************   packed single-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzps_rr(XD, XS)     /* round towards zero */                       \
        rnzos_rr(W(XD), W(XS))

#define rnzps_ld(XD, MS, DS) /* round towards zero */                       \
        rnzos_ld(W(XD), W(MS), W(DS))

#define cvzps_rr(XD, XS)     /* round towards zero */                       \
        cvzos_rr(W(XD), W(XS))

#define cvzps_ld(XD, MS, DS) /* round towards zero */                       \
        cvzos_ld(W(XD), W(MS), W(DS))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpps_rr(XD, XS)     /* round towards +inf */                       \
        rnpos_rr(W(XD), W(XS))

#define rnpps_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpos_ld(W(XD), W(MS), W(DS))

#define cvpps_rr(XD, XS)     /* round towards +inf */                       \
        cvpos_rr(W(XD), W(XS))

#define cvpps_ld(XD, MS, DS) /* round towards +inf */                       \
        cvpos_ld(W(XD), W(MS), W(DS))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmps_rr(XD, XS)     /* round towards -inf */                       \
        rnmos_rr(W(XD), W(XS))

#define rnmps_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmos_ld(W(XD), W(MS), W(DS))

#define cvmps_rr(XD, XS)     /* round towards -inf */                       \
        cvmos_rr(W(XD), W(XS))

#define cvmps_ld(XD, MS, DS) /* round towards -inf */                       \
        cvmos_ld(W(XD), W(MS), W(DS))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnps_rr(XD, XS)     /* round towards near */                       \
        rnnos_rr(W(XD), W(XS))

#define rnnps_ld(XD, MS, DS) /* round towards near */                       \
        rnnos_ld(W(XD), W(MS), W(DS))

#define cvnps_rr(XD, XS)     /* round towards near */                       \
        cvnos_rr(W(XD), W(XS))

#define cvnps_ld(XD, MS, DS) /* round towards near */                       \
        cvnos_ld(W(XD), W(MS), W(DS))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnpn_rr(XD, XS)     /* round towards near */                       \
        cvnon_rr(W(XD), W(XS))

#define cvnpn_ld(XD, MS, DS) /* round towards near */                       \
        cvnon_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndps_rr(XD, XS)                                                    \
        rndos_rr(W(XD), W(XS))

#define rndps_ld(XD, MS, DS)                                                \
        rndos_ld(W(XD), W(MS), W(DS))

#define cvtps_rr(XD, XS)                                                    \
        cvtos_rr(W(XD), W(XS))

#define cvtps_ld(XD, MS, DS)                                                \
        cvtos_ld(W(XD), W(MS), W(DS))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtpn_rr(XD, XS)                                                    \
        cvton_rr(W(XD), W(XS))

#define cvtpn_ld(XD, MS, DS)                                                \
        cvton_ld(W(XD), W(MS), W(DS))

/* cvn (D = unsigned-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnpx_rr(XD, XS)     /* round towards near */                       \
        cvnox_rr(W(XD), W(XS))

#define cvnpx_ld(XD, MS, DS) /* round towards near */                       \
        cvnox_ld(W(XD), W(MS), W(DS))

/* cvt (D = unsigned-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtpx_rr(XD, XS)                                                    \
        cvtox_rr(W(XD), W(XS))

#define cvtpx_ld(XD, MS, DS)                                                \
        cvtox_ld(W(XD), W(MS), W(DS))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnrps_rr(XD, XS, mode)                                              \
        rnros_rr(W(XD), W(XS), mode)

#define cvrps_rr(XD, XS, mode)                                              \
        cvros_rr(W(XD), W(XS), mode)

/************   packed single-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addpx_rr(XG, XS)                                                    \
        addox_rr(W(XG), W(XS))

#define addpx_ld(XG, MS, DS)                                                \
        addox_ld(W(XG), W(MS), W(DS))

#define addpx3rr(XD, XS, XT)                                                \
        addox3rr(W(XD), W(XS), W(XT))

#define addpx3ld(XD, XS, MT, DT)                                            \
        addox3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subpx_rr(XG, XS)                                                    \
        subox_rr(W(XG), W(XS))

#define subpx_ld(XG, MS, DS)                                                \
        subox_ld(W(XG), W(MS), W(DS))

#define subpx3rr(XD, XS, XT)                                                \
        subox3rr(W(XD), W(XS), W(XT))

#define subpx3ld(XD, XS, MT, DT)                                            \
        subox3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulpx_rr(XG, XS)                                                    \
        mulox_rr(W(XG), W(XS))

#define mulpx_ld(XG, MS, DS)                                                \
        mulox_ld(W(XG), W(MS), W(DS))

#define mulpx3rr(XD, XS, XT)                                                \
        mulox3rr(W(XD), W(XS), W(XT))

#define mulpx3ld(XD, XS, MT, DT)                                            \
        mulox3ld(W(XD), W(XS), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlpx_ri(XG, IS)                                                    \
        shlox_ri(W(XG), W(IS))

#define shlpx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlox_ld(W(XG), W(MS), W(DS))

#define shlpx3ri(XD, XS, IT)                                                \
        shlox3ri(W(XD), W(XS), W(IT))

#define shlpx3ld(XD, XS, MT, DT)                                            \
        shlox3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrpx_ri(XG, IS)                                                    \
        shrox_ri(W(XG), W(IS))

#define shrpx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrox_ld(W(XG), W(MS), W(DS))

#define shrpx3ri(XD, XS, IT)                                                \
        shrox3ri(W(XD), W(XS), W(IT))

#define shrpx3ld(XD, XS, MT, DT)                                            \
        shrox3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrpn_ri(XG, IS)                                                    \
        shron_ri(W(XG), W(IS))

#define shrpn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shron_ld(W(XG), W(MS), W(DS))

#define shrpn3ri(XD, XS, IT)                                                \
        shron3ri(W(XD), W(XS), W(IT))

#define shrpn3ld(XD, XS, MT, DT)                                            \
        shron3ld(W(XD), W(XS), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlpx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlox_rr(W(XG), W(XS))

#define svlpx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlox_ld(W(XG), W(MS), W(DS))

#define svlpx3rr(XD, XS, XT)                                                \
        svlox3rr(W(XD), W(XS), W(XT))

#define svlpx3ld(XD, XS, MT, DT)                                            \
        svlox3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrpx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrox_rr(W(XG), W(XS))

#define svrpx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrox_ld(W(XG), W(MS), W(DS))

#define svrpx3rr(XD, XS, XT)                                                \
        svrox3rr(W(XD), W(XS), W(XT))

#define svrpx3ld(XD, XS, MT, DT)                                            \
        svrox3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrpn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svron_rr(W(XG), W(XS))

#define svrpn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svron_ld(W(XG), W(MS), W(DS))

#define svrpn3rr(XD, XS, XT)                                                \
        svron3rr(W(XD), W(XS), W(XT))

#define svrpn3ld(XD, XS, MT, DT)                                            \
        svron3ld(W(XD), W(XS), W(MT), W(DT))

/****************   packed single-precision integer compare   *****************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minpx_rr(XG, XS)                                                    \
        minox_rr(W(XG), W(XS))

#define minpx_ld(XG, MS, DS)                                                \
        minox_ld(W(XG), W(MS), W(DS))

#define minpx3rr(XD, XS, XT)                                                \
        minox3rr(W(XD), W(XS), W(XT))

#define minpx3ld(XD, XS, MT, DT)                                            \
        minox3ld(W(XD), W(XS), W(MT), W(DT))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minpn_rr(XG, XS)                                                    \
        minon_rr(W(XG), W(XS))

#define minpn_ld(XG, MS, DS)                                                \
        minon_ld(W(XG), W(MS), W(DS))

#define minpn3rr(XD, XS, XT)                                                \
        minon3rr(W(XD), W(XS), W(XT))

#define minpn3ld(XD, XS, MT, DT)                                            \
        minon3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxpx_rr(XG, XS)                                                    \
        maxox_rr(W(XG), W(XS))

#define maxpx_ld(XG, MS, DS)                                                \
        maxox_ld(W(XG), W(MS), W(DS))

#define maxpx3rr(XD, XS, XT)                                                \
        maxox3rr(W(XD), W(XS), W(XT))

#define maxpx3ld(XD, XS, MT, DT)                                            \
        maxox3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxpn_rr(XG, XS)                                                    \
        maxon_rr(W(XG), W(XS))

#define maxpn_ld(XG, MS, DS)                                                \
        maxon_ld(W(XG), W(MS), W(DS))

#define maxpn3rr(XD, XS, XT)                                                \
        maxon3rr(W(XD), W(XS), W(XT))

#define maxpn3ld(XD, XS, MT, DT)                                            \
        maxon3ld(W(XD), W(XS), W(MT), W(DT))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqpx_rr(XG, XS)                                                    \
        ceqox_rr(W(XG), W(XS))

#define ceqpx_ld(XG, MS, DS)                                                \
        ceqox_ld(W(XG), W(MS), W(DS))

#define ceqpx3rr(XD, XS, XT)                                                \
        ceqox3rr(W(XD), W(XS), W(XT))

#define ceqpx3ld(XD, XS, MT, DT)                                            \
        ceqox3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnepx_rr(XG, XS)                                                    \
        cneox_rr(W(XG), W(XS))

#define cnepx_ld(XG, MS, DS)                                                \
        cneox_ld(W(XG), W(MS), W(DS))

#define cnepx3rr(XD, XS, XT)                                                \
        cneox3rr(W(XD), W(XS), W(XT))

#define cnepx3ld(XD, XS, MT, DT)                                            \
        cneox3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltpx_rr(XG, XS)                                                    \
        cltox_rr(W(XG), W(XS))

#define cltpx_ld(XG, MS, DS)                                                \
        cltox_ld(W(XG), W(MS), W(DS))

#define cltpx3rr(XD, XS, XT)                                                \
        cltox3rr(W(XD), W(XS), W(XT))

#define cltpx3ld(XD, XS, MT, DT)                                            \
        cltox3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltpn_rr(XG, XS)                                                    \
        clton_rr(W(XG), W(XS))

#define cltpn_ld(XG, MS, DS)                                                \
        clton_ld(W(XG), W(MS), W(DS))

#define cltpn3rr(XD, XS, XT)                                                \
        clton3rr(W(XD), W(XS), W(XT))

#define cltpn3ld(XD, XS, MT, DT)                                            \
        clton3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clepx_rr(XG, XS)                                                    \
        cleox_rr(W(XG), W(XS))

#define clepx_ld(XG, MS, DS)                                                \
        cleox_ld(W(XG), W(MS), W(DS))

#define clepx3rr(XD, XS, XT)                                                \
        cleox3rr(W(XD), W(XS), W(XT))

#define clepx3ld(XD, XS, MT, DT)                                            \
        cleox3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clepn_rr(XG, XS)                                                    \
        cleon_rr(W(XG), W(XS))

#define clepn_ld(XG, MS, DS)                                                \
        cleon_ld(W(XG), W(MS), W(DS))

#define clepn3rr(XD, XS, XT)                                                \
        cleon3rr(W(XD), W(XS), W(XT))

#define clepn3ld(XD, XS, MT, DT)                                            \
        cleon3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtpx_rr(XG, XS)                                                    \
        cgtox_rr(W(XG), W(XS))

#define cgtpx_ld(XG, MS, DS)                                                \
        cgtox_ld(W(XG), W(MS), W(DS))

#define cgtpx3rr(XD, XS, XT)                                                \
        cgtox3rr(W(XD), W(XS), W(XT))

#define cgtpx3ld(XD, XS, MT, DT)                                            \
        cgtox3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtpn_rr(XG, XS)                                                    \
        cgton_rr(W(XG), W(XS))

#define cgtpn_ld(XG, MS, DS)                                                \
        cgton_ld(W(XG), W(MS), W(DS))

#define cgtpn3rr(XD, XS, XT)                                                \
        cgton3rr(W(XD), W(XS), W(XT))

#define cgtpn3ld(XD, XS, MT, DT)                                            \
        cgton3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgepx_rr(XG, XS)                                                    \
        cgeox_rr(W(XG), W(XS))

#define cgepx_ld(XG, MS, DS)                                                \
        cgeox_ld(W(XG), W(MS), W(DS))

#define cgepx3rr(XD, XS, XT)                                                \
        cgeox3rr(W(XD), W(XS), W(XT))

#define cgepx3ld(XD, XS, MT, DT)                                            \
        cgeox3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgepn_rr(XG, XS)                                                    \
        cgeon_rr(W(XG), W(XS))

#define cgepn_ld(XG, MS, DS)                                                \
        cgeon_ld(W(XG), W(MS), W(DS))

#define cgepn3rr(XD, XS, XT)                                                \
        cgeon3rr(W(XD), W(XS), W(XT))

#define cgepn3ld(XD, XS, MT, DT)                                            \
        cgeon3ld(W(XD), W(XS), W(MT), W(DT))

/******************************************************************************/
/**** 256-bit **** SIMD instructions with configurable element **** 32-bit ****/
/******************************************************************************/

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmfx_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        elmcx_st(W(XS), W(MD), W(DD))

/***************   packed single-precision generic move/logic   ***************/

/* mov (D = S) */

#define movfx_rr(XD, XS)                                                    \
        movcx_rr(W(XD), W(XS))

#define movfx_ld(XD, MS, DS)                                                \
        movcx_ld(W(XD), W(MS), W(DS))

#define movfx_st(XS, MD, DD)                                                \
        movcx_st(W(XS), W(MD), W(DD))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvfx_rr(XD, XS)                                                    \
        mmvcx_rr(W(XD), W(XS))

#define mmvfx_ld(XG, MS, DS)                                                \
        mmvcx_ld(W(XG), W(MS), W(DS))

#define mmvfx_st(XS, MG, DG)                                                \
        mmvcx_st(W(XS), W(MG), W(DG))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andfx_rr(XG, XS)                                                    \
        andcx_rr(W(XG), W(XS))

#define andfx_ld(XG, MS, DS)                                                \
        andcx_ld(W(XG), W(MS), W(DS))

#define andfx3rr(XD, XS, XT)                                                \
        andcx3rr(W(XD), W(XS), W(XT))

#define andfx3ld(XD, XS, MT, DT)                                            \
        andcx3ld(W(XD), W(XS), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annfx_rr(XG, XS)                                                    \
        anncx_rr(W(XG), W(XS))

#define annfx_ld(XG, MS, DS)                                                \
        anncx_ld(W(XG), W(MS), W(DS))

#define annfx3rr(XD, XS, XT)                                                \
        anncx3rr(W(XD), W(XS), W(XT))

#define annfx3ld(XD, XS, MT, DT)                                            \
        anncx3ld(W(XD), W(XS), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrfx_rr(XG, XS)                                                    \
        orrcx_rr(W(XG), W(XS))

#define orrfx_ld(XG, MS, DS)                                                \
        orrcx_ld(W(XG), W(MS), W(DS))

#define orrfx3rr(XD, XS, XT)                                                \
        orrcx3rr(W(XD), W(XS), W(XT))

#define orrfx3ld(XD, XS, MT, DT)                                            \
        orrcx3ld(W(XD), W(XS), W(MT), W(DT))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define ornfx_rr(XG, XS)                                                    \
        orncx_rr(W(XG), W(XS))

#define ornfx_ld(XG, MS, DS)                                                \
        orncx_ld(W(XG), W(MS), W(DS))

#define ornfx3rr(XD, XS, XT)                                                \
        orncx3rr(W(XD), W(XS), W(XT))

#define ornfx3ld(XD, XS, MT, DT)                                            \
        orncx3ld(W(XD), W(XS), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorfx_rr(XG, XS)                                                    \
        xorcx_rr(W(XG), W(XS))

#define xorfx_ld(XG, MS, DS)                                                \
        xorcx_ld(W(XG), W(MS), W(DS))

#define xorfx3rr(XD, XS, XT)                                                \
        xorcx3rr(W(XD), W(XS), W(XT))

#define xorfx3ld(XD, XS, MT, DT)                                            \
        xorcx3ld(W(XD), W(XS), W(MT), W(DT))

/* not (G = ~G), (D = ~S) */

#define notfx_rx(XG)                                                        \
        notcx_rx(W(XG))

#define notfx_rr(XD, XS)                                                    \
        notcx_rr(W(XD), W(XS))

/************   packed single-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negfs_rx(XG)                                                        \
        negcs_rx(W(XG))

#define negfs_rr(XD, XS)                                                    \
        negcs_rr(W(XD), W(XS))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addfs_rr(XG, XS)                                                    \
        addcs_rr(W(XG), W(XS))

#define addfs_ld(XG, MS, DS)                                                \
        addcs_ld(W(XG), W(MS), W(DS))

#define addfs3rr(XD, XS, XT)                                                \
        addcs3rr(W(XD), W(XS), W(XT))

#define addfs3ld(XD, XS, MT, DT)                                            \
        addcs3ld(W(XD), W(XS), W(MT), W(DT))

#define adpfs_rr(XG, XS) /* horizontal pairwise add, first 15-regs only */  \
        adpcs_rr(W(XG), W(XS))

#define adpfs_ld(XG, MS, DS)                                                \
        adpcs_ld(W(XG), W(MS), W(DS))

#define adpfs3rr(XD, XS, XT)                                                \
        adpcs3rr(W(XD), W(XS), W(XT))

#define adpfs3ld(XD, XS, MT, DT)                                            \
        adpcs3ld(W(XD), W(XS), W(MT), W(DT))

#define adhfs_rr(XD, XS) /* horizontal reductive add, first 15-regs only */ \
        adhcs_rr(W(XD), W(XS))

#define adhfs_ld(XD, MS, DS)                                                \
        adhcs_ld(W(XD), W(MS), W(DS))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subfs_rr(XG, XS)                                                    \
        subcs_rr(W(XG), W(XS))

#define subfs_ld(XG, MS, DS)                                                \
        subcs_ld(W(XG), W(MS), W(DS))

#define subfs3rr(XD, XS, XT)                                                \
        subcs3rr(W(XD), W(XS), W(XT))

#define subfs3ld(XD, XS, MT, DT)                                            \
        subcs3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulfs_rr(XG, XS)                                                    \
        mulcs_rr(W(XG), W(XS))

#define mulfs_ld(XG, MS, DS)                                                \
        mulcs_ld(W(XG), W(MS), W(DS))

#define mulfs3rr(XD, XS, XT)                                                \
        mulcs3rr(W(XD), W(XS), W(XT))

#define mulfs3ld(XD, XS, MT, DT)                                            \
        mulcs3ld(W(XD), W(XS), W(MT), W(DT))

#define mlpfs_rr(XG, XS) /* horizontal pairwise mul */                      \
        mlpcs_rr(W(XG), W(XS))

#define mlpfs_ld(XG, MS, DS)                                                \
        mlpcs_ld(W(XG), W(MS), W(DS))

#define mlpfs3rr(XD, XS, XT)                                                \
        mlpcs3rr(W(XD), W(XS), W(XT))

#define mlpfs3ld(XD, XS, MT, DT)                                            \
        mlpcs3ld(W(XD), W(XS), W(MT), W(DT))

#define mlhfs_rr(XD, XS) /* horizontal reductive mul */                     \
        mlhcs_rr(W(XD), W(XS))

#define mlhfs_ld(XD, MS, DS)                                                \
        mlhcs_ld(W(XD), W(MS), W(DS))

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divfs_rr(XG, XS)                                                    \
        divcs_rr(W(XG), W(XS))

#define divfs_ld(XG, MS, DS)                                                \
        divcs_ld(W(XG), W(MS), W(DS))

#define divfs3rr(XD, XS, XT)                                                \
        divcs3rr(W(XD), W(XS), W(XT))

#define divfs3ld(XD, XS, MT, DT)                                            \
        divcs3ld(W(XD), W(XS), W(MT), W(DT))

/* sqr (D = sqrt S) */

#define sqrfs_rr(XD, XS)                                                    \
        sqrcs_rr(W(XD), W(XS))

#define sqrfs_ld(XD, MS, DS)                                                \
        sqrcs_ld(W(XD), W(MS), W(DS))

/* cbr (D = cbrt S) */

#define cbrfs_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbrcs_rr(W(XD), W(X1), W(X2), W(XS))

#define cbefs_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbecs_rr(W(XD), W(X1), W(X2), W(XS))

#define cbsfs_rr(XG, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbscs_rr(W(XG), W(X1), W(X2), W(XS))

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rcpfs_rr(XD, XS) /* destroys XS */                                  \
        rcpcs_rr(W(XD), W(XS))

#define rcefs_rr(XD, XS)                                                    \
        rcecs_rr(W(XD), W(XS))

#define rcsfs_rr(XG, XS) /* destroys XS */                                  \
        rcscs_rr(W(XG), W(XS))

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rsqfs_rr(XD, XS) /* destroys XS */                                  \
        rsqcs_rr(W(XD), W(XS))

#define rsefs_rr(XD, XS)                                                    \
        rsecs_rr(W(XD), W(XS))

#define rssfs_rr(XG, XS) /* destroys XS */                                  \
        rsscs_rr(W(XG), W(XS))

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmafs_rr(XG, XS, XT)                                                \
        fmacs_rr(W(XG), W(XS), W(XT))

#define fmafs_ld(XG, XS, MT, DT)                                            \
        fmacs_ld(W(XG), W(XS), W(MT), W(DT))

#define fmafs3rr(XG, XS, XT)                                                \
        fmafs_rr(W(XG), W(XS), W(XT))

#define fmafs3ld(XG, XS, MT, DT)                                            \
        fmafs_ld(W(XG), W(XS), W(MT), W(DT))

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsfs_rr(XG, XS, XT)                                                \
        fmscs_rr(W(XG), W(XS), W(XT))

#define fmsfs_ld(XG, XS, MT, DT)                                            \
        fmscs_ld(W(XG), W(XS), W(MT), W(DT))

#define fmsfs3rr(XG, XS, XT)                                                \
        fmsfs_rr(W(XG), W(XS), W(XT))

#define fmsfs3ld(XG, XS, MT, DT)                                            \
        fmsfs_ld(W(XG), W(XS), W(MT), W(DT))

/*************   packed single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minfs_rr(XG, XS)                                                    \
        mincs_rr(W(XG), W(XS))

#define minfs_ld(XG, MS, DS)                                                \
        mincs_ld(W(XG), W(MS), W(DS))

#define minfs3rr(XD, XS, XT)                                                \
        mincs3rr(W(XD), W(XS), W(XT))

#define minfs3ld(XD, XS, MT, DT)                                            \
        mincs3ld(W(XD), W(XS), W(MT), W(DT))

#define mnpfs_rr(XG, XS) /* horizontal pairwise min */                      \
        mnpcs_rr(W(XG), W(XS))

#define mnpfs_ld(XG, MS, DS)                                                \
        mnpcs_ld(W(XG), W(MS), W(DS))

#define mnpfs3rr(XD, XS, XT)                                                \
        mnpcs3rr(W(XD), W(XS), W(XT))

#define mnpfs3ld(XD, XS, MT, DT)                                            \
        mnpcs3ld(W(XD), W(XS), W(MT), W(DT))

#define mnhfs_rr(XD, XS) /* horizontal reductive min */                     \
        mnhcs_rr(W(XD), W(XS))

#define mnhfs_ld(XD, MS, DS)                                                \
        mnhcs_ld(W(XD), W(MS), W(DS))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxfs_rr(XG, XS)                                                    \
        maxcs_rr(W(XG), W(XS))

#define maxfs_ld(XG, MS, DS)                                                \
        maxcs_ld(W(XG), W(MS), W(DS))

#define maxfs3rr(XD, XS, XT)                                                \
        maxcs3rr(W(XD), W(XS), W(XT))

#define maxfs3ld(XD, XS, MT, DT)                                            \
        maxcs3ld(W(XD), W(XS), W(MT), W(DT))

#define mxpfs_rr(XG, XS) /* horizontal pairwise max */                      \
        mxpcs_rr(W(XG), W(XS))

#define mxpfs_ld(XG, MS, DS)                                                \
        mxpcs_ld(W(XG), W(MS), W(DS))

#define mxpfs3rr(XD, XS, XT)                                                \
        mxpcs3rr(W(XD), W(XS), W(XT))

#define mxpfs3ld(XD, XS, MT, DT)                                            \
        mxpcs3ld(W(XD), W(XS), W(MT), W(DT))

#define mxhfs_rr(XD, XS) /* horizontal reductive max */                     \
        mxhcs_rr(W(XD), W(XS))

#define mxhfs_ld(XD, MS, DS)                                                \
        mxhcs_ld(W(XD), W(MS), W(DS))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqfs_rr(XG, XS)                                                    \
        ceqcs_rr(W(XG), W(XS))

#define ceqfs_ld(XG, MS, DS)                                                \
        ceqcs_ld(W(XG), W(MS), W(DS))

#define ceqfs3rr(XD, XS, XT)                                                \
        ceqcs3rr(W(XD), W(XS), W(XT))

#define ceqfs3ld(XD, XS, MT, DT)                                            \
        ceqcs3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnefs_rr(XG, XS)                                                    \
        cnecs_rr(W(XG), W(XS))

#define cnefs_ld(XG, MS, DS)                                                \
        cnecs_ld(W(XG), W(MS), W(DS))

#define cnefs3rr(XD, XS, XT)                                                \
        cnecs3rr(W(XD), W(XS), W(XT))

#define cnefs3ld(XD, XS, MT, DT)                                            \
        cnecs3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltfs_rr(XG, XS)                                                    \
        cltcs_rr(W(XG), W(XS))

#define cltfs_ld(XG, MS, DS)                                                \
        cltcs_ld(W(XG), W(MS), W(DS))

#define cltfs3rr(XD, XS, XT)                                                \
        cltcs3rr(W(XD), W(XS), W(XT))

#define cltfs3ld(XD, XS, MT, DT)                                            \
        cltcs3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define clefs_rr(XG, XS)                                                    \
        clecs_rr(W(XG), W(XS))

#define clefs_ld(XG, MS, DS)                                                \
        clecs_ld(W(XG), W(MS), W(DS))

#define clefs3rr(XD, XS, XT)                                                \
        clecs3rr(W(XD), W(XS), W(XT))

#define clefs3ld(XD, XS, MT, DT)                                            \
        clecs3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtfs_rr(XG, XS)                                                    \
        cgtcs_rr(W(XG), W(XS))

#define cgtfs_ld(XG, MS, DS)                                                \
        cgtcs_ld(W(XG), W(MS), W(DS))

#define cgtfs3rr(XD, XS, XT)                                                \
        cgtcs3rr(W(XD), W(XS), W(XT))

#define cgtfs3ld(XD, XS, MT, DT)                                            \
        cgtcs3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgefs_rr(XG, XS)                                                    \
        cgecs_rr(W(XG), W(XS))

#define cgefs_ld(XG, MS, DS)                                                \
        cgecs_ld(W(XG), W(MS), W(DS))

#define cgefs3rr(XD, XS, XT)                                                \
        cgecs3rr(W(XD), W(XS), W(XT))

#define cgefs3ld(XD, XS, MT, DT)                                            \
        cgecs3ld(W(XD), W(XS), W(MT), W(DT))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define mkjfx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        mkjcx_rx(W(XS), mask, lb)

/*************   packed single-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzfs_rr(XD, XS)     /* round towards zero */                       \
        rnzcs_rr(W(XD), W(XS))

#define rnzfs_ld(XD, MS, DS) /* round towards zero */                       \
        rnzcs_ld(W(XD), W(MS), W(DS))

#define cvzfs_rr(XD, XS)     /* round towards zero */                       \
        cvzcs_rr(W(XD), W(XS))

#define cvzfs_ld(XD, MS, DS) /* round towards zero */                       \
        cvzcs_ld(W(XD), W(MS), W(DS))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpfs_rr(XD, XS)     /* round towards +inf */                       \
        rnpcs_rr(W(XD), W(XS))

#define rnpfs_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpcs_ld(W(XD), W(MS), W(DS))

#define cvpfs_rr(XD, XS)     /* round towards +inf */                       \
        cvpcs_rr(W(XD), W(XS))

#define cvpfs_ld(XD, MS, DS) /* round towards +inf */                       \
        cvpcs_ld(W(XD), W(MS), W(DS))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmfs_rr(XD, XS)     /* round towards -inf */                       \
        rnmcs_rr(W(XD), W(XS))

#define rnmfs_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmcs_ld(W(XD), W(MS), W(DS))

#define cvmfs_rr(XD, XS)     /* round towards -inf */                       \
        cvmcs_rr(W(XD), W(XS))

#define cvmfs_ld(XD, MS, DS) /* round towards -inf */                       \
        cvmcs_ld(W(XD), W(MS), W(DS))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnfs_rr(XD, XS)     /* round towards near */                       \
        rnncs_rr(W(XD), W(XS))

#define rnnfs_ld(XD, MS, DS) /* round towards near */                       \
        rnncs_ld(W(XD), W(MS), W(DS))

#define cvnfs_rr(XD, XS)     /* round towards near */                       \
        cvncs_rr(W(XD), W(XS))

#define cvnfs_ld(XD, MS, DS) /* round towards near */                       \
        cvncs_ld(W(XD), W(MS), W(DS))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnfn_rr(XD, XS)     /* round towards near */                       \
        cvncn_rr(W(XD), W(XS))

#define cvnfn_ld(XD, MS, DS) /* round towards near */                       \
        cvncn_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndfs_rr(XD, XS)                                                    \
        rndcs_rr(W(XD), W(XS))

#define rndfs_ld(XD, MS, DS)                                                \
        rndcs_ld(W(XD), W(MS), W(DS))

#define cvtfs_rr(XD, XS)                                                    \
        cvtcs_rr(W(XD), W(XS))

#define cvtfs_ld(XD, MS, DS)                                                \
        cvtcs_ld(W(XD), W(MS), W(DS))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtfn_rr(XD, XS)                                                    \
        cvtcn_rr(W(XD), W(XS))

#define cvtfn_ld(XD, MS, DS)                                                \
        cvtcn_ld(W(XD), W(MS), W(DS))

/* cvn (D = unsigned-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnfx_rr(XD, XS)     /* round towards near */                       \
        cvncx_rr(W(XD), W(XS))

#define cvnfx_ld(XD, MS, DS) /* round towards near */                       \
        cvncx_ld(W(XD), W(MS), W(DS))

/* cvt (D = unsigned-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtfx_rr(XD, XS)                                                    \
        cvtcx_rr(W(XD), W(XS))

#define cvtfx_ld(XD, MS, DS)                                                \
        cvtcx_ld(W(XD), W(MS), W(DS))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnrfs_rr(XD, XS, mode)                                              \
        rnrcs_rr(W(XD), W(XS), mode)

#define cvrfs_rr(XD, XS, mode)                                              \
        cvrcs_rr(W(XD), W(XS), mode)

/************   packed single-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addfx_rr(XG, XS)                                                    \
        addcx_rr(W(XG), W(XS))

#define addfx_ld(XG, MS, DS)                                                \
        addcx_ld(W(XG), W(MS), W(DS))

#define addfx3rr(XD, XS, XT)                                                \
        addcx3rr(W(XD), W(XS), W(XT))

#define addfx3ld(XD, XS, MT, DT)                                            \
        addcx3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subfx_rr(XG, XS)                                                    \
        subcx_rr(W(XG), W(XS))

#define subfx_ld(XG, MS, DS)                                                \
        subcx_ld(W(XG), W(MS), W(DS))

#define subfx3rr(XD, XS, XT)                                                \
        subcx3rr(W(XD), W(XS), W(XT))

#define subfx3ld(XD, XS, MT, DT)                                            \
        subcx3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulfx_rr(XG, XS)                                                    \
        mulcx_rr(W(XG), W(XS))

#define mulfx_ld(XG, MS, DS)                                                \
        mulcx_ld(W(XG), W(MS), W(DS))

#define mulfx3rr(XD, XS, XT)                                                \
        mulcx3rr(W(XD), W(XS), W(XT))

#define mulfx3ld(XD, XS, MT, DT)                                            \
        mulcx3ld(W(XD), W(XS), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlfx_ri(XG, IS)                                                    \
        shlcx_ri(W(XG), W(IS))

#define shlfx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlcx_ld(W(XG), W(MS), W(DS))

#define shlfx3ri(XD, XS, IT)                                                \
        shlcx3ri(W(XD), W(XS), W(IT))

#define shlfx3ld(XD, XS, MT, DT)                                            \
        shlcx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrfx_ri(XG, IS)                                                    \
        shrcx_ri(W(XG), W(IS))

#define shrfx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrcx_ld(W(XG), W(MS), W(DS))

#define shrfx3ri(XD, XS, IT)                                                \
        shrcx3ri(W(XD), W(XS), W(IT))

#define shrfx3ld(XD, XS, MT, DT)                                            \
        shrcx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrfn_ri(XG, IS)                                                    \
        shrcn_ri(W(XG), W(IS))

#define shrfn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrcn_ld(W(XG), W(MS), W(DS))

#define shrfn3ri(XD, XS, IT)                                                \
        shrcn3ri(W(XD), W(XS), W(IT))

#define shrfn3ld(XD, XS, MT, DT)                                            \
        shrcn3ld(W(XD), W(XS), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlfx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlcx_rr(W(XG), W(XS))

#define svlfx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlcx_ld(W(XG), W(MS), W(DS))

#define svlfx3rr(XD, XS, XT)                                                \
        svlcx3rr(W(XD), W(XS), W(XT))

#define svlfx3ld(XD, XS, MT, DT)                                            \
        svlcx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrfx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrcx_rr(W(XG), W(XS))

#define svrfx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrcx_ld(W(XG), W(MS), W(DS))

#define svrfx3rr(XD, XS, XT)                                                \
        svrcx3rr(W(XD), W(XS), W(XT))

#define svrfx3ld(XD, XS, MT, DT)                                            \
        svrcx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrfn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrcn_rr(W(XG), W(XS))

#define svrfn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrcn_ld(W(XG), W(MS), W(DS))

#define svrfn3rr(XD, XS, XT)                                                \
        svrcn3rr(W(XD), W(XS), W(XT))

#define svrfn3ld(XD, XS, MT, DT)                                            \
        svrcn3ld(W(XD), W(XS), W(MT), W(DT))

/****************   packed single-precision integer compare   *****************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minfx_rr(XG, XS)                                                    \
        mincx_rr(W(XG), W(XS))

#define minfx_ld(XG, MS, DS)                                                \
        mincx_ld(W(XG), W(MS), W(DS))

#define minfx3rr(XD, XS, XT)                                                \
        mincx3rr(W(XD), W(XS), W(XT))

#define minfx3ld(XD, XS, MT, DT)                                            \
        mincx3ld(W(XD), W(XS), W(MT), W(DT))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minfn_rr(XG, XS)                                                    \
        mincn_rr(W(XG), W(XS))

#define minfn_ld(XG, MS, DS)                                                \
        mincn_ld(W(XG), W(MS), W(DS))

#define minfn3rr(XD, XS, XT)                                                \
        mincn3rr(W(XD), W(XS), W(XT))

#define minfn3ld(XD, XS, MT, DT)                                            \
        mincn3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxfx_rr(XG, XS)                                                    \
        maxcx_rr(W(XG), W(XS))

#define maxfx_ld(XG, MS, DS)                                                \
        maxcx_ld(W(XG), W(MS), W(DS))

#define maxfx3rr(XD, XS, XT)                                                \
        maxcx3rr(W(XD), W(XS), W(XT))

#define maxfx3ld(XD, XS, MT, DT)                                            \
        maxcx3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxfn_rr(XG, XS)                                                    \
        maxcn_rr(W(XG), W(XS))

#define maxfn_ld(XG, MS, DS)                                                \
        maxcn_ld(W(XG), W(MS), W(DS))

#define maxfn3rr(XD, XS, XT)                                                \
        maxcn3rr(W(XD), W(XS), W(XT))

#define maxfn3ld(XD, XS, MT, DT)                                            \
        maxcn3ld(W(XD), W(XS), W(MT), W(DT))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqfx_rr(XG, XS)                                                    \
        ceqcx_rr(W(XG), W(XS))

#define ceqfx_ld(XG, MS, DS)                                                \
        ceqcx_ld(W(XG), W(MS), W(DS))

#define ceqfx3rr(XD, XS, XT)                                                \
        ceqcx3rr(W(XD), W(XS), W(XT))

#define ceqfx3ld(XD, XS, MT, DT)                                            \
        ceqcx3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnefx_rr(XG, XS)                                                    \
        cnecx_rr(W(XG), W(XS))

#define cnefx_ld(XG, MS, DS)                                                \
        cnecx_ld(W(XG), W(MS), W(DS))

#define cnefx3rr(XD, XS, XT)                                                \
        cnecx3rr(W(XD), W(XS), W(XT))

#define cnefx3ld(XD, XS, MT, DT)                                            \
        cnecx3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltfx_rr(XG, XS)                                                    \
        cltcx_rr(W(XG), W(XS))

#define cltfx_ld(XG, MS, DS)                                                \
        cltcx_ld(W(XG), W(MS), W(DS))

#define cltfx3rr(XD, XS, XT)                                                \
        cltcx3rr(W(XD), W(XS), W(XT))

#define cltfx3ld(XD, XS, MT, DT)                                            \
        cltcx3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltfn_rr(XG, XS)                                                    \
        cltcn_rr(W(XG), W(XS))

#define cltfn_ld(XG, MS, DS)                                                \
        cltcn_ld(W(XG), W(MS), W(DS))

#define cltfn3rr(XD, XS, XT)                                                \
        cltcn3rr(W(XD), W(XS), W(XT))

#define cltfn3ld(XD, XS, MT, DT)                                            \
        cltcn3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clefx_rr(XG, XS)                                                    \
        clecx_rr(W(XG), W(XS))

#define clefx_ld(XG, MS, DS)                                                \
        clecx_ld(W(XG), W(MS), W(DS))

#define clefx3rr(XD, XS, XT)                                                \
        clecx3rr(W(XD), W(XS), W(XT))

#define clefx3ld(XD, XS, MT, DT)                                            \
        clecx3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clefn_rr(XG, XS)                                                    \
        clecn_rr(W(XG), W(XS))

#define clefn_ld(XG, MS, DS)                                                \
        clecn_ld(W(XG), W(MS), W(DS))

#define clefn3rr(XD, XS, XT)                                                \
        clecn3rr(W(XD), W(XS), W(XT))

#define clefn3ld(XD, XS, MT, DT)                                            \
        clecn3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtfx_rr(XG, XS)                                                    \
        cgtcx_rr(W(XG), W(XS))

#define cgtfx_ld(XG, MS, DS)                                                \
        cgtcx_ld(W(XG), W(MS), W(DS))

#define cgtfx3rr(XD, XS, XT)                                                \
        cgtcx3rr(W(XD), W(XS), W(XT))

#define cgtfx3ld(XD, XS, MT, DT)                                            \
        cgtcx3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtfn_rr(XG, XS)                                                    \
        cgtcn_rr(W(XG), W(XS))

#define cgtfn_ld(XG, MS, DS)                                                \
        cgtcn_ld(W(XG), W(MS), W(DS))

#define cgtfn3rr(XD, XS, XT)                                                \
        cgtcn3rr(W(XD), W(XS), W(XT))

#define cgtfn3ld(XD, XS, MT, DT)                                            \
        cgtcn3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgefx_rr(XG, XS)                                                    \
        cgecx_rr(W(XG), W(XS))

#define cgefx_ld(XG, MS, DS)                                                \
        cgecx_ld(W(XG), W(MS), W(DS))

#define cgefx3rr(XD, XS, XT)                                                \
        cgecx3rr(W(XD), W(XS), W(XT))

#define cgefx3ld(XD, XS, MT, DT)                                            \
        cgecx3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgefn_rr(XG, XS)                                                    \
        cgecn_rr(W(XG), W(XS))

#define cgefn_ld(XG, MS, DS)                                                \
        cgecn_ld(W(XG), W(MS), W(DS))

#define cgefn3rr(XD, XS, XT)                                                \
        cgecn3rr(W(XD), W(XS), W(XT))

#define cgefn3ld(XD, XS, MT, DT)                                            \
        cgecn3ld(W(XD), W(XS), W(MT), W(DT))

/******************************************************************************/
/**** 128-bit **** SIMD instructions with configurable element **** 32-bit ****/
/******************************************************************************/

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmlx_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        elmix_st(W(XS), W(MD), W(DD))

/***************   packed single-precision generic move/logic   ***************/

/* mov (D = S) */

#define movlx_rr(XD, XS)                                                    \
        movix_rr(W(XD), W(XS))

#define movlx_ld(XD, MS, DS)                                                \
        movix_ld(W(XD), W(MS), W(DS))

#define movlx_st(XS, MD, DD)                                                \
        movix_st(W(XS), W(MD), W(DD))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvlx_rr(XD, XS)                                                    \
        mmvix_rr(W(XD), W(XS))

#define mmvlx_ld(XG, MS, DS)                                                \
        mmvix_ld(W(XG), W(MS), W(DS))

#define mmvlx_st(XS, MG, DG)                                                \
        mmvix_st(W(XS), W(MG), W(DG))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andlx_rr(XG, XS)                                                    \
        andix_rr(W(XG), W(XS))

#define andlx_ld(XG, MS, DS)                                                \
        andix_ld(W(XG), W(MS), W(DS))

#define andlx3rr(XD, XS, XT)                                                \
        andix3rr(W(XD), W(XS), W(XT))

#define andlx3ld(XD, XS, MT, DT)                                            \
        andix3ld(W(XD), W(XS), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annlx_rr(XG, XS)                                                    \
        annix_rr(W(XG), W(XS))

#define annlx_ld(XG, MS, DS)                                                \
        annix_ld(W(XG), W(MS), W(DS))

#define annlx3rr(XD, XS, XT)                                                \
        annix3rr(W(XD), W(XS), W(XT))

#define annlx3ld(XD, XS, MT, DT)                                            \
        annix3ld(W(XD), W(XS), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrlx_rr(XG, XS)                                                    \
        orrix_rr(W(XG), W(XS))

#define orrlx_ld(XG, MS, DS)                                                \
        orrix_ld(W(XG), W(MS), W(DS))

#define orrlx3rr(XD, XS, XT)                                                \
        orrix3rr(W(XD), W(XS), W(XT))

#define orrlx3ld(XD, XS, MT, DT)                                            \
        orrix3ld(W(XD), W(XS), W(MT), W(DT))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define ornlx_rr(XG, XS)                                                    \
        ornix_rr(W(XG), W(XS))

#define ornlx_ld(XG, MS, DS)                                                \
        ornix_ld(W(XG), W(MS), W(DS))

#define ornlx3rr(XD, XS, XT)                                                \
        ornix3rr(W(XD), W(XS), W(XT))

#define ornlx3ld(XD, XS, MT, DT)                                            \
        ornix3ld(W(XD), W(XS), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorlx_rr(XG, XS)                                                    \
        xorix_rr(W(XG), W(XS))

#define xorlx_ld(XG, MS, DS)                                                \
        xorix_ld(W(XG), W(MS), W(DS))

#define xorlx3rr(XD, XS, XT)                                                \
        xorix3rr(W(XD), W(XS), W(XT))

#define xorlx3ld(XD, XS, MT, DT)                                            \
        xorix3ld(W(XD), W(XS), W(MT), W(DT))

/* not (G = ~G), (D = ~S) */

#define notlx_rx(XG)                                                        \
        notix_rx(W(XG))

#define notlx_rr(XD, XS)                                                    \
        notix_rr(W(XD), W(XS))

/************   packed single-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negls_rx(XG)                                                        \
        negis_rx(W(XG))

#define negls_rr(XD, XS)                                                    \
        negis_rr(W(XD), W(XS))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addls_rr(XG, XS)                                                    \
        addis_rr(W(XG), W(XS))

#define addls_ld(XG, MS, DS)                                                \
        addis_ld(W(XG), W(MS), W(DS))

#define addls3rr(XD, XS, XT)                                                \
        addis3rr(W(XD), W(XS), W(XT))

#define addls3ld(XD, XS, MT, DT)                                            \
        addis3ld(W(XD), W(XS), W(MT), W(DT))

#define adpls_rr(XG, XS) /* horizontal pairwise add, first 15-regs only */  \
        adpis_rr(W(XG), W(XS))

#define adpls_ld(XG, MS, DS)                                                \
        adpis_ld(W(XG), W(MS), W(DS))

#define adpls3rr(XD, XS, XT)                                                \
        adpis3rr(W(XD), W(XS), W(XT))

#define adpls3ld(XD, XS, MT, DT)                                            \
        adpis3ld(W(XD), W(XS), W(MT), W(DT))

#define adhls_rr(XD, XS) /* horizontal reductive add, first 15-regs only */ \
        adhis_rr(W(XD), W(XS))

#define adhls_ld(XD, MS, DS)                                                \
        adhis_ld(W(XD), W(MS), W(DS))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subls_rr(XG, XS)                                                    \
        subis_rr(W(XG), W(XS))

#define subls_ld(XG, MS, DS)                                                \
        subis_ld(W(XG), W(MS), W(DS))

#define subls3rr(XD, XS, XT)                                                \
        subis3rr(W(XD), W(XS), W(XT))

#define subls3ld(XD, XS, MT, DT)                                            \
        subis3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulls_rr(XG, XS)                                                    \
        mulis_rr(W(XG), W(XS))

#define mulls_ld(XG, MS, DS)                                                \
        mulis_ld(W(XG), W(MS), W(DS))

#define mulls3rr(XD, XS, XT)                                                \
        mulis3rr(W(XD), W(XS), W(XT))

#define mulls3ld(XD, XS, MT, DT)                                            \
        mulis3ld(W(XD), W(XS), W(MT), W(DT))

#define mlpls_rr(XG, XS) /* horizontal pairwise mul */                      \
        mlpis_rr(W(XG), W(XS))

#define mlpls_ld(XG, MS, DS)                                                \
        mlpis_ld(W(XG), W(MS), W(DS))

#define mlpls3rr(XD, XS, XT)                                                \
        mlpis3rr(W(XD), W(XS), W(XT))

#define mlpls3ld(XD, XS, MT, DT)                                            \
        mlpis3ld(W(XD), W(XS), W(MT), W(DT))

#define mlhls_rr(XD, XS) /* horizontal reductive mul */                     \
        mlhis_rr(W(XD), W(XS))

#define mlhls_ld(XD, MS, DS)                                                \
        mlhis_ld(W(XD), W(MS), W(DS))

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divls_rr(XG, XS)                                                    \
        divis_rr(W(XG), W(XS))

#define divls_ld(XG, MS, DS)                                                \
        divis_ld(W(XG), W(MS), W(DS))

#define divls3rr(XD, XS, XT)                                                \
        divis3rr(W(XD), W(XS), W(XT))

#define divls3ld(XD, XS, MT, DT)                                            \
        divis3ld(W(XD), W(XS), W(MT), W(DT))

/* sqr (D = sqrt S) */

#define sqrls_rr(XD, XS)                                                    \
        sqris_rr(W(XD), W(XS))

#define sqrls_ld(XD, MS, DS)                                                \
        sqris_ld(W(XD), W(MS), W(DS))

/* cbr (D = cbrt S) */

#define cbrls_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbris_rr(W(XD), W(X1), W(X2), W(XS))

#define cbels_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbeis_rr(W(XD), W(X1), W(X2), W(XS))

#define cbsls_rr(XG, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbsis_rr(W(XG), W(X1), W(X2), W(XS))

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rcpls_rr(XD, XS) /* destroys XS */                                  \
        rcpis_rr(W(XD), W(XS))

#define rcels_rr(XD, XS)                                                    \
        rceis_rr(W(XD), W(XS))

#define rcsls_rr(XG, XS) /* destroys XS */                                  \
        rcsis_rr(W(XG), W(XS))

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rsqls_rr(XD, XS) /* destroys XS */                                  \
        rsqis_rr(W(XD), W(XS))

#define rsels_rr(XD, XS)                                                    \
        rseis_rr(W(XD), W(XS))

#define rssls_rr(XG, XS) /* destroys XS */                                  \
        rssis_rr(W(XG), W(XS))

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmals_rr(XG, XS, XT)                                                \
        fmais_rr(W(XG), W(XS), W(XT))

#define fmals_ld(XG, XS, MT, DT)                                            \
        fmais_ld(W(XG), W(XS), W(MT), W(DT))

#define fmals3rr(XG, XS, XT)                                                \
        fmals_rr(W(XG), W(XS), W(XT))

#define fmals3ld(XG, XS, MT, DT)                                            \
        fmals_ld(W(XG), W(XS), W(MT), W(DT))

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsls_rr(XG, XS, XT)                                                \
        fmsis_rr(W(XG), W(XS), W(XT))

#define fmsls_ld(XG, XS, MT, DT)                                            \
        fmsis_ld(W(XG), W(XS), W(MT), W(DT))

#define fmsls3rr(XG, XS, XT)                                                \
        fmsls_rr(W(XG), W(XS), W(XT))

#define fmsls3ld(XG, XS, MT, DT)                                            \
        fmsls_ld(W(XG), W(XS), W(MT), W(DT))

/*************   packed single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minls_rr(XG, XS)                                                    \
        minis_rr(W(XG), W(XS))

#define minls_ld(XG, MS, DS)                                                \
        minis_ld(W(XG), W(MS), W(DS))

#define minls3rr(XD, XS, XT)                                                \
        minis3rr(W(XD), W(XS), W(XT))

#define minls3ld(XD, XS, MT, DT)                                            \
        minis3ld(W(XD), W(XS), W(MT), W(DT))

#define mnpls_rr(XG, XS) /* horizontal pairwise min */                      \
        mnpis_rr(W(XG), W(XS))

#define mnpls_ld(XG, MS, DS)                                                \
        mnpis_ld(W(XG), W(MS), W(DS))

#define mnpls3rr(XD, XS, XT)                                                \
        mnpis3rr(W(XD), W(XS), W(XT))

#define mnpls3ld(XD, XS, MT, DT)                                            \
        mnpis3ld(W(XD), W(XS), W(MT), W(DT))

#define mnhls_rr(XD, XS) /* horizontal reductive min */                     \
        mnhis_rr(W(XD), W(XS))

#define mnhls_ld(XD, MS, DS)                                                \
        mnhis_ld(W(XD), W(MS), W(DS))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxls_rr(XG, XS)                                                    \
        maxis_rr(W(XG), W(XS))

#define maxls_ld(XG, MS, DS)                                                \
        maxis_ld(W(XG), W(MS), W(DS))

#define maxls3rr(XD, XS, XT)                                                \
        maxis3rr(W(XD), W(XS), W(XT))

#define maxls3ld(XD, XS, MT, DT)                                            \
        maxis3ld(W(XD), W(XS), W(MT), W(DT))

#define mxpls_rr(XG, XS) /* horizontal pairwise max */                      \
        mxpis_rr(W(XG), W(XS))

#define mxpls_ld(XG, MS, DS)                                                \
        mxpis_ld(W(XG), W(MS), W(DS))

#define mxpls3rr(XD, XS, XT)                                                \
        mxpis3rr(W(XD), W(XS), W(XT))

#define mxpls3ld(XD, XS, MT, DT)                                            \
        mxpis3ld(W(XD), W(XS), W(MT), W(DT))

#define mxhls_rr(XD, XS) /* horizontal reductive max */                     \
        mxhis_rr(W(XD), W(XS))

#define mxhls_ld(XD, MS, DS)                                                \
        mxhis_ld(W(XD), W(MS), W(DS))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqls_rr(XG, XS)                                                    \
        ceqis_rr(W(XG), W(XS))

#define ceqls_ld(XG, MS, DS)                                                \
        ceqis_ld(W(XG), W(MS), W(DS))

#define ceqls3rr(XD, XS, XT)                                                \
        ceqis3rr(W(XD), W(XS), W(XT))

#define ceqls3ld(XD, XS, MT, DT)                                            \
        ceqis3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnels_rr(XG, XS)                                                    \
        cneis_rr(W(XG), W(XS))

#define cnels_ld(XG, MS, DS)                                                \
        cneis_ld(W(XG), W(MS), W(DS))

#define cnels3rr(XD, XS, XT)                                                \
        cneis3rr(W(XD), W(XS), W(XT))

#define cnels3ld(XD, XS, MT, DT)                                            \
        cneis3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltls_rr(XG, XS)                                                    \
        cltis_rr(W(XG), W(XS))

#define cltls_ld(XG, MS, DS)                                                \
        cltis_ld(W(XG), W(MS), W(DS))

#define cltls3rr(XD, XS, XT)                                                \
        cltis3rr(W(XD), W(XS), W(XT))

#define cltls3ld(XD, XS, MT, DT)                                            \
        cltis3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define clels_rr(XG, XS)                                                    \
        cleis_rr(W(XG), W(XS))

#define clels_ld(XG, MS, DS)                                                \
        cleis_ld(W(XG), W(MS), W(DS))

#define clels3rr(XD, XS, XT)                                                \
        cleis3rr(W(XD), W(XS), W(XT))

#define clels3ld(XD, XS, MT, DT)                                            \
        cleis3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtls_rr(XG, XS)                                                    \
        cgtis_rr(W(XG), W(XS))

#define cgtls_ld(XG, MS, DS)                                                \
        cgtis_ld(W(XG), W(MS), W(DS))

#define cgtls3rr(XD, XS, XT)                                                \
        cgtis3rr(W(XD), W(XS), W(XT))

#define cgtls3ld(XD, XS, MT, DT)                                            \
        cgtis3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgels_rr(XG, XS)                                                    \
        cgeis_rr(W(XG), W(XS))

#define cgels_ld(XG, MS, DS)                                                \
        cgeis_ld(W(XG), W(MS), W(DS))

#define cgels3rr(XD, XS, XT)                                                \
        cgeis3rr(W(XD), W(XS), W(XT))

#define cgels3ld(XD, XS, MT, DT)                                            \
        cgeis3ld(W(XD), W(XS), W(MT), W(DT))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define mkjlx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        mkjix_rx(W(XS), mask, lb)

/*************   packed single-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzls_rr(XD, XS)     /* round towards zero */                       \
        rnzis_rr(W(XD), W(XS))

#define rnzls_ld(XD, MS, DS) /* round towards zero */                       \
        rnzis_ld(W(XD), W(MS), W(DS))

#define cvzls_rr(XD, XS)     /* round towards zero */                       \
        cvzis_rr(W(XD), W(XS))

#define cvzls_ld(XD, MS, DS) /* round towards zero */                       \
        cvzis_ld(W(XD), W(MS), W(DS))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpls_rr(XD, XS)     /* round towards +inf */                       \
        rnpis_rr(W(XD), W(XS))

#define rnpls_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpis_ld(W(XD), W(MS), W(DS))

#define cvpls_rr(XD, XS)     /* round towards +inf */                       \
        cvpis_rr(W(XD), W(XS))

#define cvpls_ld(XD, MS, DS) /* round towards +inf */                       \
        cvpis_ld(W(XD), W(MS), W(DS))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmls_rr(XD, XS)     /* round towards -inf */                       \
        rnmis_rr(W(XD), W(XS))

#define rnmls_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmis_ld(W(XD), W(MS), W(DS))

#define cvmls_rr(XD, XS)     /* round towards -inf */                       \
        cvmis_rr(W(XD), W(XS))

#define cvmls_ld(XD, MS, DS) /* round towards -inf */                       \
        cvmis_ld(W(XD), W(MS), W(DS))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnls_rr(XD, XS)     /* round towards near */                       \
        rnnis_rr(W(XD), W(XS))

#define rnnls_ld(XD, MS, DS) /* round towards near */                       \
        rnnis_ld(W(XD), W(MS), W(DS))

#define cvnls_rr(XD, XS)     /* round towards near */                       \
        cvnis_rr(W(XD), W(XS))

#define cvnls_ld(XD, MS, DS) /* round towards near */                       \
        cvnis_ld(W(XD), W(MS), W(DS))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnln_rr(XD, XS)     /* round towards near */                       \
        cvnin_rr(W(XD), W(XS))

#define cvnln_ld(XD, MS, DS) /* round towards near */                       \
        cvnin_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndls_rr(XD, XS)                                                    \
        rndis_rr(W(XD), W(XS))

#define rndls_ld(XD, MS, DS)                                                \
        rndis_ld(W(XD), W(MS), W(DS))

#define cvtls_rr(XD, XS)                                                    \
        cvtis_rr(W(XD), W(XS))

#define cvtls_ld(XD, MS, DS)                                                \
        cvtis_ld(W(XD), W(MS), W(DS))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtln_rr(XD, XS)                                                    \
        cvtin_rr(W(XD), W(XS))

#define cvtln_ld(XD, MS, DS)                                                \
        cvtin_ld(W(XD), W(MS), W(DS))

/* cvn (D = unsigned-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnlx_rr(XD, XS)     /* round towards near */                       \
        cvnix_rr(W(XD), W(XS))

#define cvnlx_ld(XD, MS, DS) /* round towards near */                       \
        cvnix_ld(W(XD), W(MS), W(DS))

/* cvt (D = unsigned-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtlx_rr(XD, XS)                                                    \
        cvtix_rr(W(XD), W(XS))

#define cvtlx_ld(XD, MS, DS)                                                \
        cvtix_ld(W(XD), W(MS), W(DS))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnrls_rr(XD, XS, mode)                                              \
        rnris_rr(W(XD), W(XS), mode)

#define cvrls_rr(XD, XS, mode)                                              \
        cvris_rr(W(XD), W(XS), mode)

/************   packed single-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addlx_rr(XG, XS)                                                    \
        addix_rr(W(XG), W(XS))

#define addlx_ld(XG, MS, DS)                                                \
        addix_ld(W(XG), W(MS), W(DS))

#define addlx3rr(XD, XS, XT)                                                \
        addix3rr(W(XD), W(XS), W(XT))

#define addlx3ld(XD, XS, MT, DT)                                            \
        addix3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define sublx_rr(XG, XS)                                                    \
        subix_rr(W(XG), W(XS))

#define sublx_ld(XG, MS, DS)                                                \
        subix_ld(W(XG), W(MS), W(DS))

#define sublx3rr(XD, XS, XT)                                                \
        subix3rr(W(XD), W(XS), W(XT))

#define sublx3ld(XD, XS, MT, DT)                                            \
        subix3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mullx_rr(XG, XS)                                                    \
        mulix_rr(W(XG), W(XS))

#define mullx_ld(XG, MS, DS)                                                \
        mulix_ld(W(XG), W(MS), W(DS))

#define mullx3rr(XD, XS, XT)                                                \
        mulix3rr(W(XD), W(XS), W(XT))

#define mullx3ld(XD, XS, MT, DT)                                            \
        mulix3ld(W(XD), W(XS), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shllx_ri(XG, IS)                                                    \
        shlix_ri(W(XG), W(IS))

#define shllx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlix_ld(W(XG), W(MS), W(DS))

#define shllx3ri(XD, XS, IT)                                                \
        shlix3ri(W(XD), W(XS), W(IT))

#define shllx3ld(XD, XS, MT, DT)                                            \
        shlix3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrlx_ri(XG, IS)                                                    \
        shrix_ri(W(XG), W(IS))

#define shrlx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrix_ld(W(XG), W(MS), W(DS))

#define shrlx3ri(XD, XS, IT)                                                \
        shrix3ri(W(XD), W(XS), W(IT))

#define shrlx3ld(XD, XS, MT, DT)                                            \
        shrix3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrln_ri(XG, IS)                                                    \
        shrin_ri(W(XG), W(IS))

#define shrln_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrin_ld(W(XG), W(MS), W(DS))

#define shrln3ri(XD, XS, IT)                                                \
        shrin3ri(W(XD), W(XS), W(IT))

#define shrln3ld(XD, XS, MT, DT)                                            \
        shrin3ld(W(XD), W(XS), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svllx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlix_rr(W(XG), W(XS))

#define svllx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlix_ld(W(XG), W(MS), W(DS))

#define svllx3rr(XD, XS, XT)                                                \
        svlix3rr(W(XD), W(XS), W(XT))

#define svllx3ld(XD, XS, MT, DT)                                            \
        svlix3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrlx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrix_rr(W(XG), W(XS))

#define svrlx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrix_ld(W(XG), W(MS), W(DS))

#define svrlx3rr(XD, XS, XT)                                                \
        svrix3rr(W(XD), W(XS), W(XT))

#define svrlx3ld(XD, XS, MT, DT)                                            \
        svrix3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrln_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrin_rr(W(XG), W(XS))

#define svrln_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrin_ld(W(XG), W(MS), W(DS))

#define svrln3rr(XD, XS, XT)                                                \
        svrin3rr(W(XD), W(XS), W(XT))

#define svrln3ld(XD, XS, MT, DT)                                            \
        svrin3ld(W(XD), W(XS), W(MT), W(DT))

/****************   packed single-precision integer compare   *****************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minlx_rr(XG, XS)                                                    \
        minix_rr(W(XG), W(XS))

#define minlx_ld(XG, MS, DS)                                                \
        minix_ld(W(XG), W(MS), W(DS))

#define minlx3rr(XD, XS, XT)                                                \
        minix3rr(W(XD), W(XS), W(XT))

#define minlx3ld(XD, XS, MT, DT)                                            \
        minix3ld(W(XD), W(XS), W(MT), W(DT))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minln_rr(XG, XS)                                                    \
        minin_rr(W(XG), W(XS))

#define minln_ld(XG, MS, DS)                                                \
        minin_ld(W(XG), W(MS), W(DS))

#define minln3rr(XD, XS, XT)                                                \
        minin3rr(W(XD), W(XS), W(XT))

#define minln3ld(XD, XS, MT, DT)                                            \
        minin3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxlx_rr(XG, XS)                                                    \
        maxix_rr(W(XG), W(XS))

#define maxlx_ld(XG, MS, DS)                                                \
        maxix_ld(W(XG), W(MS), W(DS))

#define maxlx3rr(XD, XS, XT)                                                \
        maxix3rr(W(XD), W(XS), W(XT))

#define maxlx3ld(XD, XS, MT, DT)                                            \
        maxix3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxln_rr(XG, XS)                                                    \
        maxin_rr(W(XG), W(XS))

#define maxln_ld(XG, MS, DS)                                                \
        maxin_ld(W(XG), W(MS), W(DS))

#define maxln3rr(XD, XS, XT)                                                \
        maxin3rr(W(XD), W(XS), W(XT))

#define maxln3ld(XD, XS, MT, DT)                                            \
        maxin3ld(W(XD), W(XS), W(MT), W(DT))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqlx_rr(XG, XS)                                                    \
        ceqix_rr(W(XG), W(XS))

#define ceqlx_ld(XG, MS, DS)                                                \
        ceqix_ld(W(XG), W(MS), W(DS))

#define ceqlx3rr(XD, XS, XT)                                                \
        ceqix3rr(W(XD), W(XS), W(XT))

#define ceqlx3ld(XD, XS, MT, DT)                                            \
        ceqix3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnelx_rr(XG, XS)                                                    \
        cneix_rr(W(XG), W(XS))

#define cnelx_ld(XG, MS, DS)                                                \
        cneix_ld(W(XG), W(MS), W(DS))

#define cnelx3rr(XD, XS, XT)                                                \
        cneix3rr(W(XD), W(XS), W(XT))

#define cnelx3ld(XD, XS, MT, DT)                                            \
        cneix3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltlx_rr(XG, XS)                                                    \
        cltix_rr(W(XG), W(XS))

#define cltlx_ld(XG, MS, DS)                                                \
        cltix_ld(W(XG), W(MS), W(DS))

#define cltlx3rr(XD, XS, XT)                                                \
        cltix3rr(W(XD), W(XS), W(XT))

#define cltlx3ld(XD, XS, MT, DT)                                            \
        cltix3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltln_rr(XG, XS)                                                    \
        cltin_rr(W(XG), W(XS))

#define cltln_ld(XG, MS, DS)                                                \
        cltin_ld(W(XG), W(MS), W(DS))

#define cltln3rr(XD, XS, XT)                                                \
        cltin3rr(W(XD), W(XS), W(XT))

#define cltln3ld(XD, XS, MT, DT)                                            \
        cltin3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clelx_rr(XG, XS)                                                    \
        cleix_rr(W(XG), W(XS))

#define clelx_ld(XG, MS, DS)                                                \
        cleix_ld(W(XG), W(MS), W(DS))

#define clelx3rr(XD, XS, XT)                                                \
        cleix3rr(W(XD), W(XS), W(XT))

#define clelx3ld(XD, XS, MT, DT)                                            \
        cleix3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define cleln_rr(XG, XS)                                                    \
        clein_rr(W(XG), W(XS))

#define cleln_ld(XG, MS, DS)                                                \
        clein_ld(W(XG), W(MS), W(DS))

#define cleln3rr(XD, XS, XT)                                                \
        clein3rr(W(XD), W(XS), W(XT))

#define cleln3ld(XD, XS, MT, DT)                                            \
        clein3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtlx_rr(XG, XS)                                                    \
        cgtix_rr(W(XG), W(XS))

#define cgtlx_ld(XG, MS, DS)                                                \
        cgtix_ld(W(XG), W(MS), W(DS))

#define cgtlx3rr(XD, XS, XT)                                                \
        cgtix3rr(W(XD), W(XS), W(XT))

#define cgtlx3ld(XD, XS, MT, DT)                                            \
        cgtix3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtln_rr(XG, XS)                                                    \
        cgtin_rr(W(XG), W(XS))

#define cgtln_ld(XG, MS, DS)                                                \
        cgtin_ld(W(XG), W(MS), W(DS))

#define cgtln3rr(XD, XS, XT)                                                \
        cgtin3rr(W(XD), W(XS), W(XT))

#define cgtln3ld(XD, XS, MT, DT)                                            \
        cgtin3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgelx_rr(XG, XS)                                                    \
        cgeix_rr(W(XG), W(XS))

#define cgelx_ld(XG, MS, DS)                                                \
        cgeix_ld(W(XG), W(MS), W(DS))

#define cgelx3rr(XD, XS, XT)                                                \
        cgeix3rr(W(XD), W(XS), W(XT))

#define cgelx3ld(XD, XS, MT, DT)                                            \
        cgeix3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgeln_rr(XG, XS)                                                    \
        cgein_rr(W(XG), W(XS))

#define cgeln_ld(XG, MS, DS)                                                \
        cgein_ld(W(XG), W(MS), W(DS))

#define cgeln3rr(XD, XS, XT)                                                \
        cgein3rr(W(XD), W(XS), W(XT))

#define cgeln3ld(XD, XS, MT, DT)                                            \
        cgein3ld(W(XD), W(XS), W(MT), W(DT))

/******************************************************************************/
/**** scalar ***** SIMD instructions with configurable element **** 32-bit ****/
/******************************************************************************/

/*********   scalar single-precision floating-point move/arithmetic   *********/

/* mov (D = S) */

#define movss_rr(XD, XS)                                                    \
        movrs_rr(W(XD), W(XS))

#define movss_ld(XD, MS, DS)                                                \
        movrs_ld(W(XD), W(MS), W(DS))

#define movss_st(XS, MD, DD)                                                \
        movrs_st(W(XS), W(MD), W(DD))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addss_rr(XG, XS)                                                    \
        addrs_rr(W(XG), W(XS))

#define addss_ld(XG, MS, DS)                                                \
        addrs_ld(W(XG), W(MS), W(DS))

#define addss3rr(XD, XS, XT)                                                \
        addrs3rr(W(XD), W(XS), W(XT))

#define addss3ld(XD, XS, MT, DT)                                            \
        addrs3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subss_rr(XG, XS)                                                    \
        subrs_rr(W(XG), W(XS))

#define subss_ld(XG, MS, DS)                                                \
        subrs_ld(W(XG), W(MS), W(DS))

#define subss3rr(XD, XS, XT)                                                \
        subrs3rr(W(XD), W(XS), W(XT))

#define subss3ld(XD, XS, MT, DT)                                            \
        subrs3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulss_rr(XG, XS)                                                    \
        mulrs_rr(W(XG), W(XS))

#define mulss_ld(XG, MS, DS)                                                \
        mulrs_ld(W(XG), W(MS), W(DS))

#define mulss3rr(XD, XS, XT)                                                \
        mulrs3rr(W(XD), W(XS), W(XT))

#define mulss3ld(XD, XS, MT, DT)                                            \
        mulrs3ld(W(XD), W(XS), W(MT), W(DT))

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divss_rr(XG, XS)                                                    \
        divrs_rr(W(XG), W(XS))

#define divss_ld(XG, MS, DS)                                                \
        divrs_ld(W(XG), W(MS), W(DS))

#define divss3rr(XD, XS, XT)                                                \
        divrs3rr(W(XD), W(XS), W(XT))

#define divss3ld(XD, XS, MT, DT)                                            \
        divrs3ld(W(XD), W(XS), W(MT), W(DT))

/* sqr (D = sqrt S) */

#define sqrss_rr(XD, XS)                                                    \
        sqrrs_rr(W(XD), W(XS))

#define sqrss_ld(XD, MS, DS)                                                \
        sqrrs_ld(W(XD), W(MS), W(DS))

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rcpss_rr(XD, XS) /* destroys XS */                                  \
        rcprs_rr(W(XD), W(XS))

#define rcess_rr(XD, XS)                                                    \
        rcers_rr(W(XD), W(XS))

#define rcsss_rr(XG, XS) /* destroys XS */                                  \
        rcsrs_rr(W(XG), W(XS))

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rsqss_rr(XD, XS) /* destroys XS */                                  \
        rsqrs_rr(W(XD), W(XS))

#define rsess_rr(XD, XS)                                                    \
        rsers_rr(W(XD), W(XS))

#define rssss_rr(XG, XS) /* destroys XS */                                  \
        rssrs_rr(W(XG), W(XS))

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmass_rr(XG, XS, XT)                                                \
        fmars_rr(W(XG), W(XS), W(XT))

#define fmass_ld(XG, XS, MT, DT)                                            \
        fmars_ld(W(XG), W(XS), W(MT), W(DT))

#define fmass3rr(XG, XS, XT)                                                \
        fmass_rr(W(XG), W(XS), W(XT))

#define fmass3ld(XG, XS, MT, DT)                                            \
        fmass_ld(W(XG), W(XS), W(MT), W(DT))

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsss_rr(XG, XS, XT)                                                \
        fmsrs_rr(W(XG), W(XS), W(XT))

#define fmsss_ld(XG, XS, MT, DT)                                            \
        fmsrs_ld(W(XG), W(XS), W(MT), W(DT))

#define fmsss3rr(XG, XS, XT)                                                \
        fmsss_rr(W(XG), W(XS), W(XT))

#define fmsss3ld(XG, XS, MT, DT)                                            \
        fmsss_ld(W(XG), W(XS), W(MT), W(DT))

/*************   scalar single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minss_rr(XG, XS)                                                    \
        minrs_rr(W(XG), W(XS))

#define minss_ld(XG, MS, DS)                                                \
        minrs_ld(W(XG), W(MS), W(DS))

#define minss3rr(XD, XS, XT)                                                \
        minrs3rr(W(XD), W(XS), W(XT))

#define minss3ld(XD, XS, MT, DT)                                            \
        minrs3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxss_rr(XG, XS)                                                    \
        maxrs_rr(W(XG), W(XS))

#define maxss_ld(XG, MS, DS)                                                \
        maxrs_ld(W(XG), W(MS), W(DS))

#define maxss3rr(XD, XS, XT)                                                \
        maxrs3rr(W(XD), W(XS), W(XT))

#define maxss3ld(XD, XS, MT, DT)                                            \
        maxrs3ld(W(XD), W(XS), W(MT), W(DT))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqss_rr(XG, XS)                                                    \
        ceqrs_rr(W(XG), W(XS))

#define ceqss_ld(XG, MS, DS)                                                \
        ceqrs_ld(W(XG), W(MS), W(DS))

#define ceqss3rr(XD, XS, XT)                                                \
        ceqrs3rr(W(XD), W(XS), W(XT))

#define ceqss3ld(XD, XS, MT, DT)                                            \
        ceqrs3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cness_rr(XG, XS)                                                    \
        cners_rr(W(XG), W(XS))

#define cness_ld(XG, MS, DS)                                                \
        cners_ld(W(XG), W(MS), W(DS))

#define cness3rr(XD, XS, XT)                                                \
        cners3rr(W(XD), W(XS), W(XT))

#define cness3ld(XD, XS, MT, DT)                                            \
        cners3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltss_rr(XG, XS)                                                    \
        cltrs_rr(W(XG), W(XS))

#define cltss_ld(XG, MS, DS)                                                \
        cltrs_ld(W(XG), W(MS), W(DS))

#define cltss3rr(XD, XS, XT)                                                \
        cltrs3rr(W(XD), W(XS), W(XT))

#define cltss3ld(XD, XS, MT, DT)                                            \
        cltrs3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define cless_rr(XG, XS)                                                    \
        clers_rr(W(XG), W(XS))

#define cless_ld(XG, MS, DS)                                                \
        clers_ld(W(XG), W(MS), W(DS))

#define cless3rr(XD, XS, XT)                                                \
        clers3rr(W(XD), W(XS), W(XT))

#define cless3ld(XD, XS, MT, DT)                                            \
        clers3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtss_rr(XG, XS)                                                    \
        cgtrs_rr(W(XG), W(XS))

#define cgtss_ld(XG, MS, DS)                                                \
        cgtrs_ld(W(XG), W(MS), W(DS))

#define cgtss3rr(XD, XS, XT)                                                \
        cgtrs3rr(W(XD), W(XS), W(XT))

#define cgtss3ld(XD, XS, MT, DT)                                            \
        cgtrs3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgess_rr(XG, XS)                                                    \
        cgers_rr(W(XG), W(XS))

#define cgess_ld(XG, MS, DS)                                                \
        cgers_ld(W(XG), W(MS), W(DS))

#define cgess3rr(XD, XS, XT)                                                \
        cgers3rr(W(XD), W(XS), W(XT))

#define cgess3ld(XD, XS, MT, DT)                                            \
        cgers3ld(W(XD), W(XS), W(MT), W(DT))

/******************************************************************************/
/**** var-len **** SIMD instructions with configurable element **** 64-bit ****/
/******************************************************************************/

#elif RT_ELEMENT == 64

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmpx_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        elmqx_st(W(XS), W(MD), W(DD))

/***************   packed double-precision generic move/logic   ***************/

/* mov (D = S) */

#define movpx_rr(XD, XS)                                                    \
        movqx_rr(W(XD), W(XS))

#define movpx_ld(XD, MS, DS)                                                \
        movqx_ld(W(XD), W(MS), W(DS))

#define movpx_st(XS, MD, DD)                                                \
        movqx_st(W(XS), W(MD), W(DD))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvpx_rr(XD, XS)                                                    \
        mmvqx_rr(W(XD), W(XS))

#define mmvpx_ld(XG, MS, DS)                                                \
        mmvqx_ld(W(XG), W(MS), W(DS))

#define mmvpx_st(XS, MG, DG)                                                \
        mmvqx_st(W(XS), W(MG), W(DG))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andpx_rr(XG, XS)                                                    \
        andqx_rr(W(XG), W(XS))

#define andpx_ld(XG, MS, DS)                                                \
        andqx_ld(W(XG), W(MS), W(DS))

#define andpx3rr(XD, XS, XT)                                                \
        andqx3rr(W(XD), W(XS), W(XT))

#define andpx3ld(XD, XS, MT, DT)                                            \
        andqx3ld(W(XD), W(XS), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annpx_rr(XG, XS)                                                    \
        annqx_rr(W(XG), W(XS))

#define annpx_ld(XG, MS, DS)                                                \
        annqx_ld(W(XG), W(MS), W(DS))

#define annpx3rr(XD, XS, XT)                                                \
        annqx3rr(W(XD), W(XS), W(XT))

#define annpx3ld(XD, XS, MT, DT)                                            \
        annqx3ld(W(XD), W(XS), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrpx_rr(XG, XS)                                                    \
        orrqx_rr(W(XG), W(XS))

#define orrpx_ld(XG, MS, DS)                                                \
        orrqx_ld(W(XG), W(MS), W(DS))

#define orrpx3rr(XD, XS, XT)                                                \
        orrqx3rr(W(XD), W(XS), W(XT))

#define orrpx3ld(XD, XS, MT, DT)                                            \
        orrqx3ld(W(XD), W(XS), W(MT), W(DT))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define ornpx_rr(XG, XS)                                                    \
        ornqx_rr(W(XG), W(XS))

#define ornpx_ld(XG, MS, DS)                                                \
        ornqx_ld(W(XG), W(MS), W(DS))

#define ornpx3rr(XD, XS, XT)                                                \
        ornqx3rr(W(XD), W(XS), W(XT))

#define ornpx3ld(XD, XS, MT, DT)                                            \
        ornqx3ld(W(XD), W(XS), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorpx_rr(XG, XS)                                                    \
        xorqx_rr(W(XG), W(XS))

#define xorpx_ld(XG, MS, DS)                                                \
        xorqx_ld(W(XG), W(MS), W(DS))

#define xorpx3rr(XD, XS, XT)                                                \
        xorqx3rr(W(XD), W(XS), W(XT))

#define xorpx3ld(XD, XS, MT, DT)                                            \
        xorqx3ld(W(XD), W(XS), W(MT), W(DT))

/* not (G = ~G), (D = ~S) */

#define notpx_rx(XG)                                                        \
        notqx_rx(W(XG))

#define notpx_rr(XD, XS)                                                    \
        notqx_rr(W(XD), W(XS))

/************   packed double-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negps_rx(XG)                                                        \
        negqs_rx(W(XG))

#define negps_rr(XD, XS)                                                    \
        negqs_rr(W(XD), W(XS))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addps_rr(XG, XS)                                                    \
        addqs_rr(W(XG), W(XS))

#define addps_ld(XG, MS, DS)                                                \
        addqs_ld(W(XG), W(MS), W(DS))

#define addps3rr(XD, XS, XT)                                                \
        addqs3rr(W(XD), W(XS), W(XT))

#define addps3ld(XD, XS, MT, DT)                                            \
        addqs3ld(W(XD), W(XS), W(MT), W(DT))

#define adpps_rr(XG, XS) /* horizontal pairwise add, first 15-regs only */  \
        adpqs_rr(W(XG), W(XS))

#define adpps_ld(XG, MS, DS)                                                \
        adpqs_ld(W(XG), W(MS), W(DS))

#define adpps3rr(XD, XS, XT)                                                \
        adpqs3rr(W(XD), W(XS), W(XT))

#define adpps3ld(XD, XS, MT, DT)                                            \
        adpqs3ld(W(XD), W(XS), W(MT), W(DT))

#define adhps_rr(XG, XS) /* horizontal reductive add, first 15-regs only */ \
        adhqs_rr(W(XG), W(XS))

#define adhps_ld(XG, MS, DS)                                                \
        adhqs_ld(W(XG), W(MS), W(DS))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subps_rr(XG, XS)                                                    \
        subqs_rr(W(XG), W(XS))

#define subps_ld(XG, MS, DS)                                                \
        subqs_ld(W(XG), W(MS), W(DS))

#define subps3rr(XD, XS, XT)                                                \
        subqs3rr(W(XD), W(XS), W(XT))

#define subps3ld(XD, XS, MT, DT)                                            \
        subqs3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulps_rr(XG, XS)                                                    \
        mulqs_rr(W(XG), W(XS))

#define mulps_ld(XG, MS, DS)                                                \
        mulqs_ld(W(XG), W(MS), W(DS))

#define mulps3rr(XD, XS, XT)                                                \
        mulqs3rr(W(XD), W(XS), W(XT))

#define mulps3ld(XD, XS, MT, DT)                                            \
        mulqs3ld(W(XD), W(XS), W(MT), W(DT))

#define mlpps_rr(XG, XS) /* horizontal pairwise mul */                      \
        mlpqs_rr(W(XG), W(XS))

#define mlpps_ld(XG, MS, DS)                                                \
        mlpqs_ld(W(XG), W(MS), W(DS))

#define mlpps3rr(XD, XS, XT)                                                \
        mlpqs3rr(W(XD), W(XS), W(XT))

#define mlpps3ld(XD, XS, MT, DT)                                            \
        mlpqs3ld(W(XD), W(XS), W(MT), W(DT))

#define mlhps_rr(XD, XS) /* horizontal reductive mul */                     \
        mlhqs_rr(W(XD), W(XS))

#define mlhps_ld(XD, MS, DS)                                                \
        mlhqs_ld(W(XD), W(MS), W(DS))

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divps_rr(XG, XS)                                                    \
        divqs_rr(W(XG), W(XS))

#define divps_ld(XG, MS, DS)                                                \
        divqs_ld(W(XG), W(MS), W(DS))

#define divps3rr(XD, XS, XT)                                                \
        divqs3rr(W(XD), W(XS), W(XT))

#define divps3ld(XD, XS, MT, DT)                                            \
        divqs3ld(W(XD), W(XS), W(MT), W(DT))

/* sqr (D = sqrt S) */

#define sqrps_rr(XD, XS)                                                    \
        sqrqs_rr(W(XD), W(XS))

#define sqrps_ld(XD, MS, DS)                                                \
        sqrqs_ld(W(XD), W(MS), W(DS))

/* cbr (D = cbrt S) */

#define cbrps_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbrqs_rr(W(XD), W(X1), W(X2), W(XS))

#define cbeps_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbeqs_rr(W(XD), W(X1), W(X2), W(XS))

#define cbsps_rr(XG, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbsqs_rr(W(XG), W(X1), W(X2), W(XS))

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rcpps_rr(XD, XS) /* destroys XS */                                  \
        rcpqs_rr(W(XD), W(XS))

#define rceps_rr(XD, XS)                                                    \
        rceqs_rr(W(XD), W(XS))

#define rcsps_rr(XG, XS) /* destroys XS */                                  \
        rcsqs_rr(W(XG), W(XS))

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rsqps_rr(XD, XS) /* destroys XS */                                  \
        rsqqs_rr(W(XD), W(XS))

#define rseps_rr(XD, XS)                                                    \
        rseqs_rr(W(XD), W(XS))

#define rssps_rr(XG, XS) /* destroys XS */                                  \
        rssqs_rr(W(XG), W(XS))

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmaps_rr(XG, XS, XT)                                                \
        fmaqs_rr(W(XG), W(XS), W(XT))

#define fmaps_ld(XG, XS, MT, DT)                                            \
        fmaqs_ld(W(XG), W(XS), W(MT), W(DT))

#define fmaps3rr(XG, XS, XT)                                                \
        fmaps_rr(W(XG), W(XS), W(XT))

#define fmaps3ld(XG, XS, MT, DT)                                            \
        fmaps_ld(W(XG), W(XS), W(MT), W(DT))

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsps_rr(XG, XS, XT)                                                \
        fmsqs_rr(W(XG), W(XS), W(XT))

#define fmsps_ld(XG, XS, MT, DT)                                            \
        fmsqs_ld(W(XG), W(XS), W(MT), W(DT))

#define fmsps3rr(XG, XS, XT)                                                \
        fmsps_rr(W(XG), W(XS), W(XT))

#define fmsps3ld(XG, XS, MT, DT)                                            \
        fmsps_ld(W(XG), W(XS), W(MT), W(DT))

/*************   packed double-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minps_rr(XG, XS)                                                    \
        minqs_rr(W(XG), W(XS))

#define minps_ld(XG, MS, DS)                                                \
        minqs_ld(W(XG), W(MS), W(DS))

#define minps3rr(XD, XS, XT)                                                \
        minqs3rr(W(XD), W(XS), W(XT))

#define minps3ld(XD, XS, MT, DT)                                            \
        minqs3ld(W(XD), W(XS), W(MT), W(DT))

#define mnpps_rr(XG, XS) /* horizontal pairwise min */                      \
        mnpqs_rr(W(XG), W(XS))

#define mnpps_ld(XG, MS, DS)                                                \
        mnpqs_ld(W(XG), W(MS), W(DS))

#define mnpps3rr(XD, XS, XT)                                                \
        mnpqs3rr(W(XD), W(XS), W(XT))

#define mnpps3ld(XD, XS, MT, DT)                                            \
        mnpqs3ld(W(XD), W(XS), W(MT), W(DT))

#define mnhps_rr(XD, XS) /* horizontal reductive min */                     \
        mnhqs_rr(W(XD), W(XS))

#define mnhps_ld(XD, MS, DS)                                                \
        mnhqs_ld(W(XD), W(MS), W(DS))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxps_rr(XG, XS)                                                    \
        maxqs_rr(W(XG), W(XS))

#define maxps_ld(XG, MS, DS)                                                \
        maxqs_ld(W(XG), W(MS), W(DS))

#define maxps3rr(XD, XS, XT)                                                \
        maxqs3rr(W(XD), W(XS), W(XT))

#define maxps3ld(XD, XS, MT, DT)                                            \
        maxqs3ld(W(XD), W(XS), W(MT), W(DT))

#define mxpps_rr(XG, XS) /* horizontal pairwise max */                      \
        mxpqs_rr(W(XG), W(XS))

#define mxpps_ld(XG, MS, DS)                                                \
        mxpqs_ld(W(XG), W(MS), W(DS))

#define mxpps3rr(XD, XS, XT)                                                \
        mxpqs3rr(W(XD), W(XS), W(XT))

#define mxpps3ld(XD, XS, MT, DT)                                            \
        mxpqs3ld(W(XD), W(XS), W(MT), W(DT))

#define mxhps_rr(XD, XS) /* horizontal reductive max */                     \
        mxhqs_rr(W(XD), W(XS))

#define mxhps_ld(XD, MS, DS)                                                \
        mxhqs_ld(W(XD), W(MS), W(DS))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqps_rr(XG, XS)                                                    \
        ceqqs_rr(W(XG), W(XS))

#define ceqps_ld(XG, MS, DS)                                                \
        ceqqs_ld(W(XG), W(MS), W(DS))

#define ceqps3rr(XD, XS, XT)                                                \
        ceqqs3rr(W(XD), W(XS), W(XT))

#define ceqps3ld(XD, XS, MT, DT)                                            \
        ceqqs3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneps_rr(XG, XS)                                                    \
        cneqs_rr(W(XG), W(XS))

#define cneps_ld(XG, MS, DS)                                                \
        cneqs_ld(W(XG), W(MS), W(DS))

#define cneps3rr(XD, XS, XT)                                                \
        cneqs3rr(W(XD), W(XS), W(XT))

#define cneps3ld(XD, XS, MT, DT)                                            \
        cneqs3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltps_rr(XG, XS)                                                    \
        cltqs_rr(W(XG), W(XS))

#define cltps_ld(XG, MS, DS)                                                \
        cltqs_ld(W(XG), W(MS), W(DS))

#define cltps3rr(XD, XS, XT)                                                \
        cltqs3rr(W(XD), W(XS), W(XT))

#define cltps3ld(XD, XS, MT, DT)                                            \
        cltqs3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define cleps_rr(XG, XS)                                                    \
        cleqs_rr(W(XG), W(XS))

#define cleps_ld(XG, MS, DS)                                                \
        cleqs_ld(W(XG), W(MS), W(DS))

#define cleps3rr(XD, XS, XT)                                                \
        cleqs3rr(W(XD), W(XS), W(XT))

#define cleps3ld(XD, XS, MT, DT)                                            \
        cleqs3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtps_rr(XG, XS)                                                    \
        cgtqs_rr(W(XG), W(XS))

#define cgtps_ld(XG, MS, DS)                                                \
        cgtqs_ld(W(XG), W(MS), W(DS))

#define cgtps3rr(XD, XS, XT)                                                \
        cgtqs3rr(W(XD), W(XS), W(XT))

#define cgtps3ld(XD, XS, MT, DT)                                            \
        cgtqs3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgeps_rr(XG, XS)                                                    \
        cgeqs_rr(W(XG), W(XS))

#define cgeps_ld(XG, MS, DS)                                                \
        cgeqs_ld(W(XG), W(MS), W(DS))

#define cgeps3rr(XD, XS, XT)                                                \
        cgeqs3rr(W(XD), W(XS), W(XT))

#define cgeps3ld(XD, XS, MT, DT)                                            \
        cgeqs3ld(W(XD), W(XS), W(MT), W(DT))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define mkjpx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        mkjqx_rx(W(XS), mask, lb)

/*************   packed double-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnzps_rr(XD, XS)     /* round towards zero */                       \
        rnzqs_rr(W(XD), W(XS))

#define rnzps_ld(XD, MS, DS) /* round towards zero */                       \
        rnzqs_ld(W(XD), W(MS), W(DS))

#define cvzps_rr(XD, XS)     /* round towards zero */                       \
        cvzqs_rr(W(XD), W(XS))

#define cvzps_ld(XD, MS, DS) /* round towards zero */                       \
        cvzqs_ld(W(XD), W(MS), W(DS))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnpps_rr(XD, XS)     /* round towards +inf */                       \
        rnpqs_rr(W(XD), W(XS))

#define rnpps_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpqs_ld(W(XD), W(MS), W(DS))

#define cvpps_rr(XD, XS)     /* round towards +inf */                       \
        cvpqs_rr(W(XD), W(XS))

#define cvpps_ld(XD, MS, DS) /* round towards +inf */                       \
        cvpqs_ld(W(XD), W(MS), W(DS))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnmps_rr(XD, XS)     /* round towards -inf */                       \
        rnmqs_rr(W(XD), W(XS))

#define rnmps_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmqs_ld(W(XD), W(MS), W(DS))

#define cvmps_rr(XD, XS)     /* round towards -inf */                       \
        cvmqs_rr(W(XD), W(XS))

#define cvmps_ld(XD, MS, DS) /* round towards -inf */                       \
        cvmqs_ld(W(XD), W(MS), W(DS))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnnps_rr(XD, XS)     /* round towards near */                       \
        rnnqs_rr(W(XD), W(XS))

#define rnnps_ld(XD, MS, DS) /* round towards near */                       \
        rnnqs_ld(W(XD), W(MS), W(DS))

#define cvnps_rr(XD, XS)     /* round towards near */                       \
        cvnqs_rr(W(XD), W(XS))

#define cvnps_ld(XD, MS, DS) /* round towards near */                       \
        cvnqs_ld(W(XD), W(MS), W(DS))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnpn_rr(XD, XS)     /* round towards near */                       \
        cvnqn_rr(W(XD), W(XS))

#define cvnpn_ld(XD, MS, DS) /* round towards near */                       \
        cvnqn_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndps_rr(XD, XS)                                                    \
        rndqs_rr(W(XD), W(XS))

#define rndps_ld(XD, MS, DS)                                                \
        rndqs_ld(W(XD), W(MS), W(DS))

#define cvtps_rr(XD, XS)                                                    \
        cvtqs_rr(W(XD), W(XS))

#define cvtps_ld(XD, MS, DS)                                                \
        cvtqs_ld(W(XD), W(MS), W(DS))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtpn_rr(XD, XS)                                                    \
        cvtqn_rr(W(XD), W(XS))

#define cvtpn_ld(XD, MS, DS)                                                \
        cvtqn_ld(W(XD), W(MS), W(DS))

/* cvn (D = unsigned-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnpx_rr(XD, XS)     /* round towards near */                       \
        cvnqx_rr(W(XD), W(XS))

#define cvnpx_ld(XD, MS, DS) /* round towards near */                       \
        cvnqx_ld(W(XD), W(MS), W(DS))

/* cvt (D = unsigned-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtpx_rr(XD, XS)                                                    \
        cvtqx_rr(W(XD), W(XS))

#define cvtpx_ld(XD, MS, DS)                                                \
        cvtqx_ld(W(XD), W(MS), W(DS))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnrps_rr(XD, XS, mode)                                              \
        rnrqs_rr(W(XD), W(XS), mode)

#define cvrps_rr(XD, XS, mode)                                              \
        cvrqs_rr(W(XD), W(XS), mode)

/************   packed double-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addpx_rr(XG, XS)                                                    \
        addqx_rr(W(XG), W(XS))

#define addpx_ld(XG, MS, DS)                                                \
        addqx_ld(W(XG), W(MS), W(DS))

#define addpx3rr(XD, XS, XT)                                                \
        addqx3rr(W(XD), W(XS), W(XT))

#define addpx3ld(XD, XS, MT, DT)                                            \
        addqx3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subpx_rr(XG, XS)                                                    \
        subqx_rr(W(XG), W(XS))

#define subpx_ld(XG, MS, DS)                                                \
        subqx_ld(W(XG), W(MS), W(DS))

#define subpx3rr(XD, XS, XT)                                                \
        subqx3rr(W(XD), W(XS), W(XT))

#define subpx3ld(XD, XS, MT, DT)                                            \
        subqx3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulpx_rr(XG, XS)                                                    \
        mulqx_rr(W(XG), W(XS))

#define mulpx_ld(XG, MS, DS)                                                \
        mulqx_ld(W(XG), W(MS), W(DS))

#define mulpx3rr(XD, XS, XT)                                                \
        mulqx3rr(W(XD), W(XS), W(XT))

#define mulpx3ld(XD, XS, MT, DT)                                            \
        mulqx3ld(W(XD), W(XS), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlpx_ri(XG, IS)                                                    \
        shlqx_ri(W(XG), W(IS))

#define shlpx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlqx_ld(W(XG), W(MS), W(DS))

#define shlpx3ri(XD, XS, IT)                                                \
        shlqx3ri(W(XD), W(XS), W(IT))

#define shlpx3ld(XD, XS, MT, DT)                                            \
        shlqx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrpx_ri(XG, IS)                                                    \
        shrqx_ri(W(XG), W(IS))

#define shrpx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrqx_ld(W(XG), W(MS), W(DS))

#define shrpx3ri(XD, XS, IT)                                                \
        shrqx3ri(W(XD), W(XS), W(IT))

#define shrpx3ld(XD, XS, MT, DT)                                            \
        shrqx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrpn_ri(XG, IS)                                                    \
        shrqn_ri(W(XG), W(IS))

#define shrpn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrqn_ld(W(XG), W(MS), W(DS))

#define shrpn3ri(XD, XS, IT)                                                \
        shrqn3ri(W(XD), W(XS), W(IT))

#define shrpn3ld(XD, XS, MT, DT)                                            \
        shrqn3ld(W(XD), W(XS), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlpx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlqx_rr(W(XG), W(XS))

#define svlpx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlqx_ld(W(XG), W(MS), W(DS))

#define svlpx3rr(XD, XS, XT)                                                \
        svlqx3rr(W(XD), W(XS), W(XT))

#define svlpx3ld(XD, XS, MT, DT)                                            \
        svlqx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrpx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrqx_rr(W(XG), W(XS))

#define svrpx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrqx_ld(W(XG), W(MS), W(DS))

#define svrpx3rr(XD, XS, XT)                                                \
        svrqx3rr(W(XD), W(XS), W(XT))

#define svrpx3ld(XD, XS, MT, DT)                                            \
        svrqx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrpn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrqn_rr(W(XG), W(XS))

#define svrpn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrqn_ld(W(XG), W(MS), W(DS))

#define svrpn3rr(XD, XS, XT)                                                \
        svrqn3rr(W(XD), W(XS), W(XT))

#define svrpn3ld(XD, XS, MT, DT)                                            \
        svrqn3ld(W(XD), W(XS), W(MT), W(DT))

/****************   packed double-precision integer compare   *****************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minpx_rr(XG, XS)                                                    \
        minqx_rr(W(XG), W(XS))

#define minpx_ld(XG, MS, DS)                                                \
        minqx_ld(W(XG), W(MS), W(DS))

#define minpx3rr(XD, XS, XT)                                                \
        minqx3rr(W(XD), W(XS), W(XT))

#define minpx3ld(XD, XS, MT, DT)                                            \
        minqx3ld(W(XD), W(XS), W(MT), W(DT))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minpn_rr(XG, XS)                                                    \
        minqn_rr(W(XG), W(XS))

#define minpn_ld(XG, MS, DS)                                                \
        minqn_ld(W(XG), W(MS), W(DS))

#define minpn3rr(XD, XS, XT)                                                \
        minqn3rr(W(XD), W(XS), W(XT))

#define minpn3ld(XD, XS, MT, DT)                                            \
        minqn3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxpx_rr(XG, XS)                                                    \
        maxqx_rr(W(XG), W(XS))

#define maxpx_ld(XG, MS, DS)                                                \
        maxqx_ld(W(XG), W(MS), W(DS))

#define maxpx3rr(XD, XS, XT)                                                \
        maxqx3rr(W(XD), W(XS), W(XT))

#define maxpx3ld(XD, XS, MT, DT)                                            \
        maxqx3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxpn_rr(XG, XS)                                                    \
        maxqn_rr(W(XG), W(XS))

#define maxpn_ld(XG, MS, DS)                                                \
        maxqn_ld(W(XG), W(MS), W(DS))

#define maxpn3rr(XD, XS, XT)                                                \
        maxqn3rr(W(XD), W(XS), W(XT))

#define maxpn3ld(XD, XS, MT, DT)                                            \
        maxqn3ld(W(XD), W(XS), W(MT), W(DT))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqpx_rr(XG, XS)                                                    \
        ceqqx_rr(W(XG), W(XS))

#define ceqpx_ld(XG, MS, DS)                                                \
        ceqqx_ld(W(XG), W(MS), W(DS))

#define ceqpx3rr(XD, XS, XT)                                                \
        ceqqx3rr(W(XD), W(XS), W(XT))

#define ceqpx3ld(XD, XS, MT, DT)                                            \
        ceqqx3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnepx_rr(XG, XS)                                                    \
        cneqx_rr(W(XG), W(XS))

#define cnepx_ld(XG, MS, DS)                                                \
        cneqx_ld(W(XG), W(MS), W(DS))

#define cnepx3rr(XD, XS, XT)                                                \
        cneqx3rr(W(XD), W(XS), W(XT))

#define cnepx3ld(XD, XS, MT, DT)                                            \
        cneqx3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltpx_rr(XG, XS)                                                    \
        cltqx_rr(W(XG), W(XS))

#define cltpx_ld(XG, MS, DS)                                                \
        cltqx_ld(W(XG), W(MS), W(DS))

#define cltpx3rr(XD, XS, XT)                                                \
        cltqx3rr(W(XD), W(XS), W(XT))

#define cltpx3ld(XD, XS, MT, DT)                                            \
        cltqx3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltpn_rr(XG, XS)                                                    \
        cltqn_rr(W(XG), W(XS))

#define cltpn_ld(XG, MS, DS)                                                \
        cltqn_ld(W(XG), W(MS), W(DS))

#define cltpn3rr(XD, XS, XT)                                                \
        cltqn3rr(W(XD), W(XS), W(XT))

#define cltpn3ld(XD, XS, MT, DT)                                            \
        cltqn3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clepx_rr(XG, XS)                                                    \
        cleqx_rr(W(XG), W(XS))

#define clepx_ld(XG, MS, DS)                                                \
        cleqx_ld(W(XG), W(MS), W(DS))

#define clepx3rr(XD, XS, XT)                                                \
        cleqx3rr(W(XD), W(XS), W(XT))

#define clepx3ld(XD, XS, MT, DT)                                            \
        cleqx3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clepn_rr(XG, XS)                                                    \
        cleqn_rr(W(XG), W(XS))

#define clepn_ld(XG, MS, DS)                                                \
        cleqn_ld(W(XG), W(MS), W(DS))

#define clepn3rr(XD, XS, XT)                                                \
        cleqn3rr(W(XD), W(XS), W(XT))

#define clepn3ld(XD, XS, MT, DT)                                            \
        cleqn3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtpx_rr(XG, XS)                                                    \
        cgtqx_rr(W(XG), W(XS))

#define cgtpx_ld(XG, MS, DS)                                                \
        cgtqx_ld(W(XG), W(MS), W(DS))

#define cgtpx3rr(XD, XS, XT)                                                \
        cgtqx3rr(W(XD), W(XS), W(XT))

#define cgtpx3ld(XD, XS, MT, DT)                                            \
        cgtqx3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtpn_rr(XG, XS)                                                    \
        cgtqn_rr(W(XG), W(XS))

#define cgtpn_ld(XG, MS, DS)                                                \
        cgtqn_ld(W(XG), W(MS), W(DS))

#define cgtpn3rr(XD, XS, XT)                                                \
        cgtqn3rr(W(XD), W(XS), W(XT))

#define cgtpn3ld(XD, XS, MT, DT)                                            \
        cgtqn3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgepx_rr(XG, XS)                                                    \
        cgeqx_rr(W(XG), W(XS))

#define cgepx_ld(XG, MS, DS)                                                \
        cgeqx_ld(W(XG), W(MS), W(DS))

#define cgepx3rr(XD, XS, XT)                                                \
        cgeqx3rr(W(XD), W(XS), W(XT))

#define cgepx3ld(XD, XS, MT, DT)                                            \
        cgeqx3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgepn_rr(XG, XS)                                                    \
        cgeqn_rr(W(XG), W(XS))

#define cgepn_ld(XG, MS, DS)                                                \
        cgeqn_ld(W(XG), W(MS), W(DS))

#define cgepn3rr(XD, XS, XT)                                                \
        cgeqn3rr(W(XD), W(XS), W(XT))

#define cgepn3ld(XD, XS, MT, DT)                                            \
        cgeqn3ld(W(XD), W(XS), W(MT), W(DT))

/******************************************************************************/
/**** 256-bit **** SIMD instructions with configurable element **** 64-bit ****/
/******************************************************************************/

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmfx_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        elmdx_st(W(XS), W(MD), W(DD))

/***************   packed double-precision generic move/logic   ***************/

/* mov (D = S) */

#define movfx_rr(XD, XS)                                                    \
        movdx_rr(W(XD), W(XS))

#define movfx_ld(XD, MS, DS)                                                \
        movdx_ld(W(XD), W(MS), W(DS))

#define movfx_st(XS, MD, DD)                                                \
        movdx_st(W(XS), W(MD), W(DD))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvfx_rr(XD, XS)                                                    \
        mmvdx_rr(W(XD), W(XS))

#define mmvfx_ld(XG, MS, DS)                                                \
        mmvdx_ld(W(XG), W(MS), W(DS))

#define mmvfx_st(XS, MG, DG)                                                \
        mmvdx_st(W(XS), W(MG), W(DG))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andfx_rr(XG, XS)                                                    \
        anddx_rr(W(XG), W(XS))

#define andfx_ld(XG, MS, DS)                                                \
        anddx_ld(W(XG), W(MS), W(DS))

#define andfx3rr(XD, XS, XT)                                                \
        anddx3rr(W(XD), W(XS), W(XT))

#define andfx3ld(XD, XS, MT, DT)                                            \
        anddx3ld(W(XD), W(XS), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annfx_rr(XG, XS)                                                    \
        anndx_rr(W(XG), W(XS))

#define annfx_ld(XG, MS, DS)                                                \
        anndx_ld(W(XG), W(MS), W(DS))

#define annfx3rr(XD, XS, XT)                                                \
        anndx3rr(W(XD), W(XS), W(XT))

#define annfx3ld(XD, XS, MT, DT)                                            \
        anndx3ld(W(XD), W(XS), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrfx_rr(XG, XS)                                                    \
        orrdx_rr(W(XG), W(XS))

#define orrfx_ld(XG, MS, DS)                                                \
        orrdx_ld(W(XG), W(MS), W(DS))

#define orrfx3rr(XD, XS, XT)                                                \
        orrdx3rr(W(XD), W(XS), W(XT))

#define orrfx3ld(XD, XS, MT, DT)                                            \
        orrdx3ld(W(XD), W(XS), W(MT), W(DT))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define ornfx_rr(XG, XS)                                                    \
        orndx_rr(W(XG), W(XS))

#define ornfx_ld(XG, MS, DS)                                                \
        orndx_ld(W(XG), W(MS), W(DS))

#define ornfx3rr(XD, XS, XT)                                                \
        orndx3rr(W(XD), W(XS), W(XT))

#define ornfx3ld(XD, XS, MT, DT)                                            \
        orndx3ld(W(XD), W(XS), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorfx_rr(XG, XS)                                                    \
        xordx_rr(W(XG), W(XS))

#define xorfx_ld(XG, MS, DS)                                                \
        xordx_ld(W(XG), W(MS), W(DS))

#define xorfx3rr(XD, XS, XT)                                                \
        xordx3rr(W(XD), W(XS), W(XT))

#define xorfx3ld(XD, XS, MT, DT)                                            \
        xordx3ld(W(XD), W(XS), W(MT), W(DT))

/* not (G = ~G), (D = ~S) */

#define notfx_rx(XG)                                                        \
        notdx_rx(W(XG))

#define notfx_rr(XD, XS)                                                    \
        notdx_rr(W(XD), W(XS))

/************   packed double-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negfs_rx(XG)                                                        \
        negds_rx(W(XG))

#define negfs_rr(XD, XS)                                                    \
        negds_rr(W(XD), W(XS))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addfs_rr(XG, XS)                                                    \
        addds_rr(W(XG), W(XS))

#define addfs_ld(XG, MS, DS)                                                \
        addds_ld(W(XG), W(MS), W(DS))

#define addfs3rr(XD, XS, XT)                                                \
        addds3rr(W(XD), W(XS), W(XT))

#define addfs3ld(XD, XS, MT, DT)                                            \
        addds3ld(W(XD), W(XS), W(MT), W(DT))

#define adpfs_rr(XG, XS) /* horizontal pairwise add, first 15-regs only */  \
        adpds_rr(W(XG), W(XS))

#define adpfs_ld(XG, MS, DS)                                                \
        adpds_ld(W(XG), W(MS), W(DS))

#define adpfs3rr(XD, XS, XT)                                                \
        adpds3rr(W(XD), W(XS), W(XT))

#define adpfs3ld(XD, XS, MT, DT)                                            \
        adpds3ld(W(XD), W(XS), W(MT), W(DT))

#define adhfs_rr(XD, XS) /* horizontal reductive add, first 15-regs only */ \
        adhds_rr(W(XD), W(XS))

#define adhfs_ld(XD, MS, DS)                                                \
        adhds_ld(W(XD), W(MS), W(DS))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subfs_rr(XG, XS)                                                    \
        subds_rr(W(XG), W(XS))

#define subfs_ld(XG, MS, DS)                                                \
        subds_ld(W(XG), W(MS), W(DS))

#define subfs3rr(XD, XS, XT)                                                \
        subds3rr(W(XD), W(XS), W(XT))

#define subfs3ld(XD, XS, MT, DT)                                            \
        subds3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulfs_rr(XG, XS)                                                    \
        mulds_rr(W(XG), W(XS))

#define mulfs_ld(XG, MS, DS)                                                \
        mulds_ld(W(XG), W(MS), W(DS))

#define mulfs3rr(XD, XS, XT)                                                \
        mulds3rr(W(XD), W(XS), W(XT))

#define mulfs3ld(XD, XS, MT, DT)                                            \
        mulds3ld(W(XD), W(XS), W(MT), W(DT))

#define mlpfs_rr(XG, XS) /* horizontal pairwise mul */                      \
        mlpds_rr(W(XG), W(XS))

#define mlpfs_ld(XG, MS, DS)                                                \
        mlpds_ld(W(XG), W(MS), W(DS))

#define mlpfs3rr(XD, XS, XT)                                                \
        mlpds3rr(W(XD), W(XS), W(XT))

#define mlpfs3ld(XD, XS, MT, DT)                                            \
        mlpds3ld(W(XD), W(XS), W(MT), W(DT))

#define mlhfs_rr(XD, XS) /* horizontal reductive mul */                     \
        mlhds_rr(W(XD), W(XS))

#define mlhfs_ld(XD, MS, DS)                                                \
        mlhds_ld(W(XD), W(MS), W(DS))

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divfs_rr(XG, XS)                                                    \
        divds_rr(W(XG), W(XS))

#define divfs_ld(XG, MS, DS)                                                \
        divds_ld(W(XG), W(MS), W(DS))

#define divfs3rr(XD, XS, XT)                                                \
        divds3rr(W(XD), W(XS), W(XT))

#define divfs3ld(XD, XS, MT, DT)                                            \
        divds3ld(W(XD), W(XS), W(MT), W(DT))

/* sqr (D = sqrt S) */

#define sqrfs_rr(XD, XS)                                                    \
        sqrds_rr(W(XD), W(XS))

#define sqrfs_ld(XD, MS, DS)                                                \
        sqrds_ld(W(XD), W(MS), W(DS))

/* cbr (D = cbrt S) */

#define cbrfs_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbrds_rr(W(XD), W(X1), W(X2), W(XS))

#define cbefs_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbeds_rr(W(XD), W(X1), W(X2), W(XS))

#define cbsfs_rr(XG, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbsds_rr(W(XG), W(X1), W(X2), W(XS))

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rcpfs_rr(XD, XS) /* destroys XS */                                  \
        rcpds_rr(W(XD), W(XS))

#define rcefs_rr(XD, XS)                                                    \
        rceds_rr(W(XD), W(XS))

#define rcsfs_rr(XG, XS) /* destroys XS */                                  \
        rcsds_rr(W(XG), W(XS))

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rsqfs_rr(XD, XS) /* destroys XS */                                  \
        rsqds_rr(W(XD), W(XS))

#define rsefs_rr(XD, XS)                                                    \
        rseds_rr(W(XD), W(XS))

#define rssfs_rr(XG, XS) /* destroys XS */                                  \
        rssds_rr(W(XG), W(XS))

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmafs_rr(XG, XS, XT)                                                \
        fmads_rr(W(XG), W(XS), W(XT))

#define fmafs_ld(XG, XS, MT, DT)                                            \
        fmads_ld(W(XG), W(XS), W(MT), W(DT))

#define fmafs3rr(XG, XS, XT)                                                \
        fmafs_rr(W(XG), W(XS), W(XT))

#define fmafs3ld(XG, XS, MT, DT)                                            \
        fmafs_ld(W(XG), W(XS), W(MT), W(DT))

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsfs_rr(XG, XS, XT)                                                \
        fmsds_rr(W(XG), W(XS), W(XT))

#define fmsfs_ld(XG, XS, MT, DT)                                            \
        fmsds_ld(W(XG), W(XS), W(MT), W(DT))

#define fmsfs3rr(XG, XS, XT)                                                \
        fmsfs_rr(W(XG), W(XS), W(XT))

#define fmsfs3ld(XG, XS, MT, DT)                                            \
        fmsfs_ld(W(XG), W(XS), W(MT), W(DT))

/*************   packed double-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minfs_rr(XG, XS)                                                    \
        minds_rr(W(XG), W(XS))

#define minfs_ld(XG, MS, DS)                                                \
        minds_ld(W(XG), W(MS), W(DS))

#define minfs3rr(XD, XS, XT)                                                \
        minds3rr(W(XD), W(XS), W(XT))

#define minfs3ld(XD, XS, MT, DT)                                            \
        minds3ld(W(XD), W(XS), W(MT), W(DT))

#define mnpfs_rr(XG, XS) /* horizontal pairwise min */                      \
        mnpds_rr(W(XG), W(XS))

#define mnpfs_ld(XG, MS, DS)                                                \
        mnpds_ld(W(XG), W(MS), W(DS))

#define mnpfs3rr(XD, XS, XT)                                                \
        mnpds3rr(W(XD), W(XS), W(XT))

#define mnpfs3ld(XD, XS, MT, DT)                                            \
        mnpds3ld(W(XD), W(XS), W(MT), W(DT))

#define mnhfs_rr(XD, XS) /* horizontal reductive min */                     \
        mnhds_rr(W(XD), W(XS))

#define mnhfs_ld(XD, MS, DS)                                                \
        mnhds_ld(W(XD), W(MS), W(DS))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxfs_rr(XG, XS)                                                    \
        maxds_rr(W(XG), W(XS))

#define maxfs_ld(XG, MS, DS)                                                \
        maxds_ld(W(XG), W(MS), W(DS))

#define maxfs3rr(XD, XS, XT)                                                \
        maxds3rr(W(XD), W(XS), W(XT))

#define maxfs3ld(XD, XS, MT, DT)                                            \
        maxds3ld(W(XD), W(XS), W(MT), W(DT))

#define mxpfs_rr(XG, XS) /* horizontal pairwise max */                      \
        mxpds_rr(W(XG), W(XS))

#define mxpfs_ld(XG, MS, DS)                                                \
        mxpds_ld(W(XG), W(MS), W(DS))

#define mxpfs3rr(XD, XS, XT)                                                \
        mxpds3rr(W(XD), W(XS), W(XT))

#define mxpfs3ld(XD, XS, MT, DT)                                            \
        mxpds3ld(W(XD), W(XS), W(MT), W(DT))

#define mxhfs_rr(XD, XS) /* horizontal reductive max */                     \
        mxhds_rr(W(XD), W(XS))

#define mxhfs_ld(XD, MS, DS)                                                \
        mxhds_ld(W(XD), W(MS), W(DS))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqfs_rr(XG, XS)                                                    \
        ceqds_rr(W(XG), W(XS))

#define ceqfs_ld(XG, MS, DS)                                                \
        ceqds_ld(W(XG), W(MS), W(DS))

#define ceqfs3rr(XD, XS, XT)                                                \
        ceqds3rr(W(XD), W(XS), W(XT))

#define ceqfs3ld(XD, XS, MT, DT)                                            \
        ceqds3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnefs_rr(XG, XS)                                                    \
        cneds_rr(W(XG), W(XS))

#define cnefs_ld(XG, MS, DS)                                                \
        cneds_ld(W(XG), W(MS), W(DS))

#define cnefs3rr(XD, XS, XT)                                                \
        cneds3rr(W(XD), W(XS), W(XT))

#define cnefs3ld(XD, XS, MT, DT)                                            \
        cneds3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltfs_rr(XG, XS)                                                    \
        cltds_rr(W(XG), W(XS))

#define cltfs_ld(XG, MS, DS)                                                \
        cltds_ld(W(XG), W(MS), W(DS))

#define cltfs3rr(XD, XS, XT)                                                \
        cltds3rr(W(XD), W(XS), W(XT))

#define cltfs3ld(XD, XS, MT, DT)                                            \
        cltds3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define clefs_rr(XG, XS)                                                    \
        cleds_rr(W(XG), W(XS))

#define clefs_ld(XG, MS, DS)                                                \
        cleds_ld(W(XG), W(MS), W(DS))

#define clefs3rr(XD, XS, XT)                                                \
        cleds3rr(W(XD), W(XS), W(XT))

#define clefs3ld(XD, XS, MT, DT)                                            \
        cleds3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtfs_rr(XG, XS)                                                    \
        cgtds_rr(W(XG), W(XS))

#define cgtfs_ld(XG, MS, DS)                                                \
        cgtds_ld(W(XG), W(MS), W(DS))

#define cgtfs3rr(XD, XS, XT)                                                \
        cgtds3rr(W(XD), W(XS), W(XT))

#define cgtfs3ld(XD, XS, MT, DT)                                            \
        cgtds3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgefs_rr(XG, XS)                                                    \
        cgeds_rr(W(XG), W(XS))

#define cgefs_ld(XG, MS, DS)                                                \
        cgeds_ld(W(XG), W(MS), W(DS))

#define cgefs3rr(XD, XS, XT)                                                \
        cgeds3rr(W(XD), W(XS), W(XT))

#define cgefs3ld(XD, XS, MT, DT)                                            \
        cgeds3ld(W(XD), W(XS), W(MT), W(DT))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define mkjfx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        mkjdx_rx(W(XS), mask, lb)

/*************   packed double-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnzfs_rr(XD, XS)     /* round towards zero */                       \
        rnzds_rr(W(XD), W(XS))

#define rnzfs_ld(XD, MS, DS) /* round towards zero */                       \
        rnzds_ld(W(XD), W(MS), W(DS))

#define cvzfs_rr(XD, XS)     /* round towards zero */                       \
        cvzds_rr(W(XD), W(XS))

#define cvzfs_ld(XD, MS, DS) /* round towards zero */                       \
        cvzds_ld(W(XD), W(MS), W(DS))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnpfs_rr(XD, XS)     /* round towards +inf */                       \
        rnpds_rr(W(XD), W(XS))

#define rnpfs_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpds_ld(W(XD), W(MS), W(DS))

#define cvpfs_rr(XD, XS)     /* round towards +inf */                       \
        cvpds_rr(W(XD), W(XS))

#define cvpfs_ld(XD, MS, DS) /* round towards +inf */                       \
        cvpds_ld(W(XD), W(MS), W(DS))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnmfs_rr(XD, XS)     /* round towards -inf */                       \
        rnmds_rr(W(XD), W(XS))

#define rnmfs_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmds_ld(W(XD), W(MS), W(DS))

#define cvmfs_rr(XD, XS)     /* round towards -inf */                       \
        cvmds_rr(W(XD), W(XS))

#define cvmfs_ld(XD, MS, DS) /* round towards -inf */                       \
        cvmds_ld(W(XD), W(MS), W(DS))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnnfs_rr(XD, XS)     /* round towards near */                       \
        rnnds_rr(W(XD), W(XS))

#define rnnfs_ld(XD, MS, DS) /* round towards near */                       \
        rnnds_ld(W(XD), W(MS), W(DS))

#define cvnfs_rr(XD, XS)     /* round towards near */                       \
        cvnds_rr(W(XD), W(XS))

#define cvnfs_ld(XD, MS, DS) /* round towards near */                       \
        cvnds_ld(W(XD), W(MS), W(DS))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnfn_rr(XD, XS)     /* round towards near */                       \
        cvndn_rr(W(XD), W(XS))

#define cvnfn_ld(XD, MS, DS) /* round towards near */                       \
        cvndn_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndfs_rr(XD, XS)                                                    \
        rndds_rr(W(XD), W(XS))

#define rndfs_ld(XD, MS, DS)                                                \
        rndds_ld(W(XD), W(MS), W(DS))

#define cvtfs_rr(XD, XS)                                                    \
        cvtds_rr(W(XD), W(XS))

#define cvtfs_ld(XD, MS, DS)                                                \
        cvtds_ld(W(XD), W(MS), W(DS))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtfn_rr(XD, XS)                                                    \
        cvtdn_rr(W(XD), W(XS))

#define cvtfn_ld(XD, MS, DS)                                                \
        cvtdn_ld(W(XD), W(MS), W(DS))

/* cvn (D = unsigned-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnfx_rr(XD, XS)     /* round towards near */                       \
        cvndx_rr(W(XD), W(XS))

#define cvnfx_ld(XD, MS, DS) /* round towards near */                       \
        cvndx_ld(W(XD), W(MS), W(DS))

/* cvt (D = unsigned-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtfx_rr(XD, XS)                                                    \
        cvtdx_rr(W(XD), W(XS))

#define cvtfx_ld(XD, MS, DS)                                                \
        cvtdx_ld(W(XD), W(MS), W(DS))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnrfs_rr(XD, XS, mode)                                              \
        rnrds_rr(W(XD), W(XS), mode)

#define cvrfs_rr(XD, XS, mode)                                              \
        cvrds_rr(W(XD), W(XS), mode)

/************   packed double-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addfx_rr(XG, XS)                                                    \
        adddx_rr(W(XG), W(XS))

#define addfx_ld(XG, MS, DS)                                                \
        adddx_ld(W(XG), W(MS), W(DS))

#define addfx3rr(XD, XS, XT)                                                \
        adddx3rr(W(XD), W(XS), W(XT))

#define addfx3ld(XD, XS, MT, DT)                                            \
        adddx3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subfx_rr(XG, XS)                                                    \
        subdx_rr(W(XG), W(XS))

#define subfx_ld(XG, MS, DS)                                                \
        subdx_ld(W(XG), W(MS), W(DS))

#define subfx3rr(XD, XS, XT)                                                \
        subdx3rr(W(XD), W(XS), W(XT))

#define subfx3ld(XD, XS, MT, DT)                                            \
        subdx3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulfx_rr(XG, XS)                                                    \
        muldx_rr(W(XG), W(XS))

#define mulfx_ld(XG, MS, DS)                                                \
        muldx_ld(W(XG), W(MS), W(DS))

#define mulfx3rr(XD, XS, XT)                                                \
        muldx3rr(W(XD), W(XS), W(XT))

#define mulfx3ld(XD, XS, MT, DT)                                            \
        muldx3ld(W(XD), W(XS), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlfx_ri(XG, IS)                                                    \
        shldx_ri(W(XG), W(IS))

#define shlfx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shldx_ld(W(XG), W(MS), W(DS))

#define shlfx3ri(XD, XS, IT)                                                \
        shldx3ri(W(XD), W(XS), W(IT))

#define shlfx3ld(XD, XS, MT, DT)                                            \
        shldx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrfx_ri(XG, IS)                                                    \
        shrdx_ri(W(XG), W(IS))

#define shrfx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrdx_ld(W(XG), W(MS), W(DS))

#define shrfx3ri(XD, XS, IT)                                                \
        shrdx3ri(W(XD), W(XS), W(IT))

#define shrfx3ld(XD, XS, MT, DT)                                            \
        shrdx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrfn_ri(XG, IS)                                                    \
        shrdn_ri(W(XG), W(IS))

#define shrfn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrdn_ld(W(XG), W(MS), W(DS))

#define shrfn3ri(XD, XS, IT)                                                \
        shrdn3ri(W(XD), W(XS), W(IT))

#define shrfn3ld(XD, XS, MT, DT)                                            \
        shrdn3ld(W(XD), W(XS), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlfx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svldx_rr(W(XG), W(XS))

#define svlfx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svldx_ld(W(XG), W(MS), W(DS))

#define svlfx3rr(XD, XS, XT)                                                \
        svldx3rr(W(XD), W(XS), W(XT))

#define svlfx3ld(XD, XS, MT, DT)                                            \
        svldx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrfx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrdx_rr(W(XG), W(XS))

#define svrfx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrdx_ld(W(XG), W(MS), W(DS))

#define svrfx3rr(XD, XS, XT)                                                \
        svrdx3rr(W(XD), W(XS), W(XT))

#define svrfx3ld(XD, XS, MT, DT)                                            \
        svrdx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrfn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrdn_rr(W(XG), W(XS))

#define svrfn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrdn_ld(W(XG), W(MS), W(DS))

#define svrfn3rr(XD, XS, XT)                                                \
        svrdn3rr(W(XD), W(XS), W(XT))

#define svrfn3ld(XD, XS, MT, DT)                                            \
        svrdn3ld(W(XD), W(XS), W(MT), W(DT))

/****************   packed double-precision integer compare   *****************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minfx_rr(XG, XS)                                                    \
        mindx_rr(W(XG), W(XS))

#define minfx_ld(XG, MS, DS)                                                \
        mindx_ld(W(XG), W(MS), W(DS))

#define minfx3rr(XD, XS, XT)                                                \
        mindx3rr(W(XD), W(XS), W(XT))

#define minfx3ld(XD, XS, MT, DT)                                            \
        mindx3ld(W(XD), W(XS), W(MT), W(DT))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minfn_rr(XG, XS)                                                    \
        mindn_rr(W(XG), W(XS))

#define minfn_ld(XG, MS, DS)                                                \
        mindn_ld(W(XG), W(MS), W(DS))

#define minfn3rr(XD, XS, XT)                                                \
        mindn3rr(W(XD), W(XS), W(XT))

#define minfn3ld(XD, XS, MT, DT)                                            \
        mindn3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxfx_rr(XG, XS)                                                    \
        maxdx_rr(W(XG), W(XS))

#define maxfx_ld(XG, MS, DS)                                                \
        maxdx_ld(W(XG), W(MS), W(DS))

#define maxfx3rr(XD, XS, XT)                                                \
        maxdx3rr(W(XD), W(XS), W(XT))

#define maxfx3ld(XD, XS, MT, DT)                                            \
        maxdx3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxfn_rr(XG, XS)                                                    \
        maxdn_rr(W(XG), W(XS))

#define maxfn_ld(XG, MS, DS)                                                \
        maxdn_ld(W(XG), W(MS), W(DS))

#define maxfn3rr(XD, XS, XT)                                                \
        maxdn3rr(W(XD), W(XS), W(XT))

#define maxfn3ld(XD, XS, MT, DT)                                            \
        maxdn3ld(W(XD), W(XS), W(MT), W(DT))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqfx_rr(XG, XS)                                                    \
        ceqdx_rr(W(XG), W(XS))

#define ceqfx_ld(XG, MS, DS)                                                \
        ceqdx_ld(W(XG), W(MS), W(DS))

#define ceqfx3rr(XD, XS, XT)                                                \
        ceqdx3rr(W(XD), W(XS), W(XT))

#define ceqfx3ld(XD, XS, MT, DT)                                            \
        ceqdx3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnefx_rr(XG, XS)                                                    \
        cnedx_rr(W(XG), W(XS))

#define cnefx_ld(XG, MS, DS)                                                \
        cnedx_ld(W(XG), W(MS), W(DS))

#define cnefx3rr(XD, XS, XT)                                                \
        cnedx3rr(W(XD), W(XS), W(XT))

#define cnefx3ld(XD, XS, MT, DT)                                            \
        cnedx3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltfx_rr(XG, XS)                                                    \
        cltdx_rr(W(XG), W(XS))

#define cltfx_ld(XG, MS, DS)                                                \
        cltdx_ld(W(XG), W(MS), W(DS))

#define cltfx3rr(XD, XS, XT)                                                \
        cltdx3rr(W(XD), W(XS), W(XT))

#define cltfx3ld(XD, XS, MT, DT)                                            \
        cltdx3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltfn_rr(XG, XS)                                                    \
        cltdn_rr(W(XG), W(XS))

#define cltfn_ld(XG, MS, DS)                                                \
        cltdn_ld(W(XG), W(MS), W(DS))

#define cltfn3rr(XD, XS, XT)                                                \
        cltdn3rr(W(XD), W(XS), W(XT))

#define cltfn3ld(XD, XS, MT, DT)                                            \
        cltdn3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clefx_rr(XG, XS)                                                    \
        cledx_rr(W(XG), W(XS))

#define clefx_ld(XG, MS, DS)                                                \
        cledx_ld(W(XG), W(MS), W(DS))

#define clefx3rr(XD, XS, XT)                                                \
        cledx3rr(W(XD), W(XS), W(XT))

#define clefx3ld(XD, XS, MT, DT)                                            \
        cledx3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clefn_rr(XG, XS)                                                    \
        cledn_rr(W(XG), W(XS))

#define clefn_ld(XG, MS, DS)                                                \
        cledn_ld(W(XG), W(MS), W(DS))

#define clefn3rr(XD, XS, XT)                                                \
        cledn3rr(W(XD), W(XS), W(XT))

#define clefn3ld(XD, XS, MT, DT)                                            \
        cledn3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtfx_rr(XG, XS)                                                    \
        cgtdx_rr(W(XG), W(XS))

#define cgtfx_ld(XG, MS, DS)                                                \
        cgtdx_ld(W(XG), W(MS), W(DS))

#define cgtfx3rr(XD, XS, XT)                                                \
        cgtdx3rr(W(XD), W(XS), W(XT))

#define cgtfx3ld(XD, XS, MT, DT)                                            \
        cgtdx3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtfn_rr(XG, XS)                                                    \
        cgtdn_rr(W(XG), W(XS))

#define cgtfn_ld(XG, MS, DS)                                                \
        cgtdn_ld(W(XG), W(MS), W(DS))

#define cgtfn3rr(XD, XS, XT)                                                \
        cgtdn3rr(W(XD), W(XS), W(XT))

#define cgtfn3ld(XD, XS, MT, DT)                                            \
        cgtdn3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgefx_rr(XG, XS)                                                    \
        cgedx_rr(W(XG), W(XS))

#define cgefx_ld(XG, MS, DS)                                                \
        cgedx_ld(W(XG), W(MS), W(DS))

#define cgefx3rr(XD, XS, XT)                                                \
        cgedx3rr(W(XD), W(XS), W(XT))

#define cgefx3ld(XD, XS, MT, DT)                                            \
        cgedx3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgefn_rr(XG, XS)                                                    \
        cgedn_rr(W(XG), W(XS))

#define cgefn_ld(XG, MS, DS)                                                \
        cgedn_ld(W(XG), W(MS), W(DS))

#define cgefn3rr(XD, XS, XT)                                                \
        cgedn3rr(W(XD), W(XS), W(XT))

#define cgefn3ld(XD, XS, MT, DT)                                            \
        cgedn3ld(W(XD), W(XS), W(MT), W(DT))

/******************************************************************************/
/**** 128-bit **** SIMD instructions with configurable element **** 64-bit ****/
/******************************************************************************/

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmlx_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        elmjx_st(W(XS), W(MD), W(DD))

/***************   packed double-precision generic move/logic   ***************/

/* mov (D = S) */

#define movlx_rr(XD, XS)                                                    \
        movjx_rr(W(XD), W(XS))

#define movlx_ld(XD, MS, DS)                                                \
        movjx_ld(W(XD), W(MS), W(DS))

#define movlx_st(XS, MD, DD)                                                \
        movjx_st(W(XS), W(MD), W(DD))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvlx_rr(XD, XS)                                                    \
        mmvjx_rr(W(XD), W(XS))

#define mmvlx_ld(XG, MS, DS)                                                \
        mmvjx_ld(W(XG), W(MS), W(DS))

#define mmvlx_st(XS, MG, DG)                                                \
        mmvjx_st(W(XS), W(MG), W(DG))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andlx_rr(XG, XS)                                                    \
        andjx_rr(W(XG), W(XS))

#define andlx_ld(XG, MS, DS)                                                \
        andjx_ld(W(XG), W(MS), W(DS))

#define andlx3rr(XD, XS, XT)                                                \
        andjx3rr(W(XD), W(XS), W(XT))

#define andlx3ld(XD, XS, MT, DT)                                            \
        andjx3ld(W(XD), W(XS), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annlx_rr(XG, XS)                                                    \
        annjx_rr(W(XG), W(XS))

#define annlx_ld(XG, MS, DS)                                                \
        annjx_ld(W(XG), W(MS), W(DS))

#define annlx3rr(XD, XS, XT)                                                \
        annjx3rr(W(XD), W(XS), W(XT))

#define annlx3ld(XD, XS, MT, DT)                                            \
        annjx3ld(W(XD), W(XS), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrlx_rr(XG, XS)                                                    \
        orrjx_rr(W(XG), W(XS))

#define orrlx_ld(XG, MS, DS)                                                \
        orrjx_ld(W(XG), W(MS), W(DS))

#define orrlx3rr(XD, XS, XT)                                                \
        orrjx3rr(W(XD), W(XS), W(XT))

#define orrlx3ld(XD, XS, MT, DT)                                            \
        orrjx3ld(W(XD), W(XS), W(MT), W(DT))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define ornlx_rr(XG, XS)                                                    \
        ornjx_rr(W(XG), W(XS))

#define ornlx_ld(XG, MS, DS)                                                \
        ornjx_ld(W(XG), W(MS), W(DS))

#define ornlx3rr(XD, XS, XT)                                                \
        ornjx3rr(W(XD), W(XS), W(XT))

#define ornlx3ld(XD, XS, MT, DT)                                            \
        ornjx3ld(W(XD), W(XS), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorlx_rr(XG, XS)                                                    \
        xorjx_rr(W(XG), W(XS))

#define xorlx_ld(XG, MS, DS)                                                \
        xorjx_ld(W(XG), W(MS), W(DS))

#define xorlx3rr(XD, XS, XT)                                                \
        xorjx3rr(W(XD), W(XS), W(XT))

#define xorlx3ld(XD, XS, MT, DT)                                            \
        xorjx3ld(W(XD), W(XS), W(MT), W(DT))

/* not (G = ~G), (D = ~S) */

#define notlx_rx(XG)                                                        \
        notjx_rx(W(XG))

#define notlx_rr(XD, XS)                                                    \
        notjx_rr(W(XD), W(XS))

/************   packed double-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negls_rx(XG)                                                        \
        negjs_rx(W(XG))

#define negls_rr(XD, XS)                                                    \
        negjs_rr(W(XD), W(XS))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addls_rr(XG, XS)                                                    \
        addjs_rr(W(XG), W(XS))

#define addls_ld(XG, MS, DS)                                                \
        addjs_ld(W(XG), W(MS), W(DS))

#define addls3rr(XD, XS, XT)                                                \
        addjs3rr(W(XD), W(XS), W(XT))

#define addls3ld(XD, XS, MT, DT)                                            \
        addjs3ld(W(XD), W(XS), W(MT), W(DT))

#define adpls_rr(XG, XS) /* horizontal pairwise add, first 15-regs only */  \
        adpjs_rr(W(XG), W(XS))

#define adpls_ld(XG, MS, DS)                                                \
        adpjs_ld(W(XG), W(MS), W(DS))

#define adpls3rr(XD, XS, XT)                                                \
        adpjs3rr(W(XD), W(XS), W(XT))

#define adpls3ld(XD, XS, MT, DT)                                            \
        adpjs3ld(W(XD), W(XS), W(MT), W(DT))

#define adhls_rr(XD, XS) /* horizontal reductive add, first 15-regs only */ \
        adhjs_rr(W(XD), W(XS))

#define adhls_ld(XD, MS, DS)                                                \
        adhjs_ld(W(XD), W(MS), W(DS))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subls_rr(XG, XS)                                                    \
        subjs_rr(W(XG), W(XS))

#define subls_ld(XG, MS, DS)                                                \
        subjs_ld(W(XG), W(MS), W(DS))

#define subls3rr(XD, XS, XT)                                                \
        subjs3rr(W(XD), W(XS), W(XT))

#define subls3ld(XD, XS, MT, DT)                                            \
        subjs3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulls_rr(XG, XS)                                                    \
        muljs_rr(W(XG), W(XS))

#define mulls_ld(XG, MS, DS)                                                \
        muljs_ld(W(XG), W(MS), W(DS))

#define mulls3rr(XD, XS, XT)                                                \
        muljs3rr(W(XD), W(XS), W(XT))

#define mulls3ld(XD, XS, MT, DT)                                            \
        muljs3ld(W(XD), W(XS), W(MT), W(DT))

#define mlpls_rr(XG, XS) /* horizontal pairwise mul */                      \
        mlpjs_rr(W(XG), W(XS))

#define mlpls_ld(XG, MS, DS)                                                \
        mlpjs_ld(W(XG), W(MS), W(DS))

#define mlpls3rr(XD, XS, XT)                                                \
        mlpjs3rr(W(XD), W(XS), W(XT))

#define mlpls3ld(XD, XS, MT, DT)                                            \
        mlpjs3ld(W(XD), W(XS), W(MT), W(DT))

#define mlhls_rr(XD, XS) /* horizontal reductive mul */                     \
        mlhjs_rr(W(XD), W(XS))

#define mlhls_ld(XD, MS, DS)                                                \
        mlhjs_ld(W(XD), W(MS), W(DS))

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divls_rr(XG, XS)                                                    \
        divjs_rr(W(XG), W(XS))

#define divls_ld(XG, MS, DS)                                                \
        divjs_ld(W(XG), W(MS), W(DS))

#define divls3rr(XD, XS, XT)                                                \
        divjs3rr(W(XD), W(XS), W(XT))

#define divls3ld(XD, XS, MT, DT)                                            \
        divjs3ld(W(XD), W(XS), W(MT), W(DT))

/* sqr (D = sqrt S) */

#define sqrls_rr(XD, XS)                                                    \
        sqrjs_rr(W(XD), W(XS))

#define sqrls_ld(XD, MS, DS)                                                \
        sqrjs_ld(W(XD), W(MS), W(DS))

/* cbr (D = cbrt S) */

#define cbrls_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbrjs_rr(W(XD), W(X1), W(X2), W(XS))

#define cbels_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbejs_rr(W(XD), W(X1), W(X2), W(XS))

#define cbsls_rr(XG, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbsjs_rr(W(XG), W(X1), W(X2), W(XS))

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rcpls_rr(XD, XS) /* destroys XS */                                  \
        rcpjs_rr(W(XD), W(XS))

#define rcels_rr(XD, XS)                                                    \
        rcejs_rr(W(XD), W(XS))

#define rcsls_rr(XG, XS) /* destroys XS */                                  \
        rcsjs_rr(W(XG), W(XS))

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rsqls_rr(XD, XS) /* destroys XS */                                  \
        rsqjs_rr(W(XD), W(XS))

#define rsels_rr(XD, XS)                                                    \
        rsejs_rr(W(XD), W(XS))

#define rssls_rr(XG, XS) /* destroys XS */                                  \
        rssjs_rr(W(XG), W(XS))

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmals_rr(XG, XS, XT)                                                \
        fmajs_rr(W(XG), W(XS), W(XT))

#define fmals_ld(XG, XS, MT, DT)                                            \
        fmajs_ld(W(XG), W(XS), W(MT), W(DT))

#define fmals3rr(XG, XS, XT)                                                \
        fmals_rr(W(XG), W(XS), W(XT))

#define fmals3ld(XG, XS, MT, DT)                                            \
        fmals_ld(W(XG), W(XS), W(MT), W(DT))

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsls_rr(XG, XS, XT)                                                \
        fmsjs_rr(W(XG), W(XS), W(XT))

#define fmsls_ld(XG, XS, MT, DT)                                            \
        fmsjs_ld(W(XG), W(XS), W(MT), W(DT))

#define fmsls3rr(XG, XS, XT)                                                \
        fmsls_rr(W(XG), W(XS), W(XT))

#define fmsls3ld(XG, XS, MT, DT)                                            \
        fmsls_ld(W(XG), W(XS), W(MT), W(DT))

/*************   packed double-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minls_rr(XG, XS)                                                    \
        minjs_rr(W(XG), W(XS))

#define minls_ld(XG, MS, DS)                                                \
        minjs_ld(W(XG), W(MS), W(DS))

#define minls3rr(XD, XS, XT)                                                \
        minjs3rr(W(XD), W(XS), W(XT))

#define minls3ld(XD, XS, MT, DT)                                            \
        minjs3ld(W(XD), W(XS), W(MT), W(DT))

#define mnpls_rr(XG, XS) /* horizontal pairwise min */                      \
        mnpjs_rr(W(XG), W(XS))

#define mnpls_ld(XG, MS, DS)                                                \
        mnpjs_ld(W(XG), W(MS), W(DS))

#define mnpls3rr(XD, XS, XT)                                                \
        mnpjs3rr(W(XD), W(XS), W(XT))

#define mnpls3ld(XD, XS, MT, DT)                                            \
        mnpjs3ld(W(XD), W(XS), W(MT), W(DT))

#define mnhls_rr(XD, XS) /* horizontal reductive min */                     \
        mnhjs_rr(W(XD), W(XS))

#define mnhls_ld(XD, MS, DS)                                                \
        mnhjs_ld(W(XD), W(MS), W(DS))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxls_rr(XG, XS)                                                    \
        maxjs_rr(W(XG), W(XS))

#define maxls_ld(XG, MS, DS)                                                \
        maxjs_ld(W(XG), W(MS), W(DS))

#define maxls3rr(XD, XS, XT)                                                \
        maxjs3rr(W(XD), W(XS), W(XT))

#define maxls3ld(XD, XS, MT, DT)                                            \
        maxjs3ld(W(XD), W(XS), W(MT), W(DT))

#define mxpls_rr(XG, XS) /* horizontal pairwise max */                      \
        mxpjs_rr(W(XG), W(XS))

#define mxpls_ld(XG, MS, DS)                                                \
        mxpjs_ld(W(XG), W(MS), W(DS))

#define mxpls3rr(XD, XS, XT)                                                \
        mxpjs3rr(W(XD), W(XS), W(XT))

#define mxpls3ld(XD, XS, MT, DT)                                            \
        mxpjs3ld(W(XD), W(XS), W(MT), W(DT))

#define mxhls_rr(XD, XS) /* horizontal reductive max */                     \
        mxhjs_rr(W(XD), W(XS))

#define mxhls_ld(XD, MS, DS)                                                \
        mxhjs_ld(W(XD), W(MS), W(DS))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqls_rr(XG, XS)                                                    \
        ceqjs_rr(W(XG), W(XS))

#define ceqls_ld(XG, MS, DS)                                                \
        ceqjs_ld(W(XG), W(MS), W(DS))

#define ceqls3rr(XD, XS, XT)                                                \
        ceqjs3rr(W(XD), W(XS), W(XT))

#define ceqls3ld(XD, XS, MT, DT)                                            \
        ceqjs3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnels_rr(XG, XS)                                                    \
        cnejs_rr(W(XG), W(XS))

#define cnels_ld(XG, MS, DS)                                                \
        cnejs_ld(W(XG), W(MS), W(DS))

#define cnels3rr(XD, XS, XT)                                                \
        cnejs3rr(W(XD), W(XS), W(XT))

#define cnels3ld(XD, XS, MT, DT)                                            \
        cnejs3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltls_rr(XG, XS)                                                    \
        cltjs_rr(W(XG), W(XS))

#define cltls_ld(XG, MS, DS)                                                \
        cltjs_ld(W(XG), W(MS), W(DS))

#define cltls3rr(XD, XS, XT)                                                \
        cltjs3rr(W(XD), W(XS), W(XT))

#define cltls3ld(XD, XS, MT, DT)                                            \
        cltjs3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define clels_rr(XG, XS)                                                    \
        clejs_rr(W(XG), W(XS))

#define clels_ld(XG, MS, DS)                                                \
        clejs_ld(W(XG), W(MS), W(DS))

#define clels3rr(XD, XS, XT)                                                \
        clejs3rr(W(XD), W(XS), W(XT))

#define clels3ld(XD, XS, MT, DT)                                            \
        clejs3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtls_rr(XG, XS)                                                    \
        cgtjs_rr(W(XG), W(XS))

#define cgtls_ld(XG, MS, DS)                                                \
        cgtjs_ld(W(XG), W(MS), W(DS))

#define cgtls3rr(XD, XS, XT)                                                \
        cgtjs3rr(W(XD), W(XS), W(XT))

#define cgtls3ld(XD, XS, MT, DT)                                            \
        cgtjs3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgels_rr(XG, XS)                                                    \
        cgejs_rr(W(XG), W(XS))

#define cgels_ld(XG, MS, DS)                                                \
        cgejs_ld(W(XG), W(MS), W(DS))

#define cgels3rr(XD, XS, XT)                                                \
        cgejs3rr(W(XD), W(XS), W(XT))

#define cgels3ld(XD, XS, MT, DT)                                            \
        cgejs3ld(W(XD), W(XS), W(MT), W(DT))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define mkjlx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        mkjjx_rx(W(XS), mask, lb)

/*************   packed double-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnzls_rr(XD, XS)     /* round towards zero */                       \
        rnzjs_rr(W(XD), W(XS))

#define rnzls_ld(XD, MS, DS) /* round towards zero */                       \
        rnzjs_ld(W(XD), W(MS), W(DS))

#define cvzls_rr(XD, XS)     /* round towards zero */                       \
        cvzjs_rr(W(XD), W(XS))

#define cvzls_ld(XD, MS, DS) /* round towards zero */                       \
        cvzjs_ld(W(XD), W(MS), W(DS))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnpls_rr(XD, XS)     /* round towards +inf */                       \
        rnpjs_rr(W(XD), W(XS))

#define rnpls_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpjs_ld(W(XD), W(MS), W(DS))

#define cvpls_rr(XD, XS)     /* round towards +inf */                       \
        cvpjs_rr(W(XD), W(XS))

#define cvpls_ld(XD, MS, DS) /* round towards +inf */                       \
        cvpjs_ld(W(XD), W(MS), W(DS))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnmls_rr(XD, XS)     /* round towards -inf */                       \
        rnmjs_rr(W(XD), W(XS))

#define rnmls_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmjs_ld(W(XD), W(MS), W(DS))

#define cvmls_rr(XD, XS)     /* round towards -inf */                       \
        cvmjs_rr(W(XD), W(XS))

#define cvmls_ld(XD, MS, DS) /* round towards -inf */                       \
        cvmjs_ld(W(XD), W(MS), W(DS))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnnls_rr(XD, XS)     /* round towards near */                       \
        rnnjs_rr(W(XD), W(XS))

#define rnnls_ld(XD, MS, DS) /* round towards near */                       \
        rnnjs_ld(W(XD), W(MS), W(DS))

#define cvnls_rr(XD, XS)     /* round towards near */                       \
        cvnjs_rr(W(XD), W(XS))

#define cvnls_ld(XD, MS, DS) /* round towards near */                       \
        cvnjs_ld(W(XD), W(MS), W(DS))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnln_rr(XD, XS)     /* round towards near */                       \
        cvnjn_rr(W(XD), W(XS))

#define cvnln_ld(XD, MS, DS) /* round towards near */                       \
        cvnjn_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndls_rr(XD, XS)                                                    \
        rndjs_rr(W(XD), W(XS))

#define rndls_ld(XD, MS, DS)                                                \
        rndjs_ld(W(XD), W(MS), W(DS))

#define cvtls_rr(XD, XS)                                                    \
        cvtjs_rr(W(XD), W(XS))

#define cvtls_ld(XD, MS, DS)                                                \
        cvtjs_ld(W(XD), W(MS), W(DS))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtln_rr(XD, XS)                                                    \
        cvtjn_rr(W(XD), W(XS))

#define cvtln_ld(XD, MS, DS)                                                \
        cvtjn_ld(W(XD), W(MS), W(DS))

/* cvn (D = unsigned-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnlx_rr(XD, XS)     /* round towards near */                       \
        cvnjx_rr(W(XD), W(XS))

#define cvnlx_ld(XD, MS, DS) /* round towards near */                       \
        cvnjx_ld(W(XD), W(MS), W(DS))

/* cvt (D = unsigned-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtlx_rr(XD, XS)                                                    \
        cvtjx_rr(W(XD), W(XS))

#define cvtlx_ld(XD, MS, DS)                                                \
        cvtjx_ld(W(XD), W(MS), W(DS))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnrls_rr(XD, XS, mode)                                              \
        rnrjs_rr(W(XD), W(XS), mode)

#define cvrls_rr(XD, XS, mode)                                              \
        cvrjs_rr(W(XD), W(XS), mode)

/************   packed double-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addlx_rr(XG, XS)                                                    \
        addjx_rr(W(XG), W(XS))

#define addlx_ld(XG, MS, DS)                                                \
        addjx_ld(W(XG), W(MS), W(DS))

#define addlx3rr(XD, XS, XT)                                                \
        addjx3rr(W(XD), W(XS), W(XT))

#define addlx3ld(XD, XS, MT, DT)                                            \
        addjx3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define sublx_rr(XG, XS)                                                    \
        subjx_rr(W(XG), W(XS))

#define sublx_ld(XG, MS, DS)                                                \
        subjx_ld(W(XG), W(MS), W(DS))

#define sublx3rr(XD, XS, XT)                                                \
        subjx3rr(W(XD), W(XS), W(XT))

#define sublx3ld(XD, XS, MT, DT)                                            \
        subjx3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mullx_rr(XG, XS)                                                    \
        muljx_rr(W(XG), W(XS))

#define mullx_ld(XG, MS, DS)                                                \
        muljx_ld(W(XG), W(MS), W(DS))

#define mullx3rr(XD, XS, XT)                                                \
        muljx3rr(W(XD), W(XS), W(XT))

#define mullx3ld(XD, XS, MT, DT)                                            \
        muljx3ld(W(XD), W(XS), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shllx_ri(XG, IS)                                                    \
        shljx_ri(W(XG), W(IS))

#define shllx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shljx_ld(W(XG), W(MS), W(DS))

#define shllx3ri(XD, XS, IT)                                                \
        shljx3ri(W(XD), W(XS), W(IT))

#define shllx3ld(XD, XS, MT, DT)                                            \
        shljx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrlx_ri(XG, IS)                                                    \
        shrjx_ri(W(XG), W(IS))

#define shrlx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrjx_ld(W(XG), W(MS), W(DS))

#define shrlx3ri(XD, XS, IT)                                                \
        shrjx3ri(W(XD), W(XS), W(IT))

#define shrlx3ld(XD, XS, MT, DT)                                            \
        shrjx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrln_ri(XG, IS)                                                    \
        shrjn_ri(W(XG), W(IS))

#define shrln_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrjn_ld(W(XG), W(MS), W(DS))

#define shrln3ri(XD, XS, IT)                                                \
        shrjn3ri(W(XD), W(XS), W(IT))

#define shrln3ld(XD, XS, MT, DT)                                            \
        shrjn3ld(W(XD), W(XS), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svllx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svljx_rr(W(XG), W(XS))

#define svllx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svljx_ld(W(XG), W(MS), W(DS))

#define svllx3rr(XD, XS, XT)                                                \
        svljx3rr(W(XD), W(XS), W(XT))

#define svllx3ld(XD, XS, MT, DT)                                            \
        svljx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrlx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrjx_rr(W(XG), W(XS))

#define svrlx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrjx_ld(W(XG), W(MS), W(DS))

#define svrlx3rr(XD, XS, XT)                                                \
        svrjx3rr(W(XD), W(XS), W(XT))

#define svrlx3ld(XD, XS, MT, DT)                                            \
        svrjx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrln_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrjn_rr(W(XG), W(XS))

#define svrln_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrjn_ld(W(XG), W(MS), W(DS))

#define svrln3rr(XD, XS, XT)                                                \
        svrjn3rr(W(XD), W(XS), W(XT))

#define svrln3ld(XD, XS, MT, DT)                                            \
        svrjn3ld(W(XD), W(XS), W(MT), W(DT))

/****************   packed double-precision integer compare   *****************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minlx_rr(XG, XS)                                                    \
        minjx_rr(W(XG), W(XS))

#define minlx_ld(XG, MS, DS)                                                \
        minjx_ld(W(XG), W(MS), W(DS))

#define minlx3rr(XD, XS, XT)                                                \
        minjx3rr(W(XD), W(XS), W(XT))

#define minlx3ld(XD, XS, MT, DT)                                            \
        minjx3ld(W(XD), W(XS), W(MT), W(DT))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minln_rr(XG, XS)                                                    \
        minjn_rr(W(XG), W(XS))

#define minln_ld(XG, MS, DS)                                                \
        minjn_ld(W(XG), W(MS), W(DS))

#define minln3rr(XD, XS, XT)                                                \
        minjn3rr(W(XD), W(XS), W(XT))

#define minln3ld(XD, XS, MT, DT)                                            \
        minjn3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxlx_rr(XG, XS)                                                    \
        maxjx_rr(W(XG), W(XS))

#define maxlx_ld(XG, MS, DS)                                                \
        maxjx_ld(W(XG), W(MS), W(DS))

#define maxlx3rr(XD, XS, XT)                                                \
        maxjx3rr(W(XD), W(XS), W(XT))

#define maxlx3ld(XD, XS, MT, DT)                                            \
        maxjx3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxln_rr(XG, XS)                                                    \
        maxjn_rr(W(XG), W(XS))

#define maxln_ld(XG, MS, DS)                                                \
        maxjn_ld(W(XG), W(MS), W(DS))

#define maxln3rr(XD, XS, XT)                                                \
        maxjn3rr(W(XD), W(XS), W(XT))

#define maxln3ld(XD, XS, MT, DT)                                            \
        maxjn3ld(W(XD), W(XS), W(MT), W(DT))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqlx_rr(XG, XS)                                                    \
        ceqjx_rr(W(XG), W(XS))

#define ceqlx_ld(XG, MS, DS)                                                \
        ceqjx_ld(W(XG), W(MS), W(DS))

#define ceqlx3rr(XD, XS, XT)                                                \
        ceqjx3rr(W(XD), W(XS), W(XT))

#define ceqlx3ld(XD, XS, MT, DT)                                            \
        ceqjx3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnelx_rr(XG, XS)                                                    \
        cnejx_rr(W(XG), W(XS))

#define cnelx_ld(XG, MS, DS)                                                \
        cnejx_ld(W(XG), W(MS), W(DS))

#define cnelx3rr(XD, XS, XT)                                                \
        cnejx3rr(W(XD), W(XS), W(XT))

#define cnelx3ld(XD, XS, MT, DT)                                            \
        cnejx3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltlx_rr(XG, XS)                                                    \
        cltjx_rr(W(XG), W(XS))

#define cltlx_ld(XG, MS, DS)                                                \
        cltjx_ld(W(XG), W(MS), W(DS))

#define cltlx3rr(XD, XS, XT)                                                \
        cltjx3rr(W(XD), W(XS), W(XT))

#define cltlx3ld(XD, XS, MT, DT)                                            \
        cltjx3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltln_rr(XG, XS)                                                    \
        cltjn_rr(W(XG), W(XS))

#define cltln_ld(XG, MS, DS)                                                \
        cltjn_ld(W(XG), W(MS), W(DS))

#define cltln3rr(XD, XS, XT)                                                \
        cltjn3rr(W(XD), W(XS), W(XT))

#define cltln3ld(XD, XS, MT, DT)                                            \
        cltjn3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clelx_rr(XG, XS)                                                    \
        clejx_rr(W(XG), W(XS))

#define clelx_ld(XG, MS, DS)                                                \
        clejx_ld(W(XG), W(MS), W(DS))

#define clelx3rr(XD, XS, XT)                                                \
        clejx3rr(W(XD), W(XS), W(XT))

#define clelx3ld(XD, XS, MT, DT)                                            \
        clejx3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define cleln_rr(XG, XS)                                                    \
        clejn_rr(W(XG), W(XS))

#define cleln_ld(XG, MS, DS)                                                \
        clejn_ld(W(XG), W(MS), W(DS))

#define cleln3rr(XD, XS, XT)                                                \
        clejn3rr(W(XD), W(XS), W(XT))

#define cleln3ld(XD, XS, MT, DT)                                            \
        clejn3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtlx_rr(XG, XS)                                                    \
        cgtjx_rr(W(XG), W(XS))

#define cgtlx_ld(XG, MS, DS)                                                \
        cgtjx_ld(W(XG), W(MS), W(DS))

#define cgtlx3rr(XD, XS, XT)                                                \
        cgtjx3rr(W(XD), W(XS), W(XT))

#define cgtlx3ld(XD, XS, MT, DT)                                            \
        cgtjx3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtln_rr(XG, XS)                                                    \
        cgtjn_rr(W(XG), W(XS))

#define cgtln_ld(XG, MS, DS)                                                \
        cgtjn_ld(W(XG), W(MS), W(DS))

#define cgtln3rr(XD, XS, XT)                                                \
        cgtjn3rr(W(XD), W(XS), W(XT))

#define cgtln3ld(XD, XS, MT, DT)                                            \
        cgtjn3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgelx_rr(XG, XS)                                                    \
        cgejx_rr(W(XG), W(XS))

#define cgelx_ld(XG, MS, DS)                                                \
        cgejx_ld(W(XG), W(MS), W(DS))

#define cgelx3rr(XD, XS, XT)                                                \
        cgejx3rr(W(XD), W(XS), W(XT))

#define cgelx3ld(XD, XS, MT, DT)                                            \
        cgejx3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgeln_rr(XG, XS)                                                    \
        cgejn_rr(W(XG), W(XS))

#define cgeln_ld(XG, MS, DS)                                                \
        cgejn_ld(W(XG), W(MS), W(DS))

#define cgeln3rr(XD, XS, XT)                                                \
        cgejn3rr(W(XD), W(XS), W(XT))

#define cgeln3ld(XD, XS, MT, DT)                                            \
        cgejn3ld(W(XD), W(XS), W(MT), W(DT))

/******************************************************************************/
/**** scalar ***** SIMD instructions with configurable element **** 64-bit ****/
/******************************************************************************/

/*********   scalar double-precision floating-point move/arithmetic   *********/

/* mov (D = S) */

#define movss_rr(XD, XS)                                                    \
        movts_rr(W(XD), W(XS))

#define movss_ld(XD, MS, DS)                                                \
        movts_ld(W(XD), W(MS), W(DS))

#define movss_st(XS, MD, DD)                                                \
        movts_st(W(XS), W(MD), W(DD))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addss_rr(XG, XS)                                                    \
        addts_rr(W(XG), W(XS))

#define addss_ld(XG, MS, DS)                                                \
        addts_ld(W(XG), W(MS), W(DS))

#define addss3rr(XD, XS, XT)                                                \
        addts3rr(W(XD), W(XS), W(XT))

#define addss3ld(XD, XS, MT, DT)                                            \
        addts3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subss_rr(XG, XS)                                                    \
        subts_rr(W(XG), W(XS))

#define subss_ld(XG, MS, DS)                                                \
        subts_ld(W(XG), W(MS), W(DS))

#define subss3rr(XD, XS, XT)                                                \
        subts3rr(W(XD), W(XS), W(XT))

#define subss3ld(XD, XS, MT, DT)                                            \
        subts3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulss_rr(XG, XS)                                                    \
        mults_rr(W(XG), W(XS))

#define mulss_ld(XG, MS, DS)                                                \
        mults_ld(W(XG), W(MS), W(DS))

#define mulss3rr(XD, XS, XT)                                                \
        mults3rr(W(XD), W(XS), W(XT))

#define mulss3ld(XD, XS, MT, DT)                                            \
        mults3ld(W(XD), W(XS), W(MT), W(DT))

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divss_rr(XG, XS)                                                    \
        divts_rr(W(XG), W(XS))

#define divss_ld(XG, MS, DS)                                                \
        divts_ld(W(XG), W(MS), W(DS))

#define divss3rr(XD, XS, XT)                                                \
        divts3rr(W(XD), W(XS), W(XT))

#define divss3ld(XD, XS, MT, DT)                                            \
        divts3ld(W(XD), W(XS), W(MT), W(DT))

/* sqr (D = sqrt S) */

#define sqrss_rr(XD, XS)                                                    \
        sqrts_rr(W(XD), W(XS))

#define sqrss_ld(XD, MS, DS)                                                \
        sqrts_ld(W(XD), W(MS), W(DS))

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rcpss_rr(XD, XS) /* destroys XS */                                  \
        rcpts_rr(W(XD), W(XS))

#define rcess_rr(XD, XS)                                                    \
        rcets_rr(W(XD), W(XS))

#define rcsss_rr(XG, XS) /* destroys XS */                                  \
        rcsts_rr(W(XG), W(XS))

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rsqss_rr(XD, XS) /* destroys XS */                                  \
        rsqts_rr(W(XD), W(XS))

#define rsess_rr(XD, XS)                                                    \
        rsets_rr(W(XD), W(XS))

#define rssss_rr(XG, XS) /* destroys XS */                                  \
        rssts_rr(W(XG), W(XS))

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmass_rr(XG, XS, XT)                                                \
        fmats_rr(W(XG), W(XS), W(XT))

#define fmass_ld(XG, XS, MT, DT)                                            \
        fmats_ld(W(XG), W(XS), W(MT), W(DT))

#define fmass3rr(XG, XS, XT)                                                \
        fmass_rr(W(XG), W(XS), W(XT))

#define fmass3ld(XG, XS, MT, DT)                                            \
        fmass_ld(W(XG), W(XS), W(MT), W(DT))

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsss_rr(XG, XS, XT)                                                \
        fmsts_rr(W(XG), W(XS), W(XT))

#define fmsss_ld(XG, XS, MT, DT)                                            \
        fmsts_ld(W(XG), W(XS), W(MT), W(DT))

#define fmsss3rr(XG, XS, XT)                                                \
        fmsss_rr(W(XG), W(XS), W(XT))

#define fmsss3ld(XG, XS, MT, DT)                                            \
        fmsss_ld(W(XG), W(XS), W(MT), W(DT))

/*************   scalar double-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minss_rr(XG, XS)                                                    \
        mints_rr(W(XG), W(XS))

#define minss_ld(XG, MS, DS)                                                \
        mints_ld(W(XG), W(MS), W(DS))

#define minss3rr(XD, XS, XT)                                                \
        mints3rr(W(XD), W(XS), W(XT))

#define minss3ld(XD, XS, MT, DT)                                            \
        mints3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxss_rr(XG, XS)                                                    \
        maxts_rr(W(XG), W(XS))

#define maxss_ld(XG, MS, DS)                                                \
        maxts_ld(W(XG), W(MS), W(DS))

#define maxss3rr(XD, XS, XT)                                                \
        maxts3rr(W(XD), W(XS), W(XT))

#define maxss3ld(XD, XS, MT, DT)                                            \
        maxts3ld(W(XD), W(XS), W(MT), W(DT))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqss_rr(XG, XS)                                                    \
        ceqts_rr(W(XG), W(XS))

#define ceqss_ld(XG, MS, DS)                                                \
        ceqts_ld(W(XG), W(MS), W(DS))

#define ceqss3rr(XD, XS, XT)                                                \
        ceqts3rr(W(XD), W(XS), W(XT))

#define ceqss3ld(XD, XS, MT, DT)                                            \
        ceqts3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cness_rr(XG, XS)                                                    \
        cnets_rr(W(XG), W(XS))

#define cness_ld(XG, MS, DS)                                                \
        cnets_ld(W(XG), W(MS), W(DS))

#define cness3rr(XD, XS, XT)                                                \
        cnets3rr(W(XD), W(XS), W(XT))

#define cness3ld(XD, XS, MT, DT)                                            \
        cnets3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltss_rr(XG, XS)                                                    \
        cltts_rr(W(XG), W(XS))

#define cltss_ld(XG, MS, DS)                                                \
        cltts_ld(W(XG), W(MS), W(DS))

#define cltss3rr(XD, XS, XT)                                                \
        cltts3rr(W(XD), W(XS), W(XT))

#define cltss3ld(XD, XS, MT, DT)                                            \
        cltts3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define cless_rr(XG, XS)                                                    \
        clets_rr(W(XG), W(XS))

#define cless_ld(XG, MS, DS)                                                \
        clets_ld(W(XG), W(MS), W(DS))

#define cless3rr(XD, XS, XT)                                                \
        clets3rr(W(XD), W(XS), W(XT))

#define cless3ld(XD, XS, MT, DT)                                            \
        clets3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtss_rr(XG, XS)                                                    \
        cgtts_rr(W(XG), W(XS))

#define cgtss_ld(XG, MS, DS)                                                \
        cgtts_ld(W(XG), W(MS), W(DS))

#define cgtss3rr(XD, XS, XT)                                                \
        cgtts3rr(W(XD), W(XS), W(XT))

#define cgtss3ld(XD, XS, MT, DT)                                            \
        cgtts3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgess_rr(XG, XS)                                                    \
        cgets_rr(W(XG), W(XS))

#define cgess_ld(XG, MS, DS)                                                \
        cgets_ld(W(XG), W(MS), W(DS))

#define cgess3rr(XD, XS, XT)                                                \
        cgets3rr(W(XD), W(XS), W(XT))

#define cgess3ld(XD, XS, MT, DT)                                            \
        cgets3ld(W(XD), W(XS), W(MT), W(DT))

#endif /* RT_ELEMENT */

#endif /* RT_SIMD_CODE */

/******************************************************************************/
/************************   COMMON BASE INSTRUCTIONS   ************************/
/******************************************************************************/

/*
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
/***************** address-sized instructions for 32-bit mode *****************/
/******************************************************************************/

#if   RT_ADDRESS == 32

/* mov (D = S)
 * set-flags: no */

#define movxx_ri(RD, IS)                                                    \
        movwx_ri(W(RD), W(IS))

#define movxx_mi(MD, DD, IS)                                                \
        movwx_mi(W(MD), W(DD), W(IS))

#define movxx_rr(RD, RS)                                                    \
        movwx_rr(W(RD), W(RS))

#define movxx_ld(RD, MS, DS)                                                \
        movwx_ld(W(RD), W(MS), W(DS))

#define movxx_st(RS, MD, DD)                                                \
        movwx_st(W(RS), W(MD), W(DD))


#define movxx_rj(RD, IT, IS)     /* IT - upper 32-bit, IS - lower 32-bit */ \
        movwx_ri(W(RD), W(IS))

#define movxx_mj(MD, DD, IT, IS) /* IT - upper 32-bit, IS - lower 32-bit */ \
        movwx_mi(W(MD), W(DD), W(IS))

/* and (G = G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define andxx_ri(RG, IS)                                                    \
        andwx_ri(W(RG), W(IS))

#define andxx_mi(MG, DG, IS)                                                \
        andwx_mi(W(MG), W(DG), W(IS))

#define andxx_rr(RG, RS)                                                    \
        andwx_rr(W(RG), W(RS))

#define andxx_ld(RG, MS, DS)                                                \
        andwx_ld(W(RG), W(MS), W(DS))

#define andxx_st(RS, MG, DG)                                                \
        andwx_st(W(RS), W(MG), W(DG))

#define andxx_mr(MG, DG, RS)                                                \
        andwx_mr(W(MG), W(DG), W(RS))


#define andxxZri(RG, IS)                                                    \
        andwxZri(W(RG), W(IS))

#define andxxZmi(MG, DG, IS)                                                \
        andwxZmi(W(MG), W(DG), W(IS))

#define andxxZrr(RG, RS)                                                    \
        andwxZrr(W(RG), W(RS))

#define andxxZld(RG, MS, DS)                                                \
        andwxZld(W(RG), W(MS), W(DS))

#define andxxZst(RS, MG, DG)                                                \
        andwxZst(W(RS), W(MG), W(DG))

#define andxxZmr(MG, DG, RS)                                                \
        andwxZmr(W(MG), W(DG), W(RS))

/* ann (G = ~G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define annxx_ri(RG, IS)                                                    \
        annwx_ri(W(RG), W(IS))

#define annxx_mi(MG, DG, IS)                                                \
        annwx_mi(W(MG), W(DG), W(IS))

#define annxx_rr(RG, RS)                                                    \
        annwx_rr(W(RG), W(RS))

#define annxx_ld(RG, MS, DS)                                                \
        annwx_ld(W(RG), W(MS), W(DS))

#define annxx_st(RS, MG, DG)                                                \
        annwx_st(W(RS), W(MG), W(DG))

#define annxx_mr(MG, DG, RS)                                                \
        annwx_mr(W(MG), W(DG), W(RS))


#define annxxZri(RG, IS)                                                    \
        annwxZri(W(RG), W(IS))

#define annxxZmi(MG, DG, IS)                                                \
        annwxZmi(W(MG), W(DG), W(IS))

#define annxxZrr(RG, RS)                                                    \
        annwxZrr(W(RG), W(RS))

#define annxxZld(RG, MS, DS)                                                \
        annwxZld(W(RG), W(MS), W(DS))

#define annxxZst(RS, MG, DG)                                                \
        annwxZst(W(RS), W(MG), W(DG))

#define annxxZmr(MG, DG, RS)                                                \
        annwxZmr(W(MG), W(DG), W(RS))

/* orr (G = G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define orrxx_ri(RG, IS)                                                    \
        orrwx_ri(W(RG), W(IS))

#define orrxx_mi(MG, DG, IS)                                                \
        orrwx_mi(W(MG), W(DG), W(IS))

#define orrxx_rr(RG, RS)                                                    \
        orrwx_rr(W(RG), W(RS))

#define orrxx_ld(RG, MS, DS)                                                \
        orrwx_ld(W(RG), W(MS), W(DS))

#define orrxx_st(RS, MG, DG)                                                \
        orrwx_st(W(RS), W(MG), W(DG))

#define orrxx_mr(MG, DG, RS)                                                \
        orrwx_mr(W(MG), W(DG), W(RS))


#define orrxxZri(RG, IS)                                                    \
        orrwxZri(W(RG), W(IS))

#define orrxxZmi(MG, DG, IS)                                                \
        orrwxZmi(W(MG), W(DG), W(IS))

#define orrxxZrr(RG, RS)                                                    \
        orrwxZrr(W(RG), W(RS))

#define orrxxZld(RG, MS, DS)                                                \
        orrwxZld(W(RG), W(MS), W(DS))

#define orrxxZst(RS, MG, DG)                                                \
        orrwxZst(W(RS), W(MG), W(DG))

#define orrxxZmr(MG, DG, RS)                                                \
        orrwxZmr(W(MG), W(DG), W(RS))

/* orn (G = ~G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define ornxx_ri(RG, IS)                                                    \
        ornwx_ri(W(RG), W(IS))

#define ornxx_mi(MG, DG, IS)                                                \
        ornwx_mi(W(MG), W(DG), W(IS))

#define ornxx_rr(RG, RS)                                                    \
        ornwx_rr(W(RG), W(RS))

#define ornxx_ld(RG, MS, DS)                                                \
        ornwx_ld(W(RG), W(MS), W(DS))

#define ornxx_st(RS, MG, DG)                                                \
        ornwx_st(W(RS), W(MG), W(DG))

#define ornxx_mr(MG, DG, RS)                                                \
        ornwx_mr(W(MG), W(DG), W(RS))


#define ornxxZri(RG, IS)                                                    \
        ornwxZri(W(RG), W(IS))

#define ornxxZmi(MG, DG, IS)                                                \
        ornwxZmi(W(MG), W(DG), W(IS))

#define ornxxZrr(RG, RS)                                                    \
        ornwxZrr(W(RG), W(RS))

#define ornxxZld(RG, MS, DS)                                                \
        ornwxZld(W(RG), W(MS), W(DS))

#define ornxxZst(RS, MG, DG)                                                \
        ornwxZst(W(RS), W(MG), W(DG))

#define ornxxZmr(MG, DG, RS)                                                \
        ornwxZmr(W(MG), W(DG), W(RS))

/* xor (G = G ^ S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define xorxx_ri(RG, IS)                                                    \
        xorwx_ri(W(RG), W(IS))

#define xorxx_mi(MG, DG, IS)                                                \
        xorwx_mi(W(MG), W(DG), W(IS))

#define xorxx_rr(RG, RS)                                                    \
        xorwx_rr(W(RG), W(RS))

#define xorxx_ld(RG, MS, DS)                                                \
        xorwx_ld(W(RG), W(MS), W(DS))

#define xorxx_st(RS, MG, DG)                                                \
        xorwx_st(W(RS), W(MG), W(DG))

#define xorxx_mr(MG, DG, RS)                                                \
        xorwx_mr(W(MG), W(DG), W(RS))


#define xorxxZri(RG, IS)                                                    \
        xorwxZri(W(RG), W(IS))

#define xorxxZmi(MG, DG, IS)                                                \
        xorwxZmi(W(MG), W(DG), W(IS))

#define xorxxZrr(RG, RS)                                                    \
        xorwxZrr(W(RG), W(RS))

#define xorxxZld(RG, MS, DS)                                                \
        xorwxZld(W(RG), W(MS), W(DS))

#define xorxxZst(RS, MG, DG)                                                \
        xorwxZst(W(RS), W(MG), W(DG))

#define xorxxZmr(MG, DG, RS)                                                \
        xorwxZmr(W(MG), W(DG), W(RS))

/* not (G = ~G)
 * set-flags: no */

#define notxx_rx(RG)                                                        \
        notwx_rx(W(RG))

#define notxx_mx(MG, DG)                                                    \
        notwx_mx(W(MG), W(DG))

/* neg (G = -G)
 * set-flags: undefined (*_*), yes (*Z*) */

#define negxx_rx(RG)                                                        \
        negwx_rx(W(RG))

#define negxx_mx(MG, DG)                                                    \
        negwx_mx(W(MG), W(DG))


#define negxxZrx(RG)                                                        \
        negwxZrx(W(RG))

#define negxxZmx(MG, DG)                                                    \
        negwxZmx(W(MG), W(DG))

/* add (G = G + S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define addxx_ri(RG, IS)                                                    \
        addwx_ri(W(RG), W(IS))

#define addxx_mi(MG, DG, IS)                                                \
        addwx_mi(W(MG), W(DG), W(IS))

#define addxx_rr(RG, RS)                                                    \
        addwx_rr(W(RG), W(RS))

#define addxx_ld(RG, MS, DS)                                                \
        addwx_ld(W(RG), W(MS), W(DS))

#define addxx_st(RS, MG, DG)                                                \
        addwx_st(W(RS), W(MG), W(DG))

#define addxx_mr(MG, DG, RS)                                                \
        addwx_mr(W(MG), W(DG), W(RS))


#define addxxZri(RG, IS)                                                    \
        addwxZri(W(RG), W(IS))

#define addxxZmi(MG, DG, IS)                                                \
        addwxZmi(W(MG), W(DG), W(IS))

#define addxxZrr(RG, RS)                                                    \
        addwxZrr(W(RG), W(RS))

#define addxxZld(RG, MS, DS)                                                \
        addwxZld(W(RG), W(MS), W(DS))

#define addxxZst(RS, MG, DG)                                                \
        addwxZst(W(RS), W(MG), W(DG))

#define addxxZmr(MG, DG, RS)                                                \
        addwxZmr(W(MG), W(DG), W(RS))

/* sub (G = G - S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define subxx_ri(RG, IS)                                                    \
        subwx_ri(W(RG), W(IS))

#define subxx_mi(MG, DG, IS)                                                \
        subwx_mi(W(MG), W(DG), W(IS))

#define subxx_rr(RG, RS)                                                    \
        subwx_rr(W(RG), W(RS))

#define subxx_ld(RG, MS, DS)                                                \
        subwx_ld(W(RG), W(MS), W(DS))

#define subxx_st(RS, MG, DG)                                                \
        subwx_st(W(RS), W(MG), W(DG))

#define subxx_mr(MG, DG, RS)                                                \
        subwx_mr(W(MG), W(DG), W(RS))


#define subxxZri(RG, IS)                                                    \
        subwxZri(W(RG), W(IS))

#define subxxZmi(MG, DG, IS)                                                \
        subwxZmi(W(MG), W(DG), W(IS))

#define subxxZrr(RG, RS)                                                    \
        subwxZrr(W(RG), W(RS))

#define subxxZld(RG, MS, DS)                                                \
        subwxZld(W(RG), W(MS), W(DS))

#define subxxZst(RS, MG, DG)                                                \
        subwxZst(W(RS), W(MG), W(DG))

#define subxxZmr(MG, DG, RS)                                                \
        subwxZmr(W(MG), W(DG), W(RS))

/* shl (G = G << S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlxx_rx(RG)                     /* reads Recx for shift count */   \
        shlwx_rx(W(RG))

#define shlxx_mx(MG, DG)                 /* reads Recx for shift count */   \
        shlwx_mx(W(MG), W(DG))

#define shlxx_ri(RG, IS)                                                    \
        shlwx_ri(W(RG), W(IS))

#define shlxx_mi(MG, DG, IS)                                                \
        shlwx_mi(W(MG), W(DG), W(IS))

#define shlxx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shlwx_rr(W(RG), W(RS))

#define shlxx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shlwx_ld(W(RG), W(MS), W(DS))

#define shlxx_st(RS, MG, DG)                                                \
        shlwx_st(W(RS), W(MG), W(DG))

#define shlxx_mr(MG, DG, RS)                                                \
        shlwx_mr(W(MG), W(DG), W(RS))


#define shlxxZrx(RG)                     /* reads Recx for shift count */   \
        shlwxZrx(W(RG))

#define shlxxZmx(MG, DG)                 /* reads Recx for shift count */   \
        shlwxZmx(W(MG), W(DG))

#define shlxxZri(RG, IS)                                                    \
        shlwxZri(W(RG), W(IS))

#define shlxxZmi(MG, DG, IS)                                                \
        shlwxZmi(W(MG), W(DG), W(IS))

#define shlxxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        shlwxZrr(W(RG), W(RS))

#define shlxxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shlwxZld(W(RG), W(MS), W(DS))

#define shlxxZst(RS, MG, DG)                                                \
        shlwxZst(W(RS), W(MG), W(DG))

#define shlxxZmr(MG, DG, RS)                                                \
        shlwxZmr(W(MG), W(DG), W(RS))

/* shr (G = G >> S), unsigned (logical)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrxx_rx(RG)                     /* reads Recx for shift count */   \
        shrwx_rx(W(RG))

#define shrxx_mx(MG, DG)                 /* reads Recx for shift count */   \
        shrwx_mx(W(MG), W(DG))

#define shrxx_ri(RG, IS)                                                    \
        shrwx_ri(W(RG), W(IS))

#define shrxx_mi(MG, DG, IS)                                                \
        shrwx_mi(W(MG), W(DG), W(IS))

#define shrxx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrwx_rr(W(RG), W(RS))

#define shrxx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shrwx_ld(W(RG), W(MS), W(DS))

#define shrxx_st(RS, MG, DG)                                                \
        shrwx_st(W(RS), W(MG), W(DG))

#define shrxx_mr(MG, DG, RS)                                                \
        shrwx_mr(W(MG), W(DG), W(RS))


#define shrxxZrx(RG)                     /* reads Recx for shift count */   \
        shrwxZrx(W(RG))

#define shrxxZmx(MG, DG)                 /* reads Recx for shift count */   \
        shrwxZmx(W(MG), W(DG))

#define shrxxZri(RG, IS)                                                    \
        shrwxZri(W(RG), W(IS))

#define shrxxZmi(MG, DG, IS)                                                \
        shrwxZmi(W(MG), W(DG), W(IS))

#define shrxxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrwxZrr(W(RG), W(RS))

#define shrxxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shrwxZld(W(RG), W(MS), W(DS))

#define shrxxZst(RS, MG, DG)                                                \
        shrwxZst(W(RS), W(MG), W(DG))

#define shrxxZmr(MG, DG, RS)                                                \
        shrwxZmr(W(MG), W(DG), W(RS))

/* shr (G = G >> S), signed (arithmetic)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrxn_rx(RG)                     /* reads Recx for shift count */   \
        shrwn_rx(W(RG))

#define shrxn_mx(MG, DG)                 /* reads Recx for shift count */   \
        shrwn_mx(W(MG), W(DG))

#define shrxn_ri(RG, IS)                                                    \
        shrwn_ri(W(RG), W(IS))

#define shrxn_mi(MG, DG, IS)                                                \
        shrwn_mi(W(MG), W(DG), W(IS))

#define shrxn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrwn_rr(W(RG), W(RS))

#define shrxn_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shrwn_ld(W(RG), W(MS), W(DS))

#define shrxn_st(RS, MG, DG)                                                \
        shrwn_st(W(RS), W(MG), W(DG))

#define shrxn_mr(MG, DG, RS)                                                \
        shrwn_mr(W(MG), W(DG), W(RS))


#define shrxnZrx(RG)                     /* reads Recx for shift count */   \
        shrwnZrx(W(RG))

#define shrxnZmx(MG, DG)                 /* reads Recx for shift count */   \
        shrwnZmx(W(MG), W(DG))

#define shrxnZri(RG, IS)                                                    \
        shrwnZri(W(RG), W(IS))

#define shrxnZmi(MG, DG, IS)                                                \
        shrwnZmi(W(MG), W(DG), W(IS))

#define shrxnZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrwnZrr(W(RG), W(RS))

#define shrxnZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shrwnZld(W(RG), W(MS), W(DS))

#define shrxnZst(RS, MG, DG)                                                \
        shrwnZst(W(RS), W(MG), W(DG))

#define shrxnZmr(MG, DG, RS)                                                \
        shrwnZmr(W(MG), W(DG), W(RS))

/* ror (G = G >> S | G << 32 - S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define rorxx_rx(RG)                     /* reads Recx for shift count */   \
        rorwx_rx(W(RG))

#define rorxx_mx(MG, DG)                 /* reads Recx for shift count */   \
        rorwx_mx(W(MG), W(DG))

#define rorxx_ri(RG, IS)                                                    \
        rorwx_ri(W(RG), W(IS))

#define rorxx_mi(MG, DG, IS)                                                \
        rorwx_mi(W(MG), W(DG), W(IS))

#define rorxx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        rorwx_rr(W(RG), W(RS))

#define rorxx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        rorwx_ld(W(RG), W(MS), W(DS))

#define rorxx_st(RS, MG, DG)                                                \
        rorwx_st(W(RS), W(MG), W(DG))

#define rorxx_mr(MG, DG, RS)                                                \
        rorwx_mr(W(MG), W(DG), W(RS))


#define rorxxZrx(RG)                     /* reads Recx for shift count */   \
        rorwxZrx(W(RG))

#define rorxxZmx(MG, DG)                 /* reads Recx for shift count */   \
        rorwxZmx(W(MG), W(DG))

#define rorxxZri(RG, IS)                                                    \
        rorwxZri(W(RG), W(IS))

#define rorxxZmi(MG, DG, IS)                                                \
        rorwxZmi(W(MG), W(DG), W(IS))

#define rorxxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        rorwxZrr(W(RG), W(RS))

#define rorxxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        rorwxZld(W(RG), W(MS), W(DS))

#define rorxxZst(RS, MG, DG)                                                \
        rorwxZst(W(RS), W(MG), W(DG))

#define rorxxZmr(MG, DG, RS)                                                \
        rorwxZmr(W(MG), W(DG), W(RS))

/* mul (G = G * S)
 * set-flags: undefined */

#define mulxx_ri(RG, IS)                                                    \
        mulwx_ri(W(RG), W(IS))

#define mulxx_rr(RG, RS)                                                    \
        mulwx_rr(W(RG), W(RS))

#define mulxx_ld(RG, MS, DS)                                                \
        mulwx_ld(W(RG), W(MS), W(DS))


#define mulxx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        mulwx_xr(W(RS))

#define mulxx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        mulwx_xm(W(MS), W(DS))


#define mulxn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        mulwn_xr(W(RS))

#define mulxn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        mulwn_xm(W(MS), W(DS))


#define mulxp_xr(RS)     /* Reax is in/out, prepares Redx for divxn_x* */   \
        mulwp_xr(W(RS))       /* product must not exceed operands size */

#define mulxp_xm(MS, DS) /* Reax is in/out, prepares Redx for divxn_x* */   \
        mulwp_xm(W(MS), W(DS))/* product must not exceed operands size */

/* div (G = G / S)
 * set-flags: undefined */

#define divxx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        divwx_ri(W(RG), W(IS))

#define divxx_rr(RG, RS)                 /* RG, RS no Reax, RS no Redx */   \
        divwx_rr(W(RG), W(RS))

#define divxx_ld(RG, MS, DS)   /* Reax cannot be used as first operand */   \
        divwx_ld(W(RG), W(MS), W(DS))


#define divxn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        divwn_ri(W(RG), W(IS))

#define divxn_rr(RG, RS)                 /* RG, RS no Reax, RS no Redx */   \
        divwn_rr(W(RG), W(RS))

#define divxn_ld(RG, MS, DS)   /* Reax cannot be used as first operand */   \
        divwn_ld(W(RG), W(MS), W(DS))


#define prexx_xx()   /* to be placed right before divxx_x* or remxx_xx */   \
        prewx_xx()                   /* to prepare Redx for int-divide */

#define prexn_xx()   /* to be placed right before divxn_x* or remxn_xx */   \
        prewn_xx()                   /* to prepare Redx for int-divide */


#define divxx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        divwx_xr(W(RS))

#define divxx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        divwx_xm(W(MS), W(DS))


#define divxn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xr(W(RS))

#define divxn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xm(W(MS), W(DS))


#define divxp_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwp_xr(W(RS))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divxp_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwp_xm(W(MS), W(DS))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem (G = G % S)
 * set-flags: undefined */

#define remxx_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        remwx_ri(W(RG), W(IS))

#define remxx_rr(RG, RS)                 /* RG, RS no Redx, RS no Reax */   \
        remwx_rr(W(RG), W(RS))

#define remxx_ld(RG, MS, DS)   /* Redx cannot be used as first operand */   \
        remwx_ld(W(RG), W(MS), W(DS))


#define remxn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        remwn_ri(W(RG), W(IS))

#define remxn_rr(RG, RS)                 /* RG, RS no Redx, RS no Reax */   \
        remwn_rr(W(RG), W(RS))

#define remxn_ld(RG, MS, DS)   /* Redx cannot be used as first operand */   \
        remwn_ld(W(RG), W(MS), W(DS))


#define remxx_xx() /* to be placed before divxx_x*, but after prexx_xx */   \
        remwx_xx()                   /* to prepare for rem calculation */

#define remxx_xr(RS)        /* to be placed immediately after divxx_xr */   \
        remwx_xr(W(RS))              /* to produce remainder Redx<-rem */

#define remxx_xm(MS, DS)    /* to be placed immediately after divxx_xm */   \
        remwx_xm(W(MS), W(DS))       /* to produce remainder Redx<-rem */


#define remxn_xx() /* to be placed before divxn_x*, but after prexn_xx */   \
        remwn_xx()                   /* to prepare for rem calculation */

#define remxn_xr(RS)        /* to be placed immediately after divxn_xr */   \
        remwn_xr(W(RS))              /* to produce remainder Redx<-rem */

#define remxn_xm(MS, DS)    /* to be placed immediately after divxn_xm */   \
        remwn_xm(W(MS), W(DS))       /* to produce remainder Redx<-rem */

/* arj (G = G op S, if cc G then jump lb)
 * set-flags: undefined
 * refer to individual instruction descriptions
 * to stay within special register limitations */

     /* Definitions for arj's "op" and "cc" parameters
      * are provided in 32-bit rtarch_***.h files. */

#define arjxx_rx(RG, op, cc, lb)                                            \
        arjwx_rx(W(RG), op, cc, lb)

#define arjxx_mx(MG, DG, op, cc, lb)                                        \
        arjwx_mx(W(MG), W(DG), op, cc, lb)

#define arjxx_ri(RG, IS, op, cc, lb)                                        \
        arjwx_ri(W(RG), W(IS), op, cc, lb)

#define arjxx_mi(MG, DG, IS, op, cc, lb)                                    \
        arjwx_mi(W(MG), W(DG), W(IS), op, cc, lb)

#define arjxx_rr(RG, RS, op, cc, lb)                                        \
        arjwx_rr(W(RG), W(RS), op, cc, lb)

#define arjxx_ld(RG, MS, DS, op, cc, lb)                                    \
        arjwx_ld(W(RG), W(MS), W(DS), op, cc, lb)

#define arjxx_st(RS, MG, DG, op, cc, lb)                                    \
        arjwx_st(W(RS), W(MG), W(DG), op, cc, lb)

#define arjxx_mr(MG, DG, RS, op, cc, lb)                                    \
        arjwx_mr(W(MG), W(DG), W(RS), op, cc, lb)

/* cmj (flags = S ? T, if cc flags then jump lb)
 * set-flags: undefined */

     /* Definitions for cmj's "cc" parameter
      * are provided in 32-bit rtarch_***.h files. */

#define cmjxx_rz(RS, cc, lb)                                                \
        cmjwx_rz(W(RS), cc, lb)

#define cmjxx_mz(MS, DS, cc, lb)                                            \
        cmjwx_mz(W(MS), W(DS), cc, lb)

#define cmjxx_ri(RS, IT, cc, lb)                                            \
        cmjwx_ri(W(RS), W(IT), cc, lb)

#define cmjxx_mi(MS, DS, IT, cc, lb)                                        \
        cmjwx_mi(W(MS), W(DS), W(IT), cc, lb)

#define cmjxx_rr(RS, RT, cc, lb)                                            \
        cmjwx_rr(W(RS), W(RT), cc, lb)

#define cmjxx_rm(RS, MT, DT, cc, lb)                                        \
        cmjwx_rm(W(RS), W(MT), W(DT), cc, lb)

#define cmjxx_mr(MS, DS, RT, cc, lb)                                        \
        cmjwx_mr(W(MS), W(DS), W(RT), cc, lb)

/* cmp (flags = S ? T)
 * set-flags: yes */

#define cmpxx_ri(RS, IT)                                                    \
        cmpwx_ri(W(RS), W(IT))

#define cmpxx_mi(MS, DS, IT)                                                \
        cmpwx_mi(W(MS), W(DS), W(IT))

#define cmpxx_rr(RS, RT)                                                    \
        cmpwx_rr(W(RS), W(RT))

#define cmpxx_rm(RS, MT, DT)                                                \
        cmpwx_rm(W(RS), W(MT), W(DT))

#define cmpxx_mr(MS, DS, RT)                                                \
        cmpwx_mr(W(MS), W(DS), W(RT))

/* ver (Mebp/inf_VER = SIMD-version)
 * set-flags: no
 * For interpretation of SIMD build flags check compatibility layer in rtzero.h
 * 0th byte - 128-bit version, 1st byte - 256-bit version, | plus _R8/_RX slots
 * 2nd byte - 512-bit version, 3rd byte - 1K4-bit version, | in upper halves */

     /* verxx_xx() in 32-bit rtarch_***.h files, destroys Reax, ... , Redi */

/************************* address-sized instructions *************************/

/* adr (D = adr S)
 * set-flags: no */

     /* adrxx_ld(RD, MS, DS) is defined in 32-bit rtarch_***.h files */

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

     /* jccxx_** is defined in 32-bit rtarch_***.h files */

/************************* register-size instructions *************************/

/* stack (push stack = S, D = pop stack)
 * set-flags: no (sequence cmp/stack_la/jmp is not allowed on MIPS & POWER)
 * adjust stack pointer with 8-byte (64-bit) steps on all current targets */

     /* stack_** is defined in 32-bit rtarch_***.h files */

/******************************************************************************/
/***************** address-sized instructions for 64-bit mode *****************/
/******************************************************************************/

#elif RT_ADDRESS == 64

/* mov (D = S)
 * set-flags: no */

#define movxx_ri(RD, IS)                                                    \
        movzx_ri(W(RD), W(IS))

#define movxx_mi(MD, DD, IS)                                                \
        movzx_mi(W(MD), W(DD), W(IS))

#define movxx_rr(RD, RS)                                                    \
        movzx_rr(W(RD), W(RS))

#define movxx_ld(RD, MS, DS)                                                \
        movzx_ld(W(RD), W(MS), W(DS))

#define movxx_st(RS, MD, DD)                                                \
        movzx_st(W(RS), W(MD), W(DD))


#define movxx_rj(RD, IT, IS)     /* IT - upper 32-bit, IS - lower 32-bit */ \
        movzx_ri(W(RD), W(IS))

#define movxx_mj(MD, DD, IT, IS) /* IT - upper 32-bit, IS - lower 32-bit */ \
        movzx_mi(W(MD), W(DD), W(IS))

/* and (G = G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define andxx_ri(RG, IS)                                                    \
        andzx_ri(W(RG), W(IS))

#define andxx_mi(MG, DG, IS)                                                \
        andzx_mi(W(MG), W(DG), W(IS))

#define andxx_rr(RG, RS)                                                    \
        andzx_rr(W(RG), W(RS))

#define andxx_ld(RG, MS, DS)                                                \
        andzx_ld(W(RG), W(MS), W(DS))

#define andxx_st(RS, MG, DG)                                                \
        andzx_st(W(RS), W(MG), W(DG))

#define andxx_mr(MG, DG, RS)                                                \
        andzx_mr(W(MG), W(DG), W(RS))


#define andxxZri(RG, IS)                                                    \
        andzxZri(W(RG), W(IS))

#define andxxZmi(MG, DG, IS)                                                \
        andzxZmi(W(MG), W(DG), W(IS))

#define andxxZrr(RG, RS)                                                    \
        andzxZrr(W(RG), W(RS))

#define andxxZld(RG, MS, DS)                                                \
        andzxZld(W(RG), W(MS), W(DS))

#define andxxZst(RS, MG, DG)                                                \
        andzxZst(W(RS), W(MG), W(DG))

#define andxxZmr(MG, DG, RS)                                                \
        andzxZmr(W(MG), W(DG), W(RS))

/* ann (G = ~G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define annxx_ri(RG, IS)                                                    \
        annzx_ri(W(RG), W(IS))

#define annxx_mi(MG, DG, IS)                                                \
        annzx_mi(W(MG), W(DG), W(IS))

#define annxx_rr(RG, RS)                                                    \
        annzx_rr(W(RG), W(RS))

#define annxx_ld(RG, MS, DS)                                                \
        annzx_ld(W(RG), W(MS), W(DS))

#define annxx_st(RS, MG, DG)                                                \
        annzx_st(W(RS), W(MG), W(DG))

#define annxx_mr(MG, DG, RS)                                                \
        annzx_mr(W(MG), W(DG), W(RS))


#define annxxZri(RG, IS)                                                    \
        annzxZri(W(RG), W(IS))

#define annxxZmi(MG, DG, IS)                                                \
        annzxZmi(W(MG), W(DG), W(IS))

#define annxxZrr(RG, RS)                                                    \
        annzxZrr(W(RG), W(RS))

#define annxxZld(RG, MS, DS)                                                \
        annzxZld(W(RG), W(MS), W(DS))

#define annxxZst(RS, MG, DG)                                                \
        annzxZst(W(RS), W(MG), W(DG))

#define annxxZmr(MG, DG, RS)                                                \
        annzxZmr(W(MG), W(DG), W(RS))

/* orr (G = G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define orrxx_ri(RG, IS)                                                    \
        orrzx_ri(W(RG), W(IS))

#define orrxx_mi(MG, DG, IS)                                                \
        orrzx_mi(W(MG), W(DG), W(IS))

#define orrxx_rr(RG, RS)                                                    \
        orrzx_rr(W(RG), W(RS))

#define orrxx_ld(RG, MS, DS)                                                \
        orrzx_ld(W(RG), W(MS), W(DS))

#define orrxx_st(RS, MG, DG)                                                \
        orrzx_st(W(RS), W(MG), W(DG))

#define orrxx_mr(MG, DG, RS)                                                \
        orrzx_mr(W(MG), W(DG), W(RS))


#define orrxxZri(RG, IS)                                                    \
        orrzxZri(W(RG), W(IS))

#define orrxxZmi(MG, DG, IS)                                                \
        orrzxZmi(W(MG), W(DG), W(IS))

#define orrxxZrr(RG, RS)                                                    \
        orrzxZrr(W(RG), W(RS))

#define orrxxZld(RG, MS, DS)                                                \
        orrzxZld(W(RG), W(MS), W(DS))

#define orrxxZst(RS, MG, DG)                                                \
        orrzxZst(W(RS), W(MG), W(DG))

#define orrxxZmr(MG, DG, RS)                                                \
        orrzxZmr(W(MG), W(DG), W(RS))

/* orn (G = ~G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define ornxx_ri(RG, IS)                                                    \
        ornzx_ri(W(RG), W(IS))

#define ornxx_mi(MG, DG, IS)                                                \
        ornzx_mi(W(MG), W(DG), W(IS))

#define ornxx_rr(RG, RS)                                                    \
        ornzx_rr(W(RG), W(RS))

#define ornxx_ld(RG, MS, DS)                                                \
        ornzx_ld(W(RG), W(MS), W(DS))

#define ornxx_st(RS, MG, DG)                                                \
        ornzx_st(W(RS), W(MG), W(DG))

#define ornxx_mr(MG, DG, RS)                                                \
        ornzx_mr(W(MG), W(DG), W(RS))


#define ornxxZri(RG, IS)                                                    \
        ornzxZri(W(RG), W(IS))

#define ornxxZmi(MG, DG, IS)                                                \
        ornzxZmi(W(MG), W(DG), W(IS))

#define ornxxZrr(RG, RS)                                                    \
        ornzxZrr(W(RG), W(RS))

#define ornxxZld(RG, MS, DS)                                                \
        ornzxZld(W(RG), W(MS), W(DS))

#define ornxxZst(RS, MG, DG)                                                \
        ornzxZst(W(RS), W(MG), W(DG))

#define ornxxZmr(MG, DG, RS)                                                \
        ornzxZmr(W(MG), W(DG), W(RS))

/* xor (G = G ^ S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define xorxx_ri(RG, IS)                                                    \
        xorzx_ri(W(RG), W(IS))

#define xorxx_mi(MG, DG, IS)                                                \
        xorzx_mi(W(MG), W(DG), W(IS))

#define xorxx_rr(RG, RS)                                                    \
        xorzx_rr(W(RG), W(RS))

#define xorxx_ld(RG, MS, DS)                                                \
        xorzx_ld(W(RG), W(MS), W(DS))

#define xorxx_st(RS, MG, DG)                                                \
        xorzx_st(W(RS), W(MG), W(DG))

#define xorxx_mr(MG, DG, RS)                                                \
        xorzx_mr(W(MG), W(DG), W(RS))


#define xorxxZri(RG, IS)                                                    \
        xorzxZri(W(RG), W(IS))

#define xorxxZmi(MG, DG, IS)                                                \
        xorzxZmi(W(MG), W(DG), W(IS))

#define xorxxZrr(RG, RS)                                                    \
        xorzxZrr(W(RG), W(RS))

#define xorxxZld(RG, MS, DS)                                                \
        xorzxZld(W(RG), W(MS), W(DS))

#define xorxxZst(RS, MG, DG)                                                \
        xorzxZst(W(RS), W(MG), W(DG))

#define xorxxZmr(MG, DG, RS)                                                \
        xorzxZmr(W(MG), W(DG), W(RS))

/* not (G = ~G)
 * set-flags: no */

#define notxx_rx(RG)                                                        \
        notzx_rx(W(RG))

#define notxx_mx(MG, DG)                                                    \
        notzx_mx(W(MG), W(DG))

/* neg (G = -G)
 * set-flags: undefined (*_*), yes (*Z*) */

#define negxx_rx(RG)                                                        \
        negzx_rx(W(RG))

#define negxx_mx(MG, DG)                                                    \
        negzx_mx(W(MG), W(DG))


#define negxxZrx(RG)                                                        \
        negzxZrx(W(RG))

#define negxxZmx(MG, DG)                                                    \
        negzxZmx(W(MG), W(DG))

/* add (G = G + S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define addxx_ri(RG, IS)                                                    \
        addzx_ri(W(RG), W(IS))

#define addxx_mi(MG, DG, IS)                                                \
        addzx_mi(W(MG), W(DG), W(IS))

#define addxx_rr(RG, RS)                                                    \
        addzx_rr(W(RG), W(RS))

#define addxx_ld(RG, MS, DS)                                                \
        addzx_ld(W(RG), W(MS), W(DS))

#define addxx_st(RS, MG, DG)                                                \
        addzx_st(W(RS), W(MG), W(DG))

#define addxx_mr(MG, DG, RS)                                                \
        addzx_mr(W(MG), W(DG), W(RS))


#define addxxZri(RG, IS)                                                    \
        addzxZri(W(RG), W(IS))

#define addxxZmi(MG, DG, IS)                                                \
        addzxZmi(W(MG), W(DG), W(IS))

#define addxxZrr(RG, RS)                                                    \
        addzxZrr(W(RG), W(RS))

#define addxxZld(RG, MS, DS)                                                \
        addzxZld(W(RG), W(MS), W(DS))

#define addxxZst(RS, MG, DG)                                                \
        addzxZst(W(RS), W(MG), W(DG))

#define addxxZmr(MG, DG, RS)                                                \
        addzxZmr(W(MG), W(DG), W(RS))

/* sub (G = G - S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define subxx_ri(RG, IS)                                                    \
        subzx_ri(W(RG), W(IS))

#define subxx_mi(MG, DG, IS)                                                \
        subzx_mi(W(MG), W(DG), W(IS))

#define subxx_rr(RG, RS)                                                    \
        subzx_rr(W(RG), W(RS))

#define subxx_ld(RG, MS, DS)                                                \
        subzx_ld(W(RG), W(MS), W(DS))

#define subxx_st(RS, MG, DG)                                                \
        subzx_st(W(RS), W(MG), W(DG))

#define subxx_mr(MG, DG, RS)                                                \
        subzx_mr(W(MG), W(DG), W(RS))


#define subxxZri(RG, IS)                                                    \
        subzxZri(W(RG), W(IS))

#define subxxZmi(MG, DG, IS)                                                \
        subzxZmi(W(MG), W(DG), W(IS))

#define subxxZrr(RG, RS)                                                    \
        subzxZrr(W(RG), W(RS))

#define subxxZld(RG, MS, DS)                                                \
        subzxZld(W(RG), W(MS), W(DS))

#define subxxZst(RS, MG, DG)                                                \
        subzxZst(W(RS), W(MG), W(DG))

#define subxxZmr(MG, DG, RS)                                                \
        subzxZmr(W(MG), W(DG), W(RS))

/* shl (G = G << S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlxx_rx(RG)                     /* reads Recx for shift count */   \
        shlzx_rx(W(RG))

#define shlxx_mx(MG, DG)                 /* reads Recx for shift count */   \
        shlzx_mx(W(MG), W(DG))

#define shlxx_ri(RG, IS)                                                    \
        shlzx_ri(W(RG), W(IS))

#define shlxx_mi(MG, DG, IS)                                                \
        shlzx_mi(W(MG), W(DG), W(IS))

#define shlxx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shlzx_rr(W(RG), W(RS))

#define shlxx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shlzx_ld(W(RG), W(MS), W(DS))

#define shlxx_st(RS, MG, DG)                                                \
        shlzx_st(W(RS), W(MG), W(DG))

#define shlxx_mr(MG, DG, RS)                                                \
        shlzx_mr(W(MG), W(DG), W(RS))


#define shlxxZrx(RG)                     /* reads Recx for shift count */   \
        shlzxZrx(W(RG))

#define shlxxZmx(MG, DG)                 /* reads Recx for shift count */   \
        shlzxZmx(W(MG), W(DG))

#define shlxxZri(RG, IS)                                                    \
        shlzxZri(W(RG), W(IS))

#define shlxxZmi(MG, DG, IS)                                                \
        shlzxZmi(W(MG), W(DG), W(IS))

#define shlxxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        shlzxZrr(W(RG), W(RS))

#define shlxxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shlzxZld(W(RG), W(MS), W(DS))

#define shlxxZst(RS, MG, DG)                                                \
        shlzxZst(W(RS), W(MG), W(DG))

#define shlxxZmr(MG, DG, RS)                                                \
        shlzxZmr(W(MG), W(DG), W(RS))

/* shr (G = G >> S), unsigned (logical)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrxx_rx(RG)                     /* reads Recx for shift count */   \
        shrzx_rx(W(RG))

#define shrxx_mx(MG, DG)                 /* reads Recx for shift count */   \
        shrzx_mx(W(MG), W(DG))

#define shrxx_ri(RG, IS)                                                    \
        shrzx_ri(W(RG), W(IS))

#define shrxx_mi(MG, DG, IS)                                                \
        shrzx_mi(W(MG), W(DG), W(IS))

#define shrxx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrzx_rr(W(RG), W(RS))

#define shrxx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shrzx_ld(W(RG), W(MS), W(DS))

#define shrxx_st(RS, MG, DG)                                                \
        shrzx_st(W(RS), W(MG), W(DG))

#define shrxx_mr(MG, DG, RS)                                                \
        shrzx_mr(W(MG), W(DG), W(RS))


#define shrxxZrx(RG)                     /* reads Recx for shift count */   \
        shrzxZrx(W(RG))

#define shrxxZmx(MG, DG)                 /* reads Recx for shift count */   \
        shrzxZmx(W(MG), W(DG))

#define shrxxZri(RG, IS)                                                    \
        shrzxZri(W(RG), W(IS))

#define shrxxZmi(MG, DG, IS)                                                \
        shrzxZmi(W(MG), W(DG), W(IS))

#define shrxxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrzxZrr(W(RG), W(RS))

#define shrxxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shrzxZld(W(RG), W(MS), W(DS))

#define shrxxZst(RS, MG, DG)                                                \
        shrzxZst(W(RS), W(MG), W(DG))

#define shrxxZmr(MG, DG, RS)                                                \
        shrzxZmr(W(MG), W(DG), W(RS))

/* shr (G = G >> S), signed (arithmetic)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrxn_rx(RG)                     /* reads Recx for shift count */   \
        shrzn_rx(W(RG))

#define shrxn_mx(MG, DG)                 /* reads Recx for shift count */   \
        shrzn_mx(W(MG), W(DG))

#define shrxn_ri(RG, IS)                                                    \
        shrzn_ri(W(RG), W(IS))

#define shrxn_mi(MG, DG, IS)                                                \
        shrzn_mi(W(MG), W(DG), W(IS))

#define shrxn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrzn_rr(W(RG), W(RS))

#define shrxn_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shrzn_ld(W(RG), W(MS), W(DS))

#define shrxn_st(RS, MG, DG)                                                \
        shrzn_st(W(RS), W(MG), W(DG))

#define shrxn_mr(MG, DG, RS)                                                \
        shrzn_mr(W(MG), W(DG), W(RS))


#define shrxnZrx(RG)                     /* reads Recx for shift count */   \
        shrznZrx(W(RG))

#define shrxnZmx(MG, DG)                 /* reads Recx for shift count */   \
        shrznZmx(W(MG), W(DG))

#define shrxnZri(RG, IS)                                                    \
        shrznZri(W(RG), W(IS))

#define shrxnZmi(MG, DG, IS)                                                \
        shrznZmi(W(MG), W(DG), W(IS))

#define shrxnZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrznZrr(W(RG), W(RS))

#define shrxnZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shrznZld(W(RG), W(MS), W(DS))

#define shrxnZst(RS, MG, DG)                                                \
        shrznZst(W(RS), W(MG), W(DG))

#define shrxnZmr(MG, DG, RS)                                                \
        shrznZmr(W(MG), W(DG), W(RS))

/* ror (G = G >> S | G << 64 - S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define rorxx_rx(RG)                     /* reads Recx for shift count */   \
        rorzx_rx(W(RG))

#define rorxx_mx(MG, DG)                 /* reads Recx for shift count */   \
        rorzx_mx(W(MG), W(DG))

#define rorxx_ri(RG, IS)                                                    \
        rorzx_ri(W(RG), W(IS))

#define rorxx_mi(MG, DG, IS)                                                \
        rorzx_mi(W(MG), W(DG), W(IS))

#define rorxx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        rorzx_rr(W(RG), W(RS))

#define rorxx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        rorzx_ld(W(RG), W(MS), W(DS))

#define rorxx_st(RS, MG, DG)                                                \
        rorzx_st(W(RS), W(MG), W(DG))

#define rorxx_mr(MG, DG, RS)                                                \
        rorzx_mr(W(MG), W(DG), W(RS))


#define rorxxZrx(RG)                     /* reads Recx for shift count */   \
        rorzxZrx(W(RG))

#define rorxxZmx(MG, DG)                 /* reads Recx for shift count */   \
        rorzxZmx(W(MG), W(DG))

#define rorxxZri(RG, IS)                                                    \
        rorzxZri(W(RG), W(IS))

#define rorxxZmi(MG, DG, IS)                                                \
        rorzxZmi(W(MG), W(DG), W(IS))

#define rorxxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        rorzxZrr(W(RG), W(RS))

#define rorxxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        rorzxZld(W(RG), W(MS), W(DS))

#define rorxxZst(RS, MG, DG)                                                \
        rorzxZst(W(RS), W(MG), W(DG))

#define rorxxZmr(MG, DG, RS)                                                \
        rorzxZmr(W(MG), W(DG), W(RS))

/* mul (G = G * S)
 * set-flags: undefined */

#define mulxx_ri(RG, IS)                                                    \
        mulzx_ri(W(RG), W(IS))

#define mulxx_rr(RG, RS)                                                    \
        mulzx_rr(W(RG), W(RS))

#define mulxx_ld(RG, MS, DS)                                                \
        mulzx_ld(W(RG), W(MS), W(DS))


#define mulxx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        mulzx_xr(W(RS))

#define mulxx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        mulzx_xm(W(MS), W(DS))


#define mulxn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        mulzn_xr(W(RS))

#define mulxn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        mulzn_xm(W(MS), W(DS))


#define mulxp_xr(RS)     /* Reax is in/out, prepares Redx for divxn_x* */   \
        mulzp_xr(W(RS))       /* product must not exceed operands size */

#define mulxp_xm(MS, DS) /* Reax is in/out, prepares Redx for divxn_x* */   \
        mulzp_xm(W(MS), W(DS))/* product must not exceed operands size */

/* div (G = G / S)
 * set-flags: undefined */

#define divxx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        divzx_ri(W(RG), W(IS))

#define divxx_rr(RG, RS)                 /* RG, RS no Reax, RS no Redx */   \
        divzx_rr(W(RG), W(RS))

#define divxx_ld(RG, MS, DS)   /* Reax cannot be used as first operand */   \
        divzx_ld(W(RG), W(MS), W(DS))


#define divxn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        divzn_ri(W(RG), W(IS))

#define divxn_rr(RG, RS)                 /* RG, RS no Reax, RS no Redx */   \
        divzn_rr(W(RG), W(RS))

#define divxn_ld(RG, MS, DS)   /* Reax cannot be used as first operand */   \
        divzn_ld(W(RG), W(MS), W(DS))


#define prexx_xx()   /* to be placed right before divxx_x* or remxx_xx */   \
        prezx_xx()                   /* to prepare Redx for int-divide */

#define prexn_xx()   /* to be placed right before divxn_x* or remxn_xx */   \
        prezn_xx()                   /* to prepare Redx for int-divide */


#define divxx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        divzx_xr(W(RS))

#define divxx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        divzx_xm(W(MS), W(DS))


#define divxn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divzn_xr(W(RS))

#define divxn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divzn_xm(W(MS), W(DS))


#define divxp_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divzp_xr(W(RS))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divxp_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divzp_xm(W(MS), W(DS))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem (G = G % S)
 * set-flags: undefined */

#define remxx_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        remzx_ri(W(RG), W(IS))

#define remxx_rr(RG, RS)                 /* RG, RS no Redx, RS no Reax */   \
        remzx_rr(W(RG), W(RS))

#define remxx_ld(RG, MS, DS)   /* Redx cannot be used as first operand */   \
        remzx_ld(W(RG), W(MS), W(DS))


#define remxn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        remzn_ri(W(RG), W(IS))

#define remxn_rr(RG, RS)                 /* RG, RS no Redx, RS no Reax */   \
        remzn_rr(W(RG), W(RS))

#define remxn_ld(RG, MS, DS)   /* Redx cannot be used as first operand */   \
        remzn_ld(W(RG), W(MS), W(DS))


#define remxx_xx() /* to be placed before divxx_x*, but after prexx_xx */   \
        remzx_xx()                   /* to prepare for rem calculation */

#define remxx_xr(RS)        /* to be placed immediately after divxx_xr */   \
        remzx_xr(W(RS))              /* to produce remainder Redx<-rem */

#define remxx_xm(MS, DS)    /* to be placed immediately after divxx_xm */   \
        remzx_xm(W(MS), W(DS))       /* to produce remainder Redx<-rem */


#define remxn_xx() /* to be placed before divxn_x*, but after prexn_xx */   \
        remzn_xx()                   /* to prepare for rem calculation */

#define remxn_xr(RS)        /* to be placed immediately after divxn_xr */   \
        remzn_xr(W(RS))              /* to produce remainder Redx<-rem */

#define remxn_xm(MS, DS)    /* to be placed immediately after divxn_xm */   \
        remzn_xm(W(MS), W(DS))       /* to produce remainder Redx<-rem */

/* arj (G = G op S, if cc G then jump lb)
 * set-flags: undefined
 * refer to individual instruction descriptions
 * to stay within special register limitations */

     /* Definitions for arj's "op" and "cc" parameters
      * are provided in 32-bit rtarch_***.h files. */

#define arjxx_rx(RG, op, cc, lb)                                            \
        arjzx_rx(W(RG), op, cc, lb)

#define arjxx_mx(MG, DG, op, cc, lb)                                        \
        arjzx_mx(W(MG), W(DG), op, cc, lb)

#define arjxx_ri(RG, IS, op, cc, lb)                                        \
        arjzx_ri(W(RG), W(IS), op, cc, lb)

#define arjxx_mi(MG, DG, IS, op, cc, lb)                                    \
        arjzx_mi(W(MG), W(DG), W(IS), op, cc, lb)

#define arjxx_rr(RG, RS, op, cc, lb)                                        \
        arjzx_rr(W(RG), W(RS), op, cc, lb)

#define arjxx_ld(RG, MS, DS, op, cc, lb)                                    \
        arjzx_ld(W(RG), W(MS), W(DS), op, cc, lb)

#define arjxx_st(RS, MG, DG, op, cc, lb)                                    \
        arjzx_st(W(RS), W(MG), W(DG), op, cc, lb)

#define arjxx_mr(MG, DG, RS, op, cc, lb)                                    \
        arjzx_mr(W(MG), W(DG), W(RS), op, cc, lb)

/* cmj (flags = S ? T, if cc flags then jump lb)
 * set-flags: undefined */

     /* Definitions for cmj's "cc" parameter
      * are provided in 32-bit rtarch_***.h files. */

#define cmjxx_rz(RS, cc, lb)                                                \
        cmjzx_rz(W(RS), cc, lb)

#define cmjxx_mz(MS, DS, cc, lb)                                            \
        cmjzx_mz(W(MS), W(DS), cc, lb)

#define cmjxx_ri(RS, IT, cc, lb)                                            \
        cmjzx_ri(W(RS), W(IT), cc, lb)

#define cmjxx_mi(MS, DS, IT, cc, lb)                                        \
        cmjzx_mi(W(MS), W(DS), W(IT), cc, lb)

#define cmjxx_rr(RS, RT, cc, lb)                                            \
        cmjzx_rr(W(RS), W(RT), cc, lb)

#define cmjxx_rm(RS, MT, DT, cc, lb)                                        \
        cmjzx_rm(W(RS), W(MT), W(DT), cc, lb)

#define cmjxx_mr(MS, DS, RT, cc, lb)                                        \
        cmjzx_mr(W(MS), W(DS), W(RT), cc, lb)

/* cmp (flags = S ? T)
 * set-flags: yes */

#define cmpxx_ri(RS, IT)                                                    \
        cmpzx_ri(W(RS), W(IT))

#define cmpxx_mi(MS, DS, IT)                                                \
        cmpzx_mi(W(MS), W(DS), W(IT))

#define cmpxx_rr(RS, RT)                                                    \
        cmpzx_rr(W(RS), W(RT))

#define cmpxx_rm(RS, MT, DT)                                                \
        cmpzx_rm(W(RS), W(MT), W(DT))

#define cmpxx_mr(MS, DS, RT)                                                \
        cmpzx_mr(W(MS), W(DS), W(RT))

/* ver (Mebp/inf_VER = SIMD-version)
 * set-flags: no
 * For interpretation of SIMD build flags check compatibility layer in rtzero.h
 * 0th byte - 128-bit version, 1st byte - 256-bit version, | plus _R8/_RX slots
 * 2nd byte - 512-bit version, 3rd byte - 1K4-bit version, | in upper halves */

     /* verxx_xx() in 32-bit rtarch_***.h files, destroys Reax, ... , Redi */

/************************* address-sized instructions *************************/

/* adr (D = adr S)
 * set-flags: no */

     /* adrxx_ld(RD, MS, DS) is defined in 32-bit rtarch_***.h files */

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

     /* jccxx_** is defined in 32-bit rtarch_***.h files */

/************************* register-size instructions *************************/

/* stack (push stack = S, D = pop stack)
 * set-flags: no (sequence cmp/stack_la/jmp is not allowed on MIPS & POWER)
 * adjust stack pointer with 8-byte (64-bit) steps on all current targets */

     /* stack_** is defined in 32-bit rtarch_***.h files */

#endif /* RT_ADDRESS */

/******************************************************************************/
/***************** element-sized instructions for 32-bit mode *****************/
/******************************************************************************/

#if   RT_ELEMENT == 32

/* mov (D = S)
 * set-flags: no */

#define movyx_ri(RD, IS)                                                    \
        movwx_ri(W(RD), W(IS))

#define movyx_mi(MD, DD, IS)                                                \
        movwx_mi(W(MD), W(DD), W(IS))

#define movyx_rr(RD, RS)                                                    \
        movwx_rr(W(RD), W(RS))

#define movyx_ld(RD, MS, DS)                                                \
        movwx_ld(W(RD), W(MS), W(DS))

#define movyx_st(RS, MD, DD)                                                \
        movwx_st(W(RS), W(MD), W(DD))


#define movyx_rj(RD, IT, IS)     /* IT - upper 32-bit, IS - lower 32-bit */ \
        movwx_ri(W(RD), W(IS))

#define movyx_mj(MD, DD, IT, IS) /* IT - upper 32-bit, IS - lower 32-bit */ \
        movwx_mi(W(MD), W(DD), W(IS))

/* and (G = G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define andyx_ri(RG, IS)                                                    \
        andwx_ri(W(RG), W(IS))

#define andyx_mi(MG, DG, IS)                                                \
        andwx_mi(W(MG), W(DG), W(IS))

#define andyx_rr(RG, RS)                                                    \
        andwx_rr(W(RG), W(RS))

#define andyx_ld(RG, MS, DS)                                                \
        andwx_ld(W(RG), W(MS), W(DS))

#define andyx_st(RS, MG, DG)                                                \
        andwx_st(W(RS), W(MG), W(DG))

#define andyx_mr(MG, DG, RS)                                                \
        andwx_mr(W(MG), W(DG), W(RS))


#define andyxZri(RG, IS)                                                    \
        andwxZri(W(RG), W(IS))

#define andyxZmi(MG, DG, IS)                                                \
        andwxZmi(W(MG), W(DG), W(IS))

#define andyxZrr(RG, RS)                                                    \
        andwxZrr(W(RG), W(RS))

#define andyxZld(RG, MS, DS)                                                \
        andwxZld(W(RG), W(MS), W(DS))

#define andyxZst(RS, MG, DG)                                                \
        andwxZst(W(RS), W(MG), W(DG))

#define andyxZmr(MG, DG, RS)                                                \
        andwxZmr(W(MG), W(DG), W(RS))

/* ann (G = ~G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define annyx_ri(RG, IS)                                                    \
        annwx_ri(W(RG), W(IS))

#define annyx_mi(MG, DG, IS)                                                \
        annwx_mi(W(MG), W(DG), W(IS))

#define annyx_rr(RG, RS)                                                    \
        annwx_rr(W(RG), W(RS))

#define annyx_ld(RG, MS, DS)                                                \
        annwx_ld(W(RG), W(MS), W(DS))

#define annyx_st(RS, MG, DG)                                                \
        annwx_st(W(RS), W(MG), W(DG))

#define annyx_mr(MG, DG, RS)                                                \
        annwx_mr(W(MG), W(DG), W(RS))


#define annyxZri(RG, IS)                                                    \
        annwxZri(W(RG), W(IS))

#define annyxZmi(MG, DG, IS)                                                \
        annwxZmi(W(MG), W(DG), W(IS))

#define annyxZrr(RG, RS)                                                    \
        annwxZrr(W(RG), W(RS))

#define annyxZld(RG, MS, DS)                                                \
        annwxZld(W(RG), W(MS), W(DS))

#define annyxZst(RS, MG, DG)                                                \
        annwxZst(W(RS), W(MG), W(DG))

#define annyxZmr(MG, DG, RS)                                                \
        annwxZmr(W(MG), W(DG), W(RS))

/* orr (G = G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define orryx_ri(RG, IS)                                                    \
        orrwx_ri(W(RG), W(IS))

#define orryx_mi(MG, DG, IS)                                                \
        orrwx_mi(W(MG), W(DG), W(IS))

#define orryx_rr(RG, RS)                                                    \
        orrwx_rr(W(RG), W(RS))

#define orryx_ld(RG, MS, DS)                                                \
        orrwx_ld(W(RG), W(MS), W(DS))

#define orryx_st(RS, MG, DG)                                                \
        orrwx_st(W(RS), W(MG), W(DG))

#define orryx_mr(MG, DG, RS)                                                \
        orrwx_mr(W(MG), W(DG), W(RS))


#define orryxZri(RG, IS)                                                    \
        orrwxZri(W(RG), W(IS))

#define orryxZmi(MG, DG, IS)                                                \
        orrwxZmi(W(MG), W(DG), W(IS))

#define orryxZrr(RG, RS)                                                    \
        orrwxZrr(W(RG), W(RS))

#define orryxZld(RG, MS, DS)                                                \
        orrwxZld(W(RG), W(MS), W(DS))

#define orryxZst(RS, MG, DG)                                                \
        orrwxZst(W(RS), W(MG), W(DG))

#define orryxZmr(MG, DG, RS)                                                \
        orrwxZmr(W(MG), W(DG), W(RS))

/* orn (G = ~G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define ornyx_ri(RG, IS)                                                    \
        ornwx_ri(W(RG), W(IS))

#define ornyx_mi(MG, DG, IS)                                                \
        ornwx_mi(W(MG), W(DG), W(IS))

#define ornyx_rr(RG, RS)                                                    \
        ornwx_rr(W(RG), W(RS))

#define ornyx_ld(RG, MS, DS)                                                \
        ornwx_ld(W(RG), W(MS), W(DS))

#define ornyx_st(RS, MG, DG)                                                \
        ornwx_st(W(RS), W(MG), W(DG))

#define ornyx_mr(MG, DG, RS)                                                \
        ornwx_mr(W(MG), W(DG), W(RS))


#define ornyxZri(RG, IS)                                                    \
        ornwxZri(W(RG), W(IS))

#define ornyxZmi(MG, DG, IS)                                                \
        ornwxZmi(W(MG), W(DG), W(IS))

#define ornyxZrr(RG, RS)                                                    \
        ornwxZrr(W(RG), W(RS))

#define ornyxZld(RG, MS, DS)                                                \
        ornwxZld(W(RG), W(MS), W(DS))

#define ornyxZst(RS, MG, DG)                                                \
        ornwxZst(W(RS), W(MG), W(DG))

#define ornyxZmr(MG, DG, RS)                                                \
        ornwxZmr(W(MG), W(DG), W(RS))

/* xor (G = G ^ S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define xoryx_ri(RG, IS)                                                    \
        xorwx_ri(W(RG), W(IS))

#define xoryx_mi(MG, DG, IS)                                                \
        xorwx_mi(W(MG), W(DG), W(IS))

#define xoryx_rr(RG, RS)                                                    \
        xorwx_rr(W(RG), W(RS))

#define xoryx_ld(RG, MS, DS)                                                \
        xorwx_ld(W(RG), W(MS), W(DS))

#define xoryx_st(RS, MG, DG)                                                \
        xorwx_st(W(RS), W(MG), W(DG))

#define xoryx_mr(MG, DG, RS)                                                \
        xorwx_mr(W(MG), W(DG), W(RS))


#define xoryxZri(RG, IS)                                                    \
        xorwxZri(W(RG), W(IS))

#define xoryxZmi(MG, DG, IS)                                                \
        xorwxZmi(W(MG), W(DG), W(IS))

#define xoryxZrr(RG, RS)                                                    \
        xorwxZrr(W(RG), W(RS))

#define xoryxZld(RG, MS, DS)                                                \
        xorwxZld(W(RG), W(MS), W(DS))

#define xoryxZst(RS, MG, DG)                                                \
        xorwxZst(W(RS), W(MG), W(DG))

#define xoryxZmr(MG, DG, RS)                                                \
        xorwxZmr(W(MG), W(DG), W(RS))

/* not (G = ~G)
 * set-flags: no */

#define notyx_rx(RG)                                                        \
        notwx_rx(W(RG))

#define notyx_mx(MG, DG)                                                    \
        notwx_mx(W(MG), W(DG))

/* neg (G = -G)
 * set-flags: undefined (*_*), yes (*Z*) */

#define negyx_rx(RG)                                                        \
        negwx_rx(W(RG))

#define negyx_mx(MG, DG)                                                    \
        negwx_mx(W(MG), W(DG))


#define negyxZrx(RG)                                                        \
        negwxZrx(W(RG))

#define negyxZmx(MG, DG)                                                    \
        negwxZmx(W(MG), W(DG))

/* add (G = G + S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define addyx_ri(RG, IS)                                                    \
        addwx_ri(W(RG), W(IS))

#define addyx_mi(MG, DG, IS)                                                \
        addwx_mi(W(MG), W(DG), W(IS))

#define addyx_rr(RG, RS)                                                    \
        addwx_rr(W(RG), W(RS))

#define addyx_ld(RG, MS, DS)                                                \
        addwx_ld(W(RG), W(MS), W(DS))

#define addyx_st(RS, MG, DG)                                                \
        addwx_st(W(RS), W(MG), W(DG))

#define addyx_mr(MG, DG, RS)                                                \
        addwx_mr(W(MG), W(DG), W(RS))


#define addyxZri(RG, IS)                                                    \
        addwxZri(W(RG), W(IS))

#define addyxZmi(MG, DG, IS)                                                \
        addwxZmi(W(MG), W(DG), W(IS))

#define addyxZrr(RG, RS)                                                    \
        addwxZrr(W(RG), W(RS))

#define addyxZld(RG, MS, DS)                                                \
        addwxZld(W(RG), W(MS), W(DS))

#define addyxZst(RS, MG, DG)                                                \
        addwxZst(W(RS), W(MG), W(DG))

#define addyxZmr(MG, DG, RS)                                                \
        addwxZmr(W(MG), W(DG), W(RS))

/* sub (G = G - S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define subyx_ri(RG, IS)                                                    \
        subwx_ri(W(RG), W(IS))

#define subyx_mi(MG, DG, IS)                                                \
        subwx_mi(W(MG), W(DG), W(IS))

#define subyx_rr(RG, RS)                                                    \
        subwx_rr(W(RG), W(RS))

#define subyx_ld(RG, MS, DS)                                                \
        subwx_ld(W(RG), W(MS), W(DS))

#define subyx_st(RS, MG, DG)                                                \
        subwx_st(W(RS), W(MG), W(DG))

#define subyx_mr(MG, DG, RS)                                                \
        subwx_mr(W(MG), W(DG), W(RS))


#define subyxZri(RG, IS)                                                    \
        subwxZri(W(RG), W(IS))

#define subyxZmi(MG, DG, IS)                                                \
        subwxZmi(W(MG), W(DG), W(IS))

#define subyxZrr(RG, RS)                                                    \
        subwxZrr(W(RG), W(RS))

#define subyxZld(RG, MS, DS)                                                \
        subwxZld(W(RG), W(MS), W(DS))

#define subyxZst(RS, MG, DG)                                                \
        subwxZst(W(RS), W(MG), W(DG))

#define subyxZmr(MG, DG, RS)                                                \
        subwxZmr(W(MG), W(DG), W(RS))

/* shl (G = G << S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlyx_rx(RG)                     /* reads Recx for shift count */   \
        shlwx_rx(W(RG))

#define shlyx_mx(MG, DG)                 /* reads Recx for shift count */   \
        shlwx_mx(W(MG), W(DG))

#define shlyx_ri(RG, IS)                                                    \
        shlwx_ri(W(RG), W(IS))

#define shlyx_mi(MG, DG, IS)                                                \
        shlwx_mi(W(MG), W(DG), W(IS))

#define shlyx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shlwx_rr(W(RG), W(RS))

#define shlyx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shlwx_ld(W(RG), W(MS), W(DS))

#define shlyx_st(RS, MG, DG)                                                \
        shlwx_st(W(RS), W(MG), W(DG))

#define shlyx_mr(MG, DG, RS)                                                \
        shlwx_mr(W(MG), W(DG), W(RS))


#define shlyxZrx(RG)                     /* reads Recx for shift count */   \
        shlwxZrx(W(RG))

#define shlyxZmx(MG, DG)                 /* reads Recx for shift count */   \
        shlwxZmx(W(MG), W(DG))

#define shlyxZri(RG, IS)                                                    \
        shlwxZri(W(RG), W(IS))

#define shlyxZmi(MG, DG, IS)                                                \
        shlwxZmi(W(MG), W(DG), W(IS))

#define shlyxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        shlwxZrr(W(RG), W(RS))

#define shlyxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shlwxZld(W(RG), W(MS), W(DS))

#define shlyxZst(RS, MG, DG)                                                \
        shlwxZst(W(RS), W(MG), W(DG))

#define shlyxZmr(MG, DG, RS)                                                \
        shlwxZmr(W(MG), W(DG), W(RS))

/* shr (G = G >> S), unsigned (logical)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shryx_rx(RG)                     /* reads Recx for shift count */   \
        shrwx_rx(W(RG))

#define shryx_mx(MG, DG)                 /* reads Recx for shift count */   \
        shrwx_mx(W(MG), W(DG))

#define shryx_ri(RG, IS)                                                    \
        shrwx_ri(W(RG), W(IS))

#define shryx_mi(MG, DG, IS)                                                \
        shrwx_mi(W(MG), W(DG), W(IS))

#define shryx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrwx_rr(W(RG), W(RS))

#define shryx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shrwx_ld(W(RG), W(MS), W(DS))

#define shryx_st(RS, MG, DG)                                                \
        shrwx_st(W(RS), W(MG), W(DG))

#define shryx_mr(MG, DG, RS)                                                \
        shrwx_mr(W(MG), W(DG), W(RS))


#define shryxZrx(RG)                     /* reads Recx for shift count */   \
        shrwxZrx(W(RG))

#define shryxZmx(MG, DG)                 /* reads Recx for shift count */   \
        shrwxZmx(W(MG), W(DG))

#define shryxZri(RG, IS)                                                    \
        shrwxZri(W(RG), W(IS))

#define shryxZmi(MG, DG, IS)                                                \
        shrwxZmi(W(MG), W(DG), W(IS))

#define shryxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrwxZrr(W(RG), W(RS))

#define shryxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shrwxZld(W(RG), W(MS), W(DS))

#define shryxZst(RS, MG, DG)                                                \
        shrwxZst(W(RS), W(MG), W(DG))

#define shryxZmr(MG, DG, RS)                                                \
        shrwxZmr(W(MG), W(DG), W(RS))

/* shr (G = G >> S), signed (arithmetic)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shryn_rx(RG)                     /* reads Recx for shift count */   \
        shrwn_rx(W(RG))

#define shryn_mx(MG, DG)                 /* reads Recx for shift count */   \
        shrwn_mx(W(MG), W(DG))

#define shryn_ri(RG, IS)                                                    \
        shrwn_ri(W(RG), W(IS))

#define shryn_mi(MG, DG, IS)                                                \
        shrwn_mi(W(MG), W(DG), W(IS))

#define shryn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrwn_rr(W(RG), W(RS))

#define shryn_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shrwn_ld(W(RG), W(MS), W(DS))

#define shryn_st(RS, MG, DG)                                                \
        shrwn_st(W(RS), W(MG), W(DG))

#define shryn_mr(MG, DG, RS)                                                \
        shrwn_mr(W(MG), W(DG), W(RS))


#define shrynZrx(RG)                     /* reads Recx for shift count */   \
        shrwnZrx(W(RG))

#define shrynZmx(MG, DG)                 /* reads Recx for shift count */   \
        shrwnZmx(W(MG), W(DG))

#define shrynZri(RG, IS)                                                    \
        shrwnZri(W(RG), W(IS))

#define shrynZmi(MG, DG, IS)                                                \
        shrwnZmi(W(MG), W(DG), W(IS))

#define shrynZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrwnZrr(W(RG), W(RS))

#define shrynZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shrwnZld(W(RG), W(MS), W(DS))

#define shrynZst(RS, MG, DG)                                                \
        shrwnZst(W(RS), W(MG), W(DG))

#define shrynZmr(MG, DG, RS)                                                \
        shrwnZmr(W(MG), W(DG), W(RS))

/* ror (G = G >> S | G << 32 - S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define roryx_rx(RG)                     /* reads Recx for shift count */   \
        rorwx_rx(W(RG))

#define roryx_mx(MG, DG)                 /* reads Recx for shift count */   \
        rorwx_mx(W(MG), W(DG))

#define roryx_ri(RG, IS)                                                    \
        rorwx_ri(W(RG), W(IS))

#define roryx_mi(MG, DG, IS)                                                \
        rorwx_mi(W(MG), W(DG), W(IS))

#define roryx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        rorwx_rr(W(RG), W(RS))

#define roryx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        rorwx_ld(W(RG), W(MS), W(DS))

#define roryx_st(RS, MG, DG)                                                \
        rorwx_st(W(RS), W(MG), W(DG))

#define roryx_mr(MG, DG, RS)                                                \
        rorwx_mr(W(MG), W(DG), W(RS))


#define roryxZrx(RG)                     /* reads Recx for shift count */   \
        rorwxZrx(W(RG))

#define roryxZmx(MG, DG)                 /* reads Recx for shift count */   \
        rorwxZmx(W(MG), W(DG))

#define roryxZri(RG, IS)                                                    \
        rorwxZri(W(RG), W(IS))

#define roryxZmi(MG, DG, IS)                                                \
        rorwxZmi(W(MG), W(DG), W(IS))

#define roryxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        rorwxZrr(W(RG), W(RS))

#define roryxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        rorwxZld(W(RG), W(MS), W(DS))

#define roryxZst(RS, MG, DG)                                                \
        rorwxZst(W(RS), W(MG), W(DG))

#define roryxZmr(MG, DG, RS)                                                \
        rorwxZmr(W(MG), W(DG), W(RS))

/* mul (G = G * S)
 * set-flags: undefined */

#define mulyx_ri(RG, IS)                                                    \
        mulwx_ri(W(RG), W(IS))

#define mulyx_rr(RG, RS)                                                    \
        mulwx_rr(W(RG), W(RS))

#define mulyx_ld(RG, MS, DS)                                                \
        mulwx_ld(W(RG), W(MS), W(DS))


#define mulyx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        mulwx_xr(W(RS))

#define mulyx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        mulwx_xm(W(MS), W(DS))


#define mulyn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        mulwn_xr(W(RS))

#define mulyn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        mulwn_xm(W(MS), W(DS))


#define mulyp_xr(RS)     /* Reax is in/out, prepares Redx for divyn_x* */   \
        mulwp_xr(W(RS))       /* product must not exceed operands size */

#define mulyp_xm(MS, DS) /* Reax is in/out, prepares Redx for divyn_x* */   \
        mulwp_xm(W(MS), W(DS))/* product must not exceed operands size */

/* div (G = G / S)
 * set-flags: undefined */

#define divyx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        divwx_ri(W(RG), W(IS))

#define divyx_rr(RG, RS)                 /* RG, RS no Reax, RS no Redx */   \
        divwx_rr(W(RG), W(RS))

#define divyx_ld(RG, MS, DS)   /* Reax cannot be used as first operand */   \
        divwx_ld(W(RG), W(MS), W(DS))


#define divyn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        divwn_ri(W(RG), W(IS))

#define divyn_rr(RG, RS)                 /* RG, RS no Reax, RS no Redx */   \
        divwn_rr(W(RG), W(RS))

#define divyn_ld(RG, MS, DS)   /* Reax cannot be used as first operand */   \
        divwn_ld(W(RG), W(MS), W(DS))


#define preyx_xx()   /* to be placed right before divyx_x* or remyx_xx */   \
        prewx_xx()                   /* to prepare Redx for int-divide */

#define preyn_xx()   /* to be placed right before divyn_x* or remyn_xx */   \
        prewn_xx()                   /* to prepare Redx for int-divide */


#define divyx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        divwx_xr(W(RS))

#define divyx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        divwx_xm(W(MS), W(DS))


#define divyn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xr(W(RS))

#define divyn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xm(W(MS), W(DS))


#define divyp_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwp_xr(W(RS))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divyp_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwp_xm(W(MS), W(DS))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem (G = G % S)
 * set-flags: undefined */

#define remyx_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        remwx_ri(W(RG), W(IS))

#define remyx_rr(RG, RS)                 /* RG, RS no Redx, RS no Reax */   \
        remwx_rr(W(RG), W(RS))

#define remyx_ld(RG, MS, DS)   /* Redx cannot be used as first operand */   \
        remwx_ld(W(RG), W(MS), W(DS))


#define remyn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        remwn_ri(W(RG), W(IS))

#define remyn_rr(RG, RS)                 /* RG, RS no Redx, RS no Reax */   \
        remwn_rr(W(RG), W(RS))

#define remyn_ld(RG, MS, DS)   /* Redx cannot be used as first operand */   \
        remwn_ld(W(RG), W(MS), W(DS))


#define remyx_xx() /* to be placed before divyx_x*, but after preyx_xx */   \
        remwx_xx()                   /* to prepare for rem calculation */

#define remyx_xr(RS)        /* to be placed immediately after divyx_xr */   \
        remwx_xr(W(RS))              /* to produce remainder Redx<-rem */

#define remyx_xm(MS, DS)    /* to be placed immediately after divyx_xm */   \
        remwx_xm(W(MS), W(DS))       /* to produce remainder Redx<-rem */


#define remyn_xx() /* to be placed before divyn_x*, but after preyn_xx */   \
        remwn_xx()                   /* to prepare for rem calculation */

#define remyn_xr(RS)        /* to be placed immediately after divyn_xr */   \
        remwn_xr(W(RS))              /* to produce remainder Redx<-rem */

#define remyn_xm(MS, DS)    /* to be placed immediately after divyn_xm */   \
        remwn_xm(W(MS), W(DS))       /* to produce remainder Redx<-rem */

/* arj (G = G op S, if cc G then jump lb)
 * set-flags: undefined
 * refer to individual instruction descriptions
 * to stay within special register limitations */

     /* Definitions for arj's "op" and "cc" parameters
      * are provided in 32-bit rtarch_***.h files. */

#define arjyx_rx(RG, op, cc, lb)                                            \
        arjwx_rx(W(RG), op, cc, lb)

#define arjyx_mx(MG, DG, op, cc, lb)                                        \
        arjwx_mx(W(MG), W(DG), op, cc, lb)

#define arjyx_ri(RG, IS, op, cc, lb)                                        \
        arjwx_ri(W(RG), W(IS), op, cc, lb)

#define arjyx_mi(MG, DG, IS, op, cc, lb)                                    \
        arjwx_mi(W(MG), W(DG), W(IS), op, cc, lb)

#define arjyx_rr(RG, RS, op, cc, lb)                                        \
        arjwx_rr(W(RG), W(RS), op, cc, lb)

#define arjyx_ld(RG, MS, DS, op, cc, lb)                                    \
        arjwx_ld(W(RG), W(MS), W(DS), op, cc, lb)

#define arjyx_st(RS, MG, DG, op, cc, lb)                                    \
        arjwx_st(W(RS), W(MG), W(DG), op, cc, lb)

#define arjyx_mr(MG, DG, RS, op, cc, lb)                                    \
        arjwx_mr(W(MG), W(DG), W(RS), op, cc, lb)

/* cmj (flags = S ? T, if cc flags then jump lb)
 * set-flags: undefined */

     /* Definitions for cmj's "cc" parameter
      * are provided in 32-bit rtarch_***.h files. */

#define cmjyx_rz(RS, cc, lb)                                                \
        cmjwx_rz(W(RS), cc, lb)

#define cmjyx_mz(MS, DS, cc, lb)                                            \
        cmjwx_mz(W(MS), W(DS), cc, lb)

#define cmjyx_ri(RS, IT, cc, lb)                                            \
        cmjwx_ri(W(RS), W(IT), cc, lb)

#define cmjyx_mi(MS, DS, IT, cc, lb)                                        \
        cmjwx_mi(W(MS), W(DS), W(IT), cc, lb)

#define cmjyx_rr(RS, RT, cc, lb)                                            \
        cmjwx_rr(W(RS), W(RT), cc, lb)

#define cmjyx_rm(RS, MT, DT, cc, lb)                                        \
        cmjwx_rm(W(RS), W(MT), W(DT), cc, lb)

#define cmjyx_mr(MS, DS, RT, cc, lb)                                        \
        cmjwx_mr(W(MS), W(DS), W(RT), cc, lb)

/* cmp (flags = S ? T)
 * set-flags: yes */

#define cmpyx_ri(RS, IT)                                                    \
        cmpwx_ri(W(RS), W(IT))

#define cmpyx_mi(MS, DS, IT)                                                \
        cmpwx_mi(W(MS), W(DS), W(IT))

#define cmpyx_rr(RS, RT)                                                    \
        cmpwx_rr(W(RS), W(RT))

#define cmpyx_rm(RS, MT, DT)                                                \
        cmpwx_rm(W(RS), W(MT), W(DT))

#define cmpyx_mr(MS, DS, RT)                                                \
        cmpwx_mr(W(MS), W(DS), W(RT))

/******************************************************************************/
/***************** element-sized instructions for 64-bit mode *****************/
/******************************************************************************/

#elif RT_ELEMENT == 64

/* mov (D = S)
 * set-flags: no */

#define movyx_ri(RD, IS)                                                    \
        movzx_ri(W(RD), W(IS))

#define movyx_mi(MD, DD, IS)                                                \
        movzx_mi(W(MD), W(DD), W(IS))

#define movyx_rr(RD, RS)                                                    \
        movzx_rr(W(RD), W(RS))

#define movyx_ld(RD, MS, DS)                                                \
        movzx_ld(W(RD), W(MS), W(DS))

#define movyx_st(RS, MD, DD)                                                \
        movzx_st(W(RS), W(MD), W(DD))


#define movyx_rj(RD, IT, IS)     /* IT - upper 32-bit, IS - lower 32-bit */ \
        movzx_ri(W(RD), W(IS))

#define movyx_mj(MD, DD, IT, IS) /* IT - upper 32-bit, IS - lower 32-bit */ \
        movzx_mi(W(MD), W(DD), W(IS))

/* and (G = G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define andyx_ri(RG, IS)                                                    \
        andzx_ri(W(RG), W(IS))

#define andyx_mi(MG, DG, IS)                                                \
        andzx_mi(W(MG), W(DG), W(IS))

#define andyx_rr(RG, RS)                                                    \
        andzx_rr(W(RG), W(RS))

#define andyx_ld(RG, MS, DS)                                                \
        andzx_ld(W(RG), W(MS), W(DS))

#define andyx_st(RS, MG, DG)                                                \
        andzx_st(W(RS), W(MG), W(DG))

#define andyx_mr(MG, DG, RS)                                                \
        andzx_mr(W(MG), W(DG), W(RS))


#define andyxZri(RG, IS)                                                    \
        andzxZri(W(RG), W(IS))

#define andyxZmi(MG, DG, IS)                                                \
        andzxZmi(W(MG), W(DG), W(IS))

#define andyxZrr(RG, RS)                                                    \
        andzxZrr(W(RG), W(RS))

#define andyxZld(RG, MS, DS)                                                \
        andzxZld(W(RG), W(MS), W(DS))

#define andyxZst(RS, MG, DG)                                                \
        andzxZst(W(RS), W(MG), W(DG))

#define andyxZmr(MG, DG, RS)                                                \
        andzxZmr(W(MG), W(DG), W(RS))

/* ann (G = ~G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define annyx_ri(RG, IS)                                                    \
        annzx_ri(W(RG), W(IS))

#define annyx_mi(MG, DG, IS)                                                \
        annzx_mi(W(MG), W(DG), W(IS))

#define annyx_rr(RG, RS)                                                    \
        annzx_rr(W(RG), W(RS))

#define annyx_ld(RG, MS, DS)                                                \
        annzx_ld(W(RG), W(MS), W(DS))

#define annyx_st(RS, MG, DG)                                                \
        annzx_st(W(RS), W(MG), W(DG))

#define annyx_mr(MG, DG, RS)                                                \
        annzx_mr(W(MG), W(DG), W(RS))


#define annyxZri(RG, IS)                                                    \
        annzxZri(W(RG), W(IS))

#define annyxZmi(MG, DG, IS)                                                \
        annzxZmi(W(MG), W(DG), W(IS))

#define annyxZrr(RG, RS)                                                    \
        annzxZrr(W(RG), W(RS))

#define annyxZld(RG, MS, DS)                                                \
        annzxZld(W(RG), W(MS), W(DS))

#define annyxZst(RS, MG, DG)                                                \
        annzxZst(W(RS), W(MG), W(DG))

#define annyxZmr(MG, DG, RS)                                                \
        annzxZmr(W(MG), W(DG), W(RS))

/* orr (G = G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define orryx_ri(RG, IS)                                                    \
        orrzx_ri(W(RG), W(IS))

#define orryx_mi(MG, DG, IS)                                                \
        orrzx_mi(W(MG), W(DG), W(IS))

#define orryx_rr(RG, RS)                                                    \
        orrzx_rr(W(RG), W(RS))

#define orryx_ld(RG, MS, DS)                                                \
        orrzx_ld(W(RG), W(MS), W(DS))

#define orryx_st(RS, MG, DG)                                                \
        orrzx_st(W(RS), W(MG), W(DG))

#define orryx_mr(MG, DG, RS)                                                \
        orrzx_mr(W(MG), W(DG), W(RS))


#define orryxZri(RG, IS)                                                    \
        orrzxZri(W(RG), W(IS))

#define orryxZmi(MG, DG, IS)                                                \
        orrzxZmi(W(MG), W(DG), W(IS))

#define orryxZrr(RG, RS)                                                    \
        orrzxZrr(W(RG), W(RS))

#define orryxZld(RG, MS, DS)                                                \
        orrzxZld(W(RG), W(MS), W(DS))

#define orryxZst(RS, MG, DG)                                                \
        orrzxZst(W(RS), W(MG), W(DG))

#define orryxZmr(MG, DG, RS)                                                \
        orrzxZmr(W(MG), W(DG), W(RS))

/* orn (G = ~G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define ornyx_ri(RG, IS)                                                    \
        ornzx_ri(W(RG), W(IS))

#define ornyx_mi(MG, DG, IS)                                                \
        ornzx_mi(W(MG), W(DG), W(IS))

#define ornyx_rr(RG, RS)                                                    \
        ornzx_rr(W(RG), W(RS))

#define ornyx_ld(RG, MS, DS)                                                \
        ornzx_ld(W(RG), W(MS), W(DS))

#define ornyx_st(RS, MG, DG)                                                \
        ornzx_st(W(RS), W(MG), W(DG))

#define ornyx_mr(MG, DG, RS)                                                \
        ornzx_mr(W(MG), W(DG), W(RS))


#define ornyxZri(RG, IS)                                                    \
        ornzxZri(W(RG), W(IS))

#define ornyxZmi(MG, DG, IS)                                                \
        ornzxZmi(W(MG), W(DG), W(IS))

#define ornyxZrr(RG, RS)                                                    \
        ornzxZrr(W(RG), W(RS))

#define ornyxZld(RG, MS, DS)                                                \
        ornzxZld(W(RG), W(MS), W(DS))

#define ornyxZst(RS, MG, DG)                                                \
        ornzxZst(W(RS), W(MG), W(DG))

#define ornyxZmr(MG, DG, RS)                                                \
        ornzxZmr(W(MG), W(DG), W(RS))

/* xor (G = G ^ S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define xoryx_ri(RG, IS)                                                    \
        xorzx_ri(W(RG), W(IS))

#define xoryx_mi(MG, DG, IS)                                                \
        xorzx_mi(W(MG), W(DG), W(IS))

#define xoryx_rr(RG, RS)                                                    \
        xorzx_rr(W(RG), W(RS))

#define xoryx_ld(RG, MS, DS)                                                \
        xorzx_ld(W(RG), W(MS), W(DS))

#define xoryx_st(RS, MG, DG)                                                \
        xorzx_st(W(RS), W(MG), W(DG))

#define xoryx_mr(MG, DG, RS)                                                \
        xorzx_mr(W(MG), W(DG), W(RS))


#define xoryxZri(RG, IS)                                                    \
        xorzxZri(W(RG), W(IS))

#define xoryxZmi(MG, DG, IS)                                                \
        xorzxZmi(W(MG), W(DG), W(IS))

#define xoryxZrr(RG, RS)                                                    \
        xorzxZrr(W(RG), W(RS))

#define xoryxZld(RG, MS, DS)                                                \
        xorzxZld(W(RG), W(MS), W(DS))

#define xoryxZst(RS, MG, DG)                                                \
        xorzxZst(W(RS), W(MG), W(DG))

#define xoryxZmr(MG, DG, RS)                                                \
        xorzxZmr(W(MG), W(DG), W(RS))

/* not (G = ~G)
 * set-flags: no */

#define notyx_rx(RG)                                                        \
        notzx_rx(W(RG))

#define notyx_mx(MG, DG)                                                    \
        notzx_mx(W(MG), W(DG))

/* neg (G = -G)
 * set-flags: undefined (*_*), yes (*Z*) */

#define negyx_rx(RG)                                                        \
        negzx_rx(W(RG))

#define negyx_mx(MG, DG)                                                    \
        negzx_mx(W(MG), W(DG))


#define negyxZrx(RG)                                                        \
        negzxZrx(W(RG))

#define negyxZmx(MG, DG)                                                    \
        negzxZmx(W(MG), W(DG))

/* add (G = G + S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define addyx_ri(RG, IS)                                                    \
        addzx_ri(W(RG), W(IS))

#define addyx_mi(MG, DG, IS)                                                \
        addzx_mi(W(MG), W(DG), W(IS))

#define addyx_rr(RG, RS)                                                    \
        addzx_rr(W(RG), W(RS))

#define addyx_ld(RG, MS, DS)                                                \
        addzx_ld(W(RG), W(MS), W(DS))

#define addyx_st(RS, MG, DG)                                                \
        addzx_st(W(RS), W(MG), W(DG))

#define addyx_mr(MG, DG, RS)                                                \
        addzx_mr(W(MG), W(DG), W(RS))


#define addyxZri(RG, IS)                                                    \
        addzxZri(W(RG), W(IS))

#define addyxZmi(MG, DG, IS)                                                \
        addzxZmi(W(MG), W(DG), W(IS))

#define addyxZrr(RG, RS)                                                    \
        addzxZrr(W(RG), W(RS))

#define addyxZld(RG, MS, DS)                                                \
        addzxZld(W(RG), W(MS), W(DS))

#define addyxZst(RS, MG, DG)                                                \
        addzxZst(W(RS), W(MG), W(DG))

#define addyxZmr(MG, DG, RS)                                                \
        addzxZmr(W(MG), W(DG), W(RS))

/* sub (G = G - S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define subyx_ri(RG, IS)                                                    \
        subzx_ri(W(RG), W(IS))

#define subyx_mi(MG, DG, IS)                                                \
        subzx_mi(W(MG), W(DG), W(IS))

#define subyx_rr(RG, RS)                                                    \
        subzx_rr(W(RG), W(RS))

#define subyx_ld(RG, MS, DS)                                                \
        subzx_ld(W(RG), W(MS), W(DS))

#define subyx_st(RS, MG, DG)                                                \
        subzx_st(W(RS), W(MG), W(DG))

#define subyx_mr(MG, DG, RS)                                                \
        subzx_mr(W(MG), W(DG), W(RS))


#define subyxZri(RG, IS)                                                    \
        subzxZri(W(RG), W(IS))

#define subyxZmi(MG, DG, IS)                                                \
        subzxZmi(W(MG), W(DG), W(IS))

#define subyxZrr(RG, RS)                                                    \
        subzxZrr(W(RG), W(RS))

#define subyxZld(RG, MS, DS)                                                \
        subzxZld(W(RG), W(MS), W(DS))

#define subyxZst(RS, MG, DG)                                                \
        subzxZst(W(RS), W(MG), W(DG))

#define subyxZmr(MG, DG, RS)                                                \
        subzxZmr(W(MG), W(DG), W(RS))

/* shl (G = G << S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlyx_rx(RG)                     /* reads Recx for shift count */   \
        shlzx_rx(W(RG))

#define shlyx_mx(MG, DG)                 /* reads Recx for shift count */   \
        shlzx_mx(W(MG), W(DG))

#define shlyx_ri(RG, IS)                                                    \
        shlzx_ri(W(RG), W(IS))

#define shlyx_mi(MG, DG, IS)                                                \
        shlzx_mi(W(MG), W(DG), W(IS))

#define shlyx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shlzx_rr(W(RG), W(RS))

#define shlyx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shlzx_ld(W(RG), W(MS), W(DS))

#define shlyx_st(RS, MG, DG)                                                \
        shlzx_st(W(RS), W(MG), W(DG))

#define shlyx_mr(MG, DG, RS)                                                \
        shlzx_mr(W(MG), W(DG), W(RS))


#define shlyxZrx(RG)                     /* reads Recx for shift count */   \
        shlzxZrx(W(RG))

#define shlyxZmx(MG, DG)                 /* reads Recx for shift count */   \
        shlzxZmx(W(MG), W(DG))

#define shlyxZri(RG, IS)                                                    \
        shlzxZri(W(RG), W(IS))

#define shlyxZmi(MG, DG, IS)                                                \
        shlzxZmi(W(MG), W(DG), W(IS))

#define shlyxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        shlzxZrr(W(RG), W(RS))

#define shlyxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shlzxZld(W(RG), W(MS), W(DS))

#define shlyxZst(RS, MG, DG)                                                \
        shlzxZst(W(RS), W(MG), W(DG))

#define shlyxZmr(MG, DG, RS)                                                \
        shlzxZmr(W(MG), W(DG), W(RS))

/* shr (G = G >> S), unsigned (logical)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shryx_rx(RG)                     /* reads Recx for shift count */   \
        shrzx_rx(W(RG))

#define shryx_mx(MG, DG)                 /* reads Recx for shift count */   \
        shrzx_mx(W(MG), W(DG))

#define shryx_ri(RG, IS)                                                    \
        shrzx_ri(W(RG), W(IS))

#define shryx_mi(MG, DG, IS)                                                \
        shrzx_mi(W(MG), W(DG), W(IS))

#define shryx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrzx_rr(W(RG), W(RS))

#define shryx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shrzx_ld(W(RG), W(MS), W(DS))

#define shryx_st(RS, MG, DG)                                                \
        shrzx_st(W(RS), W(MG), W(DG))

#define shryx_mr(MG, DG, RS)                                                \
        shrzx_mr(W(MG), W(DG), W(RS))


#define shryxZrx(RG)                     /* reads Recx for shift count */   \
        shrzxZrx(W(RG))

#define shryxZmx(MG, DG)                 /* reads Recx for shift count */   \
        shrzxZmx(W(MG), W(DG))

#define shryxZri(RG, IS)                                                    \
        shrzxZri(W(RG), W(IS))

#define shryxZmi(MG, DG, IS)                                                \
        shrzxZmi(W(MG), W(DG), W(IS))

#define shryxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrzxZrr(W(RG), W(RS))

#define shryxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shrzxZld(W(RG), W(MS), W(DS))

#define shryxZst(RS, MG, DG)                                                \
        shrzxZst(W(RS), W(MG), W(DG))

#define shryxZmr(MG, DG, RS)                                                \
        shrzxZmr(W(MG), W(DG), W(RS))

/* shr (G = G >> S), signed (arithmetic)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shryn_rx(RG)                     /* reads Recx for shift count */   \
        shrzn_rx(W(RG))

#define shryn_mx(MG, DG)                 /* reads Recx for shift count */   \
        shrzn_mx(W(MG), W(DG))

#define shryn_ri(RG, IS)                                                    \
        shrzn_ri(W(RG), W(IS))

#define shryn_mi(MG, DG, IS)                                                \
        shrzn_mi(W(MG), W(DG), W(IS))

#define shryn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrzn_rr(W(RG), W(RS))

#define shryn_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shrzn_ld(W(RG), W(MS), W(DS))

#define shryn_st(RS, MG, DG)                                                \
        shrzn_st(W(RS), W(MG), W(DG))

#define shryn_mr(MG, DG, RS)                                                \
        shrzn_mr(W(MG), W(DG), W(RS))


#define shrynZrx(RG)                     /* reads Recx for shift count */   \
        shrznZrx(W(RG))

#define shrynZmx(MG, DG)                 /* reads Recx for shift count */   \
        shrznZmx(W(MG), W(DG))

#define shrynZri(RG, IS)                                                    \
        shrznZri(W(RG), W(IS))

#define shrynZmi(MG, DG, IS)                                                \
        shrznZmi(W(MG), W(DG), W(IS))

#define shrynZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrznZrr(W(RG), W(RS))

#define shrynZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shrznZld(W(RG), W(MS), W(DS))

#define shrynZst(RS, MG, DG)                                                \
        shrznZst(W(RS), W(MG), W(DG))

#define shrynZmr(MG, DG, RS)                                                \
        shrznZmr(W(MG), W(DG), W(RS))

/* ror (G = G >> S | G << 64 - S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define roryx_rx(RG)                     /* reads Recx for shift count */   \
        rorzx_rx(W(RG))

#define roryx_mx(MG, DG)                 /* reads Recx for shift count */   \
        rorzx_mx(W(MG), W(DG))

#define roryx_ri(RG, IS)                                                    \
        rorzx_ri(W(RG), W(IS))

#define roryx_mi(MG, DG, IS)                                                \
        rorzx_mi(W(MG), W(DG), W(IS))

#define roryx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        rorzx_rr(W(RG), W(RS))

#define roryx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        rorzx_ld(W(RG), W(MS), W(DS))

#define roryx_st(RS, MG, DG)                                                \
        rorzx_st(W(RS), W(MG), W(DG))

#define roryx_mr(MG, DG, RS)                                                \
        rorzx_mr(W(MG), W(DG), W(RS))


#define roryxZrx(RG)                     /* reads Recx for shift count */   \
        rorzxZrx(W(RG))

#define roryxZmx(MG, DG)                 /* reads Recx for shift count */   \
        rorzxZmx(W(MG), W(DG))

#define roryxZri(RG, IS)                                                    \
        rorzxZri(W(RG), W(IS))

#define roryxZmi(MG, DG, IS)                                                \
        rorzxZmi(W(MG), W(DG), W(IS))

#define roryxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        rorzxZrr(W(RG), W(RS))

#define roryxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        rorzxZld(W(RG), W(MS), W(DS))

#define roryxZst(RS, MG, DG)                                                \
        rorzxZst(W(RS), W(MG), W(DG))

#define roryxZmr(MG, DG, RS)                                                \
        rorzxZmr(W(MG), W(DG), W(RS))

/* mul (G = G * S)
 * set-flags: undefined */

#define mulyx_ri(RG, IS)                                                    \
        mulzx_ri(W(RG), W(IS))

#define mulyx_rr(RG, RS)                                                    \
        mulzx_rr(W(RG), W(RS))

#define mulyx_ld(RG, MS, DS)                                                \
        mulzx_ld(W(RG), W(MS), W(DS))


#define mulyx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        mulzx_xr(W(RS))

#define mulyx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        mulzx_xm(W(MS), W(DS))


#define mulyn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        mulzn_xr(W(RS))

#define mulyn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        mulzn_xm(W(MS), W(DS))


#define mulyp_xr(RS)     /* Reax is in/out, prepares Redx for divyn_x* */   \
        mulzp_xr(W(RS))       /* product must not exceed operands size */

#define mulyp_xm(MS, DS) /* Reax is in/out, prepares Redx for divyn_x* */   \
        mulzp_xm(W(MS), W(DS))/* product must not exceed operands size */

/* div (G = G / S)
 * set-flags: undefined */

#define divyx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        divzx_ri(W(RG), W(IS))

#define divyx_rr(RG, RS)                 /* RG, RS no Reax, RS no Redx */   \
        divzx_rr(W(RG), W(RS))

#define divyx_ld(RG, MS, DS)   /* Reax cannot be used as first operand */   \
        divzx_ld(W(RG), W(MS), W(DS))


#define divyn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        divzn_ri(W(RG), W(IS))

#define divyn_rr(RG, RS)                 /* RG, RS no Reax, RS no Redx */   \
        divzn_rr(W(RG), W(RS))

#define divyn_ld(RG, MS, DS)   /* Reax cannot be used as first operand */   \
        divzn_ld(W(RG), W(MS), W(DS))


#define preyx_xx()   /* to be placed right before divyx_x* or remyx_xx */   \
        prezx_xx()                   /* to prepare Redx for int-divide */

#define preyn_xx()   /* to be placed right before divyn_x* or remyn_xx */   \
        prezn_xx()                   /* to prepare Redx for int-divide */


#define divyx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        divzx_xr(W(RS))

#define divyx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        divzx_xm(W(MS), W(DS))


#define divyn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divzn_xr(W(RS))

#define divyn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divzn_xm(W(MS), W(DS))


#define divyp_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divzp_xr(W(RS))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divyp_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divzp_xm(W(MS), W(DS))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem (G = G % S)
 * set-flags: undefined */

#define remyx_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        remzx_ri(W(RG), W(IS))

#define remyx_rr(RG, RS)                 /* RG, RS no Redx, RS no Reax */   \
        remzx_rr(W(RG), W(RS))

#define remyx_ld(RG, MS, DS)   /* Redx cannot be used as first operand */   \
        remzx_ld(W(RG), W(MS), W(DS))


#define remyn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        remzn_ri(W(RG), W(IS))

#define remyn_rr(RG, RS)                 /* RG, RS no Redx, RS no Reax */   \
        remzn_rr(W(RG), W(RS))

#define remyn_ld(RG, MS, DS)   /* Redx cannot be used as first operand */   \
        remzn_ld(W(RG), W(MS), W(DS))


#define remyx_xx() /* to be placed before divyx_x*, but after preyx_xx */   \
        remzx_xx()                   /* to prepare for rem calculation */

#define remyx_xr(RS)        /* to be placed immediately after divyx_xr */   \
        remzx_xr(W(RS))              /* to produce remainder Redx<-rem */

#define remyx_xm(MS, DS)    /* to be placed immediately after divyx_xm */   \
        remzx_xm(W(MS), W(DS))       /* to produce remainder Redx<-rem */


#define remyn_xx() /* to be placed before divyn_x*, but after preyn_xx */   \
        remzn_xx()                   /* to prepare for rem calculation */

#define remyn_xr(RS)        /* to be placed immediately after divyn_xr */   \
        remzn_xr(W(RS))              /* to produce remainder Redx<-rem */

#define remyn_xm(MS, DS)    /* to be placed immediately after divyn_xm */   \
        remzn_xm(W(MS), W(DS))       /* to produce remainder Redx<-rem */

/* arj (G = G op S, if cc G then jump lb)
 * set-flags: undefined
 * refer to individual instruction descriptions
 * to stay within special register limitations */

     /* Definitions for arj's "op" and "cc" parameters
      * are provided in 32-bit rtarch_***.h files. */

#define arjyx_rx(RG, op, cc, lb)                                            \
        arjzx_rx(W(RG), op, cc, lb)

#define arjyx_mx(MG, DG, op, cc, lb)                                        \
        arjzx_mx(W(MG), W(DG), op, cc, lb)

#define arjyx_ri(RG, IS, op, cc, lb)                                        \
        arjzx_ri(W(RG), W(IS), op, cc, lb)

#define arjyx_mi(MG, DG, IS, op, cc, lb)                                    \
        arjzx_mi(W(MG), W(DG), W(IS), op, cc, lb)

#define arjyx_rr(RG, RS, op, cc, lb)                                        \
        arjzx_rr(W(RG), W(RS), op, cc, lb)

#define arjyx_ld(RG, MS, DS, op, cc, lb)                                    \
        arjzx_ld(W(RG), W(MS), W(DS), op, cc, lb)

#define arjyx_st(RS, MG, DG, op, cc, lb)                                    \
        arjzx_st(W(RS), W(MG), W(DG), op, cc, lb)

#define arjyx_mr(MG, DG, RS, op, cc, lb)                                    \
        arjzx_mr(W(MG), W(DG), W(RS), op, cc, lb)

/* cmj (flags = S ? T, if cc flags then jump lb)
 * set-flags: undefined */

     /* Definitions for cmj's "cc" parameter
      * are provided in 32-bit rtarch_***.h files. */

#define cmjyx_rz(RS, cc, lb)                                                \
        cmjzx_rz(W(RS), cc, lb)

#define cmjyx_mz(MS, DS, cc, lb)                                            \
        cmjzx_mz(W(MS), W(DS), cc, lb)

#define cmjyx_ri(RS, IT, cc, lb)                                            \
        cmjzx_ri(W(RS), W(IT), cc, lb)

#define cmjyx_mi(MS, DS, IT, cc, lb)                                        \
        cmjzx_mi(W(MS), W(DS), W(IT), cc, lb)

#define cmjyx_rr(RS, RT, cc, lb)                                            \
        cmjzx_rr(W(RS), W(RT), cc, lb)

#define cmjyx_rm(RS, MT, DT, cc, lb)                                        \
        cmjzx_rm(W(RS), W(MT), W(DT), cc, lb)

#define cmjyx_mr(MS, DS, RT, cc, lb)                                        \
        cmjzx_mr(W(MS), W(DS), W(RT), cc, lb)

/* cmp (flags = S ? T)
 * set-flags: yes */

#define cmpyx_ri(RS, IT)                                                    \
        cmpzx_ri(W(RS), W(IT))

#define cmpyx_mi(MS, DS, IT)                                                \
        cmpzx_mi(W(MS), W(DS), W(IT))

#define cmpyx_rr(RS, RT)                                                    \
        cmpzx_rr(W(RS), W(RT))

#define cmpyx_rm(RS, MT, DT)                                                \
        cmpzx_rm(W(RS), W(MT), W(DT))

#define cmpyx_mr(MS, DS, RT)                                                \
        cmpzx_mr(W(MS), W(DS), W(RT))

#endif /* RT_ELEMENT */

#endif /* RT_RTCONF_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

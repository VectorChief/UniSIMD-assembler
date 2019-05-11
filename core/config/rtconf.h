/******************************************************************************/
/* Copyright (c) 2013-2019 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTCONF_H
#define RT_RTCONF_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtconf.h: Configuration file for instruction subsets mapping.
 * The table of contents is provided below.
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
 * cmdi*_** - applies [cmd] to 32-bit SIMD element args, packed-128-bit
 * cmdj*_** - applies [cmd] to 64-bit SIMD element args, packed-128-bit
 * cmdl*_** - applies [cmd] to L-size SIMD element args, packed-128-bit
 *
 * cmdc*_** - applies [cmd] to 32-bit SIMD element args, packed-256-bit
 * cmdd*_** - applies [cmd] to 64-bit SIMD element args, packed-256-bit
 * cmdf*_** - applies [cmd] to L-size SIMD element args, packed-256-bit
 *
 * cmdo*_** - applies [cmd] to 32-bit SIMD element args, packed-var-len
 * cmdp*_** - applies [cmd] to L-size SIMD element args, packed-var-len
 * cmdq*_** - applies [cmd] to 64-bit SIMD element args, packed-var-len
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
 * should propagate QNaNs unchanged, however this behavior hasn't been verified.
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

/* and (G = G & S), (D = S & T) if (#D != #S) */

#define andmx_rr(XG, XS)                                                    \
        andax_rr(W(XG), W(XS))

#define andmx_ld(XG, MS, DS)                                                \
        andax_ld(W(XG), W(MS), W(DS))

#define andmx3rr(XD, XS, XT)                                                \
        andax3rr(W(XD), W(XS), W(XT))

#define andmx3ld(XD, XS, MT, DT)                                            \
        andax3ld(W(XD), W(XS), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #S) */

#define annmx_rr(XG, XS)                                                    \
        annax_rr(W(XG), W(XS))

#define annmx_ld(XG, MS, DS)                                                \
        annax_ld(W(XG), W(MS), W(DS))

#define annmx3rr(XD, XS, XT)                                                \
        annax3rr(W(XD), W(XS), W(XT))

#define annmx3ld(XD, XS, MT, DT)                                            \
        annax3ld(W(XD), W(XS), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #S) */

#define orrmx_rr(XG, XS)                                                    \
        orrax_rr(W(XG), W(XS))

#define orrmx_ld(XG, MS, DS)                                                \
        orrax_ld(W(XG), W(MS), W(DS))

#define orrmx3rr(XD, XS, XT)                                                \
        orrax3rr(W(XD), W(XS), W(XT))

#define orrmx3ld(XD, XS, MT, DT)                                            \
        orrax3ld(W(XD), W(XS), W(MT), W(DT))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #S) */

#define ornmx_rr(XG, XS)                                                    \
        ornax_rr(W(XG), W(XS))

#define ornmx_ld(XG, MS, DS)                                                \
        ornax_ld(W(XG), W(MS), W(DS))

#define ornmx3rr(XD, XS, XT)                                                \
        ornax3rr(W(XD), W(XS), W(XT))

#define ornmx3ld(XD, XS, MT, DT)                                            \
        ornax3ld(W(XD), W(XS), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #S) */

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

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addms_rr(XG, XS)                                                    \
        addas_rr(W(XG), W(XS))

#define addms_ld(XG, MS, DS)                                                \
        addas_ld(W(XG), W(MS), W(DS))

#define addms3rr(XD, XS, XT)                                                \
        addas3rr(W(XD), W(XS), W(XT))

#define addms3ld(XD, XS, MT, DT)                                            \
        addas3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subms_rr(XG, XS)                                                    \
        subas_rr(W(XG), W(XS))

#define subms_ld(XG, MS, DS)                                                \
        subas_ld(W(XG), W(MS), W(DS))

#define subms3rr(XD, XS, XT)                                                \
        subas3rr(W(XD), W(XS), W(XT))

#define subms3ld(XD, XS, MT, DT)                                            \
        subas3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulms_rr(XG, XS)                                                    \
        mulas_rr(W(XG), W(XS))

#define mulms_ld(XG, MS, DS)                                                \
        mulas_ld(W(XG), W(MS), W(DS))

#define mulms3rr(XD, XS, XT)                                                \
        mulas3rr(W(XD), W(XS), W(XT))

#define mulms3ld(XD, XS, MT, DT)                                            \
        mulas3ld(W(XD), W(XS), W(MT), W(DT))

/* div (G = G / S), (D = S / T) if (#D != #S) */

#define divms_rr(XG, XS)                                                    \
        divas_rr(W(XG), W(XS))

#define divms_ld(XG, MS, DS)                                                \
        divas_ld(W(XG), W(MS), W(DS))

#define divms3rr(XD, XS, XT)                                                \
        divas3rr(W(XD), W(XS), W(XT))

#define divms3ld(XD, XS, MT, DT)                                            \
        divas3ld(W(XD), W(XS), W(MT), W(DT))

/* sqr (D = sqrt S) */

#define sqrms_rr(XD, XS)                                                    \
        sqras_rr(W(XD), W(XS))

#define sqrms_ld(XD, MS, DS)                                                \
        sqras_ld(W(XD), W(MS), W(DS))

/* rcp (D = 1.0 / S) */

#define rcems_rr(XD, XS)                                                    \
        rceas_rr(W(XD), W(XS))

#define rcsms_rr(XG, XS) /* destroys XS */                                  \
        rcsas_rr(W(XG), W(XS))

/* rsq (D = 1.0 / sqrt S) */

#define rsems_rr(XD, XS)                                                    \
        rseas_rr(W(XD), W(XS))

#define rssms_rr(XG, XS) /* destroys XS */                                  \
        rssas_rr(W(XG), W(XS))

/* fma (G = G + S * T) if (#G != #S && #G != #T) */

#define fmams_rr(XG, XS, XT)                                                \
        fmaas_rr(W(XG), W(XS), W(XT))

#define fmams_ld(XG, XS, MT, DT)                                            \
        fmaas_ld(W(XG), W(XS), W(MT), W(DT))

/* fms (G = G - S * T) if (#G != #S && #G != #T) */

#define fmsms_rr(XG, XS, XT)                                                \
        fmsas_rr(W(XG), W(XS), W(XT))

#define fmsms_ld(XG, XS, MT, DT)                                            \
        fmsas_ld(W(XG), W(XS), W(MT), W(DT))

/**************   packed half-precision floating-point compare   **************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

#define minms_rr(XG, XS)                                                    \
        minas_rr(W(XG), W(XS))

#define minms_ld(XG, MS, DS)                                                \
        minas_ld(W(XG), W(MS), W(DS))

#define minms3rr(XD, XS, XT)                                                \
        minas3rr(W(XD), W(XS), W(XT))

#define minms3ld(XD, XS, MT, DT)                                            \
        minas3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

#define maxms_rr(XG, XS)                                                    \
        maxas_rr(W(XG), W(XS))

#define maxms_ld(XG, MS, DS)                                                \
        maxas_ld(W(XG), W(MS), W(DS))

#define maxms3rr(XD, XS, XT)                                                \
        maxas3rr(W(XD), W(XS), W(XT))

#define maxms3ld(XD, XS, MT, DT)                                            \
        maxas3ld(W(XD), W(XS), W(MT), W(DT))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqms_rr(XG, XS)                                                    \
        ceqas_rr(W(XG), W(XS))

#define ceqms_ld(XG, MS, DS)                                                \
        ceqas_ld(W(XG), W(MS), W(DS))

#define ceqms3rr(XD, XS, XT)                                                \
        ceqas3rr(W(XD), W(XS), W(XT))

#define ceqms3ld(XD, XS, MT, DT)                                            \
        ceqas3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cnems_rr(XG, XS)                                                    \
        cneas_rr(W(XG), W(XS))

#define cnems_ld(XG, MS, DS)                                                \
        cneas_ld(W(XG), W(MS), W(DS))

#define cnems3rr(XD, XS, XT)                                                \
        cneas3rr(W(XD), W(XS), W(XT))

#define cnems3ld(XD, XS, MT, DT)                                            \
        cneas3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltms_rr(XG, XS)                                                    \
        cltas_rr(W(XG), W(XS))

#define cltms_ld(XG, MS, DS)                                                \
        cltas_ld(W(XG), W(MS), W(DS))

#define cltms3rr(XD, XS, XT)                                                \
        cltas3rr(W(XD), W(XS), W(XT))

#define cltms3ld(XD, XS, MT, DT)                                            \
        cltas3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define clems_rr(XG, XS)                                                    \
        cleas_rr(W(XG), W(XS))

#define clems_ld(XG, MS, DS)                                                \
        cleas_ld(W(XG), W(MS), W(DS))

#define clems3rr(XD, XS, XT)                                                \
        cleas3rr(W(XD), W(XS), W(XT))

#define clems3ld(XD, XS, MT, DT)                                            \
        cleas3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtms_rr(XG, XS)                                                    \
        cgtas_rr(W(XG), W(XS))

#define cgtms_ld(XG, MS, DS)                                                \
        cgtas_ld(W(XG), W(MS), W(DS))

#define cgtms3rr(XD, XS, XT)                                                \
        cgtas3rr(W(XD), W(XS), W(XT))

#define cgtms3ld(XD, XS, MT, DT)                                            \
        cgtas3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

#define cgems_rr(XG, XS)                                                    \
        cgeas_rr(W(XG), W(XS))

#define cgems_ld(XG, MS, DS)                                                \
        cgeas_ld(W(XG), W(MS), W(DS))

#define cgems3rr(XD, XS, XT)                                                \
        cgeas3rr(W(XD), W(XS), W(XT))

#define cgems3ld(XD, XS, MT, DT)                                            \
        cgeas3ld(W(XD), W(XS), W(MT), W(DT))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define mkjmx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        mkjax_rx(W(XS), mask, lb)

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

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnrms_rr(XD, XS, mode)                                              \
        rnras_rr(W(XD), W(XS), mode)

#define cvrms_rr(XD, XS, mode)                                              \
        cvras_rr(W(XD), W(XS), mode)

/*************   packed half-precision integer arithmetic/shifts   ************/

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addmx_rr(XG, XS)                                                    \
        addax_rr(W(XG), W(XS))

#define addmx_ld(XG, MS, DS)                                                \
        addax_ld(W(XG), W(MS), W(DS))

#define addmx3rr(XD, XS, XT)                                                \
        addax3rr(W(XD), W(XS), W(XT))

#define addmx3ld(XD, XS, MT, DT)                                            \
        addax3ld(W(XD), W(XS), W(MT), W(DT))

/* ads (G = G + S), (D = S + T) if (#D != #S) - saturate, unsigned */

#define adsmx_rr(XG, XS)                                                    \
        adsax_rr(W(XG), W(XS))

#define adsmx_ld(XG, MS, DS)                                                \
        adsax_ld(W(XG), W(MS), W(DS))

#define adsmx3rr(XD, XS, XT)                                                \
        adsax3rr(W(XD), W(XS), W(XT))

#define adsmx3ld(XD, XS, MT, DT)                                            \
        adsax3ld(W(XD), W(XS), W(MT), W(DT))

/* ads (G = G + S), (D = S + T) if (#D != #S) - saturate, signed */

#define adsmn_rr(XG, XS)                                                    \
        adsan_rr(W(XG), W(XS))

#define adsmn_ld(XG, MS, DS)                                                \
        adsan_ld(W(XG), W(MS), W(DS))

#define adsmn3rr(XD, XS, XT)                                                \
        adsan3rr(W(XD), W(XS), W(XT))

#define adsmn3ld(XD, XS, MT, DT)                                            \
        adsan3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define submx_rr(XG, XS)                                                    \
        subax_rr(W(XG), W(XS))

#define submx_ld(XG, MS, DS)                                                \
        subax_ld(W(XG), W(MS), W(DS))

#define submx3rr(XD, XS, XT)                                                \
        subax3rr(W(XD), W(XS), W(XT))

#define submx3ld(XD, XS, MT, DT)                                            \
        subax3ld(W(XD), W(XS), W(MT), W(DT))

/* sbs (G = G - S), (D = S - T) if (#D != #S) - saturate, unsigned */

#define sbsmx_rr(XG, XS)                                                    \
        sbsax_rr(W(XG), W(XS))

#define sbsmx_ld(XG, MS, DS)                                                \
        sbsax_ld(W(XG), W(MS), W(DS))

#define sbsmx3rr(XD, XS, XT)                                                \
        sbsax3rr(W(XD), W(XS), W(XT))

#define sbsmx3ld(XD, XS, MT, DT)                                            \
        sbsax3ld(W(XD), W(XS), W(MT), W(DT))

/* sbs (G = G - S), (D = S - T) if (#D != #S) - saturate, signed */

#define sbsmn_rr(XG, XS)                                                    \
        sbsan_rr(W(XG), W(XS))

#define sbsmn_ld(XG, MS, DS)                                                \
        sbsan_ld(W(XG), W(MS), W(DS))

#define sbsmn3rr(XD, XS, XT)                                                \
        sbsan3rr(W(XD), W(XS), W(XT))

#define sbsmn3ld(XD, XS, MT, DT)                                            \
        sbsan3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulmx_rr(XG, XS)                                                    \
        mulax_rr(W(XG), W(XS))

#define mulmx_ld(XG, MS, DS)                                                \
        mulax_ld(W(XG), W(MS), W(DS))

#define mulmx3rr(XD, XS, XT)                                                \
        mulax3rr(W(XD), W(XS), W(XT))

#define mulmx3ld(XD, XS, MT, DT)                                            \
        mulax3ld(W(XD), W(XS), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlmx_ri(XG, IS)                                                    \
        shlax_ri(W(XG), W(IS))

#define shlmx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlax_ld(W(XG), W(MS), W(DS))

#define shlmx3ri(XD, XS, IT)                                                \
        shlax3ri(W(XD), W(XS), W(IT))

#define shlmx3ld(XD, XS, MT, DT)                                            \
        shlax3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmx_ri(XG, IS)                                                    \
        shrax_ri(W(XG), W(IS))

#define shrmx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrax_ld(W(XG), W(MS), W(DS))

#define shrmx3ri(XD, XS, IT)                                                \
        shrax3ri(W(XD), W(XS), W(IT))

#define shrmx3ld(XD, XS, MT, DT)                                            \
        shrax3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmn_ri(XG, IS)                                                    \
        shran_ri(W(XG), W(IS))

#define shrmn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shran_ld(W(XG), W(MS), W(DS))

#define shrmn3ri(XD, XS, IT)                                                \
        shran3ri(W(XD), W(XS), W(IT))

#define shrmn3ld(XD, XS, MT, DT)                                            \
        shran3ld(W(XD), W(XS), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlmx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlax_rr(W(XG), W(XS))

#define svlmx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlax_ld(W(XG), W(MS), W(DS))

#define svlmx3rr(XD, XS, XT)                                                \
        svlax3rr(W(XD), W(XS), W(XT))

#define svlmx3ld(XD, XS, MT, DT)                                            \
        svlax3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrax_rr(W(XG), W(XS))

#define svrmx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrax_ld(W(XG), W(MS), W(DS))

#define svrmx3rr(XD, XS, XT)                                                \
        svrax3rr(W(XD), W(XS), W(XT))

#define svrmx3ld(XD, XS, MT, DT)                                            \
        svrax3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svran_rr(W(XG), W(XS))

#define svrmn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svran_ld(W(XG), W(MS), W(DS))

#define svrmn3rr(XD, XS, XT)                                                \
        svran3rr(W(XD), W(XS), W(XT))

#define svrmn3ld(XD, XS, MT, DT)                                            \
        svran3ld(W(XD), W(XS), W(MT), W(DT))

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

/* and (G = G & S), (D = S & T) if (#D != #S) */

#define andmx_rr(XG, XS)                                                    \
        andgx_rr(W(XG), W(XS))

#define andmx_ld(XG, MS, DS)                                                \
        andgx_ld(W(XG), W(MS), W(DS))

#define andmx3rr(XD, XS, XT)                                                \
        andgx3rr(W(XD), W(XS), W(XT))

#define andmx3ld(XD, XS, MT, DT)                                            \
        andgx3ld(W(XD), W(XS), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #S) */

#define annmx_rr(XG, XS)                                                    \
        anngx_rr(W(XG), W(XS))

#define annmx_ld(XG, MS, DS)                                                \
        anngx_ld(W(XG), W(MS), W(DS))

#define annmx3rr(XD, XS, XT)                                                \
        anngx3rr(W(XD), W(XS), W(XT))

#define annmx3ld(XD, XS, MT, DT)                                            \
        anngx3ld(W(XD), W(XS), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #S) */

#define orrmx_rr(XG, XS)                                                    \
        orrgx_rr(W(XG), W(XS))

#define orrmx_ld(XG, MS, DS)                                                \
        orrgx_ld(W(XG), W(MS), W(DS))

#define orrmx3rr(XD, XS, XT)                                                \
        orrgx3rr(W(XD), W(XS), W(XT))

#define orrmx3ld(XD, XS, MT, DT)                                            \
        orrgx3ld(W(XD), W(XS), W(MT), W(DT))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #S) */

#define ornmx_rr(XG, XS)                                                    \
        orngx_rr(W(XG), W(XS))

#define ornmx_ld(XG, MS, DS)                                                \
        orngx_ld(W(XG), W(MS), W(DS))

#define ornmx3rr(XD, XS, XT)                                                \
        orngx3rr(W(XD), W(XS), W(XT))

#define ornmx3ld(XD, XS, MT, DT)                                            \
        orngx3ld(W(XD), W(XS), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #S) */

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

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addms_rr(XG, XS)                                                    \
        addgs_rr(W(XG), W(XS))

#define addms_ld(XG, MS, DS)                                                \
        addgs_ld(W(XG), W(MS), W(DS))

#define addms3rr(XD, XS, XT)                                                \
        addgs3rr(W(XD), W(XS), W(XT))

#define addms3ld(XD, XS, MT, DT)                                            \
        addgs3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subms_rr(XG, XS)                                                    \
        subgs_rr(W(XG), W(XS))

#define subms_ld(XG, MS, DS)                                                \
        subgs_ld(W(XG), W(MS), W(DS))

#define subms3rr(XD, XS, XT)                                                \
        subgs3rr(W(XD), W(XS), W(XT))

#define subms3ld(XD, XS, MT, DT)                                            \
        subgs3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulms_rr(XG, XS)                                                    \
        mulgs_rr(W(XG), W(XS))

#define mulms_ld(XG, MS, DS)                                                \
        mulgs_ld(W(XG), W(MS), W(DS))

#define mulms3rr(XD, XS, XT)                                                \
        mulgs3rr(W(XD), W(XS), W(XT))

#define mulms3ld(XD, XS, MT, DT)                                            \
        mulgs3ld(W(XD), W(XS), W(MT), W(DT))

/* div (G = G / S), (D = S / T) if (#D != #S) */

#define divms_rr(XG, XS)                                                    \
        divgs_rr(W(XG), W(XS))

#define divms_ld(XG, MS, DS)                                                \
        divgs_ld(W(XG), W(MS), W(DS))

#define divms3rr(XD, XS, XT)                                                \
        divgs3rr(W(XD), W(XS), W(XT))

#define divms3ld(XD, XS, MT, DT)                                            \
        divgs3ld(W(XD), W(XS), W(MT), W(DT))

/* sqr (D = sqrt S) */

#define sqrms_rr(XD, XS)                                                    \
        sqrgs_rr(W(XD), W(XS))

#define sqrms_ld(XD, MS, DS)                                                \
        sqrgs_ld(W(XD), W(MS), W(DS))

/* rcp (D = 1.0 / S) */

#define rcems_rr(XD, XS)                                                    \
        rcegs_rr(W(XD), W(XS))

#define rcsms_rr(XG, XS) /* destroys XS */                                  \
        rcsgs_rr(W(XG), W(XS))

/* rsq (D = 1.0 / sqrt S) */

#define rsems_rr(XD, XS)                                                    \
        rsegs_rr(W(XD), W(XS))

#define rssms_rr(XG, XS) /* destroys XS */                                  \
        rssgs_rr(W(XG), W(XS))

/* fma (G = G + S * T) if (#G != #S && #G != #T) */

#define fmams_rr(XG, XS, XT)                                                \
        fmags_rr(W(XG), W(XS), W(XT))

#define fmams_ld(XG, XS, MT, DT)                                            \
        fmags_ld(W(XG), W(XS), W(MT), W(DT))

/* fms (G = G - S * T) if (#G != #S && #G != #T) */

#define fmsms_rr(XG, XS, XT)                                                \
        fmsgs_rr(W(XG), W(XS), W(XT))

#define fmsms_ld(XG, XS, MT, DT)                                            \
        fmsgs_ld(W(XG), W(XS), W(MT), W(DT))

/**************   packed half-precision floating-point compare   **************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

#define minms_rr(XG, XS)                                                    \
        mings_rr(W(XG), W(XS))

#define minms_ld(XG, MS, DS)                                                \
        mings_ld(W(XG), W(MS), W(DS))

#define minms3rr(XD, XS, XT)                                                \
        mings3rr(W(XD), W(XS), W(XT))

#define minms3ld(XD, XS, MT, DT)                                            \
        mings3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

#define maxms_rr(XG, XS)                                                    \
        maxgs_rr(W(XG), W(XS))

#define maxms_ld(XG, MS, DS)                                                \
        maxgs_ld(W(XG), W(MS), W(DS))

#define maxms3rr(XD, XS, XT)                                                \
        maxgs3rr(W(XD), W(XS), W(XT))

#define maxms3ld(XD, XS, MT, DT)                                            \
        maxgs3ld(W(XD), W(XS), W(MT), W(DT))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqms_rr(XG, XS)                                                    \
        ceqgs_rr(W(XG), W(XS))

#define ceqms_ld(XG, MS, DS)                                                \
        ceqgs_ld(W(XG), W(MS), W(DS))

#define ceqms3rr(XD, XS, XT)                                                \
        ceqgs3rr(W(XD), W(XS), W(XT))

#define ceqms3ld(XD, XS, MT, DT)                                            \
        ceqgs3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cnems_rr(XG, XS)                                                    \
        cnegs_rr(W(XG), W(XS))

#define cnems_ld(XG, MS, DS)                                                \
        cnegs_ld(W(XG), W(MS), W(DS))

#define cnems3rr(XD, XS, XT)                                                \
        cnegs3rr(W(XD), W(XS), W(XT))

#define cnems3ld(XD, XS, MT, DT)                                            \
        cnegs3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltms_rr(XG, XS)                                                    \
        cltgs_rr(W(XG), W(XS))

#define cltms_ld(XG, MS, DS)                                                \
        cltgs_ld(W(XG), W(MS), W(DS))

#define cltms3rr(XD, XS, XT)                                                \
        cltgs3rr(W(XD), W(XS), W(XT))

#define cltms3ld(XD, XS, MT, DT)                                            \
        cltgs3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define clems_rr(XG, XS)                                                    \
        clegs_rr(W(XG), W(XS))

#define clems_ld(XG, MS, DS)                                                \
        clegs_ld(W(XG), W(MS), W(DS))

#define clems3rr(XD, XS, XT)                                                \
        clegs3rr(W(XD), W(XS), W(XT))

#define clems3ld(XD, XS, MT, DT)                                            \
        clegs3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtms_rr(XG, XS)                                                    \
        cgtgs_rr(W(XG), W(XS))

#define cgtms_ld(XG, MS, DS)                                                \
        cgtgs_ld(W(XG), W(MS), W(DS))

#define cgtms3rr(XD, XS, XT)                                                \
        cgtgs3rr(W(XD), W(XS), W(XT))

#define cgtms3ld(XD, XS, MT, DT)                                            \
        cgtgs3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

#define cgems_rr(XG, XS)                                                    \
        cgegs_rr(W(XG), W(XS))

#define cgems_ld(XG, MS, DS)                                                \
        cgegs_ld(W(XG), W(MS), W(DS))

#define cgems3rr(XD, XS, XT)                                                \
        cgegs3rr(W(XD), W(XS), W(XT))

#define cgems3ld(XD, XS, MT, DT)                                            \
        cgegs3ld(W(XD), W(XS), W(MT), W(DT))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define mkjmx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        mkjgx_rx(W(XS), mask, lb)

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

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnrms_rr(XD, XS, mode)                                              \
        rnrgs_rr(W(XD), W(XS), mode)

#define cvrms_rr(XD, XS, mode)                                              \
        cvrgs_rr(W(XD), W(XS), mode)

/*************   packed half-precision integer arithmetic/shifts   ************/

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addmx_rr(XG, XS)                                                    \
        addgx_rr(W(XG), W(XS))

#define addmx_ld(XG, MS, DS)                                                \
        addgx_ld(W(XG), W(MS), W(DS))

#define addmx3rr(XD, XS, XT)                                                \
        addgx3rr(W(XD), W(XS), W(XT))

#define addmx3ld(XD, XS, MT, DT)                                            \
        addgx3ld(W(XD), W(XS), W(MT), W(DT))

/* ads (G = G + S), (D = S + T) if (#D != #S) - saturate, unsigned */

#define adsmx_rr(XG, XS)                                                    \
        adsgx_rr(W(XG), W(XS))

#define adsmx_ld(XG, MS, DS)                                                \
        adsgx_ld(W(XG), W(MS), W(DS))

#define adsmx3rr(XD, XS, XT)                                                \
        adsgx3rr(W(XD), W(XS), W(XT))

#define adsmx3ld(XD, XS, MT, DT)                                            \
        adsgx3ld(W(XD), W(XS), W(MT), W(DT))

/* ads (G = G + S), (D = S + T) if (#D != #S) - saturate, signed */

#define adsmn_rr(XG, XS)                                                    \
        adsgn_rr(W(XG), W(XS))

#define adsmn_ld(XG, MS, DS)                                                \
        adsgn_ld(W(XG), W(MS), W(DS))

#define adsmn3rr(XD, XS, XT)                                                \
        adsgn3rr(W(XD), W(XS), W(XT))

#define adsmn3ld(XD, XS, MT, DT)                                            \
        adsgn3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define submx_rr(XG, XS)                                                    \
        subgx_rr(W(XG), W(XS))

#define submx_ld(XG, MS, DS)                                                \
        subgx_ld(W(XG), W(MS), W(DS))

#define submx3rr(XD, XS, XT)                                                \
        subgx3rr(W(XD), W(XS), W(XT))

#define submx3ld(XD, XS, MT, DT)                                            \
        subgx3ld(W(XD), W(XS), W(MT), W(DT))

/* sbs (G = G - S), (D = S - T) if (#D != #S) - saturate, unsigned */

#define sbsmx_rr(XG, XS)                                                    \
        sbsgx_rr(W(XG), W(XS))

#define sbsmx_ld(XG, MS, DS)                                                \
        sbsgx_ld(W(XG), W(MS), W(DS))

#define sbsmx3rr(XD, XS, XT)                                                \
        sbsgx3rr(W(XD), W(XS), W(XT))

#define sbsmx3ld(XD, XS, MT, DT)                                            \
        sbsgx3ld(W(XD), W(XS), W(MT), W(DT))

/* sbs (G = G - S), (D = S - T) if (#D != #S) - saturate, signed */

#define sbsmn_rr(XG, XS)                                                    \
        sbsgn_rr(W(XG), W(XS))

#define sbsmn_ld(XG, MS, DS)                                                \
        sbsgn_ld(W(XG), W(MS), W(DS))

#define sbsmn3rr(XD, XS, XT)                                                \
        sbsgn3rr(W(XD), W(XS), W(XT))

#define sbsmn3ld(XD, XS, MT, DT)                                            \
        sbsgn3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulmx_rr(XG, XS)                                                    \
        mulgx_rr(W(XG), W(XS))

#define mulmx_ld(XG, MS, DS)                                                \
        mulgx_ld(W(XG), W(MS), W(DS))

#define mulmx3rr(XD, XS, XT)                                                \
        mulgx3rr(W(XD), W(XS), W(XT))

#define mulmx3ld(XD, XS, MT, DT)                                            \
        mulgx3ld(W(XD), W(XS), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlmx_ri(XG, IS)                                                    \
        shlgx_ri(W(XG), W(IS))

#define shlmx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlgx_ld(W(XG), W(MS), W(DS))

#define shlmx3ri(XD, XS, IT)                                                \
        shlgx3ri(W(XD), W(XS), W(IT))

#define shlmx3ld(XD, XS, MT, DT)                                            \
        shlgx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmx_ri(XG, IS)                                                    \
        shrgx_ri(W(XG), W(IS))

#define shrmx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrgx_ld(W(XG), W(MS), W(DS))

#define shrmx3ri(XD, XS, IT)                                                \
        shrgx3ri(W(XD), W(XS), W(IT))

#define shrmx3ld(XD, XS, MT, DT)                                            \
        shrgx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmn_ri(XG, IS)                                                    \
        shrgn_ri(W(XG), W(IS))

#define shrmn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrgn_ld(W(XG), W(MS), W(DS))

#define shrmn3ri(XD, XS, IT)                                                \
        shrgn3ri(W(XD), W(XS), W(IT))

#define shrmn3ld(XD, XS, MT, DT)                                            \
        shrgn3ld(W(XD), W(XS), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlmx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlgx_rr(W(XG), W(XS))

#define svlmx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlgx_ld(W(XG), W(MS), W(DS))

#define svlmx3rr(XD, XS, XT)                                                \
        svlgx3rr(W(XD), W(XS), W(XT))

#define svlmx3ld(XD, XS, MT, DT)                                            \
        svlgx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgx_rr(W(XG), W(XS))

#define svrmx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgx_ld(W(XG), W(MS), W(DS))

#define svrmx3rr(XD, XS, XT)                                                \
        svrgx3rr(W(XD), W(XS), W(XT))

#define svrmx3ld(XD, XS, MT, DT)                                            \
        svrgx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgn_rr(W(XG), W(XS))

#define svrmn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgn_ld(W(XG), W(MS), W(DS))

#define svrmn3rr(XD, XS, XT)                                                \
        svrgn3rr(W(XD), W(XS), W(XT))

#define svrmn3ld(XD, XS, MT, DT)                                            \
        svrgn3ld(W(XD), W(XS), W(MT), W(DT))

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

/* and (G = G & S), (D = S & T) if (#D != #S) */

#define andox_rr(XG, XS)                                                    \
        andcx_rr(W(XG), W(XS))

#define andox_ld(XG, MS, DS)                                                \
        andcx_ld(W(XG), W(MS), W(DS))

#define andox3rr(XD, XS, XT)                                                \
        andcx3rr(W(XD), W(XS), W(XT))

#define andox3ld(XD, XS, MT, DT)                                            \
        andcx3ld(W(XD), W(XS), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #S) */

#define annox_rr(XG, XS)                                                    \
        anncx_rr(W(XG), W(XS))

#define annox_ld(XG, MS, DS)                                                \
        anncx_ld(W(XG), W(MS), W(DS))

#define annox3rr(XD, XS, XT)                                                \
        anncx3rr(W(XD), W(XS), W(XT))

#define annox3ld(XD, XS, MT, DT)                                            \
        anncx3ld(W(XD), W(XS), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #S) */

#define orrox_rr(XG, XS)                                                    \
        orrcx_rr(W(XG), W(XS))

#define orrox_ld(XG, MS, DS)                                                \
        orrcx_ld(W(XG), W(MS), W(DS))

#define orrox3rr(XD, XS, XT)                                                \
        orrcx3rr(W(XD), W(XS), W(XT))

#define orrox3ld(XD, XS, MT, DT)                                            \
        orrcx3ld(W(XD), W(XS), W(MT), W(DT))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #S) */

#define ornox_rr(XG, XS)                                                    \
        orncx_rr(W(XG), W(XS))

#define ornox_ld(XG, MS, DS)                                                \
        orncx_ld(W(XG), W(MS), W(DS))

#define ornox3rr(XD, XS, XT)                                                \
        orncx3rr(W(XD), W(XS), W(XT))

#define ornox3ld(XD, XS, MT, DT)                                            \
        orncx3ld(W(XD), W(XS), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #S) */

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

/* add (G = G + S), (D = S + T) if (#D != #S) */

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

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subos_rr(XG, XS)                                                    \
        subcs_rr(W(XG), W(XS))

#define subos_ld(XG, MS, DS)                                                \
        subcs_ld(W(XG), W(MS), W(DS))

#define subos3rr(XD, XS, XT)                                                \
        subcs3rr(W(XD), W(XS), W(XT))

#define subos3ld(XD, XS, MT, DT)                                            \
        subcs3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

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

/* div (G = G / S), (D = S / T) if (#D != #S) */

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

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsos_rr(XG, XS, XT)                                                \
        fmscs_rr(W(XG), W(XS), W(XT))

#define fmsos_ld(XG, XS, MT, DT)                                            \
        fmscs_ld(W(XG), W(XS), W(MT), W(DT))

/*************   packed single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

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

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

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

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqos_rr(XG, XS)                                                    \
        ceqcs_rr(W(XG), W(XS))

#define ceqos_ld(XG, MS, DS)                                                \
        ceqcs_ld(W(XG), W(MS), W(DS))

#define ceqos3rr(XD, XS, XT)                                                \
        ceqcs3rr(W(XD), W(XS), W(XT))

#define ceqos3ld(XD, XS, MT, DT)                                            \
        ceqcs3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cneos_rr(XG, XS)                                                    \
        cnecs_rr(W(XG), W(XS))

#define cneos_ld(XG, MS, DS)                                                \
        cnecs_ld(W(XG), W(MS), W(DS))

#define cneos3rr(XD, XS, XT)                                                \
        cnecs3rr(W(XD), W(XS), W(XT))

#define cneos3ld(XD, XS, MT, DT)                                            \
        cnecs3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltos_rr(XG, XS)                                                    \
        cltcs_rr(W(XG), W(XS))

#define cltos_ld(XG, MS, DS)                                                \
        cltcs_ld(W(XG), W(MS), W(DS))

#define cltos3rr(XD, XS, XT)                                                \
        cltcs3rr(W(XD), W(XS), W(XT))

#define cltos3ld(XD, XS, MT, DT)                                            \
        cltcs3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define cleos_rr(XG, XS)                                                    \
        clecs_rr(W(XG), W(XS))

#define cleos_ld(XG, MS, DS)                                                \
        clecs_ld(W(XG), W(MS), W(DS))

#define cleos3rr(XD, XS, XT)                                                \
        clecs3rr(W(XD), W(XS), W(XT))

#define cleos3ld(XD, XS, MT, DT)                                            \
        clecs3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtos_rr(XG, XS)                                                    \
        cgtcs_rr(W(XG), W(XS))

#define cgtos_ld(XG, MS, DS)                                                \
        cgtcs_ld(W(XG), W(MS), W(DS))

#define cgtos3rr(XD, XS, XT)                                                \
        cgtcs3rr(W(XD), W(XS), W(XT))

#define cgtos3ld(XD, XS, MT, DT)                                            \
        cgtcs3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

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

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addox_rr(XG, XS)                                                    \
        addcx_rr(W(XG), W(XS))

#define addox_ld(XG, MS, DS)                                                \
        addcx_ld(W(XG), W(MS), W(DS))

#define addox3rr(XD, XS, XT)                                                \
        addcx3rr(W(XD), W(XS), W(XT))

#define addox3ld(XD, XS, MT, DT)                                            \
        addcx3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subox_rr(XG, XS)                                                    \
        subcx_rr(W(XG), W(XS))

#define subox_ld(XG, MS, DS)                                                \
        subcx_ld(W(XG), W(MS), W(DS))

#define subox3rr(XD, XS, XT)                                                \
        subcx3rr(W(XD), W(XS), W(XT))

#define subox3ld(XD, XS, MT, DT)                                            \
        subcx3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulox_rr(XG, XS)                                                    \
        mulcx_rr(W(XG), W(XS))

#define mulox_ld(XG, MS, DS)                                                \
        mulcx_ld(W(XG), W(MS), W(DS))

#define mulox3rr(XD, XS, XT)                                                \
        mulcx3rr(W(XD), W(XS), W(XT))

#define mulox3ld(XD, XS, MT, DT)                                            \
        mulcx3ld(W(XD), W(XS), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlox_ri(XG, IS)                                                    \
        shlcx_ri(W(XG), W(IS))

#define shlox_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlcx_ld(W(XG), W(MS), W(DS))

#define shlox3ri(XD, XS, IT)                                                \
        shlcx3ri(W(XD), W(XS), W(IT))

#define shlox3ld(XD, XS, MT, DT)                                            \
        shlcx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrox_ri(XG, IS)                                                    \
        shrcx_ri(W(XG), W(IS))

#define shrox_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrcx_ld(W(XG), W(MS), W(DS))

#define shrox3ri(XD, XS, IT)                                                \
        shrcx3ri(W(XD), W(XS), W(IT))

#define shrox3ld(XD, XS, MT, DT)                                            \
        shrcx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shron_ri(XG, IS)                                                    \
        shrcn_ri(W(XG), W(IS))

#define shron_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrcn_ld(W(XG), W(MS), W(DS))

#define shron3ri(XD, XS, IT)                                                \
        shrcn3ri(W(XD), W(XS), W(IT))

#define shron3ld(XD, XS, MT, DT)                                            \
        shrcn3ld(W(XD), W(XS), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlox_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlcx_rr(W(XG), W(XS))

#define svlox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlcx_ld(W(XG), W(MS), W(DS))

#define svlox3rr(XD, XS, XT)                                                \
        svlcx3rr(W(XD), W(XS), W(XT))

#define svlox3ld(XD, XS, MT, DT)                                            \
        svlcx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrox_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrcx_rr(W(XG), W(XS))

#define svrox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrcx_ld(W(XG), W(MS), W(DS))

#define svrox3rr(XD, XS, XT)                                                \
        svrcx3rr(W(XD), W(XS), W(XT))

#define svrox3ld(XD, XS, MT, DT)                                            \
        svrcx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svron_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrcn_rr(W(XG), W(XS))

#define svron_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrcn_ld(W(XG), W(MS), W(DS))

#define svron3rr(XD, XS, XT)                                                \
        svrcn3rr(W(XD), W(XS), W(XT))

#define svron3ld(XD, XS, MT, DT)                                            \
        svrcn3ld(W(XD), W(XS), W(MT), W(DT))

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

/* and (G = G & S), (D = S & T) if (#D != #S) */

#define andox_rr(XG, XS)                                                    \
        andix_rr(W(XG), W(XS))

#define andox_ld(XG, MS, DS)                                                \
        andix_ld(W(XG), W(MS), W(DS))

#define andox3rr(XD, XS, XT)                                                \
        andix3rr(W(XD), W(XS), W(XT))

#define andox3ld(XD, XS, MT, DT)                                            \
        andix3ld(W(XD), W(XS), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #S) */

#define annox_rr(XG, XS)                                                    \
        annix_rr(W(XG), W(XS))

#define annox_ld(XG, MS, DS)                                                \
        annix_ld(W(XG), W(MS), W(DS))

#define annox3rr(XD, XS, XT)                                                \
        annix3rr(W(XD), W(XS), W(XT))

#define annox3ld(XD, XS, MT, DT)                                            \
        annix3ld(W(XD), W(XS), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #S) */

#define orrox_rr(XG, XS)                                                    \
        orrix_rr(W(XG), W(XS))

#define orrox_ld(XG, MS, DS)                                                \
        orrix_ld(W(XG), W(MS), W(DS))

#define orrox3rr(XD, XS, XT)                                                \
        orrix3rr(W(XD), W(XS), W(XT))

#define orrox3ld(XD, XS, MT, DT)                                            \
        orrix3ld(W(XD), W(XS), W(MT), W(DT))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #S) */

#define ornox_rr(XG, XS)                                                    \
        ornix_rr(W(XG), W(XS))

#define ornox_ld(XG, MS, DS)                                                \
        ornix_ld(W(XG), W(MS), W(DS))

#define ornox3rr(XD, XS, XT)                                                \
        ornix3rr(W(XD), W(XS), W(XT))

#define ornox3ld(XD, XS, MT, DT)                                            \
        ornix3ld(W(XD), W(XS), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #S) */

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

/* add (G = G + S), (D = S + T) if (#D != #S) */

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

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subos_rr(XG, XS)                                                    \
        subis_rr(W(XG), W(XS))

#define subos_ld(XG, MS, DS)                                                \
        subis_ld(W(XG), W(MS), W(DS))

#define subos3rr(XD, XS, XT)                                                \
        subis3rr(W(XD), W(XS), W(XT))

#define subos3ld(XD, XS, MT, DT)                                            \
        subis3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

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

/* div (G = G / S), (D = S / T) if (#D != #S) */

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

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsos_rr(XG, XS, XT)                                                \
        fmsis_rr(W(XG), W(XS), W(XT))

#define fmsos_ld(XG, XS, MT, DT)                                            \
        fmsis_ld(W(XG), W(XS), W(MT), W(DT))

/*************   packed single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

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

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

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

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqos_rr(XG, XS)                                                    \
        ceqis_rr(W(XG), W(XS))

#define ceqos_ld(XG, MS, DS)                                                \
        ceqis_ld(W(XG), W(MS), W(DS))

#define ceqos3rr(XD, XS, XT)                                                \
        ceqis3rr(W(XD), W(XS), W(XT))

#define ceqos3ld(XD, XS, MT, DT)                                            \
        ceqis3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cneos_rr(XG, XS)                                                    \
        cneis_rr(W(XG), W(XS))

#define cneos_ld(XG, MS, DS)                                                \
        cneis_ld(W(XG), W(MS), W(DS))

#define cneos3rr(XD, XS, XT)                                                \
        cneis3rr(W(XD), W(XS), W(XT))

#define cneos3ld(XD, XS, MT, DT)                                            \
        cneis3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltos_rr(XG, XS)                                                    \
        cltis_rr(W(XG), W(XS))

#define cltos_ld(XG, MS, DS)                                                \
        cltis_ld(W(XG), W(MS), W(DS))

#define cltos3rr(XD, XS, XT)                                                \
        cltis3rr(W(XD), W(XS), W(XT))

#define cltos3ld(XD, XS, MT, DT)                                            \
        cltis3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define cleos_rr(XG, XS)                                                    \
        cleis_rr(W(XG), W(XS))

#define cleos_ld(XG, MS, DS)                                                \
        cleis_ld(W(XG), W(MS), W(DS))

#define cleos3rr(XD, XS, XT)                                                \
        cleis3rr(W(XD), W(XS), W(XT))

#define cleos3ld(XD, XS, MT, DT)                                            \
        cleis3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtos_rr(XG, XS)                                                    \
        cgtis_rr(W(XG), W(XS))

#define cgtos_ld(XG, MS, DS)                                                \
        cgtis_ld(W(XG), W(MS), W(DS))

#define cgtos3rr(XD, XS, XT)                                                \
        cgtis3rr(W(XD), W(XS), W(XT))

#define cgtos3ld(XD, XS, MT, DT)                                            \
        cgtis3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

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

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addox_rr(XG, XS)                                                    \
        addix_rr(W(XG), W(XS))

#define addox_ld(XG, MS, DS)                                                \
        addix_ld(W(XG), W(MS), W(DS))

#define addox3rr(XD, XS, XT)                                                \
        addix3rr(W(XD), W(XS), W(XT))

#define addox3ld(XD, XS, MT, DT)                                            \
        addix3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subox_rr(XG, XS)                                                    \
        subix_rr(W(XG), W(XS))

#define subox_ld(XG, MS, DS)                                                \
        subix_ld(W(XG), W(MS), W(DS))

#define subox3rr(XD, XS, XT)                                                \
        subix3rr(W(XD), W(XS), W(XT))

#define subox3ld(XD, XS, MT, DT)                                            \
        subix3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulox_rr(XG, XS)                                                    \
        mulix_rr(W(XG), W(XS))

#define mulox_ld(XG, MS, DS)                                                \
        mulix_ld(W(XG), W(MS), W(DS))

#define mulox3rr(XD, XS, XT)                                                \
        mulix3rr(W(XD), W(XS), W(XT))

#define mulox3ld(XD, XS, MT, DT)                                            \
        mulix3ld(W(XD), W(XS), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlox_ri(XG, IS)                                                    \
        shlix_ri(W(XG), W(IS))

#define shlox_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlix_ld(W(XG), W(MS), W(DS))

#define shlox3ri(XD, XS, IT)                                                \
        shlix3ri(W(XD), W(XS), W(IT))

#define shlox3ld(XD, XS, MT, DT)                                            \
        shlix3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrox_ri(XG, IS)                                                    \
        shrix_ri(W(XG), W(IS))

#define shrox_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrix_ld(W(XG), W(MS), W(DS))

#define shrox3ri(XD, XS, IT)                                                \
        shrix3ri(W(XD), W(XS), W(IT))

#define shrox3ld(XD, XS, MT, DT)                                            \
        shrix3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shron_ri(XG, IS)                                                    \
        shrin_ri(W(XG), W(IS))

#define shron_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrin_ld(W(XG), W(MS), W(DS))

#define shron3ri(XD, XS, IT)                                                \
        shrin3ri(W(XD), W(XS), W(IT))

#define shron3ld(XD, XS, MT, DT)                                            \
        shrin3ld(W(XD), W(XS), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlox_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlix_rr(W(XG), W(XS))

#define svlox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlix_ld(W(XG), W(MS), W(DS))

#define svlox3rr(XD, XS, XT)                                                \
        svlix3rr(W(XD), W(XS), W(XT))

#define svlox3ld(XD, XS, MT, DT)                                            \
        svlix3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrox_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrix_rr(W(XG), W(XS))

#define svrox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrix_ld(W(XG), W(MS), W(DS))

#define svrox3rr(XD, XS, XT)                                                \
        svrix3rr(W(XD), W(XS), W(XT))

#define svrox3ld(XD, XS, MT, DT)                                            \
        svrix3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svron_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrin_rr(W(XG), W(XS))

#define svron_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrin_ld(W(XG), W(MS), W(DS))

#define svron3rr(XD, XS, XT)                                                \
        svrin3rr(W(XD), W(XS), W(XT))

#define svron3ld(XD, XS, MT, DT)                                            \
        svrin3ld(W(XD), W(XS), W(MT), W(DT))

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

/* and (G = G & S), (D = S & T) if (#D != #S) */

#define andqx_rr(XG, XS)                                                    \
        anddx_rr(W(XG), W(XS))

#define andqx_ld(XG, MS, DS)                                                \
        anddx_ld(W(XG), W(MS), W(DS))

#define andqx3rr(XD, XS, XT)                                                \
        anddx3rr(W(XD), W(XS), W(XT))

#define andqx3ld(XD, XS, MT, DT)                                            \
        anddx3ld(W(XD), W(XS), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #S) */

#define annqx_rr(XG, XS)                                                    \
        anndx_rr(W(XG), W(XS))

#define annqx_ld(XG, MS, DS)                                                \
        anndx_ld(W(XG), W(MS), W(DS))

#define annqx3rr(XD, XS, XT)                                                \
        anndx3rr(W(XD), W(XS), W(XT))

#define annqx3ld(XD, XS, MT, DT)                                            \
        anndx3ld(W(XD), W(XS), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #S) */

#define orrqx_rr(XG, XS)                                                    \
        orrdx_rr(W(XG), W(XS))

#define orrqx_ld(XG, MS, DS)                                                \
        orrdx_ld(W(XG), W(MS), W(DS))

#define orrqx3rr(XD, XS, XT)                                                \
        orrdx3rr(W(XD), W(XS), W(XT))

#define orrqx3ld(XD, XS, MT, DT)                                            \
        orrdx3ld(W(XD), W(XS), W(MT), W(DT))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #S) */

#define ornqx_rr(XG, XS)                                                    \
        orndx_rr(W(XG), W(XS))

#define ornqx_ld(XG, MS, DS)                                                \
        orndx_ld(W(XG), W(MS), W(DS))

#define ornqx3rr(XD, XS, XT)                                                \
        orndx3rr(W(XD), W(XS), W(XT))

#define ornqx3ld(XD, XS, MT, DT)                                            \
        orndx3ld(W(XD), W(XS), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #S) */

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

/* add (G = G + S), (D = S + T) if (#D != #S) */

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

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subqs_rr(XG, XS)                                                    \
        subds_rr(W(XG), W(XS))

#define subqs_ld(XG, MS, DS)                                                \
        subds_ld(W(XG), W(MS), W(DS))

#define subqs3rr(XD, XS, XT)                                                \
        subds3rr(W(XD), W(XS), W(XT))

#define subqs3ld(XD, XS, MT, DT)                                            \
        subds3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

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

/* div (G = G / S), (D = S / T) if (#D != #S) */

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

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsqs_rr(XG, XS, XT)                                                \
        fmsds_rr(W(XG), W(XS), W(XT))

#define fmsqs_ld(XG, XS, MT, DT)                                            \
        fmsds_ld(W(XG), W(XS), W(MT), W(DT))

/*************   packed double-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

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

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

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

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqqs_rr(XG, XS)                                                    \
        ceqds_rr(W(XG), W(XS))

#define ceqqs_ld(XG, MS, DS)                                                \
        ceqds_ld(W(XG), W(MS), W(DS))

#define ceqqs3rr(XD, XS, XT)                                                \
        ceqds3rr(W(XD), W(XS), W(XT))

#define ceqqs3ld(XD, XS, MT, DT)                                            \
        ceqds3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cneqs_rr(XG, XS)                                                    \
        cneds_rr(W(XG), W(XS))

#define cneqs_ld(XG, MS, DS)                                                \
        cneds_ld(W(XG), W(MS), W(DS))

#define cneqs3rr(XD, XS, XT)                                                \
        cneds3rr(W(XD), W(XS), W(XT))

#define cneqs3ld(XD, XS, MT, DT)                                            \
        cneds3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltqs_rr(XG, XS)                                                    \
        cltds_rr(W(XG), W(XS))

#define cltqs_ld(XG, MS, DS)                                                \
        cltds_ld(W(XG), W(MS), W(DS))

#define cltqs3rr(XD, XS, XT)                                                \
        cltds3rr(W(XD), W(XS), W(XT))

#define cltqs3ld(XD, XS, MT, DT)                                            \
        cltds3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define cleqs_rr(XG, XS)                                                    \
        cleds_rr(W(XG), W(XS))

#define cleqs_ld(XG, MS, DS)                                                \
        cleds_ld(W(XG), W(MS), W(DS))

#define cleqs3rr(XD, XS, XT)                                                \
        cleds3rr(W(XD), W(XS), W(XT))

#define cleqs3ld(XD, XS, MT, DT)                                            \
        cleds3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtqs_rr(XG, XS)                                                    \
        cgtds_rr(W(XG), W(XS))

#define cgtqs_ld(XG, MS, DS)                                                \
        cgtds_ld(W(XG), W(MS), W(DS))

#define cgtqs3rr(XD, XS, XT)                                                \
        cgtds3rr(W(XD), W(XS), W(XT))

#define cgtqs3ld(XD, XS, MT, DT)                                            \
        cgtds3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

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

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addqx_rr(XG, XS)                                                    \
        adddx_rr(W(XG), W(XS))

#define addqx_ld(XG, MS, DS)                                                \
        adddx_ld(W(XG), W(MS), W(DS))

#define addqx3rr(XD, XS, XT)                                                \
        adddx3rr(W(XD), W(XS), W(XT))

#define addqx3ld(XD, XS, MT, DT)                                            \
        adddx3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subqx_rr(XG, XS)                                                    \
        subdx_rr(W(XG), W(XS))

#define subqx_ld(XG, MS, DS)                                                \
        subdx_ld(W(XG), W(MS), W(DS))

#define subqx3rr(XD, XS, XT)                                                \
        subdx3rr(W(XD), W(XS), W(XT))

#define subqx3ld(XD, XS, MT, DT)                                            \
        subdx3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulqx_rr(XG, XS)                                                    \
        muldx_rr(W(XG), W(XS))

#define mulqx_ld(XG, MS, DS)                                                \
        muldx_ld(W(XG), W(MS), W(DS))

#define mulqx3rr(XD, XS, XT)                                                \
        muldx3rr(W(XD), W(XS), W(XT))

#define mulqx3ld(XD, XS, MT, DT)                                            \
        muldx3ld(W(XD), W(XS), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlqx_ri(XG, IS)                                                    \
        shldx_ri(W(XG), W(IS))

#define shlqx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shldx_ld(W(XG), W(MS), W(DS))

#define shlqx3ri(XD, XS, IT)                                                \
        shldx3ri(W(XD), W(XS), W(IT))

#define shlqx3ld(XD, XS, MT, DT)                                            \
        shldx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrqx_ri(XG, IS)                                                    \
        shrdx_ri(W(XG), W(IS))

#define shrqx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrdx_ld(W(XG), W(MS), W(DS))

#define shrqx3ri(XD, XS, IT)                                                \
        shrdx3ri(W(XD), W(XS), W(IT))

#define shrqx3ld(XD, XS, MT, DT)                                            \
        shrdx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrqn_ri(XG, IS)                                                    \
        shrdn_ri(W(XG), W(IS))

#define shrqn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrdn_ld(W(XG), W(MS), W(DS))

#define shrqn3ri(XD, XS, IT)                                                \
        shrdn3ri(W(XD), W(XS), W(IT))

#define shrqn3ld(XD, XS, MT, DT)                                            \
        shrdn3ld(W(XD), W(XS), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlqx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svldx_rr(W(XG), W(XS))

#define svlqx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svldx_ld(W(XG), W(MS), W(DS))

#define svlqx3rr(XD, XS, XT)                                                \
        svldx3rr(W(XD), W(XS), W(XT))

#define svlqx3ld(XD, XS, MT, DT)                                            \
        svldx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrqx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrdx_rr(W(XG), W(XS))

#define svrqx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrdx_ld(W(XG), W(MS), W(DS))

#define svrqx3rr(XD, XS, XT)                                                \
        svrdx3rr(W(XD), W(XS), W(XT))

#define svrqx3ld(XD, XS, MT, DT)                                            \
        svrdx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrqn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrdn_rr(W(XG), W(XS))

#define svrqn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrdn_ld(W(XG), W(MS), W(DS))

#define svrqn3rr(XD, XS, XT)                                                \
        svrdn3rr(W(XD), W(XS), W(XT))

#define svrqn3ld(XD, XS, MT, DT)                                            \
        svrdn3ld(W(XD), W(XS), W(MT), W(DT))

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

/* and (G = G & S), (D = S & T) if (#D != #S) */

#define andqx_rr(XG, XS)                                                    \
        andjx_rr(W(XG), W(XS))

#define andqx_ld(XG, MS, DS)                                                \
        andjx_ld(W(XG), W(MS), W(DS))

#define andqx3rr(XD, XS, XT)                                                \
        andjx3rr(W(XD), W(XS), W(XT))

#define andqx3ld(XD, XS, MT, DT)                                            \
        andjx3ld(W(XD), W(XS), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #S) */

#define annqx_rr(XG, XS)                                                    \
        annjx_rr(W(XG), W(XS))

#define annqx_ld(XG, MS, DS)                                                \
        annjx_ld(W(XG), W(MS), W(DS))

#define annqx3rr(XD, XS, XT)                                                \
        annjx3rr(W(XD), W(XS), W(XT))

#define annqx3ld(XD, XS, MT, DT)                                            \
        annjx3ld(W(XD), W(XS), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #S) */

#define orrqx_rr(XG, XS)                                                    \
        orrjx_rr(W(XG), W(XS))

#define orrqx_ld(XG, MS, DS)                                                \
        orrjx_ld(W(XG), W(MS), W(DS))

#define orrqx3rr(XD, XS, XT)                                                \
        orrjx3rr(W(XD), W(XS), W(XT))

#define orrqx3ld(XD, XS, MT, DT)                                            \
        orrjx3ld(W(XD), W(XS), W(MT), W(DT))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #S) */

#define ornqx_rr(XG, XS)                                                    \
        ornjx_rr(W(XG), W(XS))

#define ornqx_ld(XG, MS, DS)                                                \
        ornjx_ld(W(XG), W(MS), W(DS))

#define ornqx3rr(XD, XS, XT)                                                \
        ornjx3rr(W(XD), W(XS), W(XT))

#define ornqx3ld(XD, XS, MT, DT)                                            \
        ornjx3ld(W(XD), W(XS), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #S) */

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

/* add (G = G + S), (D = S + T) if (#D != #S) */

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

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subqs_rr(XG, XS)                                                    \
        subjs_rr(W(XG), W(XS))

#define subqs_ld(XG, MS, DS)                                                \
        subjs_ld(W(XG), W(MS), W(DS))

#define subqs3rr(XD, XS, XT)                                                \
        subjs3rr(W(XD), W(XS), W(XT))

#define subqs3ld(XD, XS, MT, DT)                                            \
        subjs3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

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

/* div (G = G / S), (D = S / T) if (#D != #S) */

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

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsqs_rr(XG, XS, XT)                                                \
        fmsjs_rr(W(XG), W(XS), W(XT))

#define fmsqs_ld(XG, XS, MT, DT)                                            \
        fmsjs_ld(W(XG), W(XS), W(MT), W(DT))

/*************   packed double-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

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

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

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

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqqs_rr(XG, XS)                                                    \
        ceqjs_rr(W(XG), W(XS))

#define ceqqs_ld(XG, MS, DS)                                                \
        ceqjs_ld(W(XG), W(MS), W(DS))

#define ceqqs3rr(XD, XS, XT)                                                \
        ceqjs3rr(W(XD), W(XS), W(XT))

#define ceqqs3ld(XD, XS, MT, DT)                                            \
        ceqjs3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cneqs_rr(XG, XS)                                                    \
        cnejs_rr(W(XG), W(XS))

#define cneqs_ld(XG, MS, DS)                                                \
        cnejs_ld(W(XG), W(MS), W(DS))

#define cneqs3rr(XD, XS, XT)                                                \
        cnejs3rr(W(XD), W(XS), W(XT))

#define cneqs3ld(XD, XS, MT, DT)                                            \
        cnejs3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltqs_rr(XG, XS)                                                    \
        cltjs_rr(W(XG), W(XS))

#define cltqs_ld(XG, MS, DS)                                                \
        cltjs_ld(W(XG), W(MS), W(DS))

#define cltqs3rr(XD, XS, XT)                                                \
        cltjs3rr(W(XD), W(XS), W(XT))

#define cltqs3ld(XD, XS, MT, DT)                                            \
        cltjs3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define cleqs_rr(XG, XS)                                                    \
        clejs_rr(W(XG), W(XS))

#define cleqs_ld(XG, MS, DS)                                                \
        clejs_ld(W(XG), W(MS), W(DS))

#define cleqs3rr(XD, XS, XT)                                                \
        clejs3rr(W(XD), W(XS), W(XT))

#define cleqs3ld(XD, XS, MT, DT)                                            \
        clejs3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtqs_rr(XG, XS)                                                    \
        cgtjs_rr(W(XG), W(XS))

#define cgtqs_ld(XG, MS, DS)                                                \
        cgtjs_ld(W(XG), W(MS), W(DS))

#define cgtqs3rr(XD, XS, XT)                                                \
        cgtjs3rr(W(XD), W(XS), W(XT))

#define cgtqs3ld(XD, XS, MT, DT)                                            \
        cgtjs3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

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

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addqx_rr(XG, XS)                                                    \
        addjx_rr(W(XG), W(XS))

#define addqx_ld(XG, MS, DS)                                                \
        addjx_ld(W(XG), W(MS), W(DS))

#define addqx3rr(XD, XS, XT)                                                \
        addjx3rr(W(XD), W(XS), W(XT))

#define addqx3ld(XD, XS, MT, DT)                                            \
        addjx3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subqx_rr(XG, XS)                                                    \
        subjx_rr(W(XG), W(XS))

#define subqx_ld(XG, MS, DS)                                                \
        subjx_ld(W(XG), W(MS), W(DS))

#define subqx3rr(XD, XS, XT)                                                \
        subjx3rr(W(XD), W(XS), W(XT))

#define subqx3ld(XD, XS, MT, DT)                                            \
        subjx3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulqx_rr(XG, XS)                                                    \
        muljx_rr(W(XG), W(XS))

#define mulqx_ld(XG, MS, DS)                                                \
        muljx_ld(W(XG), W(MS), W(DS))

#define mulqx3rr(XD, XS, XT)                                                \
        muljx3rr(W(XD), W(XS), W(XT))

#define mulqx3ld(XD, XS, MT, DT)                                            \
        muljx3ld(W(XD), W(XS), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlqx_ri(XG, IS)                                                    \
        shljx_ri(W(XG), W(IS))

#define shlqx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shljx_ld(W(XG), W(MS), W(DS))

#define shlqx3ri(XD, XS, IT)                                                \
        shljx3ri(W(XD), W(XS), W(IT))

#define shlqx3ld(XD, XS, MT, DT)                                            \
        shljx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrqx_ri(XG, IS)                                                    \
        shrjx_ri(W(XG), W(IS))

#define shrqx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrjx_ld(W(XG), W(MS), W(DS))

#define shrqx3ri(XD, XS, IT)                                                \
        shrjx3ri(W(XD), W(XS), W(IT))

#define shrqx3ld(XD, XS, MT, DT)                                            \
        shrjx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrqn_ri(XG, IS)                                                    \
        shrjn_ri(W(XG), W(IS))

#define shrqn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrjn_ld(W(XG), W(MS), W(DS))

#define shrqn3ri(XD, XS, IT)                                                \
        shrjn3ri(W(XD), W(XS), W(IT))

#define shrqn3ld(XD, XS, MT, DT)                                            \
        shrjn3ld(W(XD), W(XS), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlqx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svljx_rr(W(XG), W(XS))

#define svlqx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svljx_ld(W(XG), W(MS), W(DS))

#define svlqx3rr(XD, XS, XT)                                                \
        svljx3rr(W(XD), W(XS), W(XT))

#define svlqx3ld(XD, XS, MT, DT)                                            \
        svljx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrqx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrjx_rr(W(XG), W(XS))

#define svrqx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrjx_ld(W(XG), W(MS), W(DS))

#define svrqx3rr(XD, XS, XT)                                                \
        svrjx3rr(W(XD), W(XS), W(XT))

#define svrqx3ld(XD, XS, MT, DT)                                            \
        svrjx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrqn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrjn_rr(W(XG), W(XS))

#define svrqn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrjn_ld(W(XG), W(MS), W(DS))

#define svrqn3rr(XD, XS, XT)                                                \
        svrjn3rr(W(XD), W(XS), W(XT))

#define svrqn3ld(XD, XS, MT, DT)                                            \
        svrjn3ld(W(XD), W(XS), W(MT), W(DT))

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

/* and (G = G & S), (D = S & T) if (#D != #S) */

#define andpx_rr(XG, XS)                                                    \
        andox_rr(W(XG), W(XS))

#define andpx_ld(XG, MS, DS)                                                \
        andox_ld(W(XG), W(MS), W(DS))

#define andpx3rr(XD, XS, XT)                                                \
        andox3rr(W(XD), W(XS), W(XT))

#define andpx3ld(XD, XS, MT, DT)                                            \
        andox3ld(W(XD), W(XS), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #S) */

#define annpx_rr(XG, XS)                                                    \
        annox_rr(W(XG), W(XS))

#define annpx_ld(XG, MS, DS)                                                \
        annox_ld(W(XG), W(MS), W(DS))

#define annpx3rr(XD, XS, XT)                                                \
        annox3rr(W(XD), W(XS), W(XT))

#define annpx3ld(XD, XS, MT, DT)                                            \
        annox3ld(W(XD), W(XS), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #S) */

#define orrpx_rr(XG, XS)                                                    \
        orrox_rr(W(XG), W(XS))

#define orrpx_ld(XG, MS, DS)                                                \
        orrox_ld(W(XG), W(MS), W(DS))

#define orrpx3rr(XD, XS, XT)                                                \
        orrox3rr(W(XD), W(XS), W(XT))

#define orrpx3ld(XD, XS, MT, DT)                                            \
        orrox3ld(W(XD), W(XS), W(MT), W(DT))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #S) */

#define ornpx_rr(XG, XS)                                                    \
        ornox_rr(W(XG), W(XS))

#define ornpx_ld(XG, MS, DS)                                                \
        ornox_ld(W(XG), W(MS), W(DS))

#define ornpx3rr(XD, XS, XT)                                                \
        ornox3rr(W(XD), W(XS), W(XT))

#define ornpx3ld(XD, XS, MT, DT)                                            \
        ornox3ld(W(XD), W(XS), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #S) */

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

/* add (G = G + S), (D = S + T) if (#D != #S) */

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

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subps_rr(XG, XS)                                                    \
        subos_rr(W(XG), W(XS))

#define subps_ld(XG, MS, DS)                                                \
        subos_ld(W(XG), W(MS), W(DS))

#define subps3rr(XD, XS, XT)                                                \
        subos3rr(W(XD), W(XS), W(XT))

#define subps3ld(XD, XS, MT, DT)                                            \
        subos3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

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

/* div (G = G / S), (D = S / T) if (#D != #S) */

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

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsps_rr(XG, XS, XT)                                                \
        fmsos_rr(W(XG), W(XS), W(XT))

#define fmsps_ld(XG, XS, MT, DT)                                            \
        fmsos_ld(W(XG), W(XS), W(MT), W(DT))

/*************   packed single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

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

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

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

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqps_rr(XG, XS)                                                    \
        ceqos_rr(W(XG), W(XS))

#define ceqps_ld(XG, MS, DS)                                                \
        ceqos_ld(W(XG), W(MS), W(DS))

#define ceqps3rr(XD, XS, XT)                                                \
        ceqos3rr(W(XD), W(XS), W(XT))

#define ceqps3ld(XD, XS, MT, DT)                                            \
        ceqos3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cneps_rr(XG, XS)                                                    \
        cneos_rr(W(XG), W(XS))

#define cneps_ld(XG, MS, DS)                                                \
        cneos_ld(W(XG), W(MS), W(DS))

#define cneps3rr(XD, XS, XT)                                                \
        cneos3rr(W(XD), W(XS), W(XT))

#define cneps3ld(XD, XS, MT, DT)                                            \
        cneos3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltps_rr(XG, XS)                                                    \
        cltos_rr(W(XG), W(XS))

#define cltps_ld(XG, MS, DS)                                                \
        cltos_ld(W(XG), W(MS), W(DS))

#define cltps3rr(XD, XS, XT)                                                \
        cltos3rr(W(XD), W(XS), W(XT))

#define cltps3ld(XD, XS, MT, DT)                                            \
        cltos3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define cleps_rr(XG, XS)                                                    \
        cleos_rr(W(XG), W(XS))

#define cleps_ld(XG, MS, DS)                                                \
        cleos_ld(W(XG), W(MS), W(DS))

#define cleps3rr(XD, XS, XT)                                                \
        cleos3rr(W(XD), W(XS), W(XT))

#define cleps3ld(XD, XS, MT, DT)                                            \
        cleos3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtps_rr(XG, XS)                                                    \
        cgtos_rr(W(XG), W(XS))

#define cgtps_ld(XG, MS, DS)                                                \
        cgtos_ld(W(XG), W(MS), W(DS))

#define cgtps3rr(XD, XS, XT)                                                \
        cgtos3rr(W(XD), W(XS), W(XT))

#define cgtps3ld(XD, XS, MT, DT)                                            \
        cgtos3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

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

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addpx_rr(XG, XS)                                                    \
        addox_rr(W(XG), W(XS))

#define addpx_ld(XG, MS, DS)                                                \
        addox_ld(W(XG), W(MS), W(DS))

#define addpx3rr(XD, XS, XT)                                                \
        addox3rr(W(XD), W(XS), W(XT))

#define addpx3ld(XD, XS, MT, DT)                                            \
        addox3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subpx_rr(XG, XS)                                                    \
        subox_rr(W(XG), W(XS))

#define subpx_ld(XG, MS, DS)                                                \
        subox_ld(W(XG), W(MS), W(DS))

#define subpx3rr(XD, XS, XT)                                                \
        subox3rr(W(XD), W(XS), W(XT))

#define subpx3ld(XD, XS, MT, DT)                                            \
        subox3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulpx_rr(XG, XS)                                                    \
        mulox_rr(W(XG), W(XS))

#define mulpx_ld(XG, MS, DS)                                                \
        mulox_ld(W(XG), W(MS), W(DS))

#define mulpx3rr(XD, XS, XT)                                                \
        mulox3rr(W(XD), W(XS), W(XT))

#define mulpx3ld(XD, XS, MT, DT)                                            \
        mulox3ld(W(XD), W(XS), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlpx_ri(XG, IS)                                                    \
        shlox_ri(W(XG), W(IS))

#define shlpx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlox_ld(W(XG), W(MS), W(DS))

#define shlpx3ri(XD, XS, IT)                                                \
        shlox3ri(W(XD), W(XS), W(IT))

#define shlpx3ld(XD, XS, MT, DT)                                            \
        shlox3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrpx_ri(XG, IS)                                                    \
        shrox_ri(W(XG), W(IS))

#define shrpx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrox_ld(W(XG), W(MS), W(DS))

#define shrpx3ri(XD, XS, IT)                                                \
        shrox3ri(W(XD), W(XS), W(IT))

#define shrpx3ld(XD, XS, MT, DT)                                            \
        shrox3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrpn_ri(XG, IS)                                                    \
        shron_ri(W(XG), W(IS))

#define shrpn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shron_ld(W(XG), W(MS), W(DS))

#define shrpn3ri(XD, XS, IT)                                                \
        shron3ri(W(XD), W(XS), W(IT))

#define shrpn3ld(XD, XS, MT, DT)                                            \
        shron3ld(W(XD), W(XS), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlpx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlox_rr(W(XG), W(XS))

#define svlpx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlox_ld(W(XG), W(MS), W(DS))

#define svlpx3rr(XD, XS, XT)                                                \
        svlox3rr(W(XD), W(XS), W(XT))

#define svlpx3ld(XD, XS, MT, DT)                                            \
        svlox3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrpx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrox_rr(W(XG), W(XS))

#define svrpx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrox_ld(W(XG), W(MS), W(DS))

#define svrpx3rr(XD, XS, XT)                                                \
        svrox3rr(W(XD), W(XS), W(XT))

#define svrpx3ld(XD, XS, MT, DT)                                            \
        svrox3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrpn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svron_rr(W(XG), W(XS))

#define svrpn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svron_ld(W(XG), W(MS), W(DS))

#define svrpn3rr(XD, XS, XT)                                                \
        svron3rr(W(XD), W(XS), W(XT))

#define svrpn3ld(XD, XS, MT, DT)                                            \
        svron3ld(W(XD), W(XS), W(MT), W(DT))

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

/* and (G = G & S), (D = S & T) if (#D != #S) */

#define andfx_rr(XG, XS)                                                    \
        andcx_rr(W(XG), W(XS))

#define andfx_ld(XG, MS, DS)                                                \
        andcx_ld(W(XG), W(MS), W(DS))

#define andfx3rr(XD, XS, XT)                                                \
        andcx3rr(W(XD), W(XS), W(XT))

#define andfx3ld(XD, XS, MT, DT)                                            \
        andcx3ld(W(XD), W(XS), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #S) */

#define annfx_rr(XG, XS)                                                    \
        anncx_rr(W(XG), W(XS))

#define annfx_ld(XG, MS, DS)                                                \
        anncx_ld(W(XG), W(MS), W(DS))

#define annfx3rr(XD, XS, XT)                                                \
        anncx3rr(W(XD), W(XS), W(XT))

#define annfx3ld(XD, XS, MT, DT)                                            \
        anncx3ld(W(XD), W(XS), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #S) */

#define orrfx_rr(XG, XS)                                                    \
        orrcx_rr(W(XG), W(XS))

#define orrfx_ld(XG, MS, DS)                                                \
        orrcx_ld(W(XG), W(MS), W(DS))

#define orrfx3rr(XD, XS, XT)                                                \
        orrcx3rr(W(XD), W(XS), W(XT))

#define orrfx3ld(XD, XS, MT, DT)                                            \
        orrcx3ld(W(XD), W(XS), W(MT), W(DT))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #S) */

#define ornfx_rr(XG, XS)                                                    \
        orncx_rr(W(XG), W(XS))

#define ornfx_ld(XG, MS, DS)                                                \
        orncx_ld(W(XG), W(MS), W(DS))

#define ornfx3rr(XD, XS, XT)                                                \
        orncx3rr(W(XD), W(XS), W(XT))

#define ornfx3ld(XD, XS, MT, DT)                                            \
        orncx3ld(W(XD), W(XS), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #S) */

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

/* add (G = G + S), (D = S + T) if (#D != #S) */

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

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subfs_rr(XG, XS)                                                    \
        subcs_rr(W(XG), W(XS))

#define subfs_ld(XG, MS, DS)                                                \
        subcs_ld(W(XG), W(MS), W(DS))

#define subfs3rr(XD, XS, XT)                                                \
        subcs3rr(W(XD), W(XS), W(XT))

#define subfs3ld(XD, XS, MT, DT)                                            \
        subcs3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

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

/* div (G = G / S), (D = S / T) if (#D != #S) */

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

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsfs_rr(XG, XS, XT)                                                \
        fmscs_rr(W(XG), W(XS), W(XT))

#define fmsfs_ld(XG, XS, MT, DT)                                            \
        fmscs_ld(W(XG), W(XS), W(MT), W(DT))

/*************   packed single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

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

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

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

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqfs_rr(XG, XS)                                                    \
        ceqcs_rr(W(XG), W(XS))

#define ceqfs_ld(XG, MS, DS)                                                \
        ceqcs_ld(W(XG), W(MS), W(DS))

#define ceqfs3rr(XD, XS, XT)                                                \
        ceqcs3rr(W(XD), W(XS), W(XT))

#define ceqfs3ld(XD, XS, MT, DT)                                            \
        ceqcs3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cnefs_rr(XG, XS)                                                    \
        cnecs_rr(W(XG), W(XS))

#define cnefs_ld(XG, MS, DS)                                                \
        cnecs_ld(W(XG), W(MS), W(DS))

#define cnefs3rr(XD, XS, XT)                                                \
        cnecs3rr(W(XD), W(XS), W(XT))

#define cnefs3ld(XD, XS, MT, DT)                                            \
        cnecs3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltfs_rr(XG, XS)                                                    \
        cltcs_rr(W(XG), W(XS))

#define cltfs_ld(XG, MS, DS)                                                \
        cltcs_ld(W(XG), W(MS), W(DS))

#define cltfs3rr(XD, XS, XT)                                                \
        cltcs3rr(W(XD), W(XS), W(XT))

#define cltfs3ld(XD, XS, MT, DT)                                            \
        cltcs3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define clefs_rr(XG, XS)                                                    \
        clecs_rr(W(XG), W(XS))

#define clefs_ld(XG, MS, DS)                                                \
        clecs_ld(W(XG), W(MS), W(DS))

#define clefs3rr(XD, XS, XT)                                                \
        clecs3rr(W(XD), W(XS), W(XT))

#define clefs3ld(XD, XS, MT, DT)                                            \
        clecs3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtfs_rr(XG, XS)                                                    \
        cgtcs_rr(W(XG), W(XS))

#define cgtfs_ld(XG, MS, DS)                                                \
        cgtcs_ld(W(XG), W(MS), W(DS))

#define cgtfs3rr(XD, XS, XT)                                                \
        cgtcs3rr(W(XD), W(XS), W(XT))

#define cgtfs3ld(XD, XS, MT, DT)                                            \
        cgtcs3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

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

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addfx_rr(XG, XS)                                                    \
        addcx_rr(W(XG), W(XS))

#define addfx_ld(XG, MS, DS)                                                \
        addcx_ld(W(XG), W(MS), W(DS))

#define addfx3rr(XD, XS, XT)                                                \
        addcx3rr(W(XD), W(XS), W(XT))

#define addfx3ld(XD, XS, MT, DT)                                            \
        addcx3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subfx_rr(XG, XS)                                                    \
        subcx_rr(W(XG), W(XS))

#define subfx_ld(XG, MS, DS)                                                \
        subcx_ld(W(XG), W(MS), W(DS))

#define subfx3rr(XD, XS, XT)                                                \
        subcx3rr(W(XD), W(XS), W(XT))

#define subfx3ld(XD, XS, MT, DT)                                            \
        subcx3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulfx_rr(XG, XS)                                                    \
        mulcx_rr(W(XG), W(XS))

#define mulfx_ld(XG, MS, DS)                                                \
        mulcx_ld(W(XG), W(MS), W(DS))

#define mulfx3rr(XD, XS, XT)                                                \
        mulcx3rr(W(XD), W(XS), W(XT))

#define mulfx3ld(XD, XS, MT, DT)                                            \
        mulcx3ld(W(XD), W(XS), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlfx_ri(XG, IS)                                                    \
        shlcx_ri(W(XG), W(IS))

#define shlfx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlcx_ld(W(XG), W(MS), W(DS))

#define shlfx3ri(XD, XS, IT)                                                \
        shlcx3ri(W(XD), W(XS), W(IT))

#define shlfx3ld(XD, XS, MT, DT)                                            \
        shlcx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrfx_ri(XG, IS)                                                    \
        shrcx_ri(W(XG), W(IS))

#define shrfx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrcx_ld(W(XG), W(MS), W(DS))

#define shrfx3ri(XD, XS, IT)                                                \
        shrcx3ri(W(XD), W(XS), W(IT))

#define shrfx3ld(XD, XS, MT, DT)                                            \
        shrcx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrfn_ri(XG, IS)                                                    \
        shrcn_ri(W(XG), W(IS))

#define shrfn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrcn_ld(W(XG), W(MS), W(DS))

#define shrfn3ri(XD, XS, IT)                                                \
        shrcn3ri(W(XD), W(XS), W(IT))

#define shrfn3ld(XD, XS, MT, DT)                                            \
        shrcn3ld(W(XD), W(XS), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlfx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlcx_rr(W(XG), W(XS))

#define svlfx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlcx_ld(W(XG), W(MS), W(DS))

#define svlfx3rr(XD, XS, XT)                                                \
        svlcx3rr(W(XD), W(XS), W(XT))

#define svlfx3ld(XD, XS, MT, DT)                                            \
        svlcx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrfx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrcx_rr(W(XG), W(XS))

#define svrfx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrcx_ld(W(XG), W(MS), W(DS))

#define svrfx3rr(XD, XS, XT)                                                \
        svrcx3rr(W(XD), W(XS), W(XT))

#define svrfx3ld(XD, XS, MT, DT)                                            \
        svrcx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrfn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrcn_rr(W(XG), W(XS))

#define svrfn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrcn_ld(W(XG), W(MS), W(DS))

#define svrfn3rr(XD, XS, XT)                                                \
        svrcn3rr(W(XD), W(XS), W(XT))

#define svrfn3ld(XD, XS, MT, DT)                                            \
        svrcn3ld(W(XD), W(XS), W(MT), W(DT))

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

/* and (G = G & S), (D = S & T) if (#D != #S) */

#define andlx_rr(XG, XS)                                                    \
        andix_rr(W(XG), W(XS))

#define andlx_ld(XG, MS, DS)                                                \
        andix_ld(W(XG), W(MS), W(DS))

#define andlx3rr(XD, XS, XT)                                                \
        andix3rr(W(XD), W(XS), W(XT))

#define andlx3ld(XD, XS, MT, DT)                                            \
        andix3ld(W(XD), W(XS), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #S) */

#define annlx_rr(XG, XS)                                                    \
        annix_rr(W(XG), W(XS))

#define annlx_ld(XG, MS, DS)                                                \
        annix_ld(W(XG), W(MS), W(DS))

#define annlx3rr(XD, XS, XT)                                                \
        annix3rr(W(XD), W(XS), W(XT))

#define annlx3ld(XD, XS, MT, DT)                                            \
        annix3ld(W(XD), W(XS), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #S) */

#define orrlx_rr(XG, XS)                                                    \
        orrix_rr(W(XG), W(XS))

#define orrlx_ld(XG, MS, DS)                                                \
        orrix_ld(W(XG), W(MS), W(DS))

#define orrlx3rr(XD, XS, XT)                                                \
        orrix3rr(W(XD), W(XS), W(XT))

#define orrlx3ld(XD, XS, MT, DT)                                            \
        orrix3ld(W(XD), W(XS), W(MT), W(DT))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #S) */

#define ornlx_rr(XG, XS)                                                    \
        ornix_rr(W(XG), W(XS))

#define ornlx_ld(XG, MS, DS)                                                \
        ornix_ld(W(XG), W(MS), W(DS))

#define ornlx3rr(XD, XS, XT)                                                \
        ornix3rr(W(XD), W(XS), W(XT))

#define ornlx3ld(XD, XS, MT, DT)                                            \
        ornix3ld(W(XD), W(XS), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #S) */

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

/* add (G = G + S), (D = S + T) if (#D != #S) */

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

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subls_rr(XG, XS)                                                    \
        subis_rr(W(XG), W(XS))

#define subls_ld(XG, MS, DS)                                                \
        subis_ld(W(XG), W(MS), W(DS))

#define subls3rr(XD, XS, XT)                                                \
        subis3rr(W(XD), W(XS), W(XT))

#define subls3ld(XD, XS, MT, DT)                                            \
        subis3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

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

/* div (G = G / S), (D = S / T) if (#D != #S) */

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

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsls_rr(XG, XS, XT)                                                \
        fmsis_rr(W(XG), W(XS), W(XT))

#define fmsls_ld(XG, XS, MT, DT)                                            \
        fmsis_ld(W(XG), W(XS), W(MT), W(DT))

/*************   packed single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

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

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

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

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqls_rr(XG, XS)                                                    \
        ceqis_rr(W(XG), W(XS))

#define ceqls_ld(XG, MS, DS)                                                \
        ceqis_ld(W(XG), W(MS), W(DS))

#define ceqls3rr(XD, XS, XT)                                                \
        ceqis3rr(W(XD), W(XS), W(XT))

#define ceqls3ld(XD, XS, MT, DT)                                            \
        ceqis3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cnels_rr(XG, XS)                                                    \
        cneis_rr(W(XG), W(XS))

#define cnels_ld(XG, MS, DS)                                                \
        cneis_ld(W(XG), W(MS), W(DS))

#define cnels3rr(XD, XS, XT)                                                \
        cneis3rr(W(XD), W(XS), W(XT))

#define cnels3ld(XD, XS, MT, DT)                                            \
        cneis3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltls_rr(XG, XS)                                                    \
        cltis_rr(W(XG), W(XS))

#define cltls_ld(XG, MS, DS)                                                \
        cltis_ld(W(XG), W(MS), W(DS))

#define cltls3rr(XD, XS, XT)                                                \
        cltis3rr(W(XD), W(XS), W(XT))

#define cltls3ld(XD, XS, MT, DT)                                            \
        cltis3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define clels_rr(XG, XS)                                                    \
        cleis_rr(W(XG), W(XS))

#define clels_ld(XG, MS, DS)                                                \
        cleis_ld(W(XG), W(MS), W(DS))

#define clels3rr(XD, XS, XT)                                                \
        cleis3rr(W(XD), W(XS), W(XT))

#define clels3ld(XD, XS, MT, DT)                                            \
        cleis3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtls_rr(XG, XS)                                                    \
        cgtis_rr(W(XG), W(XS))

#define cgtls_ld(XG, MS, DS)                                                \
        cgtis_ld(W(XG), W(MS), W(DS))

#define cgtls3rr(XD, XS, XT)                                                \
        cgtis3rr(W(XD), W(XS), W(XT))

#define cgtls3ld(XD, XS, MT, DT)                                            \
        cgtis3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

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

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addlx_rr(XG, XS)                                                    \
        addix_rr(W(XG), W(XS))

#define addlx_ld(XG, MS, DS)                                                \
        addix_ld(W(XG), W(MS), W(DS))

#define addlx3rr(XD, XS, XT)                                                \
        addix3rr(W(XD), W(XS), W(XT))

#define addlx3ld(XD, XS, MT, DT)                                            \
        addix3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define sublx_rr(XG, XS)                                                    \
        subix_rr(W(XG), W(XS))

#define sublx_ld(XG, MS, DS)                                                \
        subix_ld(W(XG), W(MS), W(DS))

#define sublx3rr(XD, XS, XT)                                                \
        subix3rr(W(XD), W(XS), W(XT))

#define sublx3ld(XD, XS, MT, DT)                                            \
        subix3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mullx_rr(XG, XS)                                                    \
        mulix_rr(W(XG), W(XS))

#define mullx_ld(XG, MS, DS)                                                \
        mulix_ld(W(XG), W(MS), W(DS))

#define mullx3rr(XD, XS, XT)                                                \
        mulix3rr(W(XD), W(XS), W(XT))

#define mullx3ld(XD, XS, MT, DT)                                            \
        mulix3ld(W(XD), W(XS), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shllx_ri(XG, IS)                                                    \
        shlix_ri(W(XG), W(IS))

#define shllx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlix_ld(W(XG), W(MS), W(DS))

#define shllx3ri(XD, XS, IT)                                                \
        shlix3ri(W(XD), W(XS), W(IT))

#define shllx3ld(XD, XS, MT, DT)                                            \
        shlix3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrlx_ri(XG, IS)                                                    \
        shrix_ri(W(XG), W(IS))

#define shrlx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrix_ld(W(XG), W(MS), W(DS))

#define shrlx3ri(XD, XS, IT)                                                \
        shrix3ri(W(XD), W(XS), W(IT))

#define shrlx3ld(XD, XS, MT, DT)                                            \
        shrix3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrln_ri(XG, IS)                                                    \
        shrin_ri(W(XG), W(IS))

#define shrln_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrin_ld(W(XG), W(MS), W(DS))

#define shrln3ri(XD, XS, IT)                                                \
        shrin3ri(W(XD), W(XS), W(IT))

#define shrln3ld(XD, XS, MT, DT)                                            \
        shrin3ld(W(XD), W(XS), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svllx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlix_rr(W(XG), W(XS))

#define svllx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlix_ld(W(XG), W(MS), W(DS))

#define svllx3rr(XD, XS, XT)                                                \
        svlix3rr(W(XD), W(XS), W(XT))

#define svllx3ld(XD, XS, MT, DT)                                            \
        svlix3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrlx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrix_rr(W(XG), W(XS))

#define svrlx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrix_ld(W(XG), W(MS), W(DS))

#define svrlx3rr(XD, XS, XT)                                                \
        svrix3rr(W(XD), W(XS), W(XT))

#define svrlx3ld(XD, XS, MT, DT)                                            \
        svrix3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrln_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrin_rr(W(XG), W(XS))

#define svrln_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrin_ld(W(XG), W(MS), W(DS))

#define svrln3rr(XD, XS, XT)                                                \
        svrin3rr(W(XD), W(XS), W(XT))

#define svrln3ld(XD, XS, MT, DT)                                            \
        svrin3ld(W(XD), W(XS), W(MT), W(DT))

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

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addss_rr(XG, XS)                                                    \
        addrs_rr(W(XG), W(XS))

#define addss_ld(XG, MS, DS)                                                \
        addrs_ld(W(XG), W(MS), W(DS))

#define addss3rr(XD, XS, XT)                                                \
        addrs3rr(W(XD), W(XS), W(XT))

#define addss3ld(XD, XS, MT, DT)                                            \
        addrs3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subss_rr(XG, XS)                                                    \
        subrs_rr(W(XG), W(XS))

#define subss_ld(XG, MS, DS)                                                \
        subrs_ld(W(XG), W(MS), W(DS))

#define subss3rr(XD, XS, XT)                                                \
        subrs3rr(W(XD), W(XS), W(XT))

#define subss3ld(XD, XS, MT, DT)                                            \
        subrs3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulss_rr(XG, XS)                                                    \
        mulrs_rr(W(XG), W(XS))

#define mulss_ld(XG, MS, DS)                                                \
        mulrs_ld(W(XG), W(MS), W(DS))

#define mulss3rr(XD, XS, XT)                                                \
        mulrs3rr(W(XD), W(XS), W(XT))

#define mulss3ld(XD, XS, MT, DT)                                            \
        mulrs3ld(W(XD), W(XS), W(MT), W(DT))

/* div (G = G / S), (D = S / T) if (#D != #S) */

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

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsss_rr(XG, XS, XT)                                                \
        fmsrs_rr(W(XG), W(XS), W(XT))

#define fmsss_ld(XG, XS, MT, DT)                                            \
        fmsrs_ld(W(XG), W(XS), W(MT), W(DT))

/*************   scalar single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

#define minss_rr(XG, XS)                                                    \
        minrs_rr(W(XG), W(XS))

#define minss_ld(XG, MS, DS)                                                \
        minrs_ld(W(XG), W(MS), W(DS))

#define minss3rr(XD, XS, XT)                                                \
        minrs3rr(W(XD), W(XS), W(XT))

#define minss3ld(XD, XS, MT, DT)                                            \
        minrs3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

#define maxss_rr(XG, XS)                                                    \
        maxrs_rr(W(XG), W(XS))

#define maxss_ld(XG, MS, DS)                                                \
        maxrs_ld(W(XG), W(MS), W(DS))

#define maxss3rr(XD, XS, XT)                                                \
        maxrs3rr(W(XD), W(XS), W(XT))

#define maxss3ld(XD, XS, MT, DT)                                            \
        maxrs3ld(W(XD), W(XS), W(MT), W(DT))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqss_rr(XG, XS)                                                    \
        ceqrs_rr(W(XG), W(XS))

#define ceqss_ld(XG, MS, DS)                                                \
        ceqrs_ld(W(XG), W(MS), W(DS))

#define ceqss3rr(XD, XS, XT)                                                \
        ceqrs3rr(W(XD), W(XS), W(XT))

#define ceqss3ld(XD, XS, MT, DT)                                            \
        ceqrs3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cness_rr(XG, XS)                                                    \
        cners_rr(W(XG), W(XS))

#define cness_ld(XG, MS, DS)                                                \
        cners_ld(W(XG), W(MS), W(DS))

#define cness3rr(XD, XS, XT)                                                \
        cners3rr(W(XD), W(XS), W(XT))

#define cness3ld(XD, XS, MT, DT)                                            \
        cners3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltss_rr(XG, XS)                                                    \
        cltrs_rr(W(XG), W(XS))

#define cltss_ld(XG, MS, DS)                                                \
        cltrs_ld(W(XG), W(MS), W(DS))

#define cltss3rr(XD, XS, XT)                                                \
        cltrs3rr(W(XD), W(XS), W(XT))

#define cltss3ld(XD, XS, MT, DT)                                            \
        cltrs3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define cless_rr(XG, XS)                                                    \
        clers_rr(W(XG), W(XS))

#define cless_ld(XG, MS, DS)                                                \
        clers_ld(W(XG), W(MS), W(DS))

#define cless3rr(XD, XS, XT)                                                \
        clers3rr(W(XD), W(XS), W(XT))

#define cless3ld(XD, XS, MT, DT)                                            \
        clers3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtss_rr(XG, XS)                                                    \
        cgtrs_rr(W(XG), W(XS))

#define cgtss_ld(XG, MS, DS)                                                \
        cgtrs_ld(W(XG), W(MS), W(DS))

#define cgtss3rr(XD, XS, XT)                                                \
        cgtrs3rr(W(XD), W(XS), W(XT))

#define cgtss3ld(XD, XS, MT, DT)                                            \
        cgtrs3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

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

/* and (G = G & S), (D = S & T) if (#D != #S) */

#define andpx_rr(XG, XS)                                                    \
        andqx_rr(W(XG), W(XS))

#define andpx_ld(XG, MS, DS)                                                \
        andqx_ld(W(XG), W(MS), W(DS))

#define andpx3rr(XD, XS, XT)                                                \
        andqx3rr(W(XD), W(XS), W(XT))

#define andpx3ld(XD, XS, MT, DT)                                            \
        andqx3ld(W(XD), W(XS), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #S) */

#define annpx_rr(XG, XS)                                                    \
        annqx_rr(W(XG), W(XS))

#define annpx_ld(XG, MS, DS)                                                \
        annqx_ld(W(XG), W(MS), W(DS))

#define annpx3rr(XD, XS, XT)                                                \
        annqx3rr(W(XD), W(XS), W(XT))

#define annpx3ld(XD, XS, MT, DT)                                            \
        annqx3ld(W(XD), W(XS), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #S) */

#define orrpx_rr(XG, XS)                                                    \
        orrqx_rr(W(XG), W(XS))

#define orrpx_ld(XG, MS, DS)                                                \
        orrqx_ld(W(XG), W(MS), W(DS))

#define orrpx3rr(XD, XS, XT)                                                \
        orrqx3rr(W(XD), W(XS), W(XT))

#define orrpx3ld(XD, XS, MT, DT)                                            \
        orrqx3ld(W(XD), W(XS), W(MT), W(DT))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #S) */

#define ornpx_rr(XG, XS)                                                    \
        ornqx_rr(W(XG), W(XS))

#define ornpx_ld(XG, MS, DS)                                                \
        ornqx_ld(W(XG), W(MS), W(DS))

#define ornpx3rr(XD, XS, XT)                                                \
        ornqx3rr(W(XD), W(XS), W(XT))

#define ornpx3ld(XD, XS, MT, DT)                                            \
        ornqx3ld(W(XD), W(XS), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #S) */

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

/* add (G = G + S), (D = S + T) if (#D != #S) */

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

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subps_rr(XG, XS)                                                    \
        subqs_rr(W(XG), W(XS))

#define subps_ld(XG, MS, DS)                                                \
        subqs_ld(W(XG), W(MS), W(DS))

#define subps3rr(XD, XS, XT)                                                \
        subqs3rr(W(XD), W(XS), W(XT))

#define subps3ld(XD, XS, MT, DT)                                            \
        subqs3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

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

/* div (G = G / S), (D = S / T) if (#D != #S) */

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

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsps_rr(XG, XS, XT)                                                \
        fmsqs_rr(W(XG), W(XS), W(XT))

#define fmsps_ld(XG, XS, MT, DT)                                            \
        fmsqs_ld(W(XG), W(XS), W(MT), W(DT))

/*************   packed double-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

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

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

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

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqps_rr(XG, XS)                                                    \
        ceqqs_rr(W(XG), W(XS))

#define ceqps_ld(XG, MS, DS)                                                \
        ceqqs_ld(W(XG), W(MS), W(DS))

#define ceqps3rr(XD, XS, XT)                                                \
        ceqqs3rr(W(XD), W(XS), W(XT))

#define ceqps3ld(XD, XS, MT, DT)                                            \
        ceqqs3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cneps_rr(XG, XS)                                                    \
        cneqs_rr(W(XG), W(XS))

#define cneps_ld(XG, MS, DS)                                                \
        cneqs_ld(W(XG), W(MS), W(DS))

#define cneps3rr(XD, XS, XT)                                                \
        cneqs3rr(W(XD), W(XS), W(XT))

#define cneps3ld(XD, XS, MT, DT)                                            \
        cneqs3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltps_rr(XG, XS)                                                    \
        cltqs_rr(W(XG), W(XS))

#define cltps_ld(XG, MS, DS)                                                \
        cltqs_ld(W(XG), W(MS), W(DS))

#define cltps3rr(XD, XS, XT)                                                \
        cltqs3rr(W(XD), W(XS), W(XT))

#define cltps3ld(XD, XS, MT, DT)                                            \
        cltqs3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define cleps_rr(XG, XS)                                                    \
        cleqs_rr(W(XG), W(XS))

#define cleps_ld(XG, MS, DS)                                                \
        cleqs_ld(W(XG), W(MS), W(DS))

#define cleps3rr(XD, XS, XT)                                                \
        cleqs3rr(W(XD), W(XS), W(XT))

#define cleps3ld(XD, XS, MT, DT)                                            \
        cleqs3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtps_rr(XG, XS)                                                    \
        cgtqs_rr(W(XG), W(XS))

#define cgtps_ld(XG, MS, DS)                                                \
        cgtqs_ld(W(XG), W(MS), W(DS))

#define cgtps3rr(XD, XS, XT)                                                \
        cgtqs3rr(W(XD), W(XS), W(XT))

#define cgtps3ld(XD, XS, MT, DT)                                            \
        cgtqs3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

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

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addpx_rr(XG, XS)                                                    \
        addqx_rr(W(XG), W(XS))

#define addpx_ld(XG, MS, DS)                                                \
        addqx_ld(W(XG), W(MS), W(DS))

#define addpx3rr(XD, XS, XT)                                                \
        addqx3rr(W(XD), W(XS), W(XT))

#define addpx3ld(XD, XS, MT, DT)                                            \
        addqx3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subpx_rr(XG, XS)                                                    \
        subqx_rr(W(XG), W(XS))

#define subpx_ld(XG, MS, DS)                                                \
        subqx_ld(W(XG), W(MS), W(DS))

#define subpx3rr(XD, XS, XT)                                                \
        subqx3rr(W(XD), W(XS), W(XT))

#define subpx3ld(XD, XS, MT, DT)                                            \
        subqx3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulpx_rr(XG, XS)                                                    \
        mulqx_rr(W(XG), W(XS))

#define mulpx_ld(XG, MS, DS)                                                \
        mulqx_ld(W(XG), W(MS), W(DS))

#define mulpx3rr(XD, XS, XT)                                                \
        mulqx3rr(W(XD), W(XS), W(XT))

#define mulpx3ld(XD, XS, MT, DT)                                            \
        mulqx3ld(W(XD), W(XS), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlpx_ri(XG, IS)                                                    \
        shlqx_ri(W(XG), W(IS))

#define shlpx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlqx_ld(W(XG), W(MS), W(DS))

#define shlpx3ri(XD, XS, IT)                                                \
        shlqx3ri(W(XD), W(XS), W(IT))

#define shlpx3ld(XD, XS, MT, DT)                                            \
        shlqx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrpx_ri(XG, IS)                                                    \
        shrqx_ri(W(XG), W(IS))

#define shrpx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrqx_ld(W(XG), W(MS), W(DS))

#define shrpx3ri(XD, XS, IT)                                                \
        shrqx3ri(W(XD), W(XS), W(IT))

#define shrpx3ld(XD, XS, MT, DT)                                            \
        shrqx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrpn_ri(XG, IS)                                                    \
        shrqn_ri(W(XG), W(IS))

#define shrpn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrqn_ld(W(XG), W(MS), W(DS))

#define shrpn3ri(XD, XS, IT)                                                \
        shrqn3ri(W(XD), W(XS), W(IT))

#define shrpn3ld(XD, XS, MT, DT)                                            \
        shrqn3ld(W(XD), W(XS), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlpx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlqx_rr(W(XG), W(XS))

#define svlpx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlqx_ld(W(XG), W(MS), W(DS))

#define svlpx3rr(XD, XS, XT)                                                \
        svlqx3rr(W(XD), W(XS), W(XT))

#define svlpx3ld(XD, XS, MT, DT)                                            \
        svlqx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrpx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrqx_rr(W(XG), W(XS))

#define svrpx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrqx_ld(W(XG), W(MS), W(DS))

#define svrpx3rr(XD, XS, XT)                                                \
        svrqx3rr(W(XD), W(XS), W(XT))

#define svrpx3ld(XD, XS, MT, DT)                                            \
        svrqx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrpn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrqn_rr(W(XG), W(XS))

#define svrpn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrqn_ld(W(XG), W(MS), W(DS))

#define svrpn3rr(XD, XS, XT)                                                \
        svrqn3rr(W(XD), W(XS), W(XT))

#define svrpn3ld(XD, XS, MT, DT)                                            \
        svrqn3ld(W(XD), W(XS), W(MT), W(DT))

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

/* and (G = G & S), (D = S & T) if (#D != #S) */

#define andfx_rr(XG, XS)                                                    \
        anddx_rr(W(XG), W(XS))

#define andfx_ld(XG, MS, DS)                                                \
        anddx_ld(W(XG), W(MS), W(DS))

#define andfx3rr(XD, XS, XT)                                                \
        anddx3rr(W(XD), W(XS), W(XT))

#define andfx3ld(XD, XS, MT, DT)                                            \
        anddx3ld(W(XD), W(XS), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #S) */

#define annfx_rr(XG, XS)                                                    \
        anndx_rr(W(XG), W(XS))

#define annfx_ld(XG, MS, DS)                                                \
        anndx_ld(W(XG), W(MS), W(DS))

#define annfx3rr(XD, XS, XT)                                                \
        anndx3rr(W(XD), W(XS), W(XT))

#define annfx3ld(XD, XS, MT, DT)                                            \
        anndx3ld(W(XD), W(XS), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #S) */

#define orrfx_rr(XG, XS)                                                    \
        orrdx_rr(W(XG), W(XS))

#define orrfx_ld(XG, MS, DS)                                                \
        orrdx_ld(W(XG), W(MS), W(DS))

#define orrfx3rr(XD, XS, XT)                                                \
        orrdx3rr(W(XD), W(XS), W(XT))

#define orrfx3ld(XD, XS, MT, DT)                                            \
        orrdx3ld(W(XD), W(XS), W(MT), W(DT))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #S) */

#define ornfx_rr(XG, XS)                                                    \
        orndx_rr(W(XG), W(XS))

#define ornfx_ld(XG, MS, DS)                                                \
        orndx_ld(W(XG), W(MS), W(DS))

#define ornfx3rr(XD, XS, XT)                                                \
        orndx3rr(W(XD), W(XS), W(XT))

#define ornfx3ld(XD, XS, MT, DT)                                            \
        orndx3ld(W(XD), W(XS), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #S) */

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

/* add (G = G + S), (D = S + T) if (#D != #S) */

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

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subfs_rr(XG, XS)                                                    \
        subds_rr(W(XG), W(XS))

#define subfs_ld(XG, MS, DS)                                                \
        subds_ld(W(XG), W(MS), W(DS))

#define subfs3rr(XD, XS, XT)                                                \
        subds3rr(W(XD), W(XS), W(XT))

#define subfs3ld(XD, XS, MT, DT)                                            \
        subds3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

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

/* div (G = G / S), (D = S / T) if (#D != #S) */

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

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsfs_rr(XG, XS, XT)                                                \
        fmsds_rr(W(XG), W(XS), W(XT))

#define fmsfs_ld(XG, XS, MT, DT)                                            \
        fmsds_ld(W(XG), W(XS), W(MT), W(DT))

/*************   packed double-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

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

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

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

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqfs_rr(XG, XS)                                                    \
        ceqds_rr(W(XG), W(XS))

#define ceqfs_ld(XG, MS, DS)                                                \
        ceqds_ld(W(XG), W(MS), W(DS))

#define ceqfs3rr(XD, XS, XT)                                                \
        ceqds3rr(W(XD), W(XS), W(XT))

#define ceqfs3ld(XD, XS, MT, DT)                                            \
        ceqds3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cnefs_rr(XG, XS)                                                    \
        cneds_rr(W(XG), W(XS))

#define cnefs_ld(XG, MS, DS)                                                \
        cneds_ld(W(XG), W(MS), W(DS))

#define cnefs3rr(XD, XS, XT)                                                \
        cneds3rr(W(XD), W(XS), W(XT))

#define cnefs3ld(XD, XS, MT, DT)                                            \
        cneds3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltfs_rr(XG, XS)                                                    \
        cltds_rr(W(XG), W(XS))

#define cltfs_ld(XG, MS, DS)                                                \
        cltds_ld(W(XG), W(MS), W(DS))

#define cltfs3rr(XD, XS, XT)                                                \
        cltds3rr(W(XD), W(XS), W(XT))

#define cltfs3ld(XD, XS, MT, DT)                                            \
        cltds3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define clefs_rr(XG, XS)                                                    \
        cleds_rr(W(XG), W(XS))

#define clefs_ld(XG, MS, DS)                                                \
        cleds_ld(W(XG), W(MS), W(DS))

#define clefs3rr(XD, XS, XT)                                                \
        cleds3rr(W(XD), W(XS), W(XT))

#define clefs3ld(XD, XS, MT, DT)                                            \
        cleds3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtfs_rr(XG, XS)                                                    \
        cgtds_rr(W(XG), W(XS))

#define cgtfs_ld(XG, MS, DS)                                                \
        cgtds_ld(W(XG), W(MS), W(DS))

#define cgtfs3rr(XD, XS, XT)                                                \
        cgtds3rr(W(XD), W(XS), W(XT))

#define cgtfs3ld(XD, XS, MT, DT)                                            \
        cgtds3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

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

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addfx_rr(XG, XS)                                                    \
        adddx_rr(W(XG), W(XS))

#define addfx_ld(XG, MS, DS)                                                \
        adddx_ld(W(XG), W(MS), W(DS))

#define addfx3rr(XD, XS, XT)                                                \
        adddx3rr(W(XD), W(XS), W(XT))

#define addfx3ld(XD, XS, MT, DT)                                            \
        adddx3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subfx_rr(XG, XS)                                                    \
        subdx_rr(W(XG), W(XS))

#define subfx_ld(XG, MS, DS)                                                \
        subdx_ld(W(XG), W(MS), W(DS))

#define subfx3rr(XD, XS, XT)                                                \
        subdx3rr(W(XD), W(XS), W(XT))

#define subfx3ld(XD, XS, MT, DT)                                            \
        subdx3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulfx_rr(XG, XS)                                                    \
        muldx_rr(W(XG), W(XS))

#define mulfx_ld(XG, MS, DS)                                                \
        muldx_ld(W(XG), W(MS), W(DS))

#define mulfx3rr(XD, XS, XT)                                                \
        muldx3rr(W(XD), W(XS), W(XT))

#define mulfx3ld(XD, XS, MT, DT)                                            \
        muldx3ld(W(XD), W(XS), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlfx_ri(XG, IS)                                                    \
        shldx_ri(W(XG), W(IS))

#define shlfx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shldx_ld(W(XG), W(MS), W(DS))

#define shlfx3ri(XD, XS, IT)                                                \
        shldx3ri(W(XD), W(XS), W(IT))

#define shlfx3ld(XD, XS, MT, DT)                                            \
        shldx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrfx_ri(XG, IS)                                                    \
        shrdx_ri(W(XG), W(IS))

#define shrfx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrdx_ld(W(XG), W(MS), W(DS))

#define shrfx3ri(XD, XS, IT)                                                \
        shrdx3ri(W(XD), W(XS), W(IT))

#define shrfx3ld(XD, XS, MT, DT)                                            \
        shrdx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrfn_ri(XG, IS)                                                    \
        shrdn_ri(W(XG), W(IS))

#define shrfn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrdn_ld(W(XG), W(MS), W(DS))

#define shrfn3ri(XD, XS, IT)                                                \
        shrdn3ri(W(XD), W(XS), W(IT))

#define shrfn3ld(XD, XS, MT, DT)                                            \
        shrdn3ld(W(XD), W(XS), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlfx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svldx_rr(W(XG), W(XS))

#define svlfx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svldx_ld(W(XG), W(MS), W(DS))

#define svlfx3rr(XD, XS, XT)                                                \
        svldx3rr(W(XD), W(XS), W(XT))

#define svlfx3ld(XD, XS, MT, DT)                                            \
        svldx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrfx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrdx_rr(W(XG), W(XS))

#define svrfx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrdx_ld(W(XG), W(MS), W(DS))

#define svrfx3rr(XD, XS, XT)                                                \
        svrdx3rr(W(XD), W(XS), W(XT))

#define svrfx3ld(XD, XS, MT, DT)                                            \
        svrdx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrfn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrdn_rr(W(XG), W(XS))

#define svrfn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrdn_ld(W(XG), W(MS), W(DS))

#define svrfn3rr(XD, XS, XT)                                                \
        svrdn3rr(W(XD), W(XS), W(XT))

#define svrfn3ld(XD, XS, MT, DT)                                            \
        svrdn3ld(W(XD), W(XS), W(MT), W(DT))

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

/* and (G = G & S), (D = S & T) if (#D != #S) */

#define andlx_rr(XG, XS)                                                    \
        andjx_rr(W(XG), W(XS))

#define andlx_ld(XG, MS, DS)                                                \
        andjx_ld(W(XG), W(MS), W(DS))

#define andlx3rr(XD, XS, XT)                                                \
        andjx3rr(W(XD), W(XS), W(XT))

#define andlx3ld(XD, XS, MT, DT)                                            \
        andjx3ld(W(XD), W(XS), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #S) */

#define annlx_rr(XG, XS)                                                    \
        annjx_rr(W(XG), W(XS))

#define annlx_ld(XG, MS, DS)                                                \
        annjx_ld(W(XG), W(MS), W(DS))

#define annlx3rr(XD, XS, XT)                                                \
        annjx3rr(W(XD), W(XS), W(XT))

#define annlx3ld(XD, XS, MT, DT)                                            \
        annjx3ld(W(XD), W(XS), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #S) */

#define orrlx_rr(XG, XS)                                                    \
        orrjx_rr(W(XG), W(XS))

#define orrlx_ld(XG, MS, DS)                                                \
        orrjx_ld(W(XG), W(MS), W(DS))

#define orrlx3rr(XD, XS, XT)                                                \
        orrjx3rr(W(XD), W(XS), W(XT))

#define orrlx3ld(XD, XS, MT, DT)                                            \
        orrjx3ld(W(XD), W(XS), W(MT), W(DT))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #S) */

#define ornlx_rr(XG, XS)                                                    \
        ornjx_rr(W(XG), W(XS))

#define ornlx_ld(XG, MS, DS)                                                \
        ornjx_ld(W(XG), W(MS), W(DS))

#define ornlx3rr(XD, XS, XT)                                                \
        ornjx3rr(W(XD), W(XS), W(XT))

#define ornlx3ld(XD, XS, MT, DT)                                            \
        ornjx3ld(W(XD), W(XS), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #S) */

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

/* add (G = G + S), (D = S + T) if (#D != #S) */

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

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subls_rr(XG, XS)                                                    \
        subjs_rr(W(XG), W(XS))

#define subls_ld(XG, MS, DS)                                                \
        subjs_ld(W(XG), W(MS), W(DS))

#define subls3rr(XD, XS, XT)                                                \
        subjs3rr(W(XD), W(XS), W(XT))

#define subls3ld(XD, XS, MT, DT)                                            \
        subjs3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

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

/* div (G = G / S), (D = S / T) if (#D != #S) */

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

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsls_rr(XG, XS, XT)                                                \
        fmsjs_rr(W(XG), W(XS), W(XT))

#define fmsls_ld(XG, XS, MT, DT)                                            \
        fmsjs_ld(W(XG), W(XS), W(MT), W(DT))

/*************   packed double-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

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

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

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

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqls_rr(XG, XS)                                                    \
        ceqjs_rr(W(XG), W(XS))

#define ceqls_ld(XG, MS, DS)                                                \
        ceqjs_ld(W(XG), W(MS), W(DS))

#define ceqls3rr(XD, XS, XT)                                                \
        ceqjs3rr(W(XD), W(XS), W(XT))

#define ceqls3ld(XD, XS, MT, DT)                                            \
        ceqjs3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cnels_rr(XG, XS)                                                    \
        cnejs_rr(W(XG), W(XS))

#define cnels_ld(XG, MS, DS)                                                \
        cnejs_ld(W(XG), W(MS), W(DS))

#define cnels3rr(XD, XS, XT)                                                \
        cnejs3rr(W(XD), W(XS), W(XT))

#define cnels3ld(XD, XS, MT, DT)                                            \
        cnejs3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltls_rr(XG, XS)                                                    \
        cltjs_rr(W(XG), W(XS))

#define cltls_ld(XG, MS, DS)                                                \
        cltjs_ld(W(XG), W(MS), W(DS))

#define cltls3rr(XD, XS, XT)                                                \
        cltjs3rr(W(XD), W(XS), W(XT))

#define cltls3ld(XD, XS, MT, DT)                                            \
        cltjs3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define clels_rr(XG, XS)                                                    \
        clejs_rr(W(XG), W(XS))

#define clels_ld(XG, MS, DS)                                                \
        clejs_ld(W(XG), W(MS), W(DS))

#define clels3rr(XD, XS, XT)                                                \
        clejs3rr(W(XD), W(XS), W(XT))

#define clels3ld(XD, XS, MT, DT)                                            \
        clejs3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtls_rr(XG, XS)                                                    \
        cgtjs_rr(W(XG), W(XS))

#define cgtls_ld(XG, MS, DS)                                                \
        cgtjs_ld(W(XG), W(MS), W(DS))

#define cgtls3rr(XD, XS, XT)                                                \
        cgtjs3rr(W(XD), W(XS), W(XT))

#define cgtls3ld(XD, XS, MT, DT)                                            \
        cgtjs3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

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

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addlx_rr(XG, XS)                                                    \
        addjx_rr(W(XG), W(XS))

#define addlx_ld(XG, MS, DS)                                                \
        addjx_ld(W(XG), W(MS), W(DS))

#define addlx3rr(XD, XS, XT)                                                \
        addjx3rr(W(XD), W(XS), W(XT))

#define addlx3ld(XD, XS, MT, DT)                                            \
        addjx3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define sublx_rr(XG, XS)                                                    \
        subjx_rr(W(XG), W(XS))

#define sublx_ld(XG, MS, DS)                                                \
        subjx_ld(W(XG), W(MS), W(DS))

#define sublx3rr(XD, XS, XT)                                                \
        subjx3rr(W(XD), W(XS), W(XT))

#define sublx3ld(XD, XS, MT, DT)                                            \
        subjx3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mullx_rr(XG, XS)                                                    \
        muljx_rr(W(XG), W(XS))

#define mullx_ld(XG, MS, DS)                                                \
        muljx_ld(W(XG), W(MS), W(DS))

#define mullx3rr(XD, XS, XT)                                                \
        muljx3rr(W(XD), W(XS), W(XT))

#define mullx3ld(XD, XS, MT, DT)                                            \
        muljx3ld(W(XD), W(XS), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shllx_ri(XG, IS)                                                    \
        shljx_ri(W(XG), W(IS))

#define shllx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shljx_ld(W(XG), W(MS), W(DS))

#define shllx3ri(XD, XS, IT)                                                \
        shljx3ri(W(XD), W(XS), W(IT))

#define shllx3ld(XD, XS, MT, DT)                                            \
        shljx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrlx_ri(XG, IS)                                                    \
        shrjx_ri(W(XG), W(IS))

#define shrlx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrjx_ld(W(XG), W(MS), W(DS))

#define shrlx3ri(XD, XS, IT)                                                \
        shrjx3ri(W(XD), W(XS), W(IT))

#define shrlx3ld(XD, XS, MT, DT)                                            \
        shrjx3ld(W(XD), W(XS), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrln_ri(XG, IS)                                                    \
        shrjn_ri(W(XG), W(IS))

#define shrln_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrjn_ld(W(XG), W(MS), W(DS))

#define shrln3ri(XD, XS, IT)                                                \
        shrjn3ri(W(XD), W(XS), W(IT))

#define shrln3ld(XD, XS, MT, DT)                                            \
        shrjn3ld(W(XD), W(XS), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svllx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svljx_rr(W(XG), W(XS))

#define svllx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svljx_ld(W(XG), W(MS), W(DS))

#define svllx3rr(XD, XS, XT)                                                \
        svljx3rr(W(XD), W(XS), W(XT))

#define svllx3ld(XD, XS, MT, DT)                                            \
        svljx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrlx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrjx_rr(W(XG), W(XS))

#define svrlx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrjx_ld(W(XG), W(MS), W(DS))

#define svrlx3rr(XD, XS, XT)                                                \
        svrjx3rr(W(XD), W(XS), W(XT))

#define svrlx3ld(XD, XS, MT, DT)                                            \
        svrjx3ld(W(XD), W(XS), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrln_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrjn_rr(W(XG), W(XS))

#define svrln_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrjn_ld(W(XG), W(MS), W(DS))

#define svrln3rr(XD, XS, XT)                                                \
        svrjn3rr(W(XD), W(XS), W(XT))

#define svrln3ld(XD, XS, MT, DT)                                            \
        svrjn3ld(W(XD), W(XS), W(MT), W(DT))

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

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addss_rr(XG, XS)                                                    \
        addts_rr(W(XG), W(XS))

#define addss_ld(XG, MS, DS)                                                \
        addts_ld(W(XG), W(MS), W(DS))

#define addss3rr(XD, XS, XT)                                                \
        addts3rr(W(XD), W(XS), W(XT))

#define addss3ld(XD, XS, MT, DT)                                            \
        addts3ld(W(XD), W(XS), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subss_rr(XG, XS)                                                    \
        subts_rr(W(XG), W(XS))

#define subss_ld(XG, MS, DS)                                                \
        subts_ld(W(XG), W(MS), W(DS))

#define subss3rr(XD, XS, XT)                                                \
        subts3rr(W(XD), W(XS), W(XT))

#define subss3ld(XD, XS, MT, DT)                                            \
        subts3ld(W(XD), W(XS), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulss_rr(XG, XS)                                                    \
        mults_rr(W(XG), W(XS))

#define mulss_ld(XG, MS, DS)                                                \
        mults_ld(W(XG), W(MS), W(DS))

#define mulss3rr(XD, XS, XT)                                                \
        mults3rr(W(XD), W(XS), W(XT))

#define mulss3ld(XD, XS, MT, DT)                                            \
        mults3ld(W(XD), W(XS), W(MT), W(DT))

/* div (G = G / S), (D = S / T) if (#D != #S) */

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

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsss_rr(XG, XS, XT)                                                \
        fmsts_rr(W(XG), W(XS), W(XT))

#define fmsss_ld(XG, XS, MT, DT)                                            \
        fmsts_ld(W(XG), W(XS), W(MT), W(DT))

/*************   scalar double-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

#define minss_rr(XG, XS)                                                    \
        mints_rr(W(XG), W(XS))

#define minss_ld(XG, MS, DS)                                                \
        mints_ld(W(XG), W(MS), W(DS))

#define minss3rr(XD, XS, XT)                                                \
        mints3rr(W(XD), W(XS), W(XT))

#define minss3ld(XD, XS, MT, DT)                                            \
        mints3ld(W(XD), W(XS), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

#define maxss_rr(XG, XS)                                                    \
        maxts_rr(W(XG), W(XS))

#define maxss_ld(XG, MS, DS)                                                \
        maxts_ld(W(XG), W(MS), W(DS))

#define maxss3rr(XD, XS, XT)                                                \
        maxts3rr(W(XD), W(XS), W(XT))

#define maxss3ld(XD, XS, MT, DT)                                            \
        maxts3ld(W(XD), W(XS), W(MT), W(DT))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqss_rr(XG, XS)                                                    \
        ceqts_rr(W(XG), W(XS))

#define ceqss_ld(XG, MS, DS)                                                \
        ceqts_ld(W(XG), W(MS), W(DS))

#define ceqss3rr(XD, XS, XT)                                                \
        ceqts3rr(W(XD), W(XS), W(XT))

#define ceqss3ld(XD, XS, MT, DT)                                            \
        ceqts3ld(W(XD), W(XS), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cness_rr(XG, XS)                                                    \
        cnets_rr(W(XG), W(XS))

#define cness_ld(XG, MS, DS)                                                \
        cnets_ld(W(XG), W(MS), W(DS))

#define cness3rr(XD, XS, XT)                                                \
        cnets3rr(W(XD), W(XS), W(XT))

#define cness3ld(XD, XS, MT, DT)                                            \
        cnets3ld(W(XD), W(XS), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltss_rr(XG, XS)                                                    \
        cltts_rr(W(XG), W(XS))

#define cltss_ld(XG, MS, DS)                                                \
        cltts_ld(W(XG), W(MS), W(DS))

#define cltss3rr(XD, XS, XT)                                                \
        cltts3rr(W(XD), W(XS), W(XT))

#define cltss3ld(XD, XS, MT, DT)                                            \
        cltts3ld(W(XD), W(XS), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define cless_rr(XG, XS)                                                    \
        clets_rr(W(XG), W(XS))

#define cless_ld(XG, MS, DS)                                                \
        clets_ld(W(XG), W(MS), W(DS))

#define cless3rr(XD, XS, XT)                                                \
        clets3rr(W(XD), W(XS), W(XT))

#define cless3ld(XD, XS, MT, DT)                                            \
        clets3ld(W(XD), W(XS), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtss_rr(XG, XS)                                                    \
        cgtts_rr(W(XG), W(XS))

#define cgtss_ld(XG, MS, DS)                                                \
        cgtts_ld(W(XG), W(MS), W(DS))

#define cgtss3rr(XD, XS, XT)                                                \
        cgtts3rr(W(XD), W(XS), W(XT))

#define cgtss3ld(XD, XS, MT, DT)                                            \
        cgtts3ld(W(XD), W(XS), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

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
 * Setting-flags instruction naming scheme may change again in the future for
 * better orthogonality with operand size, type and args-list. It is therefore
 * recommended to use combined-arithmetic-jump (arj) for better API stability
 * and maximum efficiency across all supported targets. For similar reasons
 * of higher performance on MIPS and POWER use combined-compare-jump (cmj).
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
 * set-flags: undefined (*x), yes (*z) */

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


#define andxz_ri(RG, IS)                                                    \
        andwz_ri(W(RG), W(IS))

#define andxz_mi(MG, DG, IS)                                                \
        andwz_mi(W(MG), W(DG), W(IS))

#define andxz_rr(RG, RS)                                                    \
        andwz_rr(W(RG), W(RS))

#define andxz_ld(RG, MS, DS)                                                \
        andwz_ld(W(RG), W(MS), W(DS))

#define andxz_st(RS, MG, DG)                                                \
        andwz_st(W(RS), W(MG), W(DG))

#define andxz_mr(MG, DG, RS)                                                \
        andwz_mr(W(MG), W(DG), W(RS))

/* ann (G = ~G & S)
 * set-flags: undefined (*x), yes (*z) */

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


#define annxz_ri(RG, IS)                                                    \
        annwz_ri(W(RG), W(IS))

#define annxz_mi(MG, DG, IS)                                                \
        annwz_mi(W(MG), W(DG), W(IS))

#define annxz_rr(RG, RS)                                                    \
        annwz_rr(W(RG), W(RS))

#define annxz_ld(RG, MS, DS)                                                \
        annwz_ld(W(RG), W(MS), W(DS))

#define annxz_st(RS, MG, DG)                                                \
        annwz_st(W(RS), W(MG), W(DG))

#define annxz_mr(MG, DG, RS)                                                \
        annwz_mr(W(MG), W(DG), W(RS))

/* orr (G = G | S)
 * set-flags: undefined (*x), yes (*z) */

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


#define orrxz_ri(RG, IS)                                                    \
        orrwz_ri(W(RG), W(IS))

#define orrxz_mi(MG, DG, IS)                                                \
        orrwz_mi(W(MG), W(DG), W(IS))

#define orrxz_rr(RG, RS)                                                    \
        orrwz_rr(W(RG), W(RS))

#define orrxz_ld(RG, MS, DS)                                                \
        orrwz_ld(W(RG), W(MS), W(DS))

#define orrxz_st(RS, MG, DG)                                                \
        orrwz_st(W(RS), W(MG), W(DG))

#define orrxz_mr(MG, DG, RS)                                                \
        orrwz_mr(W(MG), W(DG), W(RS))

/* orn (G = ~G | S)
 * set-flags: undefined (*x), yes (*z) */

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


#define ornxz_ri(RG, IS)                                                    \
        ornwz_ri(W(RG), W(IS))

#define ornxz_mi(MG, DG, IS)                                                \
        ornwz_mi(W(MG), W(DG), W(IS))

#define ornxz_rr(RG, RS)                                                    \
        ornwz_rr(W(RG), W(RS))

#define ornxz_ld(RG, MS, DS)                                                \
        ornwz_ld(W(RG), W(MS), W(DS))

#define ornxz_st(RS, MG, DG)                                                \
        ornwz_st(W(RS), W(MG), W(DG))

#define ornxz_mr(MG, DG, RS)                                                \
        ornwz_mr(W(MG), W(DG), W(RS))

/* xor (G = G ^ S)
 * set-flags: undefined (*x), yes (*z) */

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


#define xorxz_ri(RG, IS)                                                    \
        xorwz_ri(W(RG), W(IS))

#define xorxz_mi(MG, DG, IS)                                                \
        xorwz_mi(W(MG), W(DG), W(IS))

#define xorxz_rr(RG, RS)                                                    \
        xorwz_rr(W(RG), W(RS))

#define xorxz_ld(RG, MS, DS)                                                \
        xorwz_ld(W(RG), W(MS), W(DS))

#define xorxz_st(RS, MG, DG)                                                \
        xorwz_st(W(RS), W(MG), W(DG))

#define xorxz_mr(MG, DG, RS)                                                \
        xorwz_mr(W(MG), W(DG), W(RS))

/* not (G = ~G)
 * set-flags: no */

#define notxx_rx(RG)                                                        \
        notwx_rx(W(RG))

#define notxx_mx(MG, DG)                                                    \
        notwx_mx(W(MG), W(DG))

/* neg (G = -G)
 * set-flags: undefined (*x), yes (*z) */

#define negxx_rx(RG)                                                        \
        negwx_rx(W(RG))

#define negxx_mx(MG, DG)                                                    \
        negwx_mx(W(MG), W(DG))


#define negxz_rx(RG)                                                        \
        negwz_rx(W(RG))

#define negxz_mx(MG, DG)                                                    \
        negwz_mx(W(MG), W(DG))

/* add (G = G + S)
 * set-flags: undefined (*x), yes (*z) */

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


#define addxz_ri(RG, IS)                                                    \
        addwz_ri(W(RG), W(IS))

#define addxz_mi(MG, DG, IS)                                                \
        addwz_mi(W(MG), W(DG), W(IS))

#define addxz_rr(RG, RS)                                                    \
        addwz_rr(W(RG), W(RS))

#define addxz_ld(RG, MS, DS)                                                \
        addwz_ld(W(RG), W(MS), W(DS))

#define addxz_st(RS, MG, DG)                                                \
        addwz_st(W(RS), W(MG), W(DG))

#define addxz_mr(MG, DG, RS)                                                \
        addwz_mr(W(MG), W(DG), W(RS))

/* sub (G = G - S)
 * set-flags: undefined (*x), yes (*z) */

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


#define subxz_ri(RG, IS)                                                    \
        subwz_ri(W(RG), W(IS))

#define subxz_mi(MG, DG, IS)                                                \
        subwz_mi(W(MG), W(DG), W(IS))

#define subxz_rr(RG, RS)                                                    \
        subwz_rr(W(RG), W(RS))

#define subxz_ld(RG, MS, DS)                                                \
        subwz_ld(W(RG), W(MS), W(DS))

#define subxz_st(RS, MG, DG)                                                \
        subwz_st(W(RS), W(MG), W(DG))

#define subxz_mr(MG, DG, RS)                                                \
        subwz_mr(W(MG), W(DG), W(RS))

/* shl (G = G << S)
 * set-flags: undefined (*x), yes (*z)
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


#define shlxz_rx(RG)                     /* reads Recx for shift count */   \
        shlwz_rx(W(RG))

#define shlxz_mx(MG, DG)                 /* reads Recx for shift count */   \
        shlwz_mx(W(MG), W(DG))

#define shlxz_ri(RG, IS)                                                    \
        shlwz_ri(W(RG), W(IS))

#define shlxz_mi(MG, DG, IS)                                                \
        shlwz_mi(W(MG), W(DG), W(IS))

#define shlxz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shlwz_rr(W(RG), W(RS))

#define shlxz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shlwz_ld(W(RG), W(MS), W(DS))

#define shlxz_st(RS, MG, DG)                                                \
        shlwz_st(W(RS), W(MG), W(DG))

#define shlxz_mr(MG, DG, RS)                                                \
        shlwz_mr(W(MG), W(DG), W(RS))

/* shr (G = G >> S)
 * set-flags: undefined (*x), yes (*z)
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


#define shrxz_rx(RG)                     /* reads Recx for shift count */   \
        shrwz_rx(W(RG))

#define shrxz_mx(MG, DG)                 /* reads Recx for shift count */   \
        shrwz_mx(W(MG), W(DG))

#define shrxz_ri(RG, IS)                                                    \
        shrwz_ri(W(RG), W(IS))

#define shrxz_mi(MG, DG, IS)                                                \
        shrwz_mi(W(MG), W(DG), W(IS))

#define shrxz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrwz_rr(W(RG), W(RS))

#define shrxz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shrwz_ld(W(RG), W(MS), W(DS))

#define shrxz_st(RS, MG, DG)                                                \
        shrwz_st(W(RS), W(MG), W(DG))

#define shrxz_mr(MG, DG, RS)                                                \
        shrwz_mr(W(MG), W(DG), W(RS))


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

/* ror (G = G >> S | G << 32 - S)
 * set-flags: undefined (*x), yes (*z)
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


#define rorxz_rx(RG)                     /* reads Recx for shift count */   \
        rorwz_rx(W(RG))

#define rorxz_mx(MG, DG)                 /* reads Recx for shift count */   \
        rorwz_mx(W(MG), W(DG))

#define rorxz_ri(RG, IS)                                                    \
        rorwz_ri(W(RG), W(IS))

#define rorxz_mi(MG, DG, IS)                                                \
        rorwz_mi(W(MG), W(DG), W(IS))

#define rorxz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        rorwz_rr(W(RG), W(RS))

#define rorxz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        rorwz_ld(W(RG), W(MS), W(DS))

#define rorxz_st(RS, MG, DG)                                                \
        rorwz_st(W(RS), W(MG), W(DG))

#define rorxz_mr(MG, DG, RS)                                                \
        rorwz_mr(W(MG), W(DG), W(RS))

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


#define prexx_xx()          /* to be placed immediately prior divxx_x* */   \
        prewx_xx()                   /* to prepare Redx for int-divide */

#define prexn_xx()          /* to be placed immediately prior divxn_x* */   \
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


#define remxx_xx()          /* to be placed immediately prior divxx_x* */   \
        remwx_xx()                   /* to prepare for rem calculation */

#define remxx_xr(RS)        /* to be placed immediately after divxx_xr */   \
        remwx_xr(W(RS))              /* to produce remainder Redx<-rem */

#define remxx_xm(MS, DS)    /* to be placed immediately after divxx_xm */   \
        remwx_xm(W(MS), W(DS))       /* to produce remainder Redx<-rem */


#define remxn_xx()          /* to be placed immediately prior divxn_x* */   \
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
 * set-flags: undefined (*x), yes (*z) */

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


#define andxz_ri(RG, IS)                                                    \
        andzz_ri(W(RG), W(IS))

#define andxz_mi(MG, DG, IS)                                                \
        andzz_mi(W(MG), W(DG), W(IS))

#define andxz_rr(RG, RS)                                                    \
        andzz_rr(W(RG), W(RS))

#define andxz_ld(RG, MS, DS)                                                \
        andzz_ld(W(RG), W(MS), W(DS))

#define andxz_st(RS, MG, DG)                                                \
        andzz_st(W(RS), W(MG), W(DG))

#define andxz_mr(MG, DG, RS)                                                \
        andzz_mr(W(MG), W(DG), W(RS))

/* ann (G = ~G & S)
 * set-flags: undefined (*x), yes (*z) */

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


#define annxz_ri(RG, IS)                                                    \
        annzz_ri(W(RG), W(IS))

#define annxz_mi(MG, DG, IS)                                                \
        annzz_mi(W(MG), W(DG), W(IS))

#define annxz_rr(RG, RS)                                                    \
        annzz_rr(W(RG), W(RS))

#define annxz_ld(RG, MS, DS)                                                \
        annzz_ld(W(RG), W(MS), W(DS))

#define annxz_st(RS, MG, DG)                                                \
        annzz_st(W(RS), W(MG), W(DG))

#define annxz_mr(MG, DG, RS)                                                \
        annzz_mr(W(MG), W(DG), W(RS))

/* orr (G = G | S)
 * set-flags: undefined (*x), yes (*z) */

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


#define orrxz_ri(RG, IS)                                                    \
        orrzz_ri(W(RG), W(IS))

#define orrxz_mi(MG, DG, IS)                                                \
        orrzz_mi(W(MG), W(DG), W(IS))

#define orrxz_rr(RG, RS)                                                    \
        orrzz_rr(W(RG), W(RS))

#define orrxz_ld(RG, MS, DS)                                                \
        orrzz_ld(W(RG), W(MS), W(DS))

#define orrxz_st(RS, MG, DG)                                                \
        orrzz_st(W(RS), W(MG), W(DG))

#define orrxz_mr(MG, DG, RS)                                                \
        orrzz_mr(W(MG), W(DG), W(RS))

/* orn (G = ~G | S)
 * set-flags: undefined (*x), yes (*z) */

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


#define ornxz_ri(RG, IS)                                                    \
        ornzz_ri(W(RG), W(IS))

#define ornxz_mi(MG, DG, IS)                                                \
        ornzz_mi(W(MG), W(DG), W(IS))

#define ornxz_rr(RG, RS)                                                    \
        ornzz_rr(W(RG), W(RS))

#define ornxz_ld(RG, MS, DS)                                                \
        ornzz_ld(W(RG), W(MS), W(DS))

#define ornxz_st(RS, MG, DG)                                                \
        ornzz_st(W(RS), W(MG), W(DG))

#define ornxz_mr(MG, DG, RS)                                                \
        ornzz_mr(W(MG), W(DG), W(RS))

/* xor (G = G ^ S)
 * set-flags: undefined (*x), yes (*z) */

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


#define xorxz_ri(RG, IS)                                                    \
        xorzz_ri(W(RG), W(IS))

#define xorxz_mi(MG, DG, IS)                                                \
        xorzz_mi(W(MG), W(DG), W(IS))

#define xorxz_rr(RG, RS)                                                    \
        xorzz_rr(W(RG), W(RS))

#define xorxz_ld(RG, MS, DS)                                                \
        xorzz_ld(W(RG), W(MS), W(DS))

#define xorxz_st(RS, MG, DG)                                                \
        xorzz_st(W(RS), W(MG), W(DG))

#define xorxz_mr(MG, DG, RS)                                                \
        xorzz_mr(W(MG), W(DG), W(RS))

/* not (G = ~G)
 * set-flags: no */

#define notxx_rx(RG)                                                        \
        notzx_rx(W(RG))

#define notxx_mx(MG, DG)                                                    \
        notzx_mx(W(MG), W(DG))

/* neg (G = -G)
 * set-flags: undefined (*x), yes (*z) */

#define negxx_rx(RG)                                                        \
        negzx_rx(W(RG))

#define negxx_mx(MG, DG)                                                    \
        negzx_mx(W(MG), W(DG))


#define negxz_rx(RG)                                                        \
        negzz_rx(W(RG))

#define negxz_mx(MG, DG)                                                    \
        negzz_mx(W(MG), W(DG))

/* add (G = G + S)
 * set-flags: undefined (*x), yes (*z) */

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


#define addxz_ri(RG, IS)                                                    \
        addzz_ri(W(RG), W(IS))

#define addxz_mi(MG, DG, IS)                                                \
        addzz_mi(W(MG), W(DG), W(IS))

#define addxz_rr(RG, RS)                                                    \
        addzz_rr(W(RG), W(RS))

#define addxz_ld(RG, MS, DS)                                                \
        addzz_ld(W(RG), W(MS), W(DS))

#define addxz_st(RS, MG, DG)                                                \
        addzz_st(W(RS), W(MG), W(DG))

#define addxz_mr(MG, DG, RS)                                                \
        addzz_mr(W(MG), W(DG), W(RS))

/* sub (G = G - S)
 * set-flags: undefined (*x), yes (*z) */

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


#define subxz_ri(RG, IS)                                                    \
        subzz_ri(W(RG), W(IS))

#define subxz_mi(MG, DG, IS)                                                \
        subzz_mi(W(MG), W(DG), W(IS))

#define subxz_rr(RG, RS)                                                    \
        subzz_rr(W(RG), W(RS))

#define subxz_ld(RG, MS, DS)                                                \
        subzz_ld(W(RG), W(MS), W(DS))

#define subxz_st(RS, MG, DG)                                                \
        subzz_st(W(RS), W(MG), W(DG))

#define subxz_mr(MG, DG, RS)                                                \
        subzz_mr(W(MG), W(DG), W(RS))

/* shl (G = G << S)
 * set-flags: undefined (*x), yes (*z)
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


#define shlxz_rx(RG)                     /* reads Recx for shift count */   \
        shlzz_rx(W(RG))

#define shlxz_mx(MG, DG)                 /* reads Recx for shift count */   \
        shlzz_mx(W(MG), W(DG))

#define shlxz_ri(RG, IS)                                                    \
        shlzz_ri(W(RG), W(IS))

#define shlxz_mi(MG, DG, IS)                                                \
        shlzz_mi(W(MG), W(DG), W(IS))

#define shlxz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shlzz_rr(W(RG), W(RS))

#define shlxz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shlzz_ld(W(RG), W(MS), W(DS))

#define shlxz_st(RS, MG, DG)                                                \
        shlzz_st(W(RS), W(MG), W(DG))

#define shlxz_mr(MG, DG, RS)                                                \
        shlzz_mr(W(MG), W(DG), W(RS))

/* shr (G = G >> S)
 * set-flags: undefined (*x), yes (*z)
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


#define shrxz_rx(RG)                     /* reads Recx for shift count */   \
        shrzz_rx(W(RG))

#define shrxz_mx(MG, DG)                 /* reads Recx for shift count */   \
        shrzz_mx(W(MG), W(DG))

#define shrxz_ri(RG, IS)                                                    \
        shrzz_ri(W(RG), W(IS))

#define shrxz_mi(MG, DG, IS)                                                \
        shrzz_mi(W(MG), W(DG), W(IS))

#define shrxz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrzz_rr(W(RG), W(RS))

#define shrxz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shrzz_ld(W(RG), W(MS), W(DS))

#define shrxz_st(RS, MG, DG)                                                \
        shrzz_st(W(RS), W(MG), W(DG))

#define shrxz_mr(MG, DG, RS)                                                \
        shrzz_mr(W(MG), W(DG), W(RS))


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

/* ror (G = G >> S | G << 64 - S)
 * set-flags: undefined (*x), yes (*z)
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


#define rorxz_rx(RG)                     /* reads Recx for shift count */   \
        rorzz_rx(W(RG))

#define rorxz_mx(MG, DG)                 /* reads Recx for shift count */   \
        rorzz_mx(W(MG), W(DG))

#define rorxz_ri(RG, IS)                                                    \
        rorzz_ri(W(RG), W(IS))

#define rorxz_mi(MG, DG, IS)                                                \
        rorzz_mi(W(MG), W(DG), W(IS))

#define rorxz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        rorzz_rr(W(RG), W(RS))

#define rorxz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        rorzz_ld(W(RG), W(MS), W(DS))

#define rorxz_st(RS, MG, DG)                                                \
        rorzz_st(W(RS), W(MG), W(DG))

#define rorxz_mr(MG, DG, RS)                                                \
        rorzz_mr(W(MG), W(DG), W(RS))

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


#define prexx_xx()          /* to be placed immediately prior divxx_x* */   \
        prezx_xx()                   /* to prepare Redx for int-divide */

#define prexn_xx()          /* to be placed immediately prior divxn_x* */   \
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


#define remxx_xx()          /* to be placed immediately prior divxx_x* */   \
        remzx_xx()                   /* to prepare for rem calculation */

#define remxx_xr(RS)        /* to be placed immediately after divxx_xr */   \
        remzx_xr(W(RS))              /* to produce remainder Redx<-rem */

#define remxx_xm(MS, DS)    /* to be placed immediately after divxx_xm */   \
        remzx_xm(W(MS), W(DS))       /* to produce remainder Redx<-rem */


#define remxn_xx()          /* to be placed immediately prior divxn_x* */   \
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
 * set-flags: undefined (*x), yes (*z) */

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


#define andyz_ri(RG, IS)                                                    \
        andwz_ri(W(RG), W(IS))

#define andyz_mi(MG, DG, IS)                                                \
        andwz_mi(W(MG), W(DG), W(IS))

#define andyz_rr(RG, RS)                                                    \
        andwz_rr(W(RG), W(RS))

#define andyz_ld(RG, MS, DS)                                                \
        andwz_ld(W(RG), W(MS), W(DS))

#define andyz_st(RS, MG, DG)                                                \
        andwz_st(W(RS), W(MG), W(DG))

#define andyz_mr(MG, DG, RS)                                                \
        andwz_mr(W(MG), W(DG), W(RS))

/* ann (G = ~G & S)
 * set-flags: undefined (*x), yes (*z) */

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


#define annyz_ri(RG, IS)                                                    \
        annwz_ri(W(RG), W(IS))

#define annyz_mi(MG, DG, IS)                                                \
        annwz_mi(W(MG), W(DG), W(IS))

#define annyz_rr(RG, RS)                                                    \
        annwz_rr(W(RG), W(RS))

#define annyz_ld(RG, MS, DS)                                                \
        annwz_ld(W(RG), W(MS), W(DS))

#define annyz_st(RS, MG, DG)                                                \
        annwz_st(W(RS), W(MG), W(DG))

#define annyz_mr(MG, DG, RS)                                                \
        annwz_mr(W(MG), W(DG), W(RS))

/* orr (G = G | S)
 * set-flags: undefined (*x), yes (*z) */

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


#define orryz_ri(RG, IS)                                                    \
        orrwz_ri(W(RG), W(IS))

#define orryz_mi(MG, DG, IS)                                                \
        orrwz_mi(W(MG), W(DG), W(IS))

#define orryz_rr(RG, RS)                                                    \
        orrwz_rr(W(RG), W(RS))

#define orryz_ld(RG, MS, DS)                                                \
        orrwz_ld(W(RG), W(MS), W(DS))

#define orryz_st(RS, MG, DG)                                                \
        orrwz_st(W(RS), W(MG), W(DG))

#define orryz_mr(MG, DG, RS)                                                \
        orrwz_mr(W(MG), W(DG), W(RS))

/* orn (G = ~G | S)
 * set-flags: undefined (*x), yes (*z) */

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


#define ornyz_ri(RG, IS)                                                    \
        ornwz_ri(W(RG), W(IS))

#define ornyz_mi(MG, DG, IS)                                                \
        ornwz_mi(W(MG), W(DG), W(IS))

#define ornyz_rr(RG, RS)                                                    \
        ornwz_rr(W(RG), W(RS))

#define ornyz_ld(RG, MS, DS)                                                \
        ornwz_ld(W(RG), W(MS), W(DS))

#define ornyz_st(RS, MG, DG)                                                \
        ornwz_st(W(RS), W(MG), W(DG))

#define ornyz_mr(MG, DG, RS)                                                \
        ornwz_mr(W(MG), W(DG), W(RS))

/* xor (G = G ^ S)
 * set-flags: undefined (*x), yes (*z) */

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


#define xoryz_ri(RG, IS)                                                    \
        xorwz_ri(W(RG), W(IS))

#define xoryz_mi(MG, DG, IS)                                                \
        xorwz_mi(W(MG), W(DG), W(IS))

#define xoryz_rr(RG, RS)                                                    \
        xorwz_rr(W(RG), W(RS))

#define xoryz_ld(RG, MS, DS)                                                \
        xorwz_ld(W(RG), W(MS), W(DS))

#define xoryz_st(RS, MG, DG)                                                \
        xorwz_st(W(RS), W(MG), W(DG))

#define xoryz_mr(MG, DG, RS)                                                \
        xorwz_mr(W(MG), W(DG), W(RS))

/* not (G = ~G)
 * set-flags: no */

#define notyx_rx(RG)                                                        \
        notwx_rx(W(RG))

#define notyx_mx(MG, DG)                                                    \
        notwx_mx(W(MG), W(DG))

/* neg (G = -G)
 * set-flags: undefined (*x), yes (*z) */

#define negyx_rx(RG)                                                        \
        negwx_rx(W(RG))

#define negyx_mx(MG, DG)                                                    \
        negwx_mx(W(MG), W(DG))


#define negyz_rx(RG)                                                        \
        negwz_rx(W(RG))

#define negyz_mx(MG, DG)                                                    \
        negwz_mx(W(MG), W(DG))

/* add (G = G + S)
 * set-flags: undefined (*x), yes (*z) */

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


#define addyz_ri(RG, IS)                                                    \
        addwz_ri(W(RG), W(IS))

#define addyz_mi(MG, DG, IS)                                                \
        addwz_mi(W(MG), W(DG), W(IS))

#define addyz_rr(RG, RS)                                                    \
        addwz_rr(W(RG), W(RS))

#define addyz_ld(RG, MS, DS)                                                \
        addwz_ld(W(RG), W(MS), W(DS))

#define addyz_st(RS, MG, DG)                                                \
        addwz_st(W(RS), W(MG), W(DG))

#define addyz_mr(MG, DG, RS)                                                \
        addwz_mr(W(MG), W(DG), W(RS))

/* sub (G = G - S)
 * set-flags: undefined (*x), yes (*z) */

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


#define subyz_ri(RG, IS)                                                    \
        subwz_ri(W(RG), W(IS))

#define subyz_mi(MG, DG, IS)                                                \
        subwz_mi(W(MG), W(DG), W(IS))

#define subyz_rr(RG, RS)                                                    \
        subwz_rr(W(RG), W(RS))

#define subyz_ld(RG, MS, DS)                                                \
        subwz_ld(W(RG), W(MS), W(DS))

#define subyz_st(RS, MG, DG)                                                \
        subwz_st(W(RS), W(MG), W(DG))

#define subyz_mr(MG, DG, RS)                                                \
        subwz_mr(W(MG), W(DG), W(RS))

/* shl (G = G << S)
 * set-flags: undefined (*x), yes (*z)
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


#define shlyz_rx(RG)                     /* reads Recx for shift count */   \
        shlwz_rx(W(RG))

#define shlyz_mx(MG, DG)                 /* reads Recx for shift count */   \
        shlwz_mx(W(MG), W(DG))

#define shlyz_ri(RG, IS)                                                    \
        shlwz_ri(W(RG), W(IS))

#define shlyz_mi(MG, DG, IS)                                                \
        shlwz_mi(W(MG), W(DG), W(IS))

#define shlyz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shlwz_rr(W(RG), W(RS))

#define shlyz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shlwz_ld(W(RG), W(MS), W(DS))

#define shlyz_st(RS, MG, DG)                                                \
        shlwz_st(W(RS), W(MG), W(DG))

#define shlyz_mr(MG, DG, RS)                                                \
        shlwz_mr(W(MG), W(DG), W(RS))

/* shr (G = G >> S)
 * set-flags: undefined (*x), yes (*z)
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


#define shryz_rx(RG)                     /* reads Recx for shift count */   \
        shrwz_rx(W(RG))

#define shryz_mx(MG, DG)                 /* reads Recx for shift count */   \
        shrwz_mx(W(MG), W(DG))

#define shryz_ri(RG, IS)                                                    \
        shrwz_ri(W(RG), W(IS))

#define shryz_mi(MG, DG, IS)                                                \
        shrwz_mi(W(MG), W(DG), W(IS))

#define shryz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrwz_rr(W(RG), W(RS))

#define shryz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shrwz_ld(W(RG), W(MS), W(DS))

#define shryz_st(RS, MG, DG)                                                \
        shrwz_st(W(RS), W(MG), W(DG))

#define shryz_mr(MG, DG, RS)                                                \
        shrwz_mr(W(MG), W(DG), W(RS))


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

/* ror (G = G >> S | G << 32 - S)
 * set-flags: undefined (*x), yes (*z)
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


#define roryz_rx(RG)                     /* reads Recx for shift count */   \
        rorwz_rx(W(RG))

#define roryz_mx(MG, DG)                 /* reads Recx for shift count */   \
        rorwz_mx(W(MG), W(DG))

#define roryz_ri(RG, IS)                                                    \
        rorwz_ri(W(RG), W(IS))

#define roryz_mi(MG, DG, IS)                                                \
        rorwz_mi(W(MG), W(DG), W(IS))

#define roryz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        rorwz_rr(W(RG), W(RS))

#define roryz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        rorwz_ld(W(RG), W(MS), W(DS))

#define roryz_st(RS, MG, DG)                                                \
        rorwz_st(W(RS), W(MG), W(DG))

#define roryz_mr(MG, DG, RS)                                                \
        rorwz_mr(W(MG), W(DG), W(RS))

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


#define preyx_xx()          /* to be placed immediately prior divyx_x* */   \
        prewx_xx()                   /* to prepare Redx for int-divide */

#define preyn_xx()          /* to be placed immediately prior divyn_x* */   \
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


#define remyx_xx()          /* to be placed immediately prior divyx_x* */   \
        remwx_xx()                   /* to prepare for rem calculation */

#define remyx_xr(RS)        /* to be placed immediately after divyx_xr */   \
        remwx_xr(W(RS))              /* to produce remainder Redx<-rem */

#define remyx_xm(MS, DS)    /* to be placed immediately after divyx_xm */   \
        remwx_xm(W(MS), W(DS))       /* to produce remainder Redx<-rem */


#define remyn_xx()          /* to be placed immediately prior divyn_x* */   \
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
 * set-flags: undefined (*x), yes (*z) */

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


#define andyz_ri(RG, IS)                                                    \
        andzz_ri(W(RG), W(IS))

#define andyz_mi(MG, DG, IS)                                                \
        andzz_mi(W(MG), W(DG), W(IS))

#define andyz_rr(RG, RS)                                                    \
        andzz_rr(W(RG), W(RS))

#define andyz_ld(RG, MS, DS)                                                \
        andzz_ld(W(RG), W(MS), W(DS))

#define andyz_st(RS, MG, DG)                                                \
        andzz_st(W(RS), W(MG), W(DG))

#define andyz_mr(MG, DG, RS)                                                \
        andzz_mr(W(MG), W(DG), W(RS))

/* ann (G = ~G & S)
 * set-flags: undefined (*x), yes (*z) */

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


#define annyz_ri(RG, IS)                                                    \
        annzz_ri(W(RG), W(IS))

#define annyz_mi(MG, DG, IS)                                                \
        annzz_mi(W(MG), W(DG), W(IS))

#define annyz_rr(RG, RS)                                                    \
        annzz_rr(W(RG), W(RS))

#define annyz_ld(RG, MS, DS)                                                \
        annzz_ld(W(RG), W(MS), W(DS))

#define annyz_st(RS, MG, DG)                                                \
        annzz_st(W(RS), W(MG), W(DG))

#define annyz_mr(MG, DG, RS)                                                \
        annzz_mr(W(MG), W(DG), W(RS))

/* orr (G = G | S)
 * set-flags: undefined (*x), yes (*z) */

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


#define orryz_ri(RG, IS)                                                    \
        orrzz_ri(W(RG), W(IS))

#define orryz_mi(MG, DG, IS)                                                \
        orrzz_mi(W(MG), W(DG), W(IS))

#define orryz_rr(RG, RS)                                                    \
        orrzz_rr(W(RG), W(RS))

#define orryz_ld(RG, MS, DS)                                                \
        orrzz_ld(W(RG), W(MS), W(DS))

#define orryz_st(RS, MG, DG)                                                \
        orrzz_st(W(RS), W(MG), W(DG))

#define orryz_mr(MG, DG, RS)                                                \
        orrzz_mr(W(MG), W(DG), W(RS))

/* orn (G = ~G | S)
 * set-flags: undefined (*x), yes (*z) */

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


#define ornyz_ri(RG, IS)                                                    \
        ornzz_ri(W(RG), W(IS))

#define ornyz_mi(MG, DG, IS)                                                \
        ornzz_mi(W(MG), W(DG), W(IS))

#define ornyz_rr(RG, RS)                                                    \
        ornzz_rr(W(RG), W(RS))

#define ornyz_ld(RG, MS, DS)                                                \
        ornzz_ld(W(RG), W(MS), W(DS))

#define ornyz_st(RS, MG, DG)                                                \
        ornzz_st(W(RS), W(MG), W(DG))

#define ornyz_mr(MG, DG, RS)                                                \
        ornzz_mr(W(MG), W(DG), W(RS))

/* xor (G = G ^ S)
 * set-flags: undefined (*x), yes (*z) */

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


#define xoryz_ri(RG, IS)                                                    \
        xorzz_ri(W(RG), W(IS))

#define xoryz_mi(MG, DG, IS)                                                \
        xorzz_mi(W(MG), W(DG), W(IS))

#define xoryz_rr(RG, RS)                                                    \
        xorzz_rr(W(RG), W(RS))

#define xoryz_ld(RG, MS, DS)                                                \
        xorzz_ld(W(RG), W(MS), W(DS))

#define xoryz_st(RS, MG, DG)                                                \
        xorzz_st(W(RS), W(MG), W(DG))

#define xoryz_mr(MG, DG, RS)                                                \
        xorzz_mr(W(MG), W(DG), W(RS))

/* not (G = ~G)
 * set-flags: no */

#define notyx_rx(RG)                                                        \
        notzx_rx(W(RG))

#define notyx_mx(MG, DG)                                                    \
        notzx_mx(W(MG), W(DG))

/* neg (G = -G)
 * set-flags: undefined (*x), yes (*z) */

#define negyx_rx(RG)                                                        \
        negzx_rx(W(RG))

#define negyx_mx(MG, DG)                                                    \
        negzx_mx(W(MG), W(DG))


#define negyz_rx(RG)                                                        \
        negzz_rx(W(RG))

#define negyz_mx(MG, DG)                                                    \
        negzz_mx(W(MG), W(DG))

/* add (G = G + S)
 * set-flags: undefined (*x), yes (*z) */

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


#define addyz_ri(RG, IS)                                                    \
        addzz_ri(W(RG), W(IS))

#define addyz_mi(MG, DG, IS)                                                \
        addzz_mi(W(MG), W(DG), W(IS))

#define addyz_rr(RG, RS)                                                    \
        addzz_rr(W(RG), W(RS))

#define addyz_ld(RG, MS, DS)                                                \
        addzz_ld(W(RG), W(MS), W(DS))

#define addyz_st(RS, MG, DG)                                                \
        addzz_st(W(RS), W(MG), W(DG))

#define addyz_mr(MG, DG, RS)                                                \
        addzz_mr(W(MG), W(DG), W(RS))

/* sub (G = G - S)
 * set-flags: undefined (*x), yes (*z) */

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


#define subyz_ri(RG, IS)                                                    \
        subzz_ri(W(RG), W(IS))

#define subyz_mi(MG, DG, IS)                                                \
        subzz_mi(W(MG), W(DG), W(IS))

#define subyz_rr(RG, RS)                                                    \
        subzz_rr(W(RG), W(RS))

#define subyz_ld(RG, MS, DS)                                                \
        subzz_ld(W(RG), W(MS), W(DS))

#define subyz_st(RS, MG, DG)                                                \
        subzz_st(W(RS), W(MG), W(DG))

#define subyz_mr(MG, DG, RS)                                                \
        subzz_mr(W(MG), W(DG), W(RS))

/* shl (G = G << S)
 * set-flags: undefined (*x), yes (*z)
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


#define shlyz_rx(RG)                     /* reads Recx for shift count */   \
        shlzz_rx(W(RG))

#define shlyz_mx(MG, DG)                 /* reads Recx for shift count */   \
        shlzz_mx(W(MG), W(DG))

#define shlyz_ri(RG, IS)                                                    \
        shlzz_ri(W(RG), W(IS))

#define shlyz_mi(MG, DG, IS)                                                \
        shlzz_mi(W(MG), W(DG), W(IS))

#define shlyz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shlzz_rr(W(RG), W(RS))

#define shlyz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shlzz_ld(W(RG), W(MS), W(DS))

#define shlyz_st(RS, MG, DG)                                                \
        shlzz_st(W(RS), W(MG), W(DG))

#define shlyz_mr(MG, DG, RS)                                                \
        shlzz_mr(W(MG), W(DG), W(RS))

/* shr (G = G >> S)
 * set-flags: undefined (*x), yes (*z)
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


#define shryz_rx(RG)                     /* reads Recx for shift count */   \
        shrzz_rx(W(RG))

#define shryz_mx(MG, DG)                 /* reads Recx for shift count */   \
        shrzz_mx(W(MG), W(DG))

#define shryz_ri(RG, IS)                                                    \
        shrzz_ri(W(RG), W(IS))

#define shryz_mi(MG, DG, IS)                                                \
        shrzz_mi(W(MG), W(DG), W(IS))

#define shryz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrzz_rr(W(RG), W(RS))

#define shryz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shrzz_ld(W(RG), W(MS), W(DS))

#define shryz_st(RS, MG, DG)                                                \
        shrzz_st(W(RS), W(MG), W(DG))

#define shryz_mr(MG, DG, RS)                                                \
        shrzz_mr(W(MG), W(DG), W(RS))


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

/* ror (G = G >> S | G << 64 - S)
 * set-flags: undefined (*x), yes (*z)
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


#define roryz_rx(RG)                     /* reads Recx for shift count */   \
        rorzz_rx(W(RG))

#define roryz_mx(MG, DG)                 /* reads Recx for shift count */   \
        rorzz_mx(W(MG), W(DG))

#define roryz_ri(RG, IS)                                                    \
        rorzz_ri(W(RG), W(IS))

#define roryz_mi(MG, DG, IS)                                                \
        rorzz_mi(W(MG), W(DG), W(IS))

#define roryz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        rorzz_rr(W(RG), W(RS))

#define roryz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        rorzz_ld(W(RG), W(MS), W(DS))

#define roryz_st(RS, MG, DG)                                                \
        rorzz_st(W(RS), W(MG), W(DG))

#define roryz_mr(MG, DG, RS)                                                \
        rorzz_mr(W(MG), W(DG), W(RS))

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


#define preyx_xx()          /* to be placed immediately prior divyx_x* */   \
        prezx_xx()                   /* to prepare Redx for int-divide */

#define preyn_xx()          /* to be placed immediately prior divyn_x* */   \
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


#define remyx_xx()          /* to be placed immediately prior divyx_x* */   \
        remzx_xx()                   /* to prepare for rem calculation */

#define remyx_xr(RS)        /* to be placed immediately after divyx_xr */   \
        remzx_xr(W(RS))              /* to produce remainder Redx<-rem */

#define remyx_xm(MS, DS)    /* to be placed immediately after divyx_xm */   \
        remzx_xm(W(MS), W(DS))       /* to produce remainder Redx<-rem */


#define remyn_xx()          /* to be placed immediately prior divyn_x* */   \
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

/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTBASE_H
#define RT_RTBASE_H

#include <math.h>
#include <float.h>
#include <stdlib.h>

#include "rtarch.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtbase.h: Base type definitions file.
 *
 * Recommended naming scheme for C++ types and definitions:
 *
 * - All scalar type names start with rt_ followed by type's specific name
 *   in lower case in a form of rt_****. For example: rt_cell or rt_vec4.
 *
 * - All structure names start with rt_ followed by structure's specific name
 *   in upper case with _ used as separator for complex names.
 *   All SIMD-aligned structures used in backend start with rt_SIMD_ prefix.
 *   For example: rt_ELEM or rt_SIMD_INFOX.
 *
 * - All class names start with rt_ followed by class's specific name
 *   in camel case without separator. For example: rt_Scene or rt_SceneThread.
 *
 * - All function names including class methods are in lower case with _ used
 *   as separator for complex names. For example: update_slice or render_num.
 *
 * - All function type names start with rt_FUNC_ followed by function type's
 *   specific name in upper case with _ used as separator for complex names.
 *   For example: rt_FUNC_INIT or rt_FUNC_UPDATE.
 *
 * - All preprocessor definition names and macros start with RT_ followed by
 *   specific name in upper case with _ used as separator for complex names.
 *   For example: RT_ALIGN or RT_ARR_SIZE.
 */

/******************************************************************************/
/*******************************   DEFINITIONS   ******************************/
/******************************************************************************/

/*
 * Generic types
 */

/* fixed-size floating point types */
typedef float               rt_fp32;
typedef double              rt_fp64;

/* adjustable floating point types */
#if   RT_ELEMENT == 32

typedef rt_fp32             rt_real;

typedef rt_fp32             rt_vec2[2];
typedef rt_fp32             rt_mat2[2][2];

typedef rt_fp32             rt_vec3[3];
typedef rt_fp32             rt_mat3[3][3];

typedef rt_fp32             rt_vec4[4];
typedef rt_fp32             rt_mat4[4][4];

#elif RT_ELEMENT == 64

typedef rt_fp64             rt_real;

typedef rt_fp64             rt_vec2[2];
typedef rt_fp64             rt_mat2[2][2];

typedef rt_fp64             rt_vec3[3];
typedef rt_fp64             rt_mat3[3][3];

typedef rt_fp64             rt_vec4[4];
typedef rt_fp64             rt_mat4[4][4];

#else  /* RT_ELEMENT */

#error "unsupported element size, check RT_ELEMENT in makefiles"

#endif /* RT_ELEMENT */

/* fixed-size integer types */
typedef char                rt_si08;
typedef unsigned char       rt_ui08;

typedef short               rt_si16;
typedef unsigned short      rt_ui16;

typedef int                 rt_si32;
typedef unsigned int        rt_ui32;

/* fixed 64-bit integer types */
#if   defined (RT_WIN32) /* Win32, MSVC -------- for older versions --------- */

typedef __int64             rt_si64;
typedef unsigned __int64    rt_ui64;
#define    PR64 /*printf*/  "I64"
#define RT_PR64 /*printf*/  PR64

#else /* --- Win64, GCC --- Linux, GCC -------------------------------------- */

typedef long long           rt_si64;
typedef unsigned long long  rt_ui64;
#define    PR64 /*printf*/  "ll"
#define RT_PR64 /*printf*/  PR64

#endif /* ------------- OS specific ----------------------------------------- */

/* adjustable integer types */
typedef rt_si08             rt_bool;
typedef rt_si64             rt_time;

typedef rt_si08             rt_char;
typedef rt_si16             rt_shrt;
typedef rt_si64             rt_long;

typedef rt_ui08             rt_byte;
typedef rt_ui16             rt_half;
typedef rt_ui64             rt_full;

/* element-size integer types */
#if   RT_ELEMENT == 32

typedef rt_si32             rt_elem;
#define    PR_L /*printf*/  ""
#define RT_PR_L /*printf*/  PR_L

typedef rt_ui32             rt_uelm;
#define    PRuL /*printf*/  "u"
#define RT_PRuL /*printf*/  PRuL

#elif RT_ELEMENT == 64

typedef rt_si64             rt_elem;
#define    PR_L /*printf*/  "ll"
#define RT_PR_L /*printf*/  PR_L

typedef rt_ui64             rt_uelm;
#define    PRuL /*printf*/  "ull"
#define RT_PRuL /*printf*/  PRuL

#else  /* RT_ELEMENT */

#error "unsupported element size, check RT_ELEMENT in makefiles"

#endif /* RT_ELEMENT */

/* address-size integer types */
#if   RT_ADDRESS == 32

typedef rt_si32             rt_addr;
#define    PR_A /*printf*/  ""
#define RT_PR_A /*printf*/  PR_A

typedef rt_ui32             rt_uadr;
#define    PRuA /*printf*/  "u"
#define RT_PRuA /*printf*/  PRuA

#elif RT_ADDRESS == 64

typedef rt_si64             rt_addr;
#define    PR_A /*printf*/  "ll"
#define RT_PR_A /*printf*/  PR_A

typedef rt_ui64             rt_uadr;
#define    PRuA /*printf*/  "ull"
#define RT_PRuA /*printf*/  PRuA

#else  /* RT_ADDRESS */

#error "unsupported address size, check RT_ADDRESS in makefiles"

#endif /* RT_ADDRESS */

/* pointer-size integer types */
#if   defined (RT_WIN64) /* Win64, GCC -------------------------------------- */

typedef rt_si64             rt_cell;
typedef rt_si64             rt_size;
#define    PR_P /*printf*/  "ll"
#define RT_PR_P /*printf*/  PR_P

typedef rt_ui64             rt_word;
typedef rt_ui64             rt_uptr;
#define    PRuP /*printf*/  "ull"
#define RT_PRuP /*printf*/  PRuP

#else /* --- Win32, MSVC -- Linux, GCC -------------------------------------- */

typedef long                rt_cell;
typedef long                rt_size;
#define    PR_P /*printf*/  "l"
#define RT_PR_P /*printf*/  PR_P

typedef unsigned long       rt_word;
typedef unsigned long       rt_uptr;
#define    PRuP /*printf*/  "ul"
#define RT_PRuP /*printf*/  PRuP

#endif /* ------------- OS specific ----------------------------------------- */

typedef void                rt_void;
typedef void               *rt_pntr;

typedef const char          rt_astr[];
typedef const char         *rt_pstr;

/*
 * Generic definitions
 */
#define RT_NULL             0

#define RT_FALSE            0
#define RT_TRUE             1

#if   RT_POINTER == 32

#define RT_ALIGN            4
#define RT_QUAD_ALIGN       16 /* not dependent on SIMD align */

#elif RT_POINTER == 64

#define RT_ALIGN            8
#define RT_QUAD_ALIGN       32 /* not dependent on SIMD align */

#else  /* RT_POINTER */

#error "unsupported pointer size, check RT_POINTER in makefiles"

#endif /* RT_POINTER */

/*
 * Generic macros
 */
#define RT_ARR_SIZE(a)      (sizeof(a) / sizeof(a[0]))

#define RT_MIN(a, b)        ((a) < (b) ? (a) : (b))
#define RT_MAX(a, b)        ((a) > (b) ? (a) : (b))

/*
 * Vector components
 */
#define RT_X                0
#define RT_Y                1
#define RT_Z                2
#define RT_W                3   /* W - World coords */

#define RT_I                0
#define RT_J                1
#define RT_K                2
#define RT_L                3   /* L - Local coords */

#define RT_R                0
#define RT_G                1
#define RT_B                2
#define RT_A                3   /* A - Alpha channel */

/*
 * For surface's UV coords
 *  to texture's XY coords mapping
 */
#define RT_U                0
#define RT_V                1

/*
 * Math definitions
 */
#define RT_INF              FLT_MAX

#define RT_PI               3.14159265358
#define RT_2_PI             (2.0 * RT_PI)
#define RT_PI_2             (RT_PI / 2.0)

/*
 * Math macros
 */
#define ABS32(a)            (abs((rt_si32)(a)))

#if   RT_ELEMENT == 32

#define RT_ABS(a)           ABS32(a)

#define RT_FABS(a)          (fabsf(a))

#define RT_FLOOR(a)         (floorf(a))

#define RT_CEIL(a)          (ceilf(a))

#define RT_SIGN(a)          ((a)  <    0.0f ? -1 :                          \
                             (a)  >    0.0f ? +1 :                          \
                              0)

#define RT_POW(a, b)        (powf(a, b))

#define RT_SQRT(a)          ((a)  <=   0.0f ?  0.0f :                       \
                             sqrtf(a))

#define RT_ASIN(a)          ((a)  <=  -1.0f ? -(rt_real)RT_PI_2 :           \
                             (a)  >=  +1.0f ? +(rt_real)RT_PI_2 :           \
                             asinf(a))

#define RT_ACOS(a)          ((a)  <=  -1.0f ? +(rt_real)RT_PI :             \
                             (a)  >=  +1.0f ?  0.0f :                       \
                             acosf(a))

#define RT_SINA(a)          ((a) == -270.0f ? +1.0f :                       \
                             (a) == -180.0f ?  0.0f :                       \
                             (a) ==  -90.0f ? -1.0f :                       \
                             (a) ==    0.0f ?  0.0f :                       \
                             (a) ==  +90.0f ? +1.0f :                       \
                             (a) == +180.0f ?  0.0f :                       \
                             (a) == +270.0f ? -1.0f :                       \
                             sinf((rt_real)((a) * RT_PI / 180.0)))

#define RT_COSA(a)          ((a) == -270.0f ?  0.0f :                       \
                             (a) == -180.0f ? -1.0f :                       \
                             (a) ==  -90.0f ?  0.0f :                       \
                             (a) ==    0.0f ? +1.0f :                       \
                             (a) ==  +90.0f ?  0.0f :                       \
                             (a) == +180.0f ? -1.0f :                       \
                             (a) == +270.0f ?  0.0f :                       \
                             cosf((rt_real)((a) * RT_PI / 180.0)))

#elif RT_ELEMENT == 64

#define RT_ABS(a)           (llabs((rt_si64)(a)))

#define RT_FABS(a)          (fabs(a))

#define RT_FLOOR(a)         (floor(a))

#define RT_CEIL(a)          (ceil(a))

#define RT_SIGN(a)          ((a)  <    0.0 ? -1 :                           \
                             (a)  >    0.0 ? +1 :                           \
                              0)

#define RT_POW(a, b)        (pow(a, b))

#define RT_SQRT(a)          ((a)  <=   0.0 ?  0.0 :                         \
                             sqrt(a))

#define RT_ASIN(a)          ((a)  <=  -1.0 ? -(rt_real)RT_PI_2 :            \
                             (a)  >=  +1.0 ? +(rt_real)RT_PI_2 :            \
                             asin(a))

#define RT_ACOS(a)          ((a)  <=  -1.0 ? +(rt_real)RT_PI :              \
                             (a)  >=  +1.0 ?  0.0 :                         \
                             acos(a))

#define RT_SINA(a)          ((a) == -270.0 ? +1.0 :                         \
                             (a) == -180.0 ?  0.0 :                         \
                             (a) ==  -90.0 ? -1.0 :                         \
                             (a) ==    0.0 ?  0.0 :                         \
                             (a) ==  +90.0 ? +1.0 :                         \
                             (a) == +180.0 ?  0.0 :                         \
                             (a) == +270.0 ? -1.0 :                         \
                             sin((rt_real)((a) * RT_PI / 180.0)))

#define RT_COSA(a)          ((a) == -270.0 ?  0.0 :                         \
                             (a) == -180.0 ? -1.0 :                         \
                             (a) ==  -90.0 ?  0.0 :                         \
                             (a) ==    0.0 ? +1.0 :                         \
                             (a) ==  +90.0 ?  0.0 :                         \
                             (a) == +180.0 ? -1.0 :                         \
                             (a) == +270.0 ?  0.0 :                         \
                             cos((rt_real)((a) * RT_PI / 180.0)))

#endif /* RT_ELEMENT */

/******************************************************************************/
/*************************   SIMD BACKEND STRUCTURE   *************************/
/******************************************************************************/

/*
 * SIMD info structure for asm enter/leave contains internal variables
 * and general purpose constants used internally by some instructions.
 * Note that DP offsets below accept only 12-bit values (0xFFF),
 * use DF, DG, DH and DV for 14, 15, 16 and 31-bit offsets respectively.
 * SIMD width is taken into account via S and Q defined in rtarch.h.
 * Structure is read-write in backend.
 */
struct rt_SIMD_INFO
{
    /* internal variables */

    rt_ui64 scr00;          /* scratchpad 00 */
#define inf_SCR00           DP(0x000)

    rt_ui32 ver;            /* SIMD version <- cpuid */
#define inf_VER             DP(0x008)

    rt_ui32 fctrl[R-3];     /* reserved, do not use! */
#define inf_FCTRL(nx)       DP(0x00C + nx)

    /* general purpose constants */

    rt_real gpc01[S];       /* +1.0 */
#define inf_GPC01           DP(Q*0x010)

    rt_real gpc02[S];       /* -0.5 */
#define inf_GPC02           DP(Q*0x020)

    rt_real gpc03[S];       /* +3.0 */
#define inf_GPC03           DP(Q*0x030)

    rt_elem gpc04[S];       /* 0x7FFFFFFF or 0x7FFFFFFFFFFFFFFF */
#define inf_GPC04           DP(Q*0x040)

    rt_elem gpc05[S];       /* 0x3F800000 or 0x3FF0000000000000 */
#define inf_GPC05           DP(Q*0x050)

    rt_elem scr01[S];       /* scratchpad 01 */
#define inf_SCR01(nx)       DP(Q*0x060 + nx)

    rt_elem scr02[S];       /* scratchpad 02 */
#define inf_SCR02(nx)       DP(Q*0x070 + nx)

    rt_ui64 regs[T];        /* SIMD reg-file storage */
#define inf_REGS            DP(Q*0x080+C)

    rt_real pad02[S*7];     /* reserved, do not use! */
#define inf_PAD02           DP(Q*0x090)

};

struct rt_SIMD_REGS
{
    /* register file */

    rt_real file[S*32];
#define reg_FILE            DP(Q*0x000)

};

#if   RT_ELEMENT == 32

#define ASM_INIT(__Info__, __Regs__)                                        \
    RT_SIMD_SET(__Info__->gpc01, +1.0f);                                    \
    RT_SIMD_SET(__Info__->gpc02, -0.5f);                                    \
    RT_SIMD_SET(__Info__->gpc03, +3.0f);                                    \
    RT_SIMD_SET(__Info__->gpc04, 0x7FFFFFFF);                               \
    RT_SIMD_SET(__Info__->gpc05, 0x3F800000);                               \
    __Info__->regs[0] = (rt_ui64)(rt_word)__Regs__;

#define ASM_DONE(__Info__)

#elif RT_ELEMENT == 64

#define ASM_INIT(__Info__, __Regs__)                                        \
    RT_SIMD_SET(__Info__->gpc01, +1.0);                                     \
    RT_SIMD_SET(__Info__->gpc02, -0.5);                                     \
    RT_SIMD_SET(__Info__->gpc03, +3.0);                                     \
    RT_SIMD_SET(__Info__->gpc04, 0x7FFFFFFFFFFFFFFF);                       \
    RT_SIMD_SET(__Info__->gpc05, 0x3FF0000000000000);                       \
    __Info__->regs[0] = (rt_ui64)(rt_word)__Regs__;

#define ASM_DONE(__Info__)

#else  /* RT_ELEMENT */

#error "unsupported element size, check RT_ELEMENT in makefiles"

#endif /* RT_ELEMENT */

/******************************************************************************/
/************************   COMMON SIMD INSTRUCTIONS   ************************/
/******************************************************************************/

/* cbr */

/*
 * Based on the original idea by Russell Borogove (kaleja[AT]estarcion[DOT]com)
 * available at http://www.musicdsp.org/showone.php?id=206
 * converted to S-way SIMD version by VectorChief.
 */
#define cbeps_rr(RG, R1, R2, RM) /* destroys R1, R2 (temp regs) */          \
        /* cube root estimate, the exponent is divided by three             \
         * in such a way that remainder bits get shoved into                \
         * the top of the normalized mantissa */                            \
        movpx_ld(W(R2), Mebp, inf_GPC04)                                    \
        movpx_rr(W(RG), W(RM))                                              \
        andpx_rr(W(RG), W(R2))   /* exponent & mantissa in biased-127 */    \
        subpx_ld(W(RG), Mebp, inf_GPC05) /* convert to 2's complement */    \
        shrpn_ri(W(RG), IB(10))  /* RG / 1024 */                            \
        movpx_rr(W(R1), W(RG))   /* RG * 341 (next 8 ops) */                \
        shlpx_ri(W(R1), IB(2))                                              \
        addpx_rr(W(RG), W(R1))                                              \
        shlpx_ri(W(R1), IB(2))                                              \
        addpx_rr(W(RG), W(R1))                                              \
        shlpx_ri(W(R1), IB(2))                                              \
        addpx_rr(W(RG), W(R1))                                              \
        shlpx_ri(W(R1), IB(2))                                              \
        addpx_rr(W(RG), W(R1))   /* RG * (341/1024) ~= RG * (0.333) */      \
        addpx_ld(W(RG), Mebp, inf_GPC05) /* back to biased-127 */           \
        andpx_rr(W(RG), W(R2))   /* remask exponent & mantissa */           \
        annpx_rr(W(R2), W(RM))   /* original sign */                        \
        orrpx_rr(W(RG), W(R2))   /* new exponent & mantissa, old sign */

#define cbsps_rr(RG, R1, R2, RM) /* destroys R1, R2 (temp regs) */          \
        movpx_rr(W(R1), W(RG))                                              \
        mulps_rr(W(R1), W(RG))                                              \
        movpx_rr(W(R2), W(R1))                                              \
        mulps_ld(W(R1), Mebp, inf_GPC03)                                    \
        rceps_rr(W(R1), W(R1))                                              \
        mulps_rr(W(R2), W(RG))                                              \
        subps_rr(W(R2), W(RM))                                              \
        mulps_rr(W(R2), W(R1))                                              \
        subps_rr(W(RG), W(R2))

#define cbrps_rr(RG, R1, R2, RM) /* destroys R1, R2 (temp regs) */          \
        cbeps_rr(W(RG), W(R1), W(R2), W(RM))                                \
        cbsps_rr(W(RG), W(R1), W(R2), W(RM))                                \
        cbsps_rr(W(RG), W(R1), W(R2), W(RM))                                \
        cbsps_rr(W(RG), W(R1), W(R2), W(RM))

/* rcp
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP == 0

#define rcpps_rr(RG, RM) /* destroys RM */                                  \
        rceps_rr(W(RG), W(RM))                                              \
        rcsps_rr(W(RG), W(RM)) /* <- not reusable without extra temp reg */

#else /* RT_SIMD_COMPAT_RCP */

#define rcpps_rr(RG, RM) /* destroys RM */                                  \
        movpx_ld(W(RG), Mebp, inf_GPC01)                                    \
        divps_rr(W(RG), W(RM))

#define rceps_rr(RG, RM)                                                    \
        movpx_st(W(RM), Mebp, inf_SCR01(0))                                 \
        movpx_ld(W(RG), Mebp, inf_GPC01)                                    \
        divps_ld(W(RG), Mebp, inf_SCR01(0))

#define rcsps_rr(RG, RM) /* destroys RM */

#endif /* RT_SIMD_COMPAT_RCP */

/* rsq
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ == 0

#define rsqps_rr(RG, RM) /* destroys RM */                                  \
        rseps_rr(W(RG), W(RM))                                              \
        rssps_rr(W(RG), W(RM)) /* <- not reusable without extra temp reg */

#else /* RT_SIMD_COMPAT_RSQ */

#define rsqps_rr(RG, RM) /* destroys RM */                                  \
        movpx_ld(W(RG), Mebp, inf_GPC01)                                    \
        sqrps_rr(W(RM), W(RM))                                              \
        divps_rr(W(RG), W(RM))

#define rseps_rr(RG, RM)                                                    \
        sqrps_rr(W(RG), W(RM))                                              \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        movpx_ld(W(RG), Mebp, inf_GPC01)                                    \
        divps_ld(W(RG), Mebp, inf_SCR01(0))

#define rssps_rr(RG, RM) /* destroys RM */

#endif /* RT_SIMD_COMPAT_RSQ */

/****************** original FCTRL blocks (cannot be nested) ******************/

#define FCTRL_ENTER(mode) /* assumes default mode (ROUNDN) upon entry */    \
        FCTRL_SET(mode)

#define FCTRL_LEAVE(mode) /* resumes default mode (ROUNDN) upon leave */    \
        FCTRL_RESET()

#endif /* RT_RTBASE_H */

/******************************************************************************/
/************************   COMMON BASE INSTRUCTIONS   ************************/
/******************************************************************************/

/***************** original forms of deprecated cmdx* aliases *****************/

/* adr */

#define adrxx_lb(lb) /* load label to Reax */                               \
        label_ld(lb)

/* mul */

#define mulxn_ri(RM, IM)                                                    \
        mulxx_ri(W(RM), W(IM))

#define mulxn_rr(RG, RM)                                                    \
        mulxx_rr(W(RG), W(RM))

#define mulxn_ld(RG, RM, DP)                                                \
        mulxx_ld(W(RG), W(RM), W(DP))

/***************** original forms of setting-flags arithmetic *****************/

/* and */

#define andzx_ri(RM, IM)                                                    \
        andxz_ri(W(RM), W(IM))

#define andzx_mi(RM, DP, IM)                                                \
        andxz_mi(W(RM), W(DP), W(IM))

#define andzx_rr(RG, RM)                                                    \
        andxz_rr(W(RG), W(RM))

#define andzx_ld(RG, RM, DP)                                                \
        andxz_ld(W(RG), W(RM), W(DP))

#define andzx_st(RG, RM, DP)                                                \
        andxz_st(W(RG), W(RM), W(DP))

/* neg */

#define negzx_rr(RM)                                                        \
        negxz_rx(W(RM))

#define negzx_mm(RM, DP)                                                    \
        negxz_mx(W(RM), W(DP))

/* add */

#define addzx_ri(RM, IM)                                                    \
        addxz_ri(W(RM), W(IM))

#define addzx_mi(RM, DP, IM)                                                \
        addxz_mi(W(RM), W(DP), W(IM))

#define addzx_rr(RG, RM)                                                    \
        addxz_rr(W(RG), W(RM))

#define addzx_ld(RG, RM, DP)                                                \
        addxz_ld(W(RG), W(RM), W(DP))

#define addzx_st(RG, RM, DP)                                                \
        addxz_st(W(RG), W(RM), W(DP))

/* sub */

#define subzx_ri(RM, IM)                                                    \
        subxz_ri(W(RM), W(IM))

#define subzx_mi(RM, DP, IM)                                                \
        subxz_mi(W(RM), W(DP), W(IM))

#define subzx_rr(RG, RM)                                                    \
        subxz_rr(W(RG), W(RM))

#define subzx_ld(RG, RM, DP)                                                \
        subxz_ld(W(RG), W(RM), W(DP))

#define subzx_st(RG, RM, DP)                                                \
        subxz_st(W(RG), W(RM), W(DP))

#define subzx_mr(RM, DP, RG)                                                \
        subzx_st(W(RG), W(RM), W(DP))

/***************** original forms of one-operand instructions *****************/

/* not */

#define notxx_rr(RM)                                                        \
        notxx_rx(W(RM))

#define notxx_mm(RM, DP)                                                    \
        notxx_mx(W(RM), W(DP))

/* neg */

#define negxx_rr(RM)                                                        \
        negxx_rx(W(RM))

#define negxx_mm(RM, DP)                                                    \
        negxx_mx(W(RM), W(DP))

/* jmp */

#define jmpxx_rr(RM)                                                        \
        jmpxx_xr(W(RM))

#define jmpxx_mm(RM, DP)                                                    \
        jmpxx_xm(W(RM), W(DP))

/***************** address-sized instructions for 32-bit mode *****************/

#if   RT_ADDRESS == 32

/* mov
 * set-flags: no */

#define movxx_ri(RM, IM)                                                    \
        movwx_ri(W(RM), W(IM))

#define movxx_mi(RM, DP, IM)                                                \
        movwx_mi(W(RM), W(DP), W(IM))

#define movxx_rr(RG, RM)                                                    \
        movwx_rr(W(RG), W(RM))

#define movxx_ld(RG, RM, DP)                                                \
        movwx_ld(W(RG), W(RM), W(DP))

#define movxx_st(RG, RM, DP)                                                \
        movwx_st(W(RG), W(RM), W(DP))

/* and
 * set-flags: undefined (*x), yes (*z) */

#define andxx_ri(RM, IM)                                                    \
        andwx_ri(W(RM), W(IM))

#define andxx_mi(RM, DP, IM)                                                \
        andwx_mi(W(RM), W(DP), W(IM))

#define andxx_rr(RG, RM)                                                    \
        andwx_rr(W(RG), W(RM))

#define andxx_ld(RG, RM, DP)                                                \
        andwx_ld(W(RG), W(RM), W(DP))

#define andxx_st(RG, RM, DP)                                                \
        andwx_st(W(RG), W(RM), W(DP))


#define andxz_ri(RM, IM)                                                    \
        andwz_ri(W(RM), W(IM))

#define andxz_mi(RM, DP, IM)                                                \
        andwz_mi(W(RM), W(DP), W(IM))

#define andxz_rr(RG, RM)                                                    \
        andwz_rr(W(RG), W(RM))

#define andxz_ld(RG, RM, DP)                                                \
        andwz_ld(W(RG), W(RM), W(DP))

#define andxz_st(RG, RM, DP)                                                \
        andwz_st(W(RG), W(RM), W(DP))

/* orr
 * set-flags: undefined (*x), yes (*z) */

#define orrxx_ri(RM, IM)                                                    \
        orrwx_ri(W(RM), W(IM))

#define orrxx_mi(RM, DP, IM)                                                \
        orrwx_mi(W(RM), W(DP), W(IM))

#define orrxx_rr(RG, RM)                                                    \
        orrwx_rr(W(RG), W(RM))

#define orrxx_ld(RG, RM, DP)                                                \
        orrwx_ld(W(RG), W(RM), W(DP))

#define orrxx_st(RG, RM, DP)                                                \
        orrwx_st(W(RG), W(RM), W(DP))


#define orrxz_ri(RM, IM)                                                    \
        orrwz_ri(W(RM), W(IM))

#define orrxz_mi(RM, DP, IM)                                                \
        orrwz_mi(W(RM), W(DP), W(IM))

#define orrxz_rr(RG, RM)                                                    \
        orrwz_rr(W(RG), W(RM))

#define orrxz_ld(RG, RM, DP)                                                \
        orrwz_ld(W(RG), W(RM), W(DP))

#define orrxz_st(RG, RM, DP)                                                \
        orrwz_st(W(RG), W(RM), W(DP))

/* xor
 * set-flags: undefined (*x), yes (*z) */

#define xorxx_ri(RM, IM)                                                    \
        xorwx_ri(W(RM), W(IM))

#define xorxx_mi(RM, DP, IM)                                                \
        xorwx_mi(W(RM), W(DP), W(IM))

#define xorxx_rr(RG, RM)                                                    \
        xorwx_rr(W(RG), W(RM))

#define xorxx_ld(RG, RM, DP)                                                \
        xorwx_ld(W(RG), W(RM), W(DP))

#define xorxx_st(RG, RM, DP)                                                \
        xorwx_st(W(RG), W(RM), W(DP))


#define xorxz_ri(RM, IM)                                                    \
        xorwz_ri(W(RM), W(IM))

#define xorxz_mi(RM, DP, IM)                                                \
        xorwz_mi(W(RM), W(DP), W(IM))

#define xorxz_rr(RG, RM)                                                    \
        xorwz_rr(W(RG), W(RM))

#define xorxz_ld(RG, RM, DP)                                                \
        xorwz_ld(W(RG), W(RM), W(DP))

#define xorxz_st(RG, RM, DP)                                                \
        xorwz_st(W(RG), W(RM), W(DP))

/* not
 * set-flags: no */

#define notxx_rx(RM)                                                        \
        notwx_rx(W(RM))

#define notxx_mx(RM, DP)                                                    \
        notwx_mx(W(RM), W(DP))

/* neg
 * set-flags: undefined (*x), yes (*z) */

#define negxx_rx(RM)                                                        \
        negwx_rx(W(RM))

#define negxx_mx(RM, DP)                                                    \
        negwx_mx(W(RM), W(DP))


#define negxz_rx(RM)                                                        \
        negwz_rx(W(RM))

#define negxz_mx(RM, DP)                                                    \
        negwz_mx(W(RM), W(DP))

/* add
 * set-flags: undefined (*x), yes (*z) */

#define addxx_ri(RM, IM)                                                    \
        addwx_ri(W(RM), W(IM))

#define addxx_mi(RM, DP, IM)                                                \
        addwx_mi(W(RM), W(DP), W(IM))

#define addxx_rr(RG, RM)                                                    \
        addwx_rr(W(RG), W(RM))

#define addxx_ld(RG, RM, DP)                                                \
        addwx_ld(W(RG), W(RM), W(DP))

#define addxx_st(RG, RM, DP)                                                \
        addwx_st(W(RG), W(RM), W(DP))


#define addxz_ri(RM, IM)                                                    \
        addwz_ri(W(RM), W(IM))

#define addxz_mi(RM, DP, IM)                                                \
        addwz_mi(W(RM), W(DP), W(IM))

#define addxz_rr(RG, RM)                                                    \
        addwz_rr(W(RG), W(RM))

#define addxz_ld(RG, RM, DP)                                                \
        addwz_ld(W(RG), W(RM), W(DP))

#define addxz_st(RG, RM, DP)                                                \
        addwz_st(W(RG), W(RM), W(DP))

/* sub
 * set-flags: undefined (*x), yes (*z) */

#define subxx_ri(RM, IM)                                                    \
        subwx_ri(W(RM), W(IM))

#define subxx_mi(RM, DP, IM)                                                \
        subwx_mi(W(RM), W(DP), W(IM))

#define subxx_rr(RG, RM)                                                    \
        subwx_rr(W(RG), W(RM))

#define subxx_ld(RG, RM, DP)                                                \
        subwx_ld(W(RG), W(RM), W(DP))

#define subxx_st(RG, RM, DP)                                                \
        subwx_st(W(RG), W(RM), W(DP))

#define subxx_mr(RM, DP, RG)                                                \
        subxx_st(W(RG), W(RM), W(DP))


#define subxz_ri(RM, IM)                                                    \
        subwz_ri(W(RM), W(IM))

#define subxz_mi(RM, DP, IM)                                                \
        subwz_mi(W(RM), W(DP), W(IM))

#define subxz_rr(RG, RM)                                                    \
        subwz_rr(W(RG), W(RM))

#define subxz_ld(RG, RM, DP)                                                \
        subwz_ld(W(RG), W(RM), W(DP))

#define subxz_st(RG, RM, DP)                                                \
        subwz_st(W(RG), W(RM), W(DP))

#define subxz_mr(RM, DP, RG)                                                \
        subxz_st(W(RG), W(RM), W(DP))

/* shl
 * set-flags: undefined (*x), yes (*z) */

#define shlxx_rx(RM)                     /* reads Recx for shift value */   \
        shlwx_rx(W(RM))

#define shlxx_mx(RM, DP)                 /* reads Recx for shift value */   \
        shlwx_mx(W(RM), W(DP))

#define shlxx_ri(RM, IM)                                                    \
        shlwx_ri(W(RM), W(IM))

#define shlxx_mi(RM, DP, IM)                                                \
        shlwx_mi(W(RM), W(DP), W(IM))

#define shlxx_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        shlwx_rr(W(RG), W(RM))

#define shlxx_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        shlwx_ld(W(RG), W(RM), W(DP))

#define shlxx_st(RG, RM, DP)                                                \
        shlwx_st(W(RG), W(RM), W(DP))

#define shlxx_mr(RM, DP, RG)                                                \
        shlxx_st(W(RG), W(RM), W(DP))


#define shlxz_rx(RM)                     /* reads Recx for shift value */   \
        shlwz_rx(W(RM))

#define shlxz_mx(RM, DP)                 /* reads Recx for shift value */   \
        shlwz_mx(W(RM), W(DP))

#define shlxz_ri(RM, IM)                                                    \
        shlwz_ri(W(RM), W(IM))

#define shlxz_mi(RM, DP, IM)                                                \
        shlwz_mi(W(RM), W(DP), W(IM))

#define shlxz_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        shlwz_rr(W(RG), W(RM))

#define shlxz_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        shlwz_ld(W(RG), W(RM), W(DP))

#define shlxz_st(RG, RM, DP)                                                \
        shlwz_st(W(RG), W(RM), W(DP))

#define shlxz_mr(RM, DP, RG)                                                \
        shlxz_st(W(RG), W(RM), W(DP))

/* shr
 * set-flags: undefined (*x), yes (*z) */

#define shrxx_rx(RM)                     /* reads Recx for shift value */   \
        shrwx_rx(W(RM))

#define shrxx_mx(RM, DP)                 /* reads Recx for shift value */   \
        shrwx_mx(W(RM), W(DP))

#define shrxx_ri(RM, IM)                                                    \
        shrwx_ri(W(RM), W(IM))

#define shrxx_mi(RM, DP, IM)                                                \
        shrwx_mi(W(RM), W(DP), W(IM))

#define shrxx_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        shrwx_rr(W(RG), W(RM))

#define shrxx_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        shrwx_ld(W(RG), W(RM), W(DP))

#define shrxx_st(RG, RM, DP)                                                \
        shrwx_st(W(RG), W(RM), W(DP))

#define shrxx_mr(RM, DP, RG)                                                \
        shrxx_st(W(RG), W(RM), W(DP))


#define shrxz_rx(RM)                     /* reads Recx for shift value */   \
        shrwz_rx(W(RM))

#define shrxz_mx(RM, DP)                 /* reads Recx for shift value */   \
        shrwz_mx(W(RM), W(DP))

#define shrxz_ri(RM, IM)                                                    \
        shrwz_ri(W(RM), W(IM))

#define shrxz_mi(RM, DP, IM)                                                \
        shrwz_mi(W(RM), W(DP), W(IM))

#define shrxz_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        shrwz_rr(W(RG), W(RM))

#define shrxz_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        shrwz_ld(W(RG), W(RM), W(DP))

#define shrxz_st(RG, RM, DP)                                                \
        shrwz_st(W(RG), W(RM), W(DP))

#define shrxz_mr(RM, DP, RG)                                                \
        shrxz_st(W(RG), W(RM), W(DP))


#define shrxn_rx(RM)                     /* reads Recx for shift value */   \
        shrwn_rx(W(RM))

#define shrxn_mx(RM, DP)                 /* reads Recx for shift value */   \
        shrwn_mx(W(RM), W(DP))

#define shrxn_ri(RM, IM)                                                    \
        shrwn_ri(W(RM), W(IM))

#define shrxn_mi(RM, DP, IM)                                                \
        shrwn_mi(W(RM), W(DP), W(IM))

#define shrxn_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        shrwn_rr(W(RG), W(RM))

#define shrxn_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        shrwn_ld(W(RG), W(RM), W(DP))

#define shrxn_st(RG, RM, DP)                                                \
        shrwn_st(W(RG), W(RM), W(DP))

#define shrxn_mr(RM, DP, RG)                                                \
        shrxn_st(W(RG), W(RM), W(DP))

/* mul
 * set-flags: undefined */

#define mulxx_ri(RM, IM)                                                    \
        mulwx_ri(W(RM), W(IM))

#define mulxx_rr(RG, RM)                                                    \
        mulwx_rr(W(RG), W(RM))

#define mulxx_ld(RG, RM, DP)                                                \
        mulwx_ld(W(RG), W(RM), W(DP))


#define mulxx_xr(RM)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        mulwx_xr(W(RM))

#define mulxx_xm(RM, DP) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        mulwx_xm(W(RM), W(DP))


#define mulxn_xr(RM)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        mulwn_xr(W(RM))

#define mulxn_xm(RM, DP) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        mulwn_xm(W(RM), W(DP))


#define mulxp_xr(RM)     /* Reax is in/out, prepares Redx for divxn_x* */   \
        mulwp_xr(W(RM))       /* product must not exceed operands size */

#define mulxp_xm(RM, DP) /* Reax is in/out, prepares Redx for divxn_x* */   \
        mulwp_xm(W(RM), W(DP))/* product must not exceed operands size */

/* div
 * set-flags: undefined */

#define divxx_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        divwx_ri(W(RM), W(IM))

#define divxx_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        divwx_rr(W(RG), W(RM))

#define divxx_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        divwx_ld(W(RG), W(RM), W(DP))


#define divxn_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        divwn_ri(W(RM), W(IM))

#define divxn_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        divwn_rr(W(RG), W(RM))

#define divxn_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        divwn_ld(W(RG), W(RM), W(DP))


#define prexx_xx()          /* to be placed immediately prior divxx_x* */   \
        prewx_xx()                   /* to prepare Redx for int-divide */

#define prexn_xx()          /* to be placed immediately prior divxn_x* */   \
        prewn_xx()                   /* to prepare Redx for int-divide */


#define divxx_xr(RM)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        divwx_xr(W(RM))

#define divxx_xm(RM, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        divwx_xm(W(RM), W(DP))


#define divxn_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xr(W(RM))

#define divxn_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xm(W(RM), W(DP))


#define divxp_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwp_xr(W(RM))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divxp_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwp_xm(W(RM), W(DP))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem
 * set-flags: undefined */

#define remxx_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        remwx_ri(W(RM), W(IM))

#define remxx_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        remwx_rr(W(RG), W(RM))

#define remxx_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        remwx_ld(W(RG), W(RM), W(DP))


#define remxn_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        remwn_ri(W(RM), W(IM))

#define remxn_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        remwn_rr(W(RG), W(RM))

#define remxn_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        remwn_ld(W(RG), W(RM), W(DP))


#define remxx_xx()          /* to be placed immediately prior divxx_x* */   \
        remwx_xx()                   /* to prepare for rem calculation */

#define remxx_xr(RM)        /* to be placed immediately after divxx_xr */   \
        remwx_xr(W(RM))              /* to produce remainder Redx<-rem */

#define remxx_xm(RM, DP)    /* to be placed immediately after divxx_xm */   \
        remwx_xm(W(RM), W(DP))       /* to produce remainder Redx<-rem */


#define remxn_xx()          /* to be placed immediately prior divxn_x* */   \
        remwn_xx()                   /* to prepare for rem calculation */

#define remxn_xr(RM)        /* to be placed immediately after divxn_xr */   \
        remwn_xr(W(RM))              /* to produce remainder Redx<-rem */

#define remxn_xm(RM, DP)    /* to be placed immediately after divxn_xm */   \
        remwn_xm(W(RM), W(DP))       /* to produce remainder Redx<-rem */

/* arj
 * set-flags: undefined
 * refer to individual instructions' description
 * to stay within special register limitations */

#define arjxx_rx(RM, op, cc, lb)                                            \
        arjwx_rx(W(RM), op, cc, lb)

#define arjxx_mx(RM, DP, op, cc, lb)                                        \
        arjwx_mx(W(RM), W(DP), op, cc, lb)

#define arjxx_ri(RM, IM, op, cc, lb)                                        \
        arjwx_ri(W(RM), W(IM), op, cc, lb)

#define arjxx_mi(RM, DP, IM, op, cc, lb)                                    \
        arjwx_mi(W(RM), W(DP), W(IM), op, cc, lb)

#define arjxx_rr(RG, RM, op, cc, lb)                                        \
        arjwx_rr(W(RG), W(RM), op, cc, lb)

#define arjxx_ld(RG, RM, DP, op, cc, lb)                                    \
        arjwx_ld(W(RG), W(RM), W(DP), op, cc, lb)

#define arjxx_st(RG, RM, DP, op, cc, lb)                                    \
        arjwx_st(W(RG), W(RM), W(DP), op, cc, lb)

#define arjxx_mr(RM, DP, RG, op, cc, lb)                                    \
        arjxx_st(W(RG), W(RM), W(DP), op, cc, lb)

/* cmj
 * set-flags: undefined */

#define cmjxx_rz(RM, cc, lb)                                                \
        cmjxx_ri(W(RM), IC(0), cc, lb)

#define cmjxx_mz(RM, DP, cc, lb)                                            \
        cmjxx_mi(W(RM), W(DP), IC(0), cc, lb)

#define cmjxx_ri(RM, IM, cc, lb)                                            \
        cmjwx_ri(W(RM), W(IM), cc, lb)

#define cmjxx_mi(RM, DP, IM, cc, lb)                                        \
        cmjwx_mi(W(RM), W(DP), W(IM), cc, lb)

#define cmjxx_rr(RG, RM, cc, lb)                                            \
        cmjwx_rr(W(RG), W(RM), cc, lb)

#define cmjxx_rm(RG, RM, DP, cc, lb)                                        \
        cmjwx_rm(W(RG), W(RM), W(DP), cc, lb)

#define cmjxx_mr(RM, DP, RG, cc, lb)                                        \
        cmjwx_mr(W(RM), W(DP), W(RG), cc, lb)

/* cmp
 * set-flags: yes */

#define cmpxx_ri(RM, IM)                                                    \
        cmpwx_ri(W(RM), W(IM))

#define cmpxx_mi(RM, DP, IM)                                                \
        cmpwx_mi(W(RM), W(DP), W(IM))

#define cmpxx_rr(RG, RM)                                                    \
        cmpwx_rr(W(RG), W(RM))

#define cmpxx_rm(RG, RM, DP)                                                \
        cmpwx_rm(W(RG), W(RM), W(DP))

#define cmpxx_mr(RM, DP, RG)                                                \
        cmpwx_mr(W(RM), W(DP), W(RG))

#endif /* RT_ADDRESS */

/***************** element-sized instructions for 32-bit mode *****************/

#if   RT_ELEMENT == 32

/* mov
 * set-flags: no */

#define movyx_ri(RM, IM)                                                    \
        movwx_ri(W(RM), W(IM))

#define movyx_mi(RM, DP, IM)                                                \
        movwx_mi(W(RM), W(DP), W(IM))

#define movyx_rr(RG, RM)                                                    \
        movwx_rr(W(RG), W(RM))

#define movyx_ld(RG, RM, DP)                                                \
        movwx_ld(W(RG), W(RM), W(DP))

#define movyx_st(RG, RM, DP)                                                \
        movwx_st(W(RG), W(RM), W(DP))

/* and
 * set-flags: undefined (*x), yes (*z) */

#define andyx_ri(RM, IM)                                                    \
        andwx_ri(W(RM), W(IM))

#define andyx_mi(RM, DP, IM)                                                \
        andwx_mi(W(RM), W(DP), W(IM))

#define andyx_rr(RG, RM)                                                    \
        andwx_rr(W(RG), W(RM))

#define andyx_ld(RG, RM, DP)                                                \
        andwx_ld(W(RG), W(RM), W(DP))

#define andyx_st(RG, RM, DP)                                                \
        andwx_st(W(RG), W(RM), W(DP))


#define andyz_ri(RM, IM)                                                    \
        andwz_ri(W(RM), W(IM))

#define andyz_mi(RM, DP, IM)                                                \
        andwz_mi(W(RM), W(DP), W(IM))

#define andyz_rr(RG, RM)                                                    \
        andwz_rr(W(RG), W(RM))

#define andyz_ld(RG, RM, DP)                                                \
        andwz_ld(W(RG), W(RM), W(DP))

#define andyz_st(RG, RM, DP)                                                \
        andwz_st(W(RG), W(RM), W(DP))

/* orr
 * set-flags: undefined (*x), yes (*z) */

#define orryx_ri(RM, IM)                                                    \
        orrwx_ri(W(RM), W(IM))

#define orryx_mi(RM, DP, IM)                                                \
        orrwx_mi(W(RM), W(DP), W(IM))

#define orryx_rr(RG, RM)                                                    \
        orrwx_rr(W(RG), W(RM))

#define orryx_ld(RG, RM, DP)                                                \
        orrwx_ld(W(RG), W(RM), W(DP))

#define orryx_st(RG, RM, DP)                                                \
        orrwx_st(W(RG), W(RM), W(DP))


#define orryz_ri(RM, IM)                                                    \
        orrwz_ri(W(RM), W(IM))

#define orryz_mi(RM, DP, IM)                                                \
        orrwz_mi(W(RM), W(DP), W(IM))

#define orryz_rr(RG, RM)                                                    \
        orrwz_rr(W(RG), W(RM))

#define orryz_ld(RG, RM, DP)                                                \
        orrwz_ld(W(RG), W(RM), W(DP))

#define orryz_st(RG, RM, DP)                                                \
        orrwz_st(W(RG), W(RM), W(DP))

/* xor
 * set-flags: undefined (*x), yes (*z) */

#define xoryx_ri(RM, IM)                                                    \
        xorwx_ri(W(RM), W(IM))

#define xoryx_mi(RM, DP, IM)                                                \
        xorwx_mi(W(RM), W(DP), W(IM))

#define xoryx_rr(RG, RM)                                                    \
        xorwx_rr(W(RG), W(RM))

#define xoryx_ld(RG, RM, DP)                                                \
        xorwx_ld(W(RG), W(RM), W(DP))

#define xoryx_st(RG, RM, DP)                                                \
        xorwx_st(W(RG), W(RM), W(DP))


#define xoryz_ri(RM, IM)                                                    \
        xorwz_ri(W(RM), W(IM))

#define xoryz_mi(RM, DP, IM)                                                \
        xorwz_mi(W(RM), W(DP), W(IM))

#define xoryz_rr(RG, RM)                                                    \
        xorwz_rr(W(RG), W(RM))

#define xoryz_ld(RG, RM, DP)                                                \
        xorwz_ld(W(RG), W(RM), W(DP))

#define xoryz_st(RG, RM, DP)                                                \
        xorwz_st(W(RG), W(RM), W(DP))

/* not
 * set-flags: no */

#define notyx_rx(RM)                                                        \
        notwx_rx(W(RM))

#define notyx_mx(RM, DP)                                                    \
        notwx_mx(W(RM), W(DP))

/* neg
 * set-flags: undefined (*x), yes (*z) */

#define negyx_rx(RM)                                                        \
        negwx_rx(W(RM))

#define negyx_mx(RM, DP)                                                    \
        negwx_mx(W(RM), W(DP))


#define negyz_rx(RM)                                                        \
        negwz_rx(W(RM))

#define negyz_mx(RM, DP)                                                    \
        negwz_mx(W(RM), W(DP))

/* add
 * set-flags: undefined (*x), yes (*z) */

#define addyx_ri(RM, IM)                                                    \
        addwx_ri(W(RM), W(IM))

#define addyx_mi(RM, DP, IM)                                                \
        addwx_mi(W(RM), W(DP), W(IM))

#define addyx_rr(RG, RM)                                                    \
        addwx_rr(W(RG), W(RM))

#define addyx_ld(RG, RM, DP)                                                \
        addwx_ld(W(RG), W(RM), W(DP))

#define addyx_st(RG, RM, DP)                                                \
        addwx_st(W(RG), W(RM), W(DP))


#define addyz_ri(RM, IM)                                                    \
        addwz_ri(W(RM), W(IM))

#define addyz_mi(RM, DP, IM)                                                \
        addwz_mi(W(RM), W(DP), W(IM))

#define addyz_rr(RG, RM)                                                    \
        addwz_rr(W(RG), W(RM))

#define addyz_ld(RG, RM, DP)                                                \
        addwz_ld(W(RG), W(RM), W(DP))

#define addyz_st(RG, RM, DP)                                                \
        addwz_st(W(RG), W(RM), W(DP))

/* sub
 * set-flags: undefined (*x), yes (*z) */

#define subyx_ri(RM, IM)                                                    \
        subwx_ri(W(RM), W(IM))

#define subyx_mi(RM, DP, IM)                                                \
        subwx_mi(W(RM), W(DP), W(IM))

#define subyx_rr(RG, RM)                                                    \
        subwx_rr(W(RG), W(RM))

#define subyx_ld(RG, RM, DP)                                                \
        subwx_ld(W(RG), W(RM), W(DP))

#define subyx_st(RG, RM, DP)                                                \
        subwx_st(W(RG), W(RM), W(DP))

#define subyx_mr(RM, DP, RG)                                                \
        subyx_st(W(RG), W(RM), W(DP))


#define subyz_ri(RM, IM)                                                    \
        subwz_ri(W(RM), W(IM))

#define subyz_mi(RM, DP, IM)                                                \
        subwz_mi(W(RM), W(DP), W(IM))

#define subyz_rr(RG, RM)                                                    \
        subwz_rr(W(RG), W(RM))

#define subyz_ld(RG, RM, DP)                                                \
        subwz_ld(W(RG), W(RM), W(DP))

#define subyz_st(RG, RM, DP)                                                \
        subwz_st(W(RG), W(RM), W(DP))

#define subyz_mr(RM, DP, RG)                                                \
        subyz_st(W(RG), W(RM), W(DP))

/* shl
 * set-flags: undefined (*x), yes (*z) */

#define shlyx_rx(RM)                     /* reads Recx for shift value */   \
        shlwx_rx(W(RM))

#define shlyx_mx(RM, DP)                 /* reads Recx for shift value */   \
        shlwx_mx(W(RM), W(DP))

#define shlyx_ri(RM, IM)                                                    \
        shlwx_ri(W(RM), W(IM))

#define shlyx_mi(RM, DP, IM)                                                \
        shlwx_mi(W(RM), W(DP), W(IM))

#define shlyx_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        shlwx_rr(W(RG), W(RM))

#define shlyx_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        shlwx_ld(W(RG), W(RM), W(DP))

#define shlyx_st(RG, RM, DP)                                                \
        shlwx_st(W(RG), W(RM), W(DP))

#define shlyx_mr(RM, DP, RG)                                                \
        shlyx_st(W(RG), W(RM), W(DP))


#define shlyz_rx(RM)                     /* reads Recx for shift value */   \
        shlwz_rx(W(RM))

#define shlyz_mx(RM, DP)                 /* reads Recx for shift value */   \
        shlwz_mx(W(RM), W(DP))

#define shlyz_ri(RM, IM)                                                    \
        shlwz_ri(W(RM), W(IM))

#define shlyz_mi(RM, DP, IM)                                                \
        shlwz_mi(W(RM), W(DP), W(IM))

#define shlyz_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        shlwz_rr(W(RG), W(RM))

#define shlyz_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        shlwz_ld(W(RG), W(RM), W(DP))

#define shlyz_st(RG, RM, DP)                                                \
        shlwz_st(W(RG), W(RM), W(DP))

#define shlyz_mr(RM, DP, RG)                                                \
        shlyz_st(W(RG), W(RM), W(DP))

/* shr
 * set-flags: undefined (*x), yes (*z) */

#define shryx_rx(RM)                     /* reads Recx for shift value */   \
        shrwx_rx(W(RM))

#define shryx_mx(RM, DP)                 /* reads Recx for shift value */   \
        shrwx_mx(W(RM), W(DP))

#define shryx_ri(RM, IM)                                                    \
        shrwx_ri(W(RM), W(IM))

#define shryx_mi(RM, DP, IM)                                                \
        shrwx_mi(W(RM), W(DP), W(IM))

#define shryx_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        shrwx_rr(W(RG), W(RM))

#define shryx_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        shrwx_ld(W(RG), W(RM), W(DP))

#define shryx_st(RG, RM, DP)                                                \
        shrwx_st(W(RG), W(RM), W(DP))

#define shryx_mr(RM, DP, RG)                                                \
        shryx_st(W(RG), W(RM), W(DP))


#define shryz_rx(RM)                     /* reads Recx for shift value */   \
        shrwz_rx(W(RM))

#define shryz_mx(RM, DP)                 /* reads Recx for shift value */   \
        shrwz_mx(W(RM), W(DP))

#define shryz_ri(RM, IM)                                                    \
        shrwz_ri(W(RM), W(IM))

#define shryz_mi(RM, DP, IM)                                                \
        shrwz_mi(W(RM), W(DP), W(IM))

#define shryz_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        shrwz_rr(W(RG), W(RM))

#define shryz_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        shrwz_ld(W(RG), W(RM), W(DP))

#define shryz_st(RG, RM, DP)                                                \
        shrwz_st(W(RG), W(RM), W(DP))

#define shryz_mr(RM, DP, RG)                                                \
        shryz_st(W(RG), W(RM), W(DP))


#define shryn_rx(RM)                     /* reads Recx for shift value */   \
        shrwn_rx(W(RM))

#define shryn_mx(RM, DP)                 /* reads Recx for shift value */   \
        shrwn_mx(W(RM), W(DP))

#define shryn_ri(RM, IM)                                                    \
        shrwn_ri(W(RM), W(IM))

#define shryn_mi(RM, DP, IM)                                                \
        shrwn_mi(W(RM), W(DP), W(IM))

#define shryn_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        shrwn_rr(W(RG), W(RM))

#define shryn_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        shrwn_ld(W(RG), W(RM), W(DP))

#define shryn_st(RG, RM, DP)                                                \
        shrwn_st(W(RG), W(RM), W(DP))

#define shryn_mr(RM, DP, RG)                                                \
        shryn_st(W(RG), W(RM), W(DP))

/* mul
 * set-flags: undefined */

#define mulyx_ri(RM, IM)                                                    \
        mulwx_ri(W(RM), W(IM))

#define mulyx_rr(RG, RM)                                                    \
        mulwx_rr(W(RG), W(RM))

#define mulyx_ld(RG, RM, DP)                                                \
        mulwx_ld(W(RG), W(RM), W(DP))


#define mulyx_xr(RM)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        mulwx_xr(W(RM))

#define mulyx_xm(RM, DP) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        mulwx_xm(W(RM), W(DP))


#define mulyn_xr(RM)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        mulwn_xr(W(RM))

#define mulyn_xm(RM, DP) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        mulwn_xm(W(RM), W(DP))


#define mulyp_xr(RM)     /* Reax is in/out, prepares Redx for divyn_x* */   \
        mulwp_xr(W(RM))       /* product must not exceed operands size */

#define mulyp_xm(RM, DP) /* Reax is in/out, prepares Redx for divyn_x* */   \
        mulwp_xm(W(RM), W(DP))/* product must not exceed operands size */

/* div
 * set-flags: undefined */

#define divyx_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        divwx_ri(W(RM), W(IM))

#define divyx_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        divwx_rr(W(RG), W(RM))

#define divyx_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        divwx_ld(W(RG), W(RM), W(DP))


#define divyn_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        divwn_ri(W(RM), W(IM))

#define divyn_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        divwn_rr(W(RG), W(RM))

#define divyn_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        divwn_ld(W(RG), W(RM), W(DP))


#define preyx_xx()          /* to be placed immediately prior divyx_x* */   \
        prewx_xx()                   /* to prepare Redx for int-divide */

#define preyn_xx()          /* to be placed immediately prior divyn_x* */   \
        prewn_xx()                   /* to prepare Redx for int-divide */


#define divyx_xr(RM)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        divwx_xr(W(RM))

#define divyx_xm(RM, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        divwx_xm(W(RM), W(DP))


#define divyn_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xr(W(RM))

#define divyn_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xm(W(RM), W(DP))


#define divyp_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwp_xr(W(RM))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divyp_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwp_xm(W(RM), W(DP))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem
 * set-flags: undefined */

#define remyx_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        remwx_ri(W(RM), W(IM))

#define remyx_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        remwx_rr(W(RG), W(RM))

#define remyx_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        remwx_ld(W(RG), W(RM), W(DP))


#define remyn_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        remwn_ri(W(RM), W(IM))

#define remyn_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        remwn_rr(W(RG), W(RM))

#define remyn_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        remwn_ld(W(RG), W(RM), W(DP))


#define remyx_xx()          /* to be placed immediately prior divyx_x* */   \
        remwx_xx()                   /* to prepare for rem calculation */

#define remyx_xr(RM)        /* to be placed immediately after divyx_xr */   \
        remwx_xr(W(RM))              /* to produce remainder Redx<-rem */

#define remyx_xm(RM, DP)    /* to be placed immediately after divyx_xm */   \
        remwx_xm(W(RM), W(DP))       /* to produce remainder Redx<-rem */


#define remyn_xx()          /* to be placed immediately prior divyn_x* */   \
        remwn_xx()                   /* to prepare for rem calculation */

#define remyn_xr(RM)        /* to be placed immediately after divyn_xr */   \
        remwn_xr(W(RM))              /* to produce remainder Redx<-rem */

#define remyn_xm(RM, DP)    /* to be placed immediately after divyn_xm */   \
        remwn_xm(W(RM), W(DP))       /* to produce remainder Redx<-rem */

/* arj
 * set-flags: undefined
 * refer to individual instructions' description
 * to stay within special register limitations */

#define arjyx_rx(RM, op, cc, lb)                                            \
        arjwx_rx(W(RM), op, cc, lb)

#define arjyx_mx(RM, DP, op, cc, lb)                                        \
        arjwx_mx(W(RM), W(DP), op, cc, lb)

#define arjyx_ri(RM, IM, op, cc, lb)                                        \
        arjwx_ri(W(RM), W(IM), op, cc, lb)

#define arjyx_mi(RM, DP, IM, op, cc, lb)                                    \
        arjwx_mi(W(RM), W(DP), W(IM), op, cc, lb)

#define arjyx_rr(RG, RM, op, cc, lb)                                        \
        arjwx_rr(W(RG), W(RM), op, cc, lb)

#define arjyx_ld(RG, RM, DP, op, cc, lb)                                    \
        arjwx_ld(W(RG), W(RM), W(DP), op, cc, lb)

#define arjyx_st(RG, RM, DP, op, cc, lb)                                    \
        arjwx_st(W(RG), W(RM), W(DP), op, cc, lb)

#define arjyx_mr(RM, DP, RG, op, cc, lb)                                    \
        arjyx_st(W(RG), W(RM), W(DP), op, cc, lb)

/* cmj
 * set-flags: undefined */

#define cmjyx_rz(RM, cc, lb)                                                \
        cmjyx_ri(W(RM), IC(0), cc, lb)

#define cmjyx_mz(RM, DP, cc, lb)                                            \
        cmjyx_mi(W(RM), W(DP), IC(0), cc, lb)

#define cmjyx_ri(RM, IM, cc, lb)                                            \
        cmjwx_ri(W(RM), W(IM), cc, lb)

#define cmjyx_mi(RM, DP, IM, cc, lb)                                        \
        cmjwx_mi(W(RM), W(DP), W(IM), cc, lb)

#define cmjyx_rr(RG, RM, cc, lb)                                            \
        cmjwx_rr(W(RG), W(RM), cc, lb)

#define cmjyx_rm(RG, RM, DP, cc, lb)                                        \
        cmjwx_rm(W(RG), W(RM), W(DP), cc, lb)

#define cmjyx_mr(RM, DP, RG, cc, lb)                                        \
        cmjwx_mr(W(RM), W(DP), W(RG), cc, lb)

/* cmp
 * set-flags: yes */

#define cmpyx_ri(RM, IM)                                                    \
        cmpwx_ri(W(RM), W(IM))

#define cmpyx_mi(RM, DP, IM)                                                \
        cmpwx_mi(W(RM), W(DP), W(IM))

#define cmpyx_rr(RG, RM)                                                    \
        cmpwx_rr(W(RG), W(RM))

#define cmpyx_rm(RG, RM, DP)                                                \
        cmpwx_rm(W(RG), W(RM), W(DP))

#define cmpyx_mr(RM, DP, RG)                                                \
        cmpwx_mr(W(RM), W(DP), W(RG))

/***************** element-sized instructions for 64-bit mode *****************/

#elif RT_ELEMENT == 64

/* mov
 * set-flags: no */

#define movyx_ri(RM, IM)                                                    \
        movxx_ri(W(RM), W(IM))

#define movyx_mi(RM, DP, IM)                                                \
        movxx_mi(W(RM), W(DP), W(IM))

#define movyx_rr(RG, RM)                                                    \
        movxx_rr(W(RG), W(RM))

#define movyx_ld(RG, RM, DP)                                                \
        movxx_ld(W(RG), W(RM), W(DP))

#define movyx_st(RG, RM, DP)                                                \
        movxx_st(W(RG), W(RM), W(DP))

/* and
 * set-flags: undefined (*x), yes (*z) */

#define andyx_ri(RM, IM)                                                    \
        andxx_ri(W(RM), W(IM))

#define andyx_mi(RM, DP, IM)                                                \
        andxx_mi(W(RM), W(DP), W(IM))

#define andyx_rr(RG, RM)                                                    \
        andxx_rr(W(RG), W(RM))

#define andyx_ld(RG, RM, DP)                                                \
        andxx_ld(W(RG), W(RM), W(DP))

#define andyx_st(RG, RM, DP)                                                \
        andxx_st(W(RG), W(RM), W(DP))


#define andyz_ri(RM, IM)                                                    \
        andxz_ri(W(RM), W(IM))

#define andyz_mi(RM, DP, IM)                                                \
        andxz_mi(W(RM), W(DP), W(IM))

#define andyz_rr(RG, RM)                                                    \
        andxz_rr(W(RG), W(RM))

#define andyz_ld(RG, RM, DP)                                                \
        andxz_ld(W(RG), W(RM), W(DP))

#define andyz_st(RG, RM, DP)                                                \
        andxz_st(W(RG), W(RM), W(DP))

/* orr
 * set-flags: undefined (*x), yes (*z) */

#define orryx_ri(RM, IM)                                                    \
        orrxx_ri(W(RM), W(IM))

#define orryx_mi(RM, DP, IM)                                                \
        orrxx_mi(W(RM), W(DP), W(IM))

#define orryx_rr(RG, RM)                                                    \
        orrxx_rr(W(RG), W(RM))

#define orryx_ld(RG, RM, DP)                                                \
        orrxx_ld(W(RG), W(RM), W(DP))

#define orryx_st(RG, RM, DP)                                                \
        orrxx_st(W(RG), W(RM), W(DP))


#define orryz_ri(RM, IM)                                                    \
        orrxz_ri(W(RM), W(IM))

#define orryz_mi(RM, DP, IM)                                                \
        orrxz_mi(W(RM), W(DP), W(IM))

#define orryz_rr(RG, RM)                                                    \
        orrxz_rr(W(RG), W(RM))

#define orryz_ld(RG, RM, DP)                                                \
        orrxz_ld(W(RG), W(RM), W(DP))

#define orryz_st(RG, RM, DP)                                                \
        orrxz_st(W(RG), W(RM), W(DP))

/* xor
 * set-flags: undefined (*x), yes (*z) */

#define xoryx_ri(RM, IM)                                                    \
        xorxx_ri(W(RM), W(IM))

#define xoryx_mi(RM, DP, IM)                                                \
        xorxx_mi(W(RM), W(DP), W(IM))

#define xoryx_rr(RG, RM)                                                    \
        xorxx_rr(W(RG), W(RM))

#define xoryx_ld(RG, RM, DP)                                                \
        xorxx_ld(W(RG), W(RM), W(DP))

#define xoryx_st(RG, RM, DP)                                                \
        xorxx_st(W(RG), W(RM), W(DP))


#define xoryz_ri(RM, IM)                                                    \
        xorxz_ri(W(RM), W(IM))

#define xoryz_mi(RM, DP, IM)                                                \
        xorxz_mi(W(RM), W(DP), W(IM))

#define xoryz_rr(RG, RM)                                                    \
        xorxz_rr(W(RG), W(RM))

#define xoryz_ld(RG, RM, DP)                                                \
        xorxz_ld(W(RG), W(RM), W(DP))

#define xoryz_st(RG, RM, DP)                                                \
        xorxz_st(W(RG), W(RM), W(DP))

/* not
 * set-flags: no */

#define notyx_rx(RM)                                                        \
        notxx_rx(W(RM))

#define notyx_mx(RM, DP)                                                    \
        notxx_mx(W(RM), W(DP))

/* neg
 * set-flags: undefined (*x), yes (*z) */

#define negyx_rx(RM)                                                        \
        negxx_rx(W(RM))

#define negyx_mx(RM, DP)                                                    \
        negxx_mx(W(RM), W(DP))


#define negyz_rx(RM)                                                        \
        negxz_rx(W(RM))

#define negyz_mx(RM, DP)                                                    \
        negxz_mx(W(RM), W(DP))

/* add
 * set-flags: undefined (*x), yes (*z) */

#define addyx_ri(RM, IM)                                                    \
        addxx_ri(W(RM), W(IM))

#define addyx_mi(RM, DP, IM)                                                \
        addxx_mi(W(RM), W(DP), W(IM))

#define addyx_rr(RG, RM)                                                    \
        addxx_rr(W(RG), W(RM))

#define addyx_ld(RG, RM, DP)                                                \
        addxx_ld(W(RG), W(RM), W(DP))

#define addyx_st(RG, RM, DP)                                                \
        addxx_st(W(RG), W(RM), W(DP))


#define addyz_ri(RM, IM)                                                    \
        addxz_ri(W(RM), W(IM))

#define addyz_mi(RM, DP, IM)                                                \
        addxz_mi(W(RM), W(DP), W(IM))

#define addyz_rr(RG, RM)                                                    \
        addxz_rr(W(RG), W(RM))

#define addyz_ld(RG, RM, DP)                                                \
        addxz_ld(W(RG), W(RM), W(DP))

#define addyz_st(RG, RM, DP)                                                \
        addxz_st(W(RG), W(RM), W(DP))

/* sub
 * set-flags: undefined (*x), yes (*z) */

#define subyx_ri(RM, IM)                                                    \
        subxx_ri(W(RM), W(IM))

#define subyx_mi(RM, DP, IM)                                                \
        subxx_mi(W(RM), W(DP), W(IM))

#define subyx_rr(RG, RM)                                                    \
        subxx_rr(W(RG), W(RM))

#define subyx_ld(RG, RM, DP)                                                \
        subxx_ld(W(RG), W(RM), W(DP))

#define subyx_st(RG, RM, DP)                                                \
        subxx_st(W(RG), W(RM), W(DP))

#define subyx_mr(RM, DP, RG)                                                \
        subyx_st(W(RG), W(RM), W(DP))


#define subyz_ri(RM, IM)                                                    \
        subxz_ri(W(RM), W(IM))

#define subyz_mi(RM, DP, IM)                                                \
        subxz_mi(W(RM), W(DP), W(IM))

#define subyz_rr(RG, RM)                                                    \
        subxz_rr(W(RG), W(RM))

#define subyz_ld(RG, RM, DP)                                                \
        subxz_ld(W(RG), W(RM), W(DP))

#define subyz_st(RG, RM, DP)                                                \
        subxz_st(W(RG), W(RM), W(DP))

#define subyz_mr(RM, DP, RG)                                                \
        subyz_st(W(RG), W(RM), W(DP))

/* shl
 * set-flags: undefined (*x), yes (*z) */

#define shlyx_rx(RM)                     /* reads Recx for shift value */   \
        shlxx_rx(W(RM))

#define shlyx_mx(RM, DP)                 /* reads Recx for shift value */   \
        shlxx_mx(W(RM), W(DP))

#define shlyx_ri(RM, IM)                                                    \
        shlxx_ri(W(RM), W(IM))

#define shlyx_mi(RM, DP, IM)                                                \
        shlxx_mi(W(RM), W(DP), W(IM))

#define shlyx_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        shlxx_rr(W(RG), W(RM))

#define shlyx_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        shlxx_ld(W(RG), W(RM), W(DP))

#define shlyx_st(RG, RM, DP)                                                \
        shlxx_st(W(RG), W(RM), W(DP))

#define shlyx_mr(RM, DP, RG)                                                \
        shlyx_st(W(RG), W(RM), W(DP))


#define shlyz_rx(RM)                     /* reads Recx for shift value */   \
        shlxz_rx(W(RM))

#define shlyz_mx(RM, DP)                 /* reads Recx for shift value */   \
        shlxz_mx(W(RM), W(DP))

#define shlyz_ri(RM, IM)                                                    \
        shlxz_ri(W(RM), W(IM))

#define shlyz_mi(RM, DP, IM)                                                \
        shlxz_mi(W(RM), W(DP), W(IM))

#define shlyz_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        shlxz_rr(W(RG), W(RM))

#define shlyz_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        shlxz_ld(W(RG), W(RM), W(DP))

#define shlyz_st(RG, RM, DP)                                                \
        shlxz_st(W(RG), W(RM), W(DP))

#define shlyz_mr(RM, DP, RG)                                                \
        shlyz_st(W(RG), W(RM), W(DP))

/* shr
 * set-flags: undefined (*x), yes (*z) */

#define shryx_rx(RM)                     /* reads Recx for shift value */   \
        shrxx_rx(W(RM))

#define shryx_mx(RM, DP)                 /* reads Recx for shift value */   \
        shrxx_mx(W(RM), W(DP))

#define shryx_ri(RM, IM)                                                    \
        shrxx_ri(W(RM), W(IM))

#define shryx_mi(RM, DP, IM)                                                \
        shrxx_mi(W(RM), W(DP), W(IM))

#define shryx_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        shrxx_rr(W(RG), W(RM))

#define shryx_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        shrxx_ld(W(RG), W(RM), W(DP))

#define shryx_st(RG, RM, DP)                                                \
        shrxx_st(W(RG), W(RM), W(DP))

#define shryx_mr(RM, DP, RG)                                                \
        shryx_st(W(RG), W(RM), W(DP))


#define shryz_rx(RM)                     /* reads Recx for shift value */   \
        shrxz_rx(W(RM))

#define shryz_mx(RM, DP)                 /* reads Recx for shift value */   \
        shrxz_mx(W(RM), W(DP))

#define shryz_ri(RM, IM)                                                    \
        shrxz_ri(W(RM), W(IM))

#define shryz_mi(RM, DP, IM)                                                \
        shrxz_mi(W(RM), W(DP), W(IM))

#define shryz_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        shrxz_rr(W(RG), W(RM))

#define shryz_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        shrxz_ld(W(RG), W(RM), W(DP))

#define shryz_st(RG, RM, DP)                                                \
        shrxz_st(W(RG), W(RM), W(DP))

#define shryz_mr(RM, DP, RG)                                                \
        shryz_st(W(RG), W(RM), W(DP))


#define shryn_rx(RM)                     /* reads Recx for shift value */   \
        shrxn_rx(W(RM))

#define shryn_mx(RM, DP)                 /* reads Recx for shift value */   \
        shrxn_mx(W(RM), W(DP))

#define shryn_ri(RM, IM)                                                    \
        shrxn_ri(W(RM), W(IM))

#define shryn_mi(RM, DP, IM)                                                \
        shrxn_mi(W(RM), W(DP), W(IM))

#define shryn_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        shrxn_rr(W(RG), W(RM))

#define shryn_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        shrxn_ld(W(RG), W(RM), W(DP))

#define shryn_st(RG, RM, DP)                                                \
        shrxn_st(W(RG), W(RM), W(DP))

#define shryn_mr(RM, DP, RG)                                                \
        shryn_st(W(RG), W(RM), W(DP))

/* mul
 * set-flags: undefined */

#define mulyx_ri(RM, IM)                                                    \
        mulxx_ri(W(RM), W(IM))

#define mulyx_rr(RG, RM)                                                    \
        mulxx_rr(W(RG), W(RM))

#define mulyx_ld(RG, RM, DP)                                                \
        mulxx_ld(W(RG), W(RM), W(DP))


#define mulyx_xr(RM)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        mulxx_xr(W(RM))

#define mulyx_xm(RM, DP) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        mulxx_xm(W(RM), W(DP))


#define mulyn_xr(RM)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        mulxn_xr(W(RM))

#define mulyn_xm(RM, DP) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        mulxn_xm(W(RM), W(DP))


#define mulyp_xr(RM)     /* Reax is in/out, prepares Redx for divyn_x* */   \
        mulxp_xr(W(RM))       /* product must not exceed operands size */

#define mulyp_xm(RM, DP) /* Reax is in/out, prepares Redx for divyn_x* */   \
        mulxp_xm(W(RM), W(DP))/* product must not exceed operands size */

/* div
 * set-flags: undefined */

#define divyx_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        divxx_ri(W(RM), W(IM))

#define divyx_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        divxx_rr(W(RG), W(RM))

#define divyx_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        divxx_ld(W(RG), W(RM), W(DP))


#define divyn_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        divxn_ri(W(RM), W(IM))

#define divyn_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        divxn_rr(W(RG), W(RM))

#define divyn_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        divxn_ld(W(RG), W(RM), W(DP))


#define preyx_xx()          /* to be placed immediately prior divyx_x* */   \
        prexx_xx()                   /* to prepare Redx for int-divide */

#define preyn_xx()          /* to be placed immediately prior divyn_x* */   \
        prexn_xx()                   /* to prepare Redx for int-divide */


#define divyx_xr(RM)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        divxx_xr(W(RM))

#define divyx_xm(RM, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        divxx_xm(W(RM), W(DP))


#define divyn_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divxn_xr(W(RM))

#define divyn_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divxn_xm(W(RM), W(DP))


#define divyp_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divxp_xr(W(RM))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divyp_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divxp_xm(W(RM), W(DP))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem
 * set-flags: undefined */

#define remyx_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        remxx_ri(W(RM), W(IM))

#define remyx_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        remxx_rr(W(RG), W(RM))

#define remyx_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        remxx_ld(W(RG), W(RM), W(DP))


#define remyn_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        remxn_ri(W(RM), W(IM))

#define remyn_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        remxn_rr(W(RG), W(RM))

#define remyn_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        remxn_ld(W(RG), W(RM), W(DP))


#define remyx_xx()          /* to be placed immediately prior divyx_x* */   \
        remxx_xx()                   /* to prepare for rem calculation */

#define remyx_xr(RM)        /* to be placed immediately after divyx_xr */   \
        remxx_xr(W(RM))              /* to produce remainder Redx<-rem */

#define remyx_xm(RM, DP)    /* to be placed immediately after divyx_xm */   \
        remxx_xm(W(RM), W(DP))       /* to produce remainder Redx<-rem */


#define remyn_xx()          /* to be placed immediately prior divyn_x* */   \
        remxn_xx()                   /* to prepare for rem calculation */

#define remyn_xr(RM)        /* to be placed immediately after divyn_xr */   \
        remxn_xr(W(RM))              /* to produce remainder Redx<-rem */

#define remyn_xm(RM, DP)    /* to be placed immediately after divyn_xm */   \
        remxn_xm(W(RM), W(DP))       /* to produce remainder Redx<-rem */

/* arj
 * set-flags: undefined
 * refer to individual instructions' description
 * to stay within special register limitations */

#define arjyx_rx(RM, op, cc, lb)                                            \
        arjxx_rx(W(RM), op, cc, lb)

#define arjyx_mx(RM, DP, op, cc, lb)                                        \
        arjxx_mx(W(RM), W(DP), op, cc, lb)

#define arjyx_ri(RM, IM, op, cc, lb)                                        \
        arjxx_ri(W(RM), W(IM), op, cc, lb)

#define arjyx_mi(RM, DP, IM, op, cc, lb)                                    \
        arjxx_mi(W(RM), W(DP), W(IM), op, cc, lb)

#define arjyx_rr(RG, RM, op, cc, lb)                                        \
        arjxx_rr(W(RG), W(RM), op, cc, lb)

#define arjyx_ld(RG, RM, DP, op, cc, lb)                                    \
        arjxx_ld(W(RG), W(RM), W(DP), op, cc, lb)

#define arjyx_st(RG, RM, DP, op, cc, lb)                                    \
        arjxx_st(W(RG), W(RM), W(DP), op, cc, lb)

#define arjyx_mr(RM, DP, RG, op, cc, lb)                                    \
        arjyx_st(W(RG), W(RM), W(DP), op, cc, lb)

/* cmj
 * set-flags: undefined */

#define cmjyx_rz(RM, cc, lb)                                                \
        cmjyx_ri(W(RM), IC(0), cc, lb)

#define cmjyx_mz(RM, DP, cc, lb)                                            \
        cmjyx_mi(W(RM), W(DP), IC(0), cc, lb)

#define cmjyx_ri(RM, IM, cc, lb)                                            \
        cmjxx_ri(W(RM), W(IM), cc, lb)

#define cmjyx_mi(RM, DP, IM, cc, lb)                                        \
        cmjxx_mi(W(RM), W(DP), W(IM), cc, lb)

#define cmjyx_rr(RG, RM, cc, lb)                                            \
        cmjxx_rr(W(RG), W(RM), cc, lb)

#define cmjyx_rm(RG, RM, DP, cc, lb)                                        \
        cmjxx_rm(W(RG), W(RM), W(DP), cc, lb)

#define cmjyx_mr(RM, DP, RG, cc, lb)                                        \
        cmjxx_mr(W(RM), W(DP), W(RG), cc, lb)

/* cmp
 * set-flags: yes */

#define cmpyx_ri(RM, IM)                                                    \
        cmpxx_ri(W(RM), W(IM))

#define cmpyx_mi(RM, DP, IM)                                                \
        cmpxx_mi(W(RM), W(DP), W(IM))

#define cmpyx_rr(RG, RM)                                                    \
        cmpxx_rr(W(RG), W(RM))

#define cmpyx_rm(RG, RM, DP)                                                \
        cmpxx_rm(W(RG), W(RM), W(DP))

#define cmpyx_mr(RM, DP, RG)                                                \
        cmpxx_mr(W(RM), W(DP), W(RG))

#endif /* RT_ELEMENT */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

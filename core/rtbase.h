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
typedef rt_fp32             rt_real;

typedef rt_fp32             rt_vec2[2];
typedef rt_fp32             rt_mat2[2][2];

typedef rt_fp32             rt_vec3[3];
typedef rt_fp32             rt_mat3[3][3];

typedef rt_fp32             rt_vec4[4];
typedef rt_fp32             rt_mat4[4][4];

/* fixed-size integer types */
typedef char                rt_si08;
typedef unsigned char       rt_ui08;

typedef short               rt_si16;
typedef unsigned short      rt_ui16;

typedef int                 rt_si32;
typedef unsigned int        rt_ui32;

#if   defined (RT_WIN32) /* Win32, MSVC -------- for older versions --------- */

typedef __int64             rt_si64;
typedef unsigned __int64    rt_ui64;

#define RT_PR64 /*printf*/  "I64"

#elif defined (RT_LINUX) /* Linux, GCC --------- for newer versions --------- */

typedef long long           rt_si64;
typedef unsigned long long  rt_ui64;

#define RT_PR64 /*printf*/  "ll"

#endif /* ------------- OS specific ----------------------------------------- */

/* adjustable integer types */
typedef rt_si08             rt_bool;
typedef long                rt_size;
typedef rt_si64             rt_time;

typedef rt_si08             rt_char;
typedef rt_si16             rt_shrt;
typedef long                rt_cell;
typedef rt_si64             rt_long;

typedef rt_ui08             rt_byte;
typedef rt_ui16             rt_half;
typedef unsigned long       rt_word;
typedef rt_ui64             rt_full;

typedef void                rt_void;
typedef void               *rt_pntr;

typedef const char          rt_astr[];
typedef const char         *rt_pstr;

/*
 * Generic definitions
 */
#define RT_NULL             0

#if   RT_POINTER == 32
#define RT_ALIGN            4
#define RT_QUAD_ALIGN       16 /* not dependent on SIMD align */
#elif RT_POINTER == 64
#define RT_ALIGN            8
#define RT_QUAD_ALIGN       32 /* not dependent on SIMD align */
#else  /* RT_POINTER */
#error "unsupported pointer size, check RT_POINTER in makefiles"
#endif /* RT_POINTER */

#define RT_FALSE            0
#define RT_TRUE             1

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
#define RT_ABS(a)           (abs((rt_si32)(a)))

#define RT_FABS(a)          (fabsf(a))

#define RT_FLOOR(a)         ((rt_si32)floorf(a))

#define RT_SIGN(a)          ((a)  <    0.0f ? -1 :                          \
                             (a)  >    0.0f ? +1 :                          \
                              0)

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

    rt_ui32 scr00;          /* scratchpad 00 */
#define inf_SCR00           DP(0x000)

    rt_ui32 ver;            /* SIMD version <- cpuid */
#define inf_VER             DP(0x004)

    rt_ui32 pad01;          /* reserved, do not use! */
#define inf_PAD01           DP(0x008)

    rt_ui32 fctrl[S-3];     /* reserved, do not use! */
#define inf_FCTRL(nx)       DP(0x00C + nx)

    /* general purpose constants */

    rt_real gpc01[S];       /* +1.0 */
#define inf_GPC01           DP(Q*0x010)

    rt_real gpc02[S];       /* -0.5 */
#define inf_GPC02           DP(Q*0x020)

    rt_real gpc03[S];       /* +3.0 */
#define inf_GPC03           DP(Q*0x030)

    rt_ui32 gpc04[S];       /* 0x7FFFFFFF */
#define inf_GPC04           DP(Q*0x040)

    rt_ui32 gpc05[S];       /* 0x3F800000 */
#define inf_GPC05           DP(Q*0x050)

    rt_ui32 scr01[S];       /* scratchpad 01 */
#define inf_SCR01(nx)       DP(Q*0x060 + nx)

    rt_ui32 scr02[S];       /* scratchpad 02 */
#define inf_SCR02(nx)       DP(Q*0x070 + nx)

    rt_ui64 regs[S/2];      /* SIMD reg-file storage */
#define inf_REGS            DP(Q*0x080+E)

    rt_real pad02[S*7];     /* reserved, do not use! */
#define inf_PAD02           DP(Q*0x090)

};

struct rt_SIMD_REGS
{
    /* register file */

    rt_real file[S*32];
#define reg_FILE            DP(Q*0x000)

};

#define ASM_INIT(__Info__, __Regs__)                                        \
    RT_SIMD_SET(__Info__->gpc01, +1.0f);                                    \
    RT_SIMD_SET(__Info__->gpc02, -0.5f);                                    \
    RT_SIMD_SET(__Info__->gpc03, +3.0f);                                    \
    RT_SIMD_SET(__Info__->gpc04, 0x7FFFFFFF);                               \
    RT_SIMD_SET(__Info__->gpc05, 0x3F800000);                               \
    __Info__->regs[0] = (rt_ui64)(rt_word)__Regs__;

#define ASM_DONE(__Info__)

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

#define rcpps_rr(RG, RM) /* destroys RM */                                  \
        rceps_rr(W(RG), W(RM))                                              \
        rcsps_rr(W(RG), W(RM)) /* <- not reusable without extra temp reg */

/* rsq
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rsqps_rr(RG, RM) /* destroys RM */                                  \
        rseps_rr(W(RG), W(RM))                                              \
        rssps_rr(W(RG), W(RM)) /* <- not reusable without extra temp reg */

/****************** original FCTRL blocks (cannot be nested) ******************/

#define FCTRL_ENTER(mode) /* assumes default mode (ROUNDN) upon entry */    \
        FCTRL_SET(mode)

#define FCTRL_LEAVE(mode) /* resumes default mode (ROUNDN) upon leave */    \
        FCTRL_RESET()

#endif /* RT_RTBASE_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

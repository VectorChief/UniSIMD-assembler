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
 * rt_32mMxN, rt_64mMxN and rt_matMxN, where M and N are integers {1, 2, 3, 4},
 * are reserved for rectangular matrices of rt_fp32, rt_fp64 and rt_real types.
 * rt_fp16 and rt_f128 are reserved for half and quad precision floating point.
 * rt_s128 and rt_u128 are reserved for signed and unsigned 128b integer types.
 */

/* fixed-size floating point types */
typedef float               rt_fp32;

typedef rt_fp32             rt_32v2[2];
typedef rt_fp32             rt_32m2[2][2];

typedef rt_fp32             rt_32v3[3];
typedef rt_fp32             rt_32m3[3][3];

typedef rt_fp32             rt_32v4[4];
typedef rt_fp32             rt_32m4[4][4];

typedef double              rt_fp64;

typedef rt_fp64             rt_64v2[2];
typedef rt_fp64             rt_64m2[2][2];

typedef rt_fp64             rt_64v3[3];
typedef rt_fp64             rt_64m3[3][3];

typedef rt_fp64             rt_64v4[4];
typedef rt_fp64             rt_64m4[4][4];

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
#define    PR_Z /*printf*/  "I64"

typedef unsigned __int64    rt_ui64;
#define    PRuZ /*printf*/  "I64u"

#else /* --- Win64, GCC --- Linux, GCC -------------------------------------- */

typedef long long           rt_si64;
#define    PR_Z /*printf*/  "ll"

typedef unsigned long long  rt_ui64;
#define    PRuZ /*printf*/  "llu"

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

typedef rt_ui32             rt_uelm;
#define    PRuL /*printf*/  "u"

#elif RT_ELEMENT == 64

typedef rt_si64             rt_elem;
#define    PR_L /*printf*/  "ll"

typedef rt_ui64             rt_uelm;
#define    PRuL /*printf*/  "llu"

#else  /* RT_ELEMENT */

#error "unsupported element size, check RT_ELEMENT in makefiles"

#endif /* RT_ELEMENT */

/* address-size integer types */
#if   RT_ADDRESS == 32

typedef rt_si32             rt_addr;
#define    PR_A /*printf*/  ""

typedef rt_ui32             rt_uadr;
#define    PRuA /*printf*/  "u"

#elif RT_ADDRESS == 64

typedef rt_si64             rt_addr;
#define    PR_A /*printf*/  "ll"

typedef rt_ui64             rt_uadr;
#define    PRuA /*printf*/  "llu"

#else  /* RT_ADDRESS */

#error "unsupported address size, check RT_ADDRESS in makefiles"

#endif /* RT_ADDRESS */

/* pointer-size integer types */
#if   defined (RT_WIN64) /* Win64, GCC -------------------------------------- */

typedef rt_si64             rt_cell;
typedef rt_si64             rt_size;
#define    PR_P /*printf*/  "ll"

typedef rt_ui64             rt_word;
typedef rt_ui64             rt_uptr;
#define    PRuP /*printf*/  "llu"

#else /* --- Win32, MSVC -- Linux, GCC -------------------------------------- */

typedef long                rt_cell;
typedef long                rt_size;
#define    PR_P /*printf*/  "l"

typedef unsigned long       rt_word;
typedef unsigned long       rt_uptr;
#define    PRuP /*printf*/  "lu"

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
#define RT_PI               (3.14159265358979323846)
#define RT_2_PI             (2.0 * RT_PI)
#define RT_PI_2             (RT_PI / 2.0)


#define RT_INF32            (FLT_MAX)

#define RT_ABS32(a)         (abs((rt_si32)(a)))

#define RT_FABS32(a)        (fabsf(a))

#define RT_FLOOR32(a)       (floorf(a))

#define RT_CEIL32(a)        (ceilf(a))

#define RT_SIGN32(a)        ((a)  <    0.0f ? -1 :                          \
                             (a)  >    0.0f ? +1 :                          \
                              0)

#define RT_POW32(a, b)      (powf(a, b))

#define RT_SQRT32(a)        ((a)  <=   0.0f ?  0.0f :                       \
                             sqrtf(a))

#define RT_ASIN32(a)        ((a)  <=  -1.0f ? -(rt_fp32)RT_PI_2 :           \
                             (a)  >=  +1.0f ? +(rt_fp32)RT_PI_2 :           \
                             asinf(a))

#define RT_ACOS32(a)        ((a)  <=  -1.0f ? +(rt_fp32)RT_PI :             \
                             (a)  >=  +1.0f ?  0.0f :                       \
                             acosf(a))

#define RT_SINA32(a)        ((a) == -270.0f ? +1.0f :                       \
                             (a) == -180.0f ?  0.0f :                       \
                             (a) ==  -90.0f ? -1.0f :                       \
                             (a) ==    0.0f ?  0.0f :                       \
                             (a) ==  +90.0f ? +1.0f :                       \
                             (a) == +180.0f ?  0.0f :                       \
                             (a) == +270.0f ? -1.0f :                       \
                             sinf((rt_fp32)((a) * RT_PI / 180.0)))

#define RT_COSA32(a)        ((a) == -270.0f ?  0.0f :                       \
                             (a) == -180.0f ? -1.0f :                       \
                             (a) ==  -90.0f ?  0.0f :                       \
                             (a) ==    0.0f ? +1.0f :                       \
                             (a) ==  +90.0f ?  0.0f :                       \
                             (a) == +180.0f ? -1.0f :                       \
                             (a) == +270.0f ?  0.0f :                       \
                             cosf((rt_fp32)((a) * RT_PI / 180.0)))


#define RT_INF64            (DBL_MAX)

#define RT_ABS64(a)         (llabs((rt_si64)(a)))

#define RT_FABS64(a)        (fabs(a))

#define RT_FLOOR64(a)       (floor(a))

#define RT_CEIL64(a)        (ceil(a))

#define RT_SIGN64(a)        ((a)  <    0.0 ? -1 :                           \
                             (a)  >    0.0 ? +1 :                           \
                              0)

#define RT_POW64(a, b)      (pow(a, b))

#define RT_SQRT64(a)        ((a)  <=   0.0 ?  0.0 :                         \
                             sqrt(a))

#define RT_ASIN64(a)        ((a)  <=  -1.0 ? -(rt_fp64)RT_PI_2 :            \
                             (a)  >=  +1.0 ? +(rt_fp64)RT_PI_2 :            \
                             asin(a))

#define RT_ACOS64(a)        ((a)  <=  -1.0 ? +(rt_fp64)RT_PI :              \
                             (a)  >=  +1.0 ?  0.0 :                         \
                             acos(a))

#define RT_SINA64(a)        ((a) == -270.0 ? +1.0 :                         \
                             (a) == -180.0 ?  0.0 :                         \
                             (a) ==  -90.0 ? -1.0 :                         \
                             (a) ==    0.0 ?  0.0 :                         \
                             (a) ==  +90.0 ? +1.0 :                         \
                             (a) == +180.0 ?  0.0 :                         \
                             (a) == +270.0 ? -1.0 :                         \
                             sin((rt_fp64)((a) * RT_PI / 180.0)))

#define RT_COSA64(a)        ((a) == -270.0 ?  0.0 :                         \
                             (a) == -180.0 ? -1.0 :                         \
                             (a) ==  -90.0 ?  0.0 :                         \
                             (a) ==    0.0 ? +1.0 :                         \
                             (a) ==  +90.0 ?  0.0 :                         \
                             (a) == +180.0 ? -1.0 :                         \
                             (a) == +270.0 ?  0.0 :                         \
                             cos((rt_fp64)((a) * RT_PI / 180.0)))

#if   RT_ELEMENT == 32

#define RT_INF              RT_INF32
#define RT_ABS(a)           RT_ABS32(a)
#define RT_FABS(a)          RT_FABS32(a)
#define RT_FLOOR(a)         RT_FLOOR32(a)
#define RT_CEIL(a)          RT_CEIL32(a)
#define RT_SIGN(a)          RT_SIGN32(a)
#define RT_POW(a, b)        RT_POW32(a, b)
#define RT_SQRT(a)          RT_SQRT32(a)
#define RT_ASIN(a)          RT_ASIN32(a)
#define RT_ACOS(a)          RT_ACOS32(a)
#define RT_SINA(a)          RT_SINA32(a)
#define RT_COSA(a)          RT_COSA32(a)

#elif RT_ELEMENT == 64

#define RT_INF              RT_INF64
#define RT_ABS(a)           RT_ABS64(a)
#define RT_FABS(a)          RT_FABS64(a)
#define RT_FLOOR(a)         RT_FLOOR64(a)
#define RT_CEIL(a)          RT_CEIL64(a)
#define RT_SIGN(a)          RT_SIGN64(a)
#define RT_POW(a, b)        RT_POW64(a, b)
#define RT_SQRT(a)          RT_SQRT64(a)
#define RT_ASIN(a)          RT_ASIN64(a)
#define RT_ACOS(a)          RT_ACOS64(a)
#define RT_SINA(a)          RT_SINA64(a)
#define RT_COSA(a)          RT_COSA64(a)

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

    rt_ui64 regs;           /* SIMD reg-file storage */
#define inf_REGS            DP(0x000+C)

    rt_ui32 ver;            /* SIMD version <- cpuid */
#define inf_VER             DP(0x008)

    rt_ui32 fctrl[R-3];     /* reserved, do not use! */
#define inf_FCTRL(nx)       DP(0x00C + nx)

    /* general purpose constants (32-bit) */

    rt_fp32 gpc01_32[R];    /* +1.0f */
#define inf_GPC01_32        DP(Q*0x010)

    rt_fp32 gpc02_32[R];    /* -0.5f */
#define inf_GPC02_32        DP(Q*0x020)

    rt_fp32 gpc03_32[R];    /* +3.0f */
#define inf_GPC03_32        DP(Q*0x030)

    rt_si32 gpc04_32[R];    /* 0x7FFFFFFF */
#define inf_GPC04_32        DP(Q*0x040)

    rt_si32 gpc05_32[R];    /* 0x3F800000 */
#define inf_GPC05_32        DP(Q*0x050)

    rt_si32 gpc06_32[R];    /* 0x80000000 */
#define inf_GPC06_32        DP(Q*0x060)

    /* internal variables */

    rt_elem scr01[S];       /* scratchpad1, SIMD ISA */
#define inf_SCR01(nx)       DP(Q*0x070 + nx)

    rt_elem scr02[S];       /* scratchpad2, SIMD ISA */
#define inf_SCR02(nx)       DP(Q*0x080 + nx)

    rt_si32 gpc07[R];       /* 0xFFFFFFFF */
#define inf_GPC07           DP(Q*0x090)

    /* general purpose constants (64-bit) */

    rt_fp64 gpc01_64[T];    /* +1.0 */
#define inf_GPC01_64        DP(Q*0x0A0)

    rt_fp64 gpc02_64[T];    /* -0.5 */
#define inf_GPC02_64        DP(Q*0x0B0)

    rt_fp64 gpc03_64[T];    /* +3.0 */
#define inf_GPC03_64        DP(Q*0x0C0)

    rt_si64 gpc04_64[T];    /* 0x7FFFFFFFFFFFFFFF */
#define inf_GPC04_64        DP(Q*0x0D0)

    rt_si64 gpc05_64[T];    /* 0x3FF0000000000000 */
#define inf_GPC05_64        DP(Q*0x0E0)

    rt_si64 gpc06_64[T];    /* 0x8000000000000000 */
#define inf_GPC06_64        DP(Q*0x0F0)

};

#if   RT_ELEMENT == 32

#define inf_GPC01           inf_GPC01_32
#define inf_GPC02           inf_GPC02_32
#define inf_GPC03           inf_GPC03_32
#define inf_GPC04           inf_GPC04_32
#define inf_GPC05           inf_GPC05_32
#define inf_GPC06           inf_GPC06_32

#define RT_SIMD_WIDTH       RT_SIMD_WIDTH32
#define RT_SIMD_SET(s, v)   RT_SIMD_SET32(s, v)

#elif RT_ELEMENT == 64

#define inf_GPC01           inf_GPC01_64
#define inf_GPC02           inf_GPC02_64
#define inf_GPC03           inf_GPC03_64
#define inf_GPC04           inf_GPC04_64
#define inf_GPC05           inf_GPC05_64
#define inf_GPC06           inf_GPC06_64

#define RT_SIMD_WIDTH       RT_SIMD_WIDTH64
#define RT_SIMD_SET(s, v)   RT_SIMD_SET64(s, v)

#endif /* RT_ELEMENT */


struct rt_SIMD_REGS
{
    /* register file */

    rt_real file[S*32];
#define reg_FILE            DP(Q*0x000)

};

#define ASM_INIT(__Info__, __Regs__)                                        \
    RT_SIMD_SET32(__Info__->gpc01_32, +1.0f);                               \
    RT_SIMD_SET32(__Info__->gpc02_32, -0.5f);                               \
    RT_SIMD_SET32(__Info__->gpc03_32, +3.0f);                               \
    RT_SIMD_SET32(__Info__->gpc04_32, 0x7FFFFFFF);                          \
    RT_SIMD_SET32(__Info__->gpc05_32, 0x3F800000);                          \
    RT_SIMD_SET32(__Info__->gpc06_32, 0x80000000);                          \
    RT_SIMD_SET32(__Info__->gpc07,    0xFFFFFFFF);                          \
    RT_SIMD_SET64(__Info__->gpc01_64, +1.0);                                \
    RT_SIMD_SET64(__Info__->gpc02_64, -0.5);                                \
    RT_SIMD_SET64(__Info__->gpc03_64, +3.0);                                \
    RT_SIMD_SET64(__Info__->gpc04_64, 0x7FFFFFFFFFFFFFFF);                  \
    RT_SIMD_SET64(__Info__->gpc05_64, 0x3FF0000000000000);                  \
    RT_SIMD_SET64(__Info__->gpc06_64, 0x8000000000000000);                  \
    __Info__->regs = (rt_ui64)(rt_word)__Regs__;

#define ASM_DONE(__Info__)

/******************************************************************************/
/************************   COMMON SIMD INSTRUCTIONS   ************************/
/******************************************************************************/

/***************** element-sized adjustable SIMD instructions *****************/

/* cbr */

/*
 * Based on the original idea by Russell Borogove (kaleja[AT]estarcion[DOT]com)
 * available at http://www.musicdsp.org/showone.php?id=206
 * converted to S-way SIMD version by VectorChief.
 */
#define cbrps_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbeps_rr(W(XD), W(X1), W(X2), W(XS))                                \
        cbsps_rr(W(XD), W(X1), W(X2), W(XS))                                \
        cbsps_rr(W(XD), W(X1), W(X2), W(XS))                                \
        cbsps_rr(W(XD), W(X1), W(X2), W(XS))

#define cbeps_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        /* cube root estimate, the exponent is divided by three             \
         * in such a way that remainder bits get shoved into                \
         * the top of the normalized mantissa */                            \
        movpx_ld(W(X2), Mebp, inf_GPC04)                                    \
        movpx_rr(W(XD), W(XS))                                              \
        andpx_rr(W(XD), W(X2))   /* exponent & mantissa in biased-127 */    \
        subpx_ld(W(XD), Mebp, inf_GPC05) /* convert to 2's complement */    \
        shrpn_ri(W(XD), IB(10))  /* XD / 1024 */                            \
        movpx_rr(W(X1), W(XD))   /* XD * 341 (next 8 ops) */                \
        shlpx_ri(W(X1), IB(2))                                              \
        addpx_rr(W(XD), W(X1))                                              \
        shlpx_ri(W(X1), IB(2))                                              \
        addpx_rr(W(XD), W(X1))                                              \
        shlpx_ri(W(X1), IB(2))                                              \
        addpx_rr(W(XD), W(X1))                                              \
        shlpx_ri(W(X1), IB(2))                                              \
        addpx_rr(W(XD), W(X1))   /* XD * (341/1024) ~= XD * (0.333) */      \
        addpx_ld(W(XD), Mebp, inf_GPC05) /* back to biased-127 */           \
        andpx_rr(W(XD), W(X2))   /* remask exponent & mantissa */           \
        annpx_rr(W(X2), W(XS))   /* original sign */                        \
        orrpx_rr(W(XD), W(X2))   /* new exponent & mantissa, old sign */

#define cbsps_rr(XG, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        movpx_rr(W(X1), W(XG))                                              \
        mulps_rr(W(X1), W(XG))                                              \
        movpx_rr(W(X2), W(X1))                                              \
        mulps_ld(W(X1), Mebp, inf_GPC03)                                    \
        rceps_rr(W(X1), W(X1))                                              \
        mulps_rr(W(X2), W(XG))                                              \
        subps_rr(W(X2), W(XS))                                              \
        mulps_rr(W(X2), W(X1))                                              \
        subps_rr(W(XG), W(X2))

/* rcp
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP == 0

#define rcpps_rr(XD, XS) /* destroys XS */                                  \
        rceps_rr(W(XD), W(XS))                                              \
        rcsps_rr(W(XD), W(XS)) /* <- not reusable without extra temp reg */

#else /* RT_SIMD_COMPAT_RCP */

#define rcpps_rr(XD, XS) /* destroys XS */                                  \
        movpx_ld(W(XD), Mebp, inf_GPC01)                                    \
        divps_rr(W(XD), W(XS))

#define rceps_rr(XD, XS)                                                    \
        movpx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movpx_ld(W(XD), Mebp, inf_GPC01)                                    \
        divps_ld(W(XD), Mebp, inf_SCR02(0))

#define rcsps_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

/* rsq
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ == 0

#define rsqps_rr(XD, XS) /* destroys XS */                                  \
        rseps_rr(W(XD), W(XS))                                              \
        rssps_rr(W(XD), W(XS)) /* <- not reusable without extra temp reg */

#else /* RT_SIMD_COMPAT_RSQ */

#define rsqps_rr(XD, XS) /* destroys XS */                                  \
        sqrps_rr(W(XS), W(XS))                                              \
        movpx_ld(W(XD), Mebp, inf_GPC01)                                    \
        divps_rr(W(XD), W(XS))

#define rseps_rr(XD, XS)                                                    \
        sqrps_rr(W(XD), W(XS))                                              \
        movpx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movpx_ld(W(XD), Mebp, inf_GPC01)                                    \
        divps_ld(W(XD), Mebp, inf_SCR02(0))

#define rssps_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

/****************** instructions for fixed-sized 32-bit SIMD ******************/

/* cbr */

/*
 * Based on the original idea by Russell Borogove (kaleja[AT]estarcion[DOT]com)
 * available at http://www.musicdsp.org/showone.php?id=206
 * converted to S-way SIMD version by VectorChief.
 */
#define cbros_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbeos_rr(W(XD), W(X1), W(X2), W(XS))                                \
        cbsos_rr(W(XD), W(X1), W(X2), W(XS))                                \
        cbsos_rr(W(XD), W(X1), W(X2), W(XS))                                \
        cbsos_rr(W(XD), W(X1), W(X2), W(XS))

#define cbeos_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        /* cube root estimate, the exponent is divided by three             \
         * in such a way that remainder bits get shoved into                \
         * the top of the normalized mantissa */                            \
        movox_ld(W(X2), Mebp, inf_GPC04_32)                                 \
        movox_rr(W(XD), W(XS))                                              \
        andox_rr(W(XD), W(X2))   /* exponent & mantissa in biased-127 */    \
        subox_ld(W(XD), Mebp, inf_GPC05_32) /* convert to 2's complement */ \
        shron_ri(W(XD), IB(10))  /* XD / 1024 */                            \
        movox_rr(W(X1), W(XD))   /* XD * 341 (next 8 ops) */                \
        shlox_ri(W(X1), IB(2))                                              \
        addox_rr(W(XD), W(X1))                                              \
        shlox_ri(W(X1), IB(2))                                              \
        addox_rr(W(XD), W(X1))                                              \
        shlox_ri(W(X1), IB(2))                                              \
        addox_rr(W(XD), W(X1))                                              \
        shlox_ri(W(X1), IB(2))                                              \
        addox_rr(W(XD), W(X1))   /* XD * (341/1024) ~= XD * (0.333) */      \
        addox_ld(W(XD), Mebp, inf_GPC05_32) /* back to biased-127 */        \
        andox_rr(W(XD), W(X2))   /* remask exponent & mantissa */           \
        annox_rr(W(X2), W(XS))   /* original sign */                        \
        orrox_rr(W(XD), W(X2))   /* new exponent & mantissa, old sign */

#define cbsos_rr(XG, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        movox_rr(W(X1), W(XG))                                              \
        mulos_rr(W(X1), W(XG))                                              \
        movox_rr(W(X2), W(X1))                                              \
        mulos_ld(W(X1), Mebp, inf_GPC03_32)                                 \
        rceos_rr(W(X1), W(X1))                                              \
        mulos_rr(W(X2), W(XG))                                              \
        subos_rr(W(X2), W(XS))                                              \
        mulos_rr(W(X2), W(X1))                                              \
        subos_rr(W(XG), W(X2))

/* rcp
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP == 0

#define rcpos_rr(XD, XS) /* destroys XS */                                  \
        rceos_rr(W(XD), W(XS))                                              \
        rcsos_rr(W(XD), W(XS)) /* <- not reusable without extra temp reg */

#else /* RT_SIMD_COMPAT_RCP */

#define rcpos_rr(XD, XS) /* destroys XS */                                  \
        movox_ld(W(XD), Mebp, inf_GPC01_32)                                 \
        divos_rr(W(XD), W(XS))

#define rceos_rr(XD, XS)                                                    \
        movox_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movox_ld(W(XD), Mebp, inf_GPC01_32)                                 \
        divos_ld(W(XD), Mebp, inf_SCR02(0))

#define rcsos_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

/* rsq
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ == 0

#define rsqos_rr(XD, XS) /* destroys XS */                                  \
        rseos_rr(W(XD), W(XS))                                              \
        rssos_rr(W(XD), W(XS)) /* <- not reusable without extra temp reg */

#else /* RT_SIMD_COMPAT_RSQ */

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

/****************** instructions for fixed-sized 64-bit SIMD ******************/

/* cbr */

/*
 * Based on the original idea by Russell Borogove (kaleja[AT]estarcion[DOT]com)
 * available at http://www.musicdsp.org/showone.php?id=206
 * converted to S-way SIMD version by VectorChief.
 */
#define cbrqs_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbeqs_rr(W(XD), W(X1), W(X2), W(XS))                                \
        cbsqs_rr(W(XD), W(X1), W(X2), W(XS))                                \
        cbsqs_rr(W(XD), W(X1), W(X2), W(XS))                                \
        cbsqs_rr(W(XD), W(X1), W(X2), W(XS))

#define cbeqs_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        /* cube root estimate, the exponent is divided by three             \
         * in such a way that remainder bits get shoved into                \
         * the top of the normalized mantissa */                            \
        movqx_ld(W(X2), Mebp, inf_GPC04_64)                                 \
        movqx_rr(W(XD), W(XS))                                              \
        andqx_rr(W(XD), W(X2))   /* exponent & mantissa in biased-127 */    \
        subqx_ld(W(XD), Mebp, inf_GPC05_64) /* convert to 2's complement */ \
        shrqn_ri(W(XD), IB(10))  /* XD / 1024 */                            \
        movqx_rr(W(X1), W(XD))   /* XD * 341 (next 8 ops) */                \
        shlqx_ri(W(X1), IB(2))                                              \
        addqx_rr(W(XD), W(X1))                                              \
        shlqx_ri(W(X1), IB(2))                                              \
        addqx_rr(W(XD), W(X1))                                              \
        shlqx_ri(W(X1), IB(2))                                              \
        addqx_rr(W(XD), W(X1))                                              \
        shlqx_ri(W(X1), IB(2))                                              \
        addqx_rr(W(XD), W(X1))   /* XD * (341/1024) ~= XD * (0.333) */      \
        addqx_ld(W(XD), Mebp, inf_GPC05_64) /* back to biased-127 */        \
        andqx_rr(W(XD), W(X2))   /* remask exponent & mantissa */           \
        annqx_rr(W(X2), W(XS))   /* original sign */                        \
        orrqx_rr(W(XD), W(X2))   /* new exponent & mantissa, old sign */

#define cbsqs_rr(XG, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        movqx_rr(W(X1), W(XG))                                              \
        mulqs_rr(W(X1), W(XG))                                              \
        movqx_rr(W(X2), W(X1))                                              \
        mulqs_ld(W(X1), Mebp, inf_GPC03_64)                                 \
        rceqs_rr(W(X1), W(X1))                                              \
        mulqs_rr(W(X2), W(XG))                                              \
        subqs_rr(W(X2), W(XS))                                              \
        mulqs_rr(W(X2), W(X1))                                              \
        subqs_rr(W(XG), W(X2))

/* rcp
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP == 0

#define rcpqs_rr(XD, XS) /* destroys XS */                                  \
        rceqs_rr(W(XD), W(XS))                                              \
        rcsqs_rr(W(XD), W(XS)) /* <- not reusable without extra temp reg */

#else /* RT_SIMD_COMPAT_RCP */

#define rcpqs_rr(XD, XS) /* destroys XS */                                  \
        movqx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divqs_rr(W(XD), W(XS))

#define rceqs_rr(XD, XS)                                                    \
        movqx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movqx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divqs_ld(W(XD), Mebp, inf_SCR02(0))

#define rcsqs_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

/* rsq
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ == 0

#define rsqqs_rr(XD, XS) /* destroys XS */                                  \
        rseqs_rr(W(XD), W(XS))                                              \
        rssqs_rr(W(XD), W(XS)) /* <- not reusable without extra temp reg */

#else /* RT_SIMD_COMPAT_RSQ */

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

/****************** original FCTRL blocks (cannot be nested) ******************/

#define FCTRL_ENTER(mode) /* assumes default mode (ROUNDN) upon entry */    \
        FCTRL_SET(mode)

#define FCTRL_LEAVE(mode) /* resumes default mode (ROUNDN) upon leave */    \
        FCTRL_RESET()

/***************** instructions for element-sized 32-bit SIMD *****************/

#if   RT_ELEMENT == 32

/* mov */

#define movpx_rr(XD, XS)                                                    \
        movox_rr(W(XD), W(XS))

#define movpx_ld(XD, MS, DP)                                                \
        movox_ld(W(XD), W(MS), W(DP))

#define movpx_st(XS, MD, DP)                                                \
        movox_st(W(XS), W(MD), W(DP))

/* and */

#define andpx_rr(XG, XS)                                                    \
        andox_rr(W(XG), W(XS))

#define andpx_ld(XG, MS, DP)                                                \
        andox_ld(W(XG), W(MS), W(DP))

/* ann (G = ~G & S) */

#define annpx_rr(XG, XS)                                                    \
        annox_rr(W(XG), W(XS))

#define annpx_ld(XG, MS, DP)                                                \
        annox_ld(W(XG), W(MS), W(DP))

/* orr */

#define orrpx_rr(XG, XS)                                                    \
        orrox_rr(W(XG), W(XS))

#define orrpx_ld(XG, MS, DP)                                                \
        orrox_ld(W(XG), W(MS), W(DP))

/* orn (G = ~G | S) */

#define ornpx_rr(XG, XS)                                                    \
        ornox_rr(W(XG), W(XS))

#define ornpx_ld(XG, MS, DP)                                                \
        ornox_ld(W(XG), W(MS), W(DP))

/* xor */

#define xorpx_rr(XG, XS)                                                    \
        xorox_rr(W(XG), W(XS))

#define xorpx_ld(XG, MS, DP)                                                \
        xorox_ld(W(XG), W(MS), W(DP))

/* not */

#define notpx_rx(XG)                                                        \
        notox_rx(W(XG))

/* neg */

#define negps_rx(XG)                                                        \
        negos_rx(W(XG))

/* add */

#define addps_rr(XG, XS)                                                    \
        addos_rr(W(XG), W(XS))

#define addps_ld(XG, MS, DP)                                                \
        addos_ld(W(XG), W(MS), W(DP))

/* sub */

#define subps_rr(XG, XS)                                                    \
        subos_rr(W(XG), W(XS))

#define subps_ld(XG, MS, DP)                                                \
        subos_ld(W(XG), W(MS), W(DP))

/* mul */

#define mulps_rr(XG, XS)                                                    \
        mulos_rr(W(XG), W(XS))

#define mulps_ld(XG, MS, DP)                                                \
        mulos_ld(W(XG), W(MS), W(DP))

/* div */

#define divps_rr(XG, XS)                                                    \
        divos_rr(W(XG), W(XS))

#define divps_ld(XG, MS, DP)                                                \
        divos_ld(W(XG), W(MS), W(DP))

/* sqr */

#define sqrps_rr(XD, XS)                                                    \
        sqros_rr(W(XD), W(XS))

#define sqrps_ld(XD, MS, DP)                                                \
        sqros_ld(W(XD), W(MS), W(DP))

/* cbr */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP == 0

#define rceps_rr(XD, XS)                                                    \
        rceos_rr(W(XD), W(XS))

#define rcsps_rr(XG, XS) /* destroys RM */                                  \
        rcsos_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ == 0

#define rseps_rr(XD, XS)                                                    \
        rseos_rr(W(XD), W(XS))

#define rssps_rr(XG, XS) /* destroys RM */                                  \
        rssos_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* min */

#define minps_rr(XG, XS)                                                    \
        minos_rr(W(XG), W(XS))

#define minps_ld(XG, MS, DP)                                                \
        minos_ld(W(XG), W(MS), W(DP))

/* max */

#define maxps_rr(XG, XS)                                                    \
        maxos_rr(W(XG), W(XS))

#define maxps_ld(XG, MS, DP)                                                \
        maxos_ld(W(XG), W(MS), W(DP))

/* cmp */

#define ceqps_rr(XG, XS)                                                    \
        ceqos_rr(W(XG), W(XS))

#define ceqps_ld(XG, MS, DP)                                                \
        ceqos_ld(W(XG), W(MS), W(DP))

#define cneps_rr(XG, XS)                                                    \
        cneos_rr(W(XG), W(XS))

#define cneps_ld(XG, MS, DP)                                                \
        cneos_ld(W(XG), W(MS), W(DP))

#define cltps_rr(XG, XS)                                                    \
        cltos_rr(W(XG), W(XS))

#define cltps_ld(XG, MS, DP)                                                \
        cltos_ld(W(XG), W(MS), W(DP))

#define cleps_rr(XG, XS)                                                    \
        cleos_rr(W(XG), W(XS))

#define cleps_ld(XG, MS, DP)                                                \
        cleos_ld(W(XG), W(MS), W(DP))

#define cgtps_rr(XG, XS)                                                    \
        cgtos_rr(W(XG), W(XS))

#define cgtps_ld(XG, MS, DP)                                                \
        cgtos_ld(W(XG), W(MS), W(DP))

#define cgeps_rr(XG, XS)                                                    \
        cgeos_rr(W(XG), W(XS))

#define cgeps_ld(XG, MS, DP)                                                \
        cgeos_ld(W(XG), W(MS), W(DP))


/* cvz (fp-to-signed-int)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnzps_rr(XD, XS)     /* round towards zero */                       \
        rnzos_rr(W(XD), W(XS))

#define rnzps_ld(XD, MS, DP) /* round towards zero */                       \
        rnzos_ld(W(XD), W(MS), W(DP))

#define cvzps_rr(XD, XS)     /* round towards zero */                       \
        cvzos_rr(W(XD), W(XS))

#define cvzps_ld(XD, MS, DP) /* round towards zero */                       \
        cvzos_ld(W(XD), W(MS), W(DP))

/* cvp (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnpps_rr(XD, XS)     /* round towards +inf */                       \
        rnpos_rr(W(XD), W(XS))

#define rnpps_ld(XD, MS, DP) /* round towards +inf */                       \
        rnpos_ld(W(XD), W(MS), W(DP))

#define cvpps_rr(XD, XS)     /* round towards +inf */                       \
        cvpos_rr(W(XD), W(XS))

#define cvpps_ld(XD, MS, DP) /* round towards +inf */                       \
        cvpos_ld(W(XD), W(MS), W(DP))

/* cvm (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnmps_rr(XD, XS)     /* round towards -inf */                       \
        rnmos_rr(W(XD), W(XS))

#define rnmps_ld(XD, MS, DP) /* round towards -inf */                       \
        rnmos_ld(W(XD), W(MS), W(DP))

#define cvmps_rr(XD, XS)     /* round towards -inf */                       \
        cvmos_rr(W(XD), W(XS))

#define cvmps_ld(XD, MS, DP) /* round towards -inf */                       \
        cvmos_ld(W(XD), W(MS), W(DP))

/* cvn (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnnps_rr(XD, XS)     /* round towards near */                       \
        rnnos_rr(W(XD), W(XS))

#define rnnps_ld(XD, MS, DP) /* round towards near */                       \
        rnnos_ld(W(XD), W(MS), W(DP))

#define cvnps_rr(XD, XS)     /* round towards near */                       \
        cvnos_rr(W(XD), W(XS))

#define cvnps_ld(XD, MS, DP) /* round towards near */                       \
        cvnos_ld(W(XD), W(MS), W(DP))

/* cvn (signed-int-to-fp)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnpn_rr(XD, XS)     /* round towards near */                       \
        cvnon_rr(W(XD), W(XS))

#define cvnpn_ld(XD, MS, DP) /* round towards near */                       \
        cvnon_ld(W(XD), W(MS), W(DP))

/* add */

#define addpx_rr(XG, XS)                                                    \
        addox_rr(W(XG), W(XS))

#define addpx_ld(XG, MS, DP)                                                \
        addox_ld(W(XG), W(MS), W(DP))

/* sub */

#define subpx_rr(XG, XS)                                                    \
        subox_rr(W(XG), W(XS))

#define subpx_ld(XG, MS, DP)                                                \
        subox_ld(W(XG), W(MS), W(DP))

/* shl */

#define shlpx_ri(XG, IM)                                                    \
        shlox_ri(W(XG), W(IM))

#define shlpx_ld(XG, MS, DP) /* loads SIMD, uses 1 elem at given address */ \
        shlox_ld(W(XG), W(MS), W(DP))

/* shr */

#define shrpx_ri(XG, IM)                                                    \
        shrox_ri(W(XG), W(IM))

#define shrpx_ld(XG, MS, DP) /* loads SIMD, uses 1 elem at given address */ \
        shrox_ld(W(XG), W(MS), W(DP))

#define shrpn_ri(XG, IM)                                                    \
        shron_ri(W(XG), W(IM))

#define shrpn_ld(XG, MS, DP) /* loads SIMD, uses 1 elem at given address */ \
        shron_ld(W(XG), W(MS), W(DP))


/* cvt (fp-to-signed-int)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndps_rr(XD, XS)                                                    \
        rndos_rr(W(XD), W(XS))

#define rndps_ld(XD, MS, DP)                                                \
        rndos_ld(W(XD), W(MS), W(DP))

#define cvtps_rr(XD, XS)                                                    \
        cvtos_rr(W(XD), W(XS))

#define cvtps_ld(XD, MS, DP)                                                \
        cvtos_ld(W(XD), W(MS), W(DP))

/* cvt (signed-int-to-fp)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvtpn_rr(XD, XS)                                                    \
        cvton_rr(W(XD), W(XS))

#define cvtpn_ld(XD, MS, DP)                                                \
        cvton_ld(W(XD), W(MS), W(DP))

/* cvr (fp-to-signed-int)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnrps_rr(XD, XS, mode)                                              \
        rnros_rr(W(XD), W(XS), mode)

#define cvrps_rr(XD, XS, mode)                                              \
        cvros_rr(W(XD), W(XS), mode)

/* mmv
 * uses Xmm0 implicitly as a mask register */

#define mmvpx_ld(XD, MS, DP) /* not portable, use conditionally */          \
        mmvox_ld(W(XD), W(MS), W(DP))

#define mmvpx_st(XS, MD, DP) /* not portable, use conditionally */          \
        mmvox_st(W(XS), W(MD), W(DP))

/***************** instructions for element-sized 64-bit SIMD *****************/

#elif RT_ELEMENT == 64

/* mov */

#define movpx_rr(XD, XS)                                                    \
        movqx_rr(W(XD), W(XS))

#define movpx_ld(XD, MS, DP)                                                \
        movqx_ld(W(XD), W(MS), W(DP))

#define movpx_st(XS, MD, DP)                                                \
        movqx_st(W(XS), W(MD), W(DP))

/* and */

#define andpx_rr(XG, XS)                                                    \
        andqx_rr(W(XG), W(XS))

#define andpx_ld(XG, MS, DP)                                                \
        andqx_ld(W(XG), W(MS), W(DP))

/* ann (G = ~G & S) */

#define annpx_rr(XG, XS)                                                    \
        annqx_rr(W(XG), W(XS))

#define annpx_ld(XG, MS, DP)                                                \
        annqx_ld(W(XG), W(MS), W(DP))

/* orr */

#define orrpx_rr(XG, XS)                                                    \
        orrqx_rr(W(XG), W(XS))

#define orrpx_ld(XG, MS, DP)                                                \
        orrqx_ld(W(XG), W(MS), W(DP))

/* orn (G = ~G | S) */

#define ornpx_rr(XG, XS)                                                    \
        ornqx_rr(W(XG), W(XS))

#define ornpx_ld(XG, MS, DP)                                                \
        ornqx_ld(W(XG), W(MS), W(DP))

/* xor */

#define xorpx_rr(XG, XS)                                                    \
        xorqx_rr(W(XG), W(XS))

#define xorpx_ld(XG, MS, DP)                                                \
        xorqx_ld(W(XG), W(MS), W(DP))

/* not */

#define notpx_rx(XG)                                                        \
        notqx_rx(W(XG))

/* neg */

#define negps_rx(XG)                                                        \
        negqs_rx(W(XG))

/* add */

#define addps_rr(XG, XS)                                                    \
        addqs_rr(W(XG), W(XS))

#define addps_ld(XG, MS, DP)                                                \
        addqs_ld(W(XG), W(MS), W(DP))

/* sub */

#define subps_rr(XG, XS)                                                    \
        subqs_rr(W(XG), W(XS))

#define subps_ld(XG, MS, DP)                                                \
        subqs_ld(W(XG), W(MS), W(DP))

/* mul */

#define mulps_rr(XG, XS)                                                    \
        mulqs_rr(W(XG), W(XS))

#define mulps_ld(XG, MS, DP)                                                \
        mulqs_ld(W(XG), W(MS), W(DP))

/* div */

#define divps_rr(XG, XS)                                                    \
        divqs_rr(W(XG), W(XS))

#define divps_ld(XG, MS, DP)                                                \
        divqs_ld(W(XG), W(MS), W(DP))

/* sqr */

#define sqrps_rr(XD, XS)                                                    \
        sqrqs_rr(W(XD), W(XS))

#define sqrps_ld(XD, MS, DP)                                                \
        sqrqs_ld(W(XD), W(MS), W(DP))

/* cbr */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP == 0

#define rceps_rr(XD, XS)                                                    \
        rceqs_rr(W(XD), W(XS))

#define rcsps_rr(XG, XS) /* destroys RM */                                  \
        rcsqs_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ == 0

#define rseps_rr(XD, XS)                                                    \
        rseqs_rr(W(XD), W(XS))

#define rssps_rr(XG, XS) /* destroys RM */                                  \
        rssqs_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* min */

#define minps_rr(XG, XS)                                                    \
        minqs_rr(W(XG), W(XS))

#define minps_ld(XG, MS, DP)                                                \
        minqs_ld(W(XG), W(MS), W(DP))

/* max */

#define maxps_rr(XG, XS)                                                    \
        maxqs_rr(W(XG), W(XS))

#define maxps_ld(XG, MS, DP)                                                \
        maxqs_ld(W(XG), W(MS), W(DP))

/* cmp */

#define ceqps_rr(XG, XS)                                                    \
        ceqqs_rr(W(XG), W(XS))

#define ceqps_ld(XG, MS, DP)                                                \
        ceqqs_ld(W(XG), W(MS), W(DP))

#define cneps_rr(XG, XS)                                                    \
        cneqs_rr(W(XG), W(XS))

#define cneps_ld(XG, MS, DP)                                                \
        cneqs_ld(W(XG), W(MS), W(DP))

#define cltps_rr(XG, XS)                                                    \
        cltqs_rr(W(XG), W(XS))

#define cltps_ld(XG, MS, DP)                                                \
        cltqs_ld(W(XG), W(MS), W(DP))

#define cleps_rr(XG, XS)                                                    \
        cleqs_rr(W(XG), W(XS))

#define cleps_ld(XG, MS, DP)                                                \
        cleqs_ld(W(XG), W(MS), W(DP))

#define cgtps_rr(XG, XS)                                                    \
        cgtqs_rr(W(XG), W(XS))

#define cgtps_ld(XG, MS, DP)                                                \
        cgtqs_ld(W(XG), W(MS), W(DP))

#define cgeps_rr(XG, XS)                                                    \
        cgeqs_rr(W(XG), W(XS))

#define cgeps_ld(XG, MS, DP)                                                \
        cgeqs_ld(W(XG), W(MS), W(DP))


/* cvz (fp-to-signed-int)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnzps_rr(XD, XS)     /* round towards zero */                       \
        rnzqs_rr(W(XD), W(XS))

#define rnzps_ld(XD, MS, DP) /* round towards zero */                       \
        rnzqs_ld(W(XD), W(MS), W(DP))

#define cvzps_rr(XD, XS)     /* round towards zero */                       \
        cvzqs_rr(W(XD), W(XS))

#define cvzps_ld(XD, MS, DP) /* round towards zero */                       \
        cvzqs_ld(W(XD), W(MS), W(DP))

/* cvp (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnpps_rr(XD, XS)     /* round towards +inf */                       \
        rnpqs_rr(W(XD), W(XS))

#define rnpps_ld(XD, MS, DP) /* round towards +inf */                       \
        rnpqs_ld(W(XD), W(MS), W(DP))

#define cvpps_rr(XD, XS)     /* round towards +inf */                       \
        cvpqs_rr(W(XD), W(XS))

#define cvpps_ld(XD, MS, DP) /* round towards +inf */                       \
        cvpqs_ld(W(XD), W(MS), W(DP))

/* cvm (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnmps_rr(XD, XS)     /* round towards -inf */                       \
        rnmqs_rr(W(XD), W(XS))

#define rnmps_ld(XD, MS, DP) /* round towards -inf */                       \
        rnmqs_ld(W(XD), W(MS), W(DP))

#define cvmps_rr(XD, XS)     /* round towards -inf */                       \
        cvmqs_rr(W(XD), W(XS))

#define cvmps_ld(XD, MS, DP) /* round towards -inf */                       \
        cvmqs_ld(W(XD), W(MS), W(DP))

/* cvn (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnnps_rr(XD, XS)     /* round towards near */                       \
        rnnqs_rr(W(XD), W(XS))

#define rnnps_ld(XD, MS, DP) /* round towards near */                       \
        rnnqs_ld(W(XD), W(MS), W(DP))

#define cvnps_rr(XD, XS)     /* round towards near */                       \
        cvnqs_rr(W(XD), W(XS))

#define cvnps_ld(XD, MS, DP) /* round towards near */                       \
        cvnqs_ld(W(XD), W(MS), W(DP))

/* cvn (signed-int-to-fp)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnpn_rr(XD, XS)     /* round towards near */                       \
        cvnqn_rr(W(XD), W(XS))

#define cvnpn_ld(XD, MS, DP) /* round towards near */                       \
        cvnqn_ld(W(XD), W(MS), W(DP))

/* add */

#define addpx_rr(XG, XS)                                                    \
        addqx_rr(W(XG), W(XS))

#define addpx_ld(XG, MS, DP)                                                \
        addqx_ld(W(XG), W(MS), W(DP))

/* sub */

#define subpx_rr(XG, XS)                                                    \
        subqx_rr(W(XG), W(XS))

#define subpx_ld(XG, MS, DP)                                                \
        subqx_ld(W(XG), W(MS), W(DP))

/* shl */

#define shlpx_ri(XG, IM)                                                    \
        shlqx_ri(W(XG), W(IM))

#define shlpx_ld(XG, MS, DP) /* loads SIMD, uses 1 elem at given address */ \
        shlqx_ld(W(XG), W(MS), W(DP))

/* shr */

#define shrpx_ri(XG, IM)                                                    \
        shrqx_ri(W(XG), W(IM))

#define shrpx_ld(XG, MS, DP) /* loads SIMD, uses 1 elem at given address */ \
        shrqx_ld(W(XG), W(MS), W(DP))

#define shrpn_ri(XG, IM)                                                    \
        shrqn_ri(W(XG), W(IM))

#define shrpn_ld(XG, MS, DP) /* loads SIMD, uses 1 elem at given address */ \
        shrqn_ld(W(XG), W(MS), W(DP))


/* cvt (fp-to-signed-int)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndps_rr(XD, XS)                                                    \
        rndqs_rr(W(XD), W(XS))

#define rndps_ld(XD, MS, DP)                                                \
        rndqs_ld(W(XD), W(MS), W(DP))

#define cvtps_rr(XD, XS)                                                    \
        cvtqs_rr(W(XD), W(XS))

#define cvtps_ld(XD, MS, DP)                                                \
        cvtqs_ld(W(XD), W(MS), W(DP))

/* cvt (signed-int-to-fp)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvtpn_rr(XD, XS)                                                    \
        cvtqn_rr(W(XD), W(XS))

#define cvtpn_ld(XD, MS, DP)                                                \
        cvtqn_ld(W(XD), W(MS), W(DP))

/* cvr (fp-to-signed-int)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnrps_rr(XD, XS, mode)                                              \
        rnrqs_rr(W(XD), W(XS), mode)

#define cvrps_rr(XD, XS, mode)                                              \
        cvrqs_rr(W(XD), W(XS), mode)

/* mmv
 * uses Xmm0 implicitly as a mask register */

#define mmvpx_ld(XD, MS, DP) /* not portable, use conditionally */          \
        mmvqx_ld(W(XD), W(MS), W(DP))

#define mmvpx_st(XS, MD, DP) /* not portable, use conditionally */          \
        mmvqx_st(W(XS), W(MD), W(DP))

#endif /* RT_ELEMENT */

/******************************************************************************/
/************************   COMMON BASE INSTRUCTIONS   ************************/
/******************************************************************************/

/***************** original forms of deprecated cmdx* aliases *****************/

/* adr */

#define adrxx_lb(lb) /* load label to Reax */                               \
        label_ld(lb)

/* mul */

#define mulxn_ri(RG, IM)                                                    \
        mulxx_ri(W(RG), W(IM))

#define mulxn_rr(RG, RS)                                                    \
        mulxx_rr(W(RG), W(RS))

#define mulxn_ld(RG, MS, DP)                                                \
        mulxx_ld(W(RG), W(MS), W(DP))

/***************** original forms of one-operand instructions *****************/

/* not */

#define notxx_rr(RG)                                                        \
        notxx_rx(W(RG))

#define notxx_mm(MG, DP)                                                    \
        notxx_mx(W(MG), W(DP))

/* neg */

#define negxx_rr(RG)                                                        \
        negxx_rx(W(RG))

#define negxx_mm(MG, DP)                                                    \
        negxx_mx(W(MG), W(DP))

/* jmp */

#define jmpxx_rr(RS)                                                        \
        jmpxx_xr(W(RS))

#define jmpxx_mm(MS, DP)                                                    \
        jmpxx_xm(W(MS), W(DP))

/***************** address-sized instructions for 32-bit mode *****************/

#if   RT_ADDRESS == 32

/* mov
 * set-flags: no */

#define movxx_ri(RD, IM)                                                    \
        movwx_ri(W(RD), W(IM))

#define movxx_mi(MD, DP, IM)                                                \
        movwx_mi(W(MD), W(DP), W(IM))

#define movxx_rr(RD, RS)                                                    \
        movwx_rr(W(RD), W(RS))

#define movxx_ld(RD, MS, DP)                                                \
        movwx_ld(W(RD), W(MS), W(DP))

#define movxx_st(RS, MD, DP)                                                \
        movwx_st(W(RS), W(MD), W(DP))

/* and
 * set-flags: undefined (*x), yes (*z) */

#define andxx_ri(RG, IM)                                                    \
        andwx_ri(W(RG), W(IM))

#define andxx_mi(MG, DP, IM)                                                \
        andwx_mi(W(MG), W(DP), W(IM))

#define andxx_rr(RG, RS)                                                    \
        andwx_rr(W(RG), W(RS))

#define andxx_ld(RG, MS, DP)                                                \
        andwx_ld(W(RG), W(MS), W(DP))

#define andxx_st(RS, MG, DP)                                                \
        andwx_st(W(RS), W(MG), W(DP))


#define andxz_ri(RG, IM)                                                    \
        andwz_ri(W(RG), W(IM))

#define andxz_mi(MG, DP, IM)                                                \
        andwz_mi(W(MG), W(DP), W(IM))

#define andxz_rr(RG, RS)                                                    \
        andwz_rr(W(RG), W(RS))

#define andxz_ld(RG, MS, DP)                                                \
        andwz_ld(W(RG), W(MS), W(DP))

#define andxz_st(RS, MG, DP)                                                \
        andwz_st(W(RS), W(MG), W(DP))

/* ann (G = ~G & S)
 * set-flags: undefined (*x), yes (*z) */

#define annxx_ri(RG, IM)                                                    \
        annwx_ri(W(RG), W(IM))

#define annxx_mi(MG, DP, IM)                                                \
        annwx_mi(W(MG), W(DP), W(IM))

#define annxx_rr(RG, RS)                                                    \
        annwx_rr(W(RG), W(RS))

#define annxx_ld(RG, MS, DP)                                                \
        annwx_ld(W(RG), W(MS), W(DP))

#define annxx_st(RS, MG, DP)                                                \
        annwx_st(W(RS), W(MG), W(DP))

#define annxx_mr(MG, DP, RS)                                                \
        annwx_mr(W(MG), W(DP), W(RS))


#define annxz_ri(RG, IM)                                                    \
        annwz_ri(W(RG), W(IM))

#define annxz_mi(MG, DP, IM)                                                \
        annwz_mi(W(MG), W(DP), W(IM))

#define annxz_rr(RG, RS)                                                    \
        annwz_rr(W(RG), W(RS))

#define annxz_ld(RG, MS, DP)                                                \
        annwz_ld(W(RG), W(MS), W(DP))

#define annxz_st(RS, MG, DP)                                                \
        annwz_st(W(RS), W(MG), W(DP))

#define annxz_mr(MG, DP, RS)                                                \
        annwz_mr(W(MG), W(DP), W(RS))

/* orr
 * set-flags: undefined (*x), yes (*z) */

#define orrxx_ri(RG, IM)                                                    \
        orrwx_ri(W(RG), W(IM))

#define orrxx_mi(MG, DP, IM)                                                \
        orrwx_mi(W(MG), W(DP), W(IM))

#define orrxx_rr(RG, RS)                                                    \
        orrwx_rr(W(RG), W(RS))

#define orrxx_ld(RG, MS, DP)                                                \
        orrwx_ld(W(RG), W(MS), W(DP))

#define orrxx_st(RS, MG, DP)                                                \
        orrwx_st(W(RS), W(MG), W(DP))


#define orrxz_ri(RG, IM)                                                    \
        orrwz_ri(W(RG), W(IM))

#define orrxz_mi(MG, DP, IM)                                                \
        orrwz_mi(W(MG), W(DP), W(IM))

#define orrxz_rr(RG, RS)                                                    \
        orrwz_rr(W(RG), W(RS))

#define orrxz_ld(RG, MS, DP)                                                \
        orrwz_ld(W(RG), W(MS), W(DP))

#define orrxz_st(RS, MG, DP)                                                \
        orrwz_st(W(RS), W(MG), W(DP))

/* orn (G = ~G | S)
 * set-flags: undefined (*x), yes (*z) */

#define ornxx_ri(RG, IM)                                                    \
        ornwx_ri(W(RG), W(IM))

#define ornxx_mi(MG, DP, IM)                                                \
        ornwx_mi(W(MG), W(DP), W(IM))

#define ornxx_rr(RG, RS)                                                    \
        ornwx_rr(W(RG), W(RS))

#define ornxx_ld(RG, MS, DP)                                                \
        ornwx_ld(W(RG), W(MS), W(DP))

#define ornxx_st(RS, MG, DP)                                                \
        ornwx_st(W(RS), W(MG), W(DP))

#define ornxx_mr(MG, DP, RS)                                                \
        ornwx_mr(W(MG), W(DP), W(RS))


#define ornxz_ri(RG, IM)                                                    \
        ornwz_ri(W(RG), W(IM))

#define ornxz_mi(MG, DP, IM)                                                \
        ornwz_mi(W(MG), W(DP), W(IM))

#define ornxz_rr(RG, RS)                                                    \
        ornwz_rr(W(RG), W(RS))

#define ornxz_ld(RG, MS, DP)                                                \
        ornwz_ld(W(RG), W(MS), W(DP))

#define ornxz_st(RS, MG, DP)                                                \
        ornwz_st(W(RS), W(MG), W(DP))

#define ornxz_mr(MG, DP, RS)                                                \
        ornwz_mr(W(MG), W(DP), W(RS))

/* xor
 * set-flags: undefined (*x), yes (*z) */

#define xorxx_ri(RG, IM)                                                    \
        xorwx_ri(W(RG), W(IM))

#define xorxx_mi(MG, DP, IM)                                                \
        xorwx_mi(W(MG), W(DP), W(IM))

#define xorxx_rr(RG, RS)                                                    \
        xorwx_rr(W(RG), W(RS))

#define xorxx_ld(RG, MS, DP)                                                \
        xorwx_ld(W(RG), W(MS), W(DP))

#define xorxx_st(RS, MG, DP)                                                \
        xorwx_st(W(RS), W(MG), W(DP))


#define xorxz_ri(RG, IM)                                                    \
        xorwz_ri(W(RG), W(IM))

#define xorxz_mi(MG, DP, IM)                                                \
        xorwz_mi(W(MG), W(DP), W(IM))

#define xorxz_rr(RG, RS)                                                    \
        xorwz_rr(W(RG), W(RS))

#define xorxz_ld(RG, MS, DP)                                                \
        xorwz_ld(W(RG), W(MS), W(DP))

#define xorxz_st(RS, MG, DP)                                                \
        xorwz_st(W(RS), W(MG), W(DP))

/* not
 * set-flags: no */

#define notxx_rx(RG)                                                        \
        notwx_rx(W(RG))

#define notxx_mx(MG, DP)                                                    \
        notwx_mx(W(MG), W(DP))

/* neg
 * set-flags: undefined (*x), yes (*z) */

#define negxx_rx(RG)                                                        \
        negwx_rx(W(RG))

#define negxx_mx(MG, DP)                                                    \
        negwx_mx(W(MG), W(DP))


#define negxz_rx(RG)                                                        \
        negwz_rx(W(RG))

#define negxz_mx(MG, DP)                                                    \
        negwz_mx(W(MG), W(DP))

/* add
 * set-flags: undefined (*x), yes (*z) */

#define addxx_ri(RG, IM)                                                    \
        addwx_ri(W(RG), W(IM))

#define addxx_mi(MG, DP, IM)                                                \
        addwx_mi(W(MG), W(DP), W(IM))

#define addxx_rr(RG, RS)                                                    \
        addwx_rr(W(RG), W(RS))

#define addxx_ld(RG, MS, DP)                                                \
        addwx_ld(W(RG), W(MS), W(DP))

#define addxx_st(RS, MG, DP)                                                \
        addwx_st(W(RS), W(MG), W(DP))


#define addxz_ri(RG, IM)                                                    \
        addwz_ri(W(RG), W(IM))

#define addxz_mi(MG, DP, IM)                                                \
        addwz_mi(W(MG), W(DP), W(IM))

#define addxz_rr(RG, RS)                                                    \
        addwz_rr(W(RG), W(RS))

#define addxz_ld(RG, MS, DP)                                                \
        addwz_ld(W(RG), W(MS), W(DP))

#define addxz_st(RS, MG, DP)                                                \
        addwz_st(W(RS), W(MG), W(DP))

/* sub
 * set-flags: undefined (*x), yes (*z) */

#define subxx_ri(RG, IM)                                                    \
        subwx_ri(W(RG), W(IM))

#define subxx_mi(MG, DP, IM)                                                \
        subwx_mi(W(MG), W(DP), W(IM))

#define subxx_rr(RG, RS)                                                    \
        subwx_rr(W(RG), W(RS))

#define subxx_ld(RG, MS, DP)                                                \
        subwx_ld(W(RG), W(MS), W(DP))

#define subxx_st(RS, MG, DP)                                                \
        subwx_st(W(RS), W(MG), W(DP))

#define subxx_mr(MG, DP, RS)                                                \
        subwx_mr(W(MG), W(DP), W(RS))


#define subxz_ri(RG, IM)                                                    \
        subwz_ri(W(RG), W(IM))

#define subxz_mi(MG, DP, IM)                                                \
        subwz_mi(W(MG), W(DP), W(IM))

#define subxz_rr(RG, RS)                                                    \
        subwz_rr(W(RG), W(RS))

#define subxz_ld(RG, MS, DP)                                                \
        subwz_ld(W(RG), W(MS), W(DP))

#define subxz_st(RS, MG, DP)                                                \
        subwz_st(W(RS), W(MG), W(DP))

#define subxz_mr(MG, DP, RS)                                                \
        subwz_mr(W(MG), W(DP), W(RS))

/* shl
 * set-flags: undefined (*x), yes (*z) */

#define shlxx_rx(RG)                     /* reads Recx for shift value */   \
        shlwx_rx(W(RG))

#define shlxx_mx(MG, DP)                 /* reads Recx for shift value */   \
        shlwx_mx(W(MG), W(DP))

#define shlxx_ri(RG, IM)                                                    \
        shlwx_ri(W(RG), W(IM))

#define shlxx_mi(MG, DP, IM)                                                \
        shlwx_mi(W(MG), W(DP), W(IM))

#define shlxx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shlwx_rr(W(RG), W(RS))

#define shlxx_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        shlwx_ld(W(RG), W(MS), W(DP))

#define shlxx_st(RS, MG, DP)                                                \
        shlwx_st(W(RS), W(MG), W(DP))

#define shlxx_mr(MG, DP, RS)                                                \
        shlwx_mr(W(MG), W(DP), W(RS))


#define shlxz_rx(RG)                     /* reads Recx for shift value */   \
        shlwz_rx(W(RG))

#define shlxz_mx(MG, DP)                 /* reads Recx for shift value */   \
        shlwz_mx(W(MG), W(DP))

#define shlxz_ri(RG, IM)                                                    \
        shlwz_ri(W(RG), W(IM))

#define shlxz_mi(MG, DP, IM)                                                \
        shlwz_mi(W(MG), W(DP), W(IM))

#define shlxz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shlwz_rr(W(RG), W(RS))

#define shlxz_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        shlwz_ld(W(RG), W(MS), W(DP))

#define shlxz_st(RS, MG, DP)                                                \
        shlwz_st(W(RS), W(MG), W(DP))

#define shlxz_mr(MG, DP, RS)                                                \
        shlwz_mr(W(MG), W(DP), W(RS))

/* shr
 * set-flags: undefined (*x), yes (*z) */

#define shrxx_rx(RG)                     /* reads Recx for shift value */   \
        shrwx_rx(W(RG))

#define shrxx_mx(MG, DP)                 /* reads Recx for shift value */   \
        shrwx_mx(W(MG), W(DP))

#define shrxx_ri(RG, IM)                                                    \
        shrwx_ri(W(RG), W(IM))

#define shrxx_mi(MG, DP, IM)                                                \
        shrwx_mi(W(MG), W(DP), W(IM))

#define shrxx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrwx_rr(W(RG), W(RS))

#define shrxx_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        shrwx_ld(W(RG), W(MS), W(DP))

#define shrxx_st(RS, MG, DP)                                                \
        shrwx_st(W(RS), W(MG), W(DP))

#define shrxx_mr(MG, DP, RS)                                                \
        shrwx_mr(W(MG), W(DP), W(RS))


#define shrxz_rx(RG)                     /* reads Recx for shift value */   \
        shrwz_rx(W(RG))

#define shrxz_mx(MG, DP)                 /* reads Recx for shift value */   \
        shrwz_mx(W(MG), W(DP))

#define shrxz_ri(RG, IM)                                                    \
        shrwz_ri(W(RG), W(IM))

#define shrxz_mi(MG, DP, IM)                                                \
        shrwz_mi(W(MG), W(DP), W(IM))

#define shrxz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrwz_rr(W(RG), W(RS))

#define shrxz_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        shrwz_ld(W(RG), W(MS), W(DP))

#define shrxz_st(RS, MG, DP)                                                \
        shrwz_st(W(RS), W(MG), W(DP))

#define shrxz_mr(MG, DP, RS)                                                \
        shrwz_mr(W(MG), W(DP), W(RS))


#define shrxn_rx(RG)                     /* reads Recx for shift value */   \
        shrwn_rx(W(RG))

#define shrxn_mx(MG, DP)                 /* reads Recx for shift value */   \
        shrwn_mx(W(MG), W(DP))

#define shrxn_ri(RG, IM)                                                    \
        shrwn_ri(W(RG), W(IM))

#define shrxn_mi(MG, DP, IM)                                                \
        shrwn_mi(W(MG), W(DP), W(IM))

#define shrxn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrwn_rr(W(RG), W(RS))

#define shrxn_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        shrwn_ld(W(RG), W(MS), W(DP))

#define shrxn_st(RS, MG, DP)                                                \
        shrwn_st(W(RS), W(MG), W(DP))

#define shrxn_mr(MG, DP, RS)                                                \
        shrwn_mr(W(MG), W(DP), W(RS))

/* mul
 * set-flags: undefined */

#define mulxx_ri(RG, IM)                                                    \
        mulwx_ri(W(RG), W(IM))

#define mulxx_rr(RG, RS)                                                    \
        mulwx_rr(W(RG), W(RS))

#define mulxx_ld(RG, MS, DP)                                                \
        mulwx_ld(W(RG), W(MS), W(DP))


#define mulxx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        mulwx_xr(W(RS))

#define mulxx_xm(MS, DP) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        mulwx_xm(W(MS), W(DP))


#define mulxn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        mulwn_xr(W(RS))

#define mulxn_xm(MS, DP) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        mulwn_xm(W(MS), W(DP))


#define mulxp_xr(RS)     /* Reax is in/out, prepares Redx for divxn_x* */   \
        mulwp_xr(W(RS))       /* product must not exceed operands size */

#define mulxp_xm(MS, DP) /* Reax is in/out, prepares Redx for divxn_x* */   \
        mulwp_xm(W(MS), W(DP))/* product must not exceed operands size */

/* div
 * set-flags: undefined */

#define divxx_ri(RG, IM)       /* Reax cannot be used as first operand */   \
        divwx_ri(W(RG), W(IM))

#define divxx_rr(RG, RS)                 /* RG, RS no Reax, RS no Redx */   \
        divwx_rr(W(RG), W(RS))

#define divxx_ld(RG, MS, DP)   /* Reax cannot be used as first operand */   \
        divwx_ld(W(RG), W(MS), W(DP))


#define divxn_ri(RG, IM)       /* Reax cannot be used as first operand */   \
        divwn_ri(W(RG), W(IM))

#define divxn_rr(RG, RS)                 /* RG, RS no Reax, RS no Redx */   \
        divwn_rr(W(RG), W(RS))

#define divxn_ld(RG, MS, DP)   /* Reax cannot be used as first operand */   \
        divwn_ld(W(RG), W(MS), W(DP))


#define prexx_xx()          /* to be placed immediately prior divxx_x* */   \
        prewx_xx()                   /* to prepare Redx for int-divide */

#define prexn_xx()          /* to be placed immediately prior divxn_x* */   \
        prewn_xx()                   /* to prepare Redx for int-divide */


#define divxx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        divwx_xr(W(RS))

#define divxx_xm(MS, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        divwx_xm(W(MS), W(DP))


#define divxn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xr(W(RS))

#define divxn_xm(MS, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xm(W(MS), W(DP))


#define divxp_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwp_xr(W(RS))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divxp_xm(MS, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwp_xm(W(MS), W(DP))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem
 * set-flags: undefined */

#define remxx_ri(RG, IM)       /* Redx cannot be used as first operand */   \
        remwx_ri(W(RG), W(IM))

#define remxx_rr(RG, RS)                 /* RG, RS no Redx, RS no Reax */   \
        remwx_rr(W(RG), W(RS))

#define remxx_ld(RG, MS, DP)   /* Redx cannot be used as first operand */   \
        remwx_ld(W(RG), W(MS), W(DP))


#define remxn_ri(RG, IM)       /* Redx cannot be used as first operand */   \
        remwn_ri(W(RG), W(IM))

#define remxn_rr(RG, RS)                 /* RG, RS no Redx, RS no Reax */   \
        remwn_rr(W(RG), W(RS))

#define remxn_ld(RG, MS, DP)   /* Redx cannot be used as first operand */   \
        remwn_ld(W(RG), W(MS), W(DP))


#define remxx_xx()          /* to be placed immediately prior divxx_x* */   \
        remwx_xx()                   /* to prepare for rem calculation */

#define remxx_xr(RS)        /* to be placed immediately after divxx_xr */   \
        remwx_xr(W(RS))              /* to produce remainder Redx<-rem */

#define remxx_xm(MS, DP)    /* to be placed immediately after divxx_xm */   \
        remwx_xm(W(MS), W(DP))       /* to produce remainder Redx<-rem */


#define remxn_xx()          /* to be placed immediately prior divxn_x* */   \
        remwn_xx()                   /* to prepare for rem calculation */

#define remxn_xr(RS)        /* to be placed immediately after divxn_xr */   \
        remwn_xr(W(RS))              /* to produce remainder Redx<-rem */

#define remxn_xm(MS, DP)    /* to be placed immediately after divxn_xm */   \
        remwn_xm(W(MS), W(DP))       /* to produce remainder Redx<-rem */

/* arj
 * set-flags: undefined
 * refer to individual instruction descriptions
 * to stay within special register limitations */

#define arjxx_rx(RG, op, cc, lb)                                            \
        arjwx_rx(W(RG), op, cc, lb)

#define arjxx_mx(MG, DP, op, cc, lb)                                        \
        arjwx_mx(W(MG), W(DP), op, cc, lb)

#define arjxx_ri(RG, IM, op, cc, lb)                                        \
        arjwx_ri(W(RG), W(IM), op, cc, lb)

#define arjxx_mi(MG, DP, IM, op, cc, lb)                                    \
        arjwx_mi(W(MG), W(DP), W(IM), op, cc, lb)

#define arjxx_rr(RG, RS, op, cc, lb)                                        \
        arjwx_rr(W(RG), W(RS), op, cc, lb)

#define arjxx_ld(RG, MS, DP, op, cc, lb)                                    \
        arjwx_ld(W(RG), W(MS), W(DP), op, cc, lb)

#define arjxx_st(RS, MG, DP, op, cc, lb)                                    \
        arjwx_st(W(RS), W(MG), W(DP), op, cc, lb)

#define arjxx_mr(MG, DP, RS, op, cc, lb)                                    \
        arjwx_mr(W(MG), W(DP), W(RS), op, cc, lb)

/* cmj
 * set-flags: undefined */

#define cmjxx_rz(RS, cc, lb)                                                \
        cmjwx_rz(W(RS), cc, lb)

#define cmjxx_mz(MS, DP, cc, lb)                                            \
        cmjwx_mz(W(MS), W(DP), cc, lb)

#define cmjxx_ri(RS, IM, cc, lb)                                            \
        cmjwx_ri(W(RS), W(IM), cc, lb)

#define cmjxx_mi(MS, DP, IM, cc, lb)                                        \
        cmjwx_mi(W(MS), W(DP), W(IM), cc, lb)

#define cmjxx_rr(RS, RT, cc, lb)                                            \
        cmjwx_rr(W(RS), W(RT), cc, lb)

#define cmjxx_rm(RS, MT, DP, cc, lb)                                        \
        cmjwx_rm(W(RS), W(MT), W(DP), cc, lb)

#define cmjxx_mr(MS, DP, RT, cc, lb)                                        \
        cmjwx_mr(W(MS), W(DP), W(RT), cc, lb)

/* cmp
 * set-flags: yes */

#define cmpxx_ri(RS, IM)                                                    \
        cmpwx_ri(W(RS), W(IM))

#define cmpxx_mi(MS, DP, IM)                                                \
        cmpwx_mi(W(MS), W(DP), W(IM))

#define cmpxx_rr(RS, RT)                                                    \
        cmpwx_rr(W(RS), W(RT))

#define cmpxx_rm(RS, MT, DP)                                                \
        cmpwx_rm(W(RS), W(MT), W(DP))

#define cmpxx_mr(MS, DP, RT)                                                \
        cmpwx_mr(W(MS), W(DP), W(RT))

/***************** address-sized instructions for 64-bit mode *****************/

#elif RT_ADDRESS == 64

/* mov
 * set-flags: no */

#define movxx_ri(RD, IM)                                                    \
        movzx_ri(W(RD), W(IM))

#define movxx_mi(MD, DP, IM)                                                \
        movzx_mi(W(MD), W(DP), W(IM))

#define movxx_rr(RD, RS)                                                    \
        movzx_rr(W(RD), W(RS))

#define movxx_ld(RD, MS, DP)                                                \
        movzx_ld(W(RD), W(MS), W(DP))

#define movxx_st(RS, MD, DP)                                                \
        movzx_st(W(RS), W(MD), W(DP))

/* and
 * set-flags: undefined (*x), yes (*z) */

#define andxx_ri(RG, IM)                                                    \
        andzx_ri(W(RG), W(IM))

#define andxx_mi(MG, DP, IM)                                                \
        andzx_mi(W(MG), W(DP), W(IM))

#define andxx_rr(RG, RS)                                                    \
        andzx_rr(W(RG), W(RS))

#define andxx_ld(RG, MS, DP)                                                \
        andzx_ld(W(RG), W(MS), W(DP))

#define andxx_st(RS, MG, DP)                                                \
        andzx_st(W(RS), W(MG), W(DP))


#define andxz_ri(RG, IM)                                                    \
        andzz_ri(W(RG), W(IM))

#define andxz_mi(MG, DP, IM)                                                \
        andzz_mi(W(MG), W(DP), W(IM))

#define andxz_rr(RG, RS)                                                    \
        andzz_rr(W(RG), W(RS))

#define andxz_ld(RG, MS, DP)                                                \
        andzz_ld(W(RG), W(MS), W(DP))

#define andxz_st(RS, MG, DP)                                                \
        andzz_st(W(RS), W(MG), W(DP))

/* ann (G = ~G & S)
 * set-flags: undefined (*x), yes (*z) */

#define annxx_ri(RG, IM)                                                    \
        annzx_ri(W(RG), W(IM))

#define annxx_mi(MG, DP, IM)                                                \
        annzx_mi(W(MG), W(DP), W(IM))

#define annxx_rr(RG, RS)                                                    \
        annzx_rr(W(RG), W(RS))

#define annxx_ld(RG, MS, DP)                                                \
        annzx_ld(W(RG), W(MS), W(DP))

#define annxx_st(RS, MG, DP)                                                \
        annzx_st(W(RS), W(MG), W(DP))

#define annxx_mr(MG, DP, RS)                                                \
        annzx_mr(W(MG), W(DP), W(RS))


#define annxz_ri(RG, IM)                                                    \
        annzz_ri(W(RG), W(IM))

#define annxz_mi(MG, DP, IM)                                                \
        annzz_mi(W(MG), W(DP), W(IM))

#define annxz_rr(RG, RS)                                                    \
        annzz_rr(W(RG), W(RS))

#define annxz_ld(RG, MS, DP)                                                \
        annzz_ld(W(RG), W(MS), W(DP))

#define annxz_st(RS, MG, DP)                                                \
        annzz_st(W(RS), W(MG), W(DP))

#define annxz_mr(MG, DP, RS)                                                \
        annzz_mr(W(MG), W(DP), W(RS))

/* orr
 * set-flags: undefined (*x), yes (*z) */

#define orrxx_ri(RG, IM)                                                    \
        orrzx_ri(W(RG), W(IM))

#define orrxx_mi(MG, DP, IM)                                                \
        orrzx_mi(W(MG), W(DP), W(IM))

#define orrxx_rr(RG, RS)                                                    \
        orrzx_rr(W(RG), W(RS))

#define orrxx_ld(RG, MS, DP)                                                \
        orrzx_ld(W(RG), W(MS), W(DP))

#define orrxx_st(RS, MG, DP)                                                \
        orrzx_st(W(RS), W(MG), W(DP))


#define orrxz_ri(RG, IM)                                                    \
        orrzz_ri(W(RG), W(IM))

#define orrxz_mi(MG, DP, IM)                                                \
        orrzz_mi(W(MG), W(DP), W(IM))

#define orrxz_rr(RG, RS)                                                    \
        orrzz_rr(W(RG), W(RS))

#define orrxz_ld(RG, MS, DP)                                                \
        orrzz_ld(W(RG), W(MS), W(DP))

#define orrxz_st(RS, MG, DP)                                                \
        orrzz_st(W(RS), W(MG), W(DP))

/* orn (G = ~G | S)
 * set-flags: undefined (*x), yes (*z) */

#define ornxx_ri(RG, IM)                                                    \
        ornzx_ri(W(RG), W(IM))

#define ornxx_mi(MG, DP, IM)                                                \
        ornzx_mi(W(MG), W(DP), W(IM))

#define ornxx_rr(RG, RS)                                                    \
        ornzx_rr(W(RG), W(RS))

#define ornxx_ld(RG, MS, DP)                                                \
        ornzx_ld(W(RG), W(MS), W(DP))

#define ornxx_st(RS, MG, DP)                                                \
        ornzx_st(W(RS), W(MG), W(DP))

#define ornxx_mr(MG, DP, RS)                                                \
        ornzx_mr(W(MG), W(DP), W(RS))


#define ornxz_ri(RG, IM)                                                    \
        ornzz_ri(W(RG), W(IM))

#define ornxz_mi(MG, DP, IM)                                                \
        ornzz_mi(W(MG), W(DP), W(IM))

#define ornxz_rr(RG, RS)                                                    \
        ornzz_rr(W(RG), W(RS))

#define ornxz_ld(RG, MS, DP)                                                \
        ornzz_ld(W(RG), W(MS), W(DP))

#define ornxz_st(RS, MG, DP)                                                \
        ornzz_st(W(RS), W(MG), W(DP))

#define ornxz_mr(MG, DP, RS)                                                \
        ornzz_mr(W(MG), W(DP), W(RS))

/* xor
 * set-flags: undefined (*x), yes (*z) */

#define xorxx_ri(RG, IM)                                                    \
        xorzx_ri(W(RG), W(IM))

#define xorxx_mi(MG, DP, IM)                                                \
        xorzx_mi(W(MG), W(DP), W(IM))

#define xorxx_rr(RG, RS)                                                    \
        xorzx_rr(W(RG), W(RS))

#define xorxx_ld(RG, MS, DP)                                                \
        xorzx_ld(W(RG), W(MS), W(DP))

#define xorxx_st(RS, MG, DP)                                                \
        xorzx_st(W(RS), W(MG), W(DP))


#define xorxz_ri(RG, IM)                                                    \
        xorzz_ri(W(RG), W(IM))

#define xorxz_mi(MG, DP, IM)                                                \
        xorzz_mi(W(MG), W(DP), W(IM))

#define xorxz_rr(RG, RS)                                                    \
        xorzz_rr(W(RG), W(RS))

#define xorxz_ld(RG, MS, DP)                                                \
        xorzz_ld(W(RG), W(MS), W(DP))

#define xorxz_st(RS, MG, DP)                                                \
        xorzz_st(W(RS), W(MG), W(DP))

/* not
 * set-flags: no */

#define notxx_rx(RG)                                                        \
        notzx_rx(W(RG))

#define notxx_mx(MG, DP)                                                    \
        notzx_mx(W(MG), W(DP))

/* neg
 * set-flags: undefined (*x), yes (*z) */

#define negxx_rx(RG)                                                        \
        negzx_rx(W(RG))

#define negxx_mx(MG, DP)                                                    \
        negzx_mx(W(MG), W(DP))


#define negxz_rx(RG)                                                        \
        negzz_rx(W(RG))

#define negxz_mx(MG, DP)                                                    \
        negzz_mx(W(MG), W(DP))

/* add
 * set-flags: undefined (*x), yes (*z) */

#define addxx_ri(RG, IM)                                                    \
        addzx_ri(W(RG), W(IM))

#define addxx_mi(MG, DP, IM)                                                \
        addzx_mi(W(MG), W(DP), W(IM))

#define addxx_rr(RG, RS)                                                    \
        addzx_rr(W(RG), W(RS))

#define addxx_ld(RG, MS, DP)                                                \
        addzx_ld(W(RG), W(MS), W(DP))

#define addxx_st(RS, MG, DP)                                                \
        addzx_st(W(RS), W(MG), W(DP))


#define addxz_ri(RG, IM)                                                    \
        addzz_ri(W(RG), W(IM))

#define addxz_mi(MG, DP, IM)                                                \
        addzz_mi(W(MG), W(DP), W(IM))

#define addxz_rr(RG, RS)                                                    \
        addzz_rr(W(RG), W(RS))

#define addxz_ld(RG, MS, DP)                                                \
        addzz_ld(W(RG), W(MS), W(DP))

#define addxz_st(RS, MG, DP)                                                \
        addzz_st(W(RS), W(MG), W(DP))

/* sub
 * set-flags: undefined (*x), yes (*z) */

#define subxx_ri(RG, IM)                                                    \
        subzx_ri(W(RG), W(IM))

#define subxx_mi(MG, DP, IM)                                                \
        subzx_mi(W(MG), W(DP), W(IM))

#define subxx_rr(RG, RS)                                                    \
        subzx_rr(W(RG), W(RS))

#define subxx_ld(RG, MS, DP)                                                \
        subzx_ld(W(RG), W(MS), W(DP))

#define subxx_st(RS, MG, DP)                                                \
        subzx_st(W(RS), W(MG), W(DP))

#define subxx_mr(MG, DP, RS)                                                \
        subzx_mr(W(MG), W(DP), W(RS))


#define subxz_ri(RG, IM)                                                    \
        subzz_ri(W(RG), W(IM))

#define subxz_mi(MG, DP, IM)                                                \
        subzz_mi(W(MG), W(DP), W(IM))

#define subxz_rr(RG, RS)                                                    \
        subzz_rr(W(RG), W(RS))

#define subxz_ld(RG, MS, DP)                                                \
        subzz_ld(W(RG), W(MS), W(DP))

#define subxz_st(RS, MG, DP)                                                \
        subzz_st(W(RS), W(MG), W(DP))

#define subxz_mr(MG, DP, RS)                                                \
        subzz_mr(W(MG), W(DP), W(RS))

/* shl
 * set-flags: undefined (*x), yes (*z) */

#define shlxx_rx(RG)                     /* reads Recx for shift value */   \
        shlzx_rx(W(RG))

#define shlxx_mx(MG, DP)                 /* reads Recx for shift value */   \
        shlzx_mx(W(MG), W(DP))

#define shlxx_ri(RG, IM)                                                    \
        shlzx_ri(W(RG), W(IM))

#define shlxx_mi(MG, DP, IM)                                                \
        shlzx_mi(W(MG), W(DP), W(IM))

#define shlxx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shlzx_rr(W(RG), W(RS))

#define shlxx_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        shlzx_ld(W(RG), W(MS), W(DP))

#define shlxx_st(RS, MG, DP)                                                \
        shlzx_st(W(RS), W(MG), W(DP))

#define shlxx_mr(MG, DP, RS)                                                \
        shlzx_mr(W(MG), W(DP), W(RS))


#define shlxz_rx(RG)                     /* reads Recx for shift value */   \
        shlzz_rx(W(RG))

#define shlxz_mx(MG, DP)                 /* reads Recx for shift value */   \
        shlzz_mx(W(MG), W(DP))

#define shlxz_ri(RG, IM)                                                    \
        shlzz_ri(W(RG), W(IM))

#define shlxz_mi(MG, DP, IM)                                                \
        shlzz_mi(W(MG), W(DP), W(IM))

#define shlxz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shlzz_rr(W(RG), W(RS))

#define shlxz_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        shlzz_ld(W(RG), W(MS), W(DP))

#define shlxz_st(RS, MG, DP)                                                \
        shlzz_st(W(RS), W(MG), W(DP))

#define shlxz_mr(MG, DP, RS)                                                \
        shlzz_mr(W(MG), W(DP), W(RS))

/* shr
 * set-flags: undefined (*x), yes (*z) */

#define shrxx_rx(RG)                     /* reads Recx for shift value */   \
        shrzx_rx(W(RG))

#define shrxx_mx(MG, DP)                 /* reads Recx for shift value */   \
        shrzx_mx(W(MG), W(DP))

#define shrxx_ri(RG, IM)                                                    \
        shrzx_ri(W(RG), W(IM))

#define shrxx_mi(MG, DP, IM)                                                \
        shrzx_mi(W(MG), W(DP), W(IM))

#define shrxx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrzx_rr(W(RG), W(RS))

#define shrxx_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        shrzx_ld(W(RG), W(MS), W(DP))

#define shrxx_st(RS, MG, DP)                                                \
        shrzx_st(W(RS), W(MG), W(DP))

#define shrxx_mr(MG, DP, RS)                                                \
        shrzx_mr(W(MG), W(DP), W(RS))


#define shrxz_rx(RG)                     /* reads Recx for shift value */   \
        shrzz_rx(W(RG))

#define shrxz_mx(MG, DP)                 /* reads Recx for shift value */   \
        shrzz_mx(W(MG), W(DP))

#define shrxz_ri(RG, IM)                                                    \
        shrzz_ri(W(RG), W(IM))

#define shrxz_mi(MG, DP, IM)                                                \
        shrzz_mi(W(MG), W(DP), W(IM))

#define shrxz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrzz_rr(W(RG), W(RS))

#define shrxz_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        shrzz_ld(W(RG), W(MS), W(DP))

#define shrxz_st(RS, MG, DP)                                                \
        shrzz_st(W(RS), W(MG), W(DP))

#define shrxz_mr(MG, DP, RS)                                                \
        shrzz_mr(W(MG), W(DP), W(RS))


#define shrxn_rx(RG)                     /* reads Recx for shift value */   \
        shrzn_rx(W(RG))

#define shrxn_mx(MG, DP)                 /* reads Recx for shift value */   \
        shrzn_mx(W(MG), W(DP))

#define shrxn_ri(RG, IM)                                                    \
        shrzn_ri(W(RG), W(IM))

#define shrxn_mi(MG, DP, IM)                                                \
        shrzn_mi(W(MG), W(DP), W(IM))

#define shrxn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrzn_rr(W(RG), W(RS))

#define shrxn_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        shrzn_ld(W(RG), W(MS), W(DP))

#define shrxn_st(RS, MG, DP)                                                \
        shrzn_st(W(RS), W(MG), W(DP))

#define shrxn_mr(MG, DP, RS)                                                \
        shrzn_mr(W(MG), W(DP), W(RS))

/* mul
 * set-flags: undefined */

#define mulxx_ri(RG, IM)                                                    \
        mulzx_ri(W(RG), W(IM))

#define mulxx_rr(RG, RS)                                                    \
        mulzx_rr(W(RG), W(RS))

#define mulxx_ld(RG, MS, DP)                                                \
        mulzx_ld(W(RG), W(MS), W(DP))


#define mulxx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        mulzx_xr(W(RS))

#define mulxx_xm(MS, DP) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        mulzx_xm(W(MS), W(DP))


#define mulxn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        mulzn_xr(W(RS))

#define mulxn_xm(MS, DP) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        mulzn_xm(W(MS), W(DP))


#define mulxp_xr(RS)     /* Reax is in/out, prepares Redx for divxn_x* */   \
        mulzp_xr(W(RS))       /* product must not exceed operands size */

#define mulxp_xm(MS, DP) /* Reax is in/out, prepares Redx for divxn_x* */   \
        mulzp_xm(W(MS), W(DP))/* product must not exceed operands size */

/* div
 * set-flags: undefined */

#define divxx_ri(RG, IM)       /* Reax cannot be used as first operand */   \
        divzx_ri(W(RG), W(IM))

#define divxx_rr(RG, RS)                 /* RG, RS no Reax, RS no Redx */   \
        divzx_rr(W(RG), W(RS))

#define divxx_ld(RG, MS, DP)   /* Reax cannot be used as first operand */   \
        divzx_ld(W(RG), W(MS), W(DP))


#define divxn_ri(RG, IM)       /* Reax cannot be used as first operand */   \
        divzn_ri(W(RG), W(IM))

#define divxn_rr(RG, RS)                 /* RG, RS no Reax, RS no Redx */   \
        divzn_rr(W(RG), W(RS))

#define divxn_ld(RG, MS, DP)   /* Reax cannot be used as first operand */   \
        divzn_ld(W(RG), W(MS), W(DP))


#define prexx_xx()          /* to be placed immediately prior divxx_x* */   \
        prezx_xx()                   /* to prepare Redx for int-divide */

#define prexn_xx()          /* to be placed immediately prior divxn_x* */   \
        prezn_xx()                   /* to prepare Redx for int-divide */


#define divxx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        divzx_xr(W(RS))

#define divxx_xm(MS, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        divzx_xm(W(MS), W(DP))


#define divxn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divzn_xr(W(RS))

#define divxn_xm(MS, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divzn_xm(W(MS), W(DP))


#define divxp_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divzp_xr(W(RS))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divxp_xm(MS, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divzp_xm(W(MS), W(DP))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem
 * set-flags: undefined */

#define remxx_ri(RG, IM)       /* Redx cannot be used as first operand */   \
        remzx_ri(W(RG), W(IM))

#define remxx_rr(RG, RS)                 /* RG, RS no Redx, RS no Reax */   \
        remzx_rr(W(RG), W(RS))

#define remxx_ld(RG, MS, DP)   /* Redx cannot be used as first operand */   \
        remzx_ld(W(RG), W(MS), W(DP))


#define remxn_ri(RG, IM)       /* Redx cannot be used as first operand */   \
        remzn_ri(W(RG), W(IM))

#define remxn_rr(RG, RS)                 /* RG, RS no Redx, RS no Reax */   \
        remzn_rr(W(RG), W(RS))

#define remxn_ld(RG, MS, DP)   /* Redx cannot be used as first operand */   \
        remzn_ld(W(RG), W(MS), W(DP))


#define remxx_xx()          /* to be placed immediately prior divxx_x* */   \
        remzx_xx()                   /* to prepare for rem calculation */

#define remxx_xr(RS)        /* to be placed immediately after divxx_xr */   \
        remzx_xr(W(RS))              /* to produce remainder Redx<-rem */

#define remxx_xm(MS, DP)    /* to be placed immediately after divxx_xm */   \
        remzx_xm(W(MS), W(DP))       /* to produce remainder Redx<-rem */


#define remxn_xx()          /* to be placed immediately prior divxn_x* */   \
        remzn_xx()                   /* to prepare for rem calculation */

#define remxn_xr(RS)        /* to be placed immediately after divxn_xr */   \
        remzn_xr(W(RS))              /* to produce remainder Redx<-rem */

#define remxn_xm(MS, DP)    /* to be placed immediately after divxn_xm */   \
        remzn_xm(W(MS), W(DP))       /* to produce remainder Redx<-rem */

/* arj
 * set-flags: undefined
 * refer to individual instruction descriptions
 * to stay within special register limitations */

#define arjxx_rx(RG, op, cc, lb)                                            \
        arjzx_rx(W(RG), op, cc, lb)

#define arjxx_mx(MG, DP, op, cc, lb)                                        \
        arjzx_mx(W(MG), W(DP), op, cc, lb)

#define arjxx_ri(RG, IM, op, cc, lb)                                        \
        arjzx_ri(W(RG), W(IM), op, cc, lb)

#define arjxx_mi(MG, DP, IM, op, cc, lb)                                    \
        arjzx_mi(W(MG), W(DP), W(IM), op, cc, lb)

#define arjxx_rr(RG, RS, op, cc, lb)                                        \
        arjzx_rr(W(RG), W(RS), op, cc, lb)

#define arjxx_ld(RG, MS, DP, op, cc, lb)                                    \
        arjzx_ld(W(RG), W(MS), W(DP), op, cc, lb)

#define arjxx_st(RS, MG, DP, op, cc, lb)                                    \
        arjzx_st(W(RS), W(MG), W(DP), op, cc, lb)

#define arjxx_mr(MG, DP, RS, op, cc, lb)                                    \
        arjzx_mr(W(MG), W(DP), W(RS), op, cc, lb)

/* cmj
 * set-flags: undefined */

#define cmjxx_rz(RS, cc, lb)                                                \
        cmjzx_rz(W(RS), cc, lb)

#define cmjxx_mz(MS, DP, cc, lb)                                            \
        cmjzx_mz(W(MS), W(DP), cc, lb)

#define cmjxx_ri(RS, IM, cc, lb)                                            \
        cmjzx_ri(W(RS), W(IM), cc, lb)

#define cmjxx_mi(MS, DP, IM, cc, lb)                                        \
        cmjzx_mi(W(MS), W(DP), W(IM), cc, lb)

#define cmjxx_rr(RS, RT, cc, lb)                                            \
        cmjzx_rr(W(RS), W(RT), cc, lb)

#define cmjxx_rm(RS, MT, DP, cc, lb)                                        \
        cmjzx_rm(W(RS), W(MT), W(DP), cc, lb)

#define cmjxx_mr(MS, DP, RT, cc, lb)                                        \
        cmjzx_mr(W(MS), W(DP), W(RT), cc, lb)

/* cmp
 * set-flags: yes */

#define cmpxx_ri(RS, IM)                                                    \
        cmpzx_ri(W(RS), W(IM))

#define cmpxx_mi(MS, DP, IM)                                                \
        cmpzx_mi(W(MS), W(DP), W(IM))

#define cmpxx_rr(RS, RT)                                                    \
        cmpzx_rr(W(RS), W(RT))

#define cmpxx_rm(RS, MT, DP)                                                \
        cmpzx_rm(W(RS), W(MT), W(DP))

#define cmpxx_mr(MS, DP, RT)                                                \
        cmpzx_mr(W(MS), W(DP), W(RT))

#endif /* RT_ADDRESS */

/***************** element-sized instructions for 32-bit mode *****************/

#if   RT_ELEMENT == 32

/* mov
 * set-flags: no */

#define movyx_ri(RD, IM)                                                    \
        movwx_ri(W(RD), W(IM))

#define movyx_mi(MD, DP, IM)                                                \
        movwx_mi(W(MD), W(DP), W(IM))

#define movyx_rr(RD, RS)                                                    \
        movwx_rr(W(RD), W(RS))

#define movyx_ld(RD, MS, DP)                                                \
        movwx_ld(W(RD), W(MS), W(DP))

#define movyx_st(RS, MD, DP)                                                \
        movwx_st(W(RS), W(MD), W(DP))

/* and
 * set-flags: undefined (*x), yes (*z) */

#define andyx_ri(RG, IM)                                                    \
        andwx_ri(W(RG), W(IM))

#define andyx_mi(MG, DP, IM)                                                \
        andwx_mi(W(MG), W(DP), W(IM))

#define andyx_rr(RG, RS)                                                    \
        andwx_rr(W(RG), W(RS))

#define andyx_ld(RG, MS, DP)                                                \
        andwx_ld(W(RG), W(MS), W(DP))

#define andyx_st(RS, MG, DP)                                                \
        andwx_st(W(RS), W(MG), W(DP))


#define andyz_ri(RG, IM)                                                    \
        andwz_ri(W(RG), W(IM))

#define andyz_mi(MG, DP, IM)                                                \
        andwz_mi(W(MG), W(DP), W(IM))

#define andyz_rr(RG, RS)                                                    \
        andwz_rr(W(RG), W(RS))

#define andyz_ld(RG, MS, DP)                                                \
        andwz_ld(W(RG), W(MS), W(DP))

#define andyz_st(RS, MG, DP)                                                \
        andwz_st(W(RS), W(MG), W(DP))

/* ann (G = ~G & S)
 * set-flags: undefined (*x), yes (*z) */

#define annyx_ri(RG, IM)                                                    \
        annwx_ri(W(RG), W(IM))

#define annyx_mi(MG, DP, IM)                                                \
        annwx_mi(W(MG), W(DP), W(IM))

#define annyx_rr(RG, RS)                                                    \
        annwx_rr(W(RG), W(RS))

#define annyx_ld(RG, MS, DP)                                                \
        annwx_ld(W(RG), W(MS), W(DP))

#define annyx_st(RS, MG, DP)                                                \
        annwx_st(W(RS), W(MG), W(DP))

#define annyx_mr(MG, DP, RS)                                                \
        annwx_mr(W(MG), W(DP), W(RS))


#define annyz_ri(RG, IM)                                                    \
        annwz_ri(W(RG), W(IM))

#define annyz_mi(MG, DP, IM)                                                \
        annwz_mi(W(MG), W(DP), W(IM))

#define annyz_rr(RG, RS)                                                    \
        annwz_rr(W(RG), W(RS))

#define annyz_ld(RG, MS, DP)                                                \
        annwz_ld(W(RG), W(MS), W(DP))

#define annyz_st(RS, MG, DP)                                                \
        annwz_st(W(RS), W(MG), W(DP))

#define annyz_mr(MG, DP, RS)                                                \
        annwz_mr(W(MG), W(DP), W(RS))

/* orr
 * set-flags: undefined (*x), yes (*z) */

#define orryx_ri(RG, IM)                                                    \
        orrwx_ri(W(RG), W(IM))

#define orryx_mi(MG, DP, IM)                                                \
        orrwx_mi(W(MG), W(DP), W(IM))

#define orryx_rr(RG, RS)                                                    \
        orrwx_rr(W(RG), W(RS))

#define orryx_ld(RG, MS, DP)                                                \
        orrwx_ld(W(RG), W(MS), W(DP))

#define orryx_st(RS, MG, DP)                                                \
        orrwx_st(W(RS), W(MG), W(DP))


#define orryz_ri(RG, IM)                                                    \
        orrwz_ri(W(RG), W(IM))

#define orryz_mi(MG, DP, IM)                                                \
        orrwz_mi(W(MG), W(DP), W(IM))

#define orryz_rr(RG, RS)                                                    \
        orrwz_rr(W(RG), W(RS))

#define orryz_ld(RG, MS, DP)                                                \
        orrwz_ld(W(RG), W(MS), W(DP))

#define orryz_st(RS, MG, DP)                                                \
        orrwz_st(W(RS), W(MG), W(DP))

/* orn (G = ~G | S)
 * set-flags: undefined (*x), yes (*z) */

#define ornyx_ri(RG, IM)                                                    \
        ornwx_ri(W(RG), W(IM))

#define ornyx_mi(MG, DP, IM)                                                \
        ornwx_mi(W(MG), W(DP), W(IM))

#define ornyx_rr(RG, RS)                                                    \
        ornwx_rr(W(RG), W(RS))

#define ornyx_ld(RG, MS, DP)                                                \
        ornwx_ld(W(RG), W(MS), W(DP))

#define ornyx_st(RS, MG, DP)                                                \
        ornwx_st(W(RS), W(MG), W(DP))

#define ornyx_mr(MG, DP, RS)                                                \
        ornwx_mr(W(MG), W(DP), W(RS))


#define ornyz_ri(RG, IM)                                                    \
        ornwz_ri(W(RG), W(IM))

#define ornyz_mi(MG, DP, IM)                                                \
        ornwz_mi(W(MG), W(DP), W(IM))

#define ornyz_rr(RG, RS)                                                    \
        ornwz_rr(W(RG), W(RS))

#define ornyz_ld(RG, MS, DP)                                                \
        ornwz_ld(W(RG), W(MS), W(DP))

#define ornyz_st(RS, MG, DP)                                                \
        ornwz_st(W(RS), W(MG), W(DP))

#define ornyz_mr(MG, DP, RS)                                                \
        ornwz_mr(W(MG), W(DP), W(RS))

/* xor
 * set-flags: undefined (*x), yes (*z) */

#define xoryx_ri(RG, IM)                                                    \
        xorwx_ri(W(RG), W(IM))

#define xoryx_mi(MG, DP, IM)                                                \
        xorwx_mi(W(MG), W(DP), W(IM))

#define xoryx_rr(RG, RS)                                                    \
        xorwx_rr(W(RG), W(RS))

#define xoryx_ld(RG, MS, DP)                                                \
        xorwx_ld(W(RG), W(MS), W(DP))

#define xoryx_st(RS, MG, DP)                                                \
        xorwx_st(W(RS), W(MG), W(DP))


#define xoryz_ri(RG, IM)                                                    \
        xorwz_ri(W(RG), W(IM))

#define xoryz_mi(MG, DP, IM)                                                \
        xorwz_mi(W(MG), W(DP), W(IM))

#define xoryz_rr(RG, RS)                                                    \
        xorwz_rr(W(RG), W(RS))

#define xoryz_ld(RG, MS, DP)                                                \
        xorwz_ld(W(RG), W(MS), W(DP))

#define xoryz_st(RS, MG, DP)                                                \
        xorwz_st(W(RS), W(MG), W(DP))

/* not
 * set-flags: no */

#define notyx_rx(RG)                                                        \
        notwx_rx(W(RG))

#define notyx_mx(MG, DP)                                                    \
        notwx_mx(W(MG), W(DP))

/* neg
 * set-flags: undefined (*x), yes (*z) */

#define negyx_rx(RG)                                                        \
        negwx_rx(W(RG))

#define negyx_mx(MG, DP)                                                    \
        negwx_mx(W(MG), W(DP))


#define negyz_rx(RG)                                                        \
        negwz_rx(W(RG))

#define negyz_mx(MG, DP)                                                    \
        negwz_mx(W(MG), W(DP))

/* add
 * set-flags: undefined (*x), yes (*z) */

#define addyx_ri(RG, IM)                                                    \
        addwx_ri(W(RG), W(IM))

#define addyx_mi(MG, DP, IM)                                                \
        addwx_mi(W(MG), W(DP), W(IM))

#define addyx_rr(RG, RS)                                                    \
        addwx_rr(W(RG), W(RS))

#define addyx_ld(RG, MS, DP)                                                \
        addwx_ld(W(RG), W(MS), W(DP))

#define addyx_st(RS, MG, DP)                                                \
        addwx_st(W(RS), W(MG), W(DP))


#define addyz_ri(RG, IM)                                                    \
        addwz_ri(W(RG), W(IM))

#define addyz_mi(MG, DP, IM)                                                \
        addwz_mi(W(MG), W(DP), W(IM))

#define addyz_rr(RG, RS)                                                    \
        addwz_rr(W(RG), W(RS))

#define addyz_ld(RG, MS, DP)                                                \
        addwz_ld(W(RG), W(MS), W(DP))

#define addyz_st(RS, MG, DP)                                                \
        addwz_st(W(RS), W(MG), W(DP))

/* sub
 * set-flags: undefined (*x), yes (*z) */

#define subyx_ri(RG, IM)                                                    \
        subwx_ri(W(RG), W(IM))

#define subyx_mi(MG, DP, IM)                                                \
        subwx_mi(W(MG), W(DP), W(IM))

#define subyx_rr(RG, RS)                                                    \
        subwx_rr(W(RG), W(RS))

#define subyx_ld(RG, MS, DP)                                                \
        subwx_ld(W(RG), W(MS), W(DP))

#define subyx_st(RS, MG, DP)                                                \
        subwx_st(W(RS), W(MG), W(DP))

#define subyx_mr(MG, DP, RS)                                                \
        subwx_mr(W(MG), W(DP), W(RS))


#define subyz_ri(RG, IM)                                                    \
        subwz_ri(W(RG), W(IM))

#define subyz_mi(MG, DP, IM)                                                \
        subwz_mi(W(MG), W(DP), W(IM))

#define subyz_rr(RG, RS)                                                    \
        subwz_rr(W(RG), W(RS))

#define subyz_ld(RG, MS, DP)                                                \
        subwz_ld(W(RG), W(MS), W(DP))

#define subyz_st(RS, MG, DP)                                                \
        subwz_st(W(RS), W(MG), W(DP))

#define subyz_mr(MG, DP, RS)                                                \
        subwz_mr(W(MG), W(DP), W(RS))

/* shl
 * set-flags: undefined (*x), yes (*z) */

#define shlyx_rx(RG)                     /* reads Recx for shift value */   \
        shlwx_rx(W(RG))

#define shlyx_mx(MG, DP)                 /* reads Recx for shift value */   \
        shlwx_mx(W(MG), W(DP))

#define shlyx_ri(RG, IM)                                                    \
        shlwx_ri(W(RG), W(IM))

#define shlyx_mi(MG, DP, IM)                                                \
        shlwx_mi(W(MG), W(DP), W(IM))

#define shlyx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shlwx_rr(W(RG), W(RS))

#define shlyx_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        shlwx_ld(W(RG), W(MS), W(DP))

#define shlyx_st(RS, MG, DP)                                                \
        shlwx_st(W(RS), W(MG), W(DP))

#define shlyx_mr(MG, DP, RS)                                                \
        shlwx_mr(W(MG), W(DP), W(RS))


#define shlyz_rx(RG)                     /* reads Recx for shift value */   \
        shlwz_rx(W(RG))

#define shlyz_mx(MG, DP)                 /* reads Recx for shift value */   \
        shlwz_mx(W(MG), W(DP))

#define shlyz_ri(RG, IM)                                                    \
        shlwz_ri(W(RG), W(IM))

#define shlyz_mi(MG, DP, IM)                                                \
        shlwz_mi(W(MG), W(DP), W(IM))

#define shlyz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shlwz_rr(W(RG), W(RS))

#define shlyz_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        shlwz_ld(W(RG), W(MS), W(DP))

#define shlyz_st(RS, MG, DP)                                                \
        shlwz_st(W(RS), W(MG), W(DP))

#define shlyz_mr(MG, DP, RS)                                                \
        shlwz_mr(W(MG), W(DP), W(RS))

/* shr
 * set-flags: undefined (*x), yes (*z) */

#define shryx_rx(RG)                     /* reads Recx for shift value */   \
        shrwx_rx(W(RG))

#define shryx_mx(MG, DP)                 /* reads Recx for shift value */   \
        shrwx_mx(W(MG), W(DP))

#define shryx_ri(RG, IM)                                                    \
        shrwx_ri(W(RG), W(IM))

#define shryx_mi(MG, DP, IM)                                                \
        shrwx_mi(W(MG), W(DP), W(IM))

#define shryx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrwx_rr(W(RG), W(RS))

#define shryx_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        shrwx_ld(W(RG), W(MS), W(DP))

#define shryx_st(RS, MG, DP)                                                \
        shrwx_st(W(RS), W(MG), W(DP))

#define shryx_mr(MG, DP, RS)                                                \
        shrwx_mr(W(MG), W(DP), W(RS))


#define shryz_rx(RG)                     /* reads Recx for shift value */   \
        shrwz_rx(W(RG))

#define shryz_mx(MG, DP)                 /* reads Recx for shift value */   \
        shrwz_mx(W(MG), W(DP))

#define shryz_ri(RG, IM)                                                    \
        shrwz_ri(W(RG), W(IM))

#define shryz_mi(MG, DP, IM)                                                \
        shrwz_mi(W(MG), W(DP), W(IM))

#define shryz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrwz_rr(W(RG), W(RS))

#define shryz_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        shrwz_ld(W(RG), W(MS), W(DP))

#define shryz_st(RS, MG, DP)                                                \
        shrwz_st(W(RS), W(MG), W(DP))

#define shryz_mr(MG, DP, RS)                                                \
        shrwz_mr(W(MG), W(DP), W(RS))


#define shryn_rx(RG)                     /* reads Recx for shift value */   \
        shrwn_rx(W(RG))

#define shryn_mx(MG, DP)                 /* reads Recx for shift value */   \
        shrwn_mx(W(MG), W(DP))

#define shryn_ri(RG, IM)                                                    \
        shrwn_ri(W(RG), W(IM))

#define shryn_mi(MG, DP, IM)                                                \
        shrwn_mi(W(MG), W(DP), W(IM))

#define shryn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrwn_rr(W(RG), W(RS))

#define shryn_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        shrwn_ld(W(RG), W(MS), W(DP))

#define shryn_st(RS, MG, DP)                                                \
        shrwn_st(W(RS), W(MG), W(DP))

#define shryn_mr(MG, DP, RS)                                                \
        shrwn_mr(W(MG), W(DP), W(RS))

/* mul
 * set-flags: undefined */

#define mulyx_ri(RG, IM)                                                    \
        mulwx_ri(W(RG), W(IM))

#define mulyx_rr(RG, RS)                                                    \
        mulwx_rr(W(RG), W(RS))

#define mulyx_ld(RG, MS, DP)                                                \
        mulwx_ld(W(RG), W(MS), W(DP))


#define mulyx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        mulwx_xr(W(RS))

#define mulyx_xm(MS, DP) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        mulwx_xm(W(MS), W(DP))


#define mulyn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        mulwn_xr(W(RS))

#define mulyn_xm(MS, DP) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        mulwn_xm(W(MS), W(DP))


#define mulyp_xr(RS)     /* Reax is in/out, prepares Redx for divyn_x* */   \
        mulwp_xr(W(RS))       /* product must not exceed operands size */

#define mulyp_xm(MS, DP) /* Reax is in/out, prepares Redx for divyn_x* */   \
        mulwp_xm(W(MS), W(DP))/* product must not exceed operands size */

/* div
 * set-flags: undefined */

#define divyx_ri(RG, IM)       /* Reax cannot be used as first operand */   \
        divwx_ri(W(RG), W(IM))

#define divyx_rr(RG, RS)                 /* RG, RS no Reax, RS no Redx */   \
        divwx_rr(W(RG), W(RS))

#define divyx_ld(RG, MS, DP)   /* Reax cannot be used as first operand */   \
        divwx_ld(W(RG), W(MS), W(DP))


#define divyn_ri(RG, IM)       /* Reax cannot be used as first operand */   \
        divwn_ri(W(RG), W(IM))

#define divyn_rr(RG, RS)                 /* RG, RS no Reax, RS no Redx */   \
        divwn_rr(W(RG), W(RS))

#define divyn_ld(RG, MS, DP)   /* Reax cannot be used as first operand */   \
        divwn_ld(W(RG), W(MS), W(DP))


#define preyx_xx()          /* to be placed immediately prior divyx_x* */   \
        prewx_xx()                   /* to prepare Redx for int-divide */

#define preyn_xx()          /* to be placed immediately prior divyn_x* */   \
        prewn_xx()                   /* to prepare Redx for int-divide */


#define divyx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        divwx_xr(W(RS))

#define divyx_xm(MS, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        divwx_xm(W(MS), W(DP))


#define divyn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xr(W(RS))

#define divyn_xm(MS, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwn_xm(W(MS), W(DP))


#define divyp_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwp_xr(W(RS))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divyp_xm(MS, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divwp_xm(W(MS), W(DP))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem
 * set-flags: undefined */

#define remyx_ri(RG, IM)       /* Redx cannot be used as first operand */   \
        remwx_ri(W(RG), W(IM))

#define remyx_rr(RG, RS)                 /* RG, RS no Redx, RS no Reax */   \
        remwx_rr(W(RG), W(RS))

#define remyx_ld(RG, MS, DP)   /* Redx cannot be used as first operand */   \
        remwx_ld(W(RG), W(MS), W(DP))


#define remyn_ri(RG, IM)       /* Redx cannot be used as first operand */   \
        remwn_ri(W(RG), W(IM))

#define remyn_rr(RG, RS)                 /* RG, RS no Redx, RS no Reax */   \
        remwn_rr(W(RG), W(RS))

#define remyn_ld(RG, MS, DP)   /* Redx cannot be used as first operand */   \
        remwn_ld(W(RG), W(MS), W(DP))


#define remyx_xx()          /* to be placed immediately prior divyx_x* */   \
        remwx_xx()                   /* to prepare for rem calculation */

#define remyx_xr(RS)        /* to be placed immediately after divyx_xr */   \
        remwx_xr(W(RS))              /* to produce remainder Redx<-rem */

#define remyx_xm(MS, DP)    /* to be placed immediately after divyx_xm */   \
        remwx_xm(W(MS), W(DP))       /* to produce remainder Redx<-rem */


#define remyn_xx()          /* to be placed immediately prior divyn_x* */   \
        remwn_xx()                   /* to prepare for rem calculation */

#define remyn_xr(RS)        /* to be placed immediately after divyn_xr */   \
        remwn_xr(W(RS))              /* to produce remainder Redx<-rem */

#define remyn_xm(MS, DP)    /* to be placed immediately after divyn_xm */   \
        remwn_xm(W(MS), W(DP))       /* to produce remainder Redx<-rem */

/* arj
 * set-flags: undefined
 * refer to individual instruction descriptions
 * to stay within special register limitations */

#define arjyx_rx(RG, op, cc, lb)                                            \
        arjwx_rx(W(RG), op, cc, lb)

#define arjyx_mx(MG, DP, op, cc, lb)                                        \
        arjwx_mx(W(MG), W(DP), op, cc, lb)

#define arjyx_ri(RG, IM, op, cc, lb)                                        \
        arjwx_ri(W(RG), W(IM), op, cc, lb)

#define arjyx_mi(MG, DP, IM, op, cc, lb)                                    \
        arjwx_mi(W(MG), W(DP), W(IM), op, cc, lb)

#define arjyx_rr(RG, RS, op, cc, lb)                                        \
        arjwx_rr(W(RG), W(RS), op, cc, lb)

#define arjyx_ld(RG, MS, DP, op, cc, lb)                                    \
        arjwx_ld(W(RG), W(MS), W(DP), op, cc, lb)

#define arjyx_st(RS, MG, DP, op, cc, lb)                                    \
        arjwx_st(W(RS), W(MG), W(DP), op, cc, lb)

#define arjyx_mr(MG, DP, RS, op, cc, lb)                                    \
        arjwx_mr(W(MG), W(DP), W(RS), op, cc, lb)

/* cmj
 * set-flags: undefined */

#define cmjyx_rz(RS, cc, lb)                                                \
        cmjwx_rz(W(RS), cc, lb)

#define cmjyx_mz(MS, DP, cc, lb)                                            \
        cmjwx_mz(W(MS), W(DP), cc, lb)

#define cmjyx_ri(RS, IM, cc, lb)                                            \
        cmjwx_ri(W(RS), W(IM), cc, lb)

#define cmjyx_mi(MS, DP, IM, cc, lb)                                        \
        cmjwx_mi(W(MS), W(DP), W(IM), cc, lb)

#define cmjyx_rr(RS, RT, cc, lb)                                            \
        cmjwx_rr(W(RS), W(RT), cc, lb)

#define cmjyx_rm(RS, MT, DP, cc, lb)                                        \
        cmjwx_rm(W(RS), W(MT), W(DP), cc, lb)

#define cmjyx_mr(MS, DP, RT, cc, lb)                                        \
        cmjwx_mr(W(MS), W(DP), W(RT), cc, lb)

/* cmp
 * set-flags: yes */

#define cmpyx_ri(RS, IM)                                                    \
        cmpwx_ri(W(RS), W(IM))

#define cmpyx_mi(MS, DP, IM)                                                \
        cmpwx_mi(W(MS), W(DP), W(IM))

#define cmpyx_rr(RS, RT)                                                    \
        cmpwx_rr(W(RS), W(RT))

#define cmpyx_rm(RS, MT, DP)                                                \
        cmpwx_rm(W(RS), W(MT), W(DP))

#define cmpyx_mr(MS, DP, RT)                                                \
        cmpwx_mr(W(MS), W(DP), W(RT))

/***************** element-sized instructions for 64-bit mode *****************/

#elif RT_ELEMENT == 64

/* mov
 * set-flags: no */

#define movyx_ri(RD, IM)                                                    \
        movzx_ri(W(RD), W(IM))

#define movyx_mi(MD, DP, IM)                                                \
        movzx_mi(W(MD), W(DP), W(IM))

#define movyx_rr(RD, RS)                                                    \
        movzx_rr(W(RD), W(RS))

#define movyx_ld(RD, MS, DP)                                                \
        movzx_ld(W(RD), W(MS), W(DP))

#define movyx_st(RS, MD, DP)                                                \
        movzx_st(W(RS), W(MD), W(DP))

/* and
 * set-flags: undefined (*x), yes (*z) */

#define andyx_ri(RG, IM)                                                    \
        andzx_ri(W(RG), W(IM))

#define andyx_mi(MG, DP, IM)                                                \
        andzx_mi(W(MG), W(DP), W(IM))

#define andyx_rr(RG, RS)                                                    \
        andzx_rr(W(RG), W(RS))

#define andyx_ld(RG, MS, DP)                                                \
        andzx_ld(W(RG), W(MS), W(DP))

#define andyx_st(RS, MG, DP)                                                \
        andzx_st(W(RS), W(MG), W(DP))


#define andyz_ri(RG, IM)                                                    \
        andzz_ri(W(RG), W(IM))

#define andyz_mi(MG, DP, IM)                                                \
        andzz_mi(W(MG), W(DP), W(IM))

#define andyz_rr(RG, RS)                                                    \
        andzz_rr(W(RG), W(RS))

#define andyz_ld(RG, MS, DP)                                                \
        andzz_ld(W(RG), W(MS), W(DP))

#define andyz_st(RS, MG, DP)                                                \
        andzz_st(W(RS), W(MG), W(DP))

/* ann (G = ~G & S)
 * set-flags: undefined (*x), yes (*z) */

#define annyx_ri(RG, IM)                                                    \
        annzx_ri(W(RG), W(IM))

#define annyx_mi(MG, DP, IM)                                                \
        annzx_mi(W(MG), W(DP), W(IM))

#define annyx_rr(RG, RS)                                                    \
        annzx_rr(W(RG), W(RS))

#define annyx_ld(RG, MS, DP)                                                \
        annzx_ld(W(RG), W(MS), W(DP))

#define annyx_st(RS, MG, DP)                                                \
        annzx_st(W(RS), W(MG), W(DP))

#define annyx_mr(MG, DP, RS)                                                \
        annzx_mr(W(MG), W(DP), W(RS))


#define annyz_ri(RG, IM)                                                    \
        annzz_ri(W(RG), W(IM))

#define annyz_mi(MG, DP, IM)                                                \
        annzz_mi(W(MG), W(DP), W(IM))

#define annyz_rr(RG, RS)                                                    \
        annzz_rr(W(RG), W(RS))

#define annyz_ld(RG, MS, DP)                                                \
        annzz_ld(W(RG), W(MS), W(DP))

#define annyz_st(RS, MG, DP)                                                \
        annzz_st(W(RS), W(MG), W(DP))

#define annyz_mr(MG, DP, RS)                                                \
        annzz_mr(W(MG), W(DP), W(RS))

/* orr
 * set-flags: undefined (*x), yes (*z) */

#define orryx_ri(RG, IM)                                                    \
        orrzx_ri(W(RG), W(IM))

#define orryx_mi(MG, DP, IM)                                                \
        orrzx_mi(W(MG), W(DP), W(IM))

#define orryx_rr(RG, RS)                                                    \
        orrzx_rr(W(RG), W(RS))

#define orryx_ld(RG, MS, DP)                                                \
        orrzx_ld(W(RG), W(MS), W(DP))

#define orryx_st(RS, MG, DP)                                                \
        orrzx_st(W(RS), W(MG), W(DP))


#define orryz_ri(RG, IM)                                                    \
        orrzz_ri(W(RG), W(IM))

#define orryz_mi(MG, DP, IM)                                                \
        orrzz_mi(W(MG), W(DP), W(IM))

#define orryz_rr(RG, RS)                                                    \
        orrzz_rr(W(RG), W(RS))

#define orryz_ld(RG, MS, DP)                                                \
        orrzz_ld(W(RG), W(MS), W(DP))

#define orryz_st(RS, MG, DP)                                                \
        orrzz_st(W(RS), W(MG), W(DP))

/* orn (G = ~G | S)
 * set-flags: undefined (*x), yes (*z) */

#define ornyx_ri(RG, IM)                                                    \
        ornzx_ri(W(RG), W(IM))

#define ornyx_mi(MG, DP, IM)                                                \
        ornzx_mi(W(MG), W(DP), W(IM))

#define ornyx_rr(RG, RS)                                                    \
        ornzx_rr(W(RG), W(RS))

#define ornyx_ld(RG, MS, DP)                                                \
        ornzx_ld(W(RG), W(MS), W(DP))

#define ornyx_st(RS, MG, DP)                                                \
        ornzx_st(W(RS), W(MG), W(DP))

#define ornyx_mr(MG, DP, RS)                                                \
        ornzx_mr(W(MG), W(DP), W(RS))


#define ornyz_ri(RG, IM)                                                    \
        ornzz_ri(W(RG), W(IM))

#define ornyz_mi(MG, DP, IM)                                                \
        ornzz_mi(W(MG), W(DP), W(IM))

#define ornyz_rr(RG, RS)                                                    \
        ornzz_rr(W(RG), W(RS))

#define ornyz_ld(RG, MS, DP)                                                \
        ornzz_ld(W(RG), W(MS), W(DP))

#define ornyz_st(RS, MG, DP)                                                \
        ornzz_st(W(RS), W(MG), W(DP))

#define ornyz_mr(MG, DP, RS)                                                \
        ornzz_mr(W(MG), W(DP), W(RS))

/* xor
 * set-flags: undefined (*x), yes (*z) */

#define xoryx_ri(RG, IM)                                                    \
        xorzx_ri(W(RG), W(IM))

#define xoryx_mi(MG, DP, IM)                                                \
        xorzx_mi(W(MG), W(DP), W(IM))

#define xoryx_rr(RG, RS)                                                    \
        xorzx_rr(W(RG), W(RS))

#define xoryx_ld(RG, MS, DP)                                                \
        xorzx_ld(W(RG), W(MS), W(DP))

#define xoryx_st(RS, MG, DP)                                                \
        xorzx_st(W(RS), W(MG), W(DP))


#define xoryz_ri(RG, IM)                                                    \
        xorzz_ri(W(RG), W(IM))

#define xoryz_mi(MG, DP, IM)                                                \
        xorzz_mi(W(MG), W(DP), W(IM))

#define xoryz_rr(RG, RS)                                                    \
        xorzz_rr(W(RG), W(RS))

#define xoryz_ld(RG, MS, DP)                                                \
        xorzz_ld(W(RG), W(MS), W(DP))

#define xoryz_st(RS, MG, DP)                                                \
        xorzz_st(W(RS), W(MG), W(DP))

/* not
 * set-flags: no */

#define notyx_rx(RG)                                                        \
        notzx_rx(W(RG))

#define notyx_mx(MG, DP)                                                    \
        notzx_mx(W(MG), W(DP))

/* neg
 * set-flags: undefined (*x), yes (*z) */

#define negyx_rx(RG)                                                        \
        negzx_rx(W(RG))

#define negyx_mx(MG, DP)                                                    \
        negzx_mx(W(MG), W(DP))


#define negyz_rx(RG)                                                        \
        negzz_rx(W(RG))

#define negyz_mx(MG, DP)                                                    \
        negzz_mx(W(MG), W(DP))

/* add
 * set-flags: undefined (*x), yes (*z) */

#define addyx_ri(RG, IM)                                                    \
        addzx_ri(W(RG), W(IM))

#define addyx_mi(MG, DP, IM)                                                \
        addzx_mi(W(MG), W(DP), W(IM))

#define addyx_rr(RG, RS)                                                    \
        addzx_rr(W(RG), W(RS))

#define addyx_ld(RG, MS, DP)                                                \
        addzx_ld(W(RG), W(MS), W(DP))

#define addyx_st(RS, MG, DP)                                                \
        addzx_st(W(RS), W(MG), W(DP))


#define addyz_ri(RG, IM)                                                    \
        addzz_ri(W(RG), W(IM))

#define addyz_mi(MG, DP, IM)                                                \
        addzz_mi(W(MG), W(DP), W(IM))

#define addyz_rr(RG, RS)                                                    \
        addzz_rr(W(RG), W(RS))

#define addyz_ld(RG, MS, DP)                                                \
        addzz_ld(W(RG), W(MS), W(DP))

#define addyz_st(RS, MG, DP)                                                \
        addzz_st(W(RS), W(MG), W(DP))

/* sub
 * set-flags: undefined (*x), yes (*z) */

#define subyx_ri(RG, IM)                                                    \
        subzx_ri(W(RG), W(IM))

#define subyx_mi(MG, DP, IM)                                                \
        subzx_mi(W(MG), W(DP), W(IM))

#define subyx_rr(RG, RS)                                                    \
        subzx_rr(W(RG), W(RS))

#define subyx_ld(RG, MS, DP)                                                \
        subzx_ld(W(RG), W(MS), W(DP))

#define subyx_st(RS, MG, DP)                                                \
        subzx_st(W(RS), W(MG), W(DP))

#define subyx_mr(MG, DP, RS)                                                \
        subzx_mr(W(MG), W(DP), W(RS))


#define subyz_ri(RG, IM)                                                    \
        subzz_ri(W(RG), W(IM))

#define subyz_mi(MG, DP, IM)                                                \
        subzz_mi(W(MG), W(DP), W(IM))

#define subyz_rr(RG, RS)                                                    \
        subzz_rr(W(RG), W(RS))

#define subyz_ld(RG, MS, DP)                                                \
        subzz_ld(W(RG), W(MS), W(DP))

#define subyz_st(RS, MG, DP)                                                \
        subzz_st(W(RS), W(MG), W(DP))

#define subyz_mr(MG, DP, RS)                                                \
        subzz_mr(W(MG), W(DP), W(RS))

/* shl
 * set-flags: undefined (*x), yes (*z) */

#define shlyx_rx(RG)                     /* reads Recx for shift value */   \
        shlzx_rx(W(RG))

#define shlyx_mx(MG, DP)                 /* reads Recx for shift value */   \
        shlzx_mx(W(MG), W(DP))

#define shlyx_ri(RG, IM)                                                    \
        shlzx_ri(W(RG), W(IM))

#define shlyx_mi(MG, DP, IM)                                                \
        shlzx_mi(W(MG), W(DP), W(IM))

#define shlyx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shlzx_rr(W(RG), W(RS))

#define shlyx_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        shlzx_ld(W(RG), W(MS), W(DP))

#define shlyx_st(RS, MG, DP)                                                \
        shlzx_st(W(RS), W(MG), W(DP))

#define shlyx_mr(MG, DP, RS)                                                \
        shlzx_mr(W(MG), W(DP), W(RS))


#define shlyz_rx(RG)                     /* reads Recx for shift value */   \
        shlzz_rx(W(RG))

#define shlyz_mx(MG, DP)                 /* reads Recx for shift value */   \
        shlzz_mx(W(MG), W(DP))

#define shlyz_ri(RG, IM)                                                    \
        shlzz_ri(W(RG), W(IM))

#define shlyz_mi(MG, DP, IM)                                                \
        shlzz_mi(W(MG), W(DP), W(IM))

#define shlyz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shlzz_rr(W(RG), W(RS))

#define shlyz_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        shlzz_ld(W(RG), W(MS), W(DP))

#define shlyz_st(RS, MG, DP)                                                \
        shlzz_st(W(RS), W(MG), W(DP))

#define shlyz_mr(MG, DP, RS)                                                \
        shlzz_mr(W(MG), W(DP), W(RS))

/* shr
 * set-flags: undefined (*x), yes (*z) */

#define shryx_rx(RG)                     /* reads Recx for shift value */   \
        shrzx_rx(W(RG))

#define shryx_mx(MG, DP)                 /* reads Recx for shift value */   \
        shrzx_mx(W(MG), W(DP))

#define shryx_ri(RG, IM)                                                    \
        shrzx_ri(W(RG), W(IM))

#define shryx_mi(MG, DP, IM)                                                \
        shrzx_mi(W(MG), W(DP), W(IM))

#define shryx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrzx_rr(W(RG), W(RS))

#define shryx_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        shrzx_ld(W(RG), W(MS), W(DP))

#define shryx_st(RS, MG, DP)                                                \
        shrzx_st(W(RS), W(MG), W(DP))

#define shryx_mr(MG, DP, RS)                                                \
        shrzx_mr(W(MG), W(DP), W(RS))


#define shryz_rx(RG)                     /* reads Recx for shift value */   \
        shrzz_rx(W(RG))

#define shryz_mx(MG, DP)                 /* reads Recx for shift value */   \
        shrzz_mx(W(MG), W(DP))

#define shryz_ri(RG, IM)                                                    \
        shrzz_ri(W(RG), W(IM))

#define shryz_mi(MG, DP, IM)                                                \
        shrzz_mi(W(MG), W(DP), W(IM))

#define shryz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrzz_rr(W(RG), W(RS))

#define shryz_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        shrzz_ld(W(RG), W(MS), W(DP))

#define shryz_st(RS, MG, DP)                                                \
        shrzz_st(W(RS), W(MG), W(DP))

#define shryz_mr(MG, DP, RS)                                                \
        shrzz_mr(W(MG), W(DP), W(RS))


#define shryn_rx(RG)                     /* reads Recx for shift value */   \
        shrzn_rx(W(RG))

#define shryn_mx(MG, DP)                 /* reads Recx for shift value */   \
        shrzn_mx(W(MG), W(DP))

#define shryn_ri(RG, IM)                                                    \
        shrzn_ri(W(RG), W(IM))

#define shryn_mi(MG, DP, IM)                                                \
        shrzn_mi(W(MG), W(DP), W(IM))

#define shryn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrzn_rr(W(RG), W(RS))

#define shryn_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        shrzn_ld(W(RG), W(MS), W(DP))

#define shryn_st(RS, MG, DP)                                                \
        shrzn_st(W(RS), W(MG), W(DP))

#define shryn_mr(MG, DP, RS)                                                \
        shrzn_mr(W(MG), W(DP), W(RS))

/* mul
 * set-flags: undefined */

#define mulyx_ri(RG, IM)                                                    \
        mulzx_ri(W(RG), W(IM))

#define mulyx_rr(RG, RS)                                                    \
        mulzx_rr(W(RG), W(RS))

#define mulyx_ld(RG, MS, DP)                                                \
        mulzx_ld(W(RG), W(MS), W(DP))


#define mulyx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        mulzx_xr(W(RS))

#define mulyx_xm(MS, DP) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        mulzx_xm(W(MS), W(DP))


#define mulyn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        mulzn_xr(W(RS))

#define mulyn_xm(MS, DP) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        mulzn_xm(W(MS), W(DP))


#define mulyp_xr(RS)     /* Reax is in/out, prepares Redx for divyn_x* */   \
        mulzp_xr(W(RS))       /* product must not exceed operands size */

#define mulyp_xm(MS, DP) /* Reax is in/out, prepares Redx for divyn_x* */   \
        mulzp_xm(W(MS), W(DP))/* product must not exceed operands size */

/* div
 * set-flags: undefined */

#define divyx_ri(RG, IM)       /* Reax cannot be used as first operand */   \
        divzx_ri(W(RG), W(IM))

#define divyx_rr(RG, RS)                 /* RG, RS no Reax, RS no Redx */   \
        divzx_rr(W(RG), W(RS))

#define divyx_ld(RG, MS, DP)   /* Reax cannot be used as first operand */   \
        divzx_ld(W(RG), W(MS), W(DP))


#define divyn_ri(RG, IM)       /* Reax cannot be used as first operand */   \
        divzn_ri(W(RG), W(IM))

#define divyn_rr(RG, RS)                 /* RG, RS no Reax, RS no Redx */   \
        divzn_rr(W(RG), W(RS))

#define divyn_ld(RG, MS, DP)   /* Reax cannot be used as first operand */   \
        divzn_ld(W(RG), W(MS), W(DP))


#define preyx_xx()          /* to be placed immediately prior divyx_x* */   \
        prezx_xx()                   /* to prepare Redx for int-divide */

#define preyn_xx()          /* to be placed immediately prior divyn_x* */   \
        prezn_xx()                   /* to prepare Redx for int-divide */


#define divyx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        divzx_xr(W(RS))

#define divyx_xm(MS, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        divzx_xm(W(MS), W(DP))


#define divyn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divzn_xr(W(RS))

#define divyn_xm(MS, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divzn_xm(W(MS), W(DP))


#define divyp_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divzp_xr(W(RS))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divyp_xm(MS, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divzp_xm(W(MS), W(DP))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem
 * set-flags: undefined */

#define remyx_ri(RG, IM)       /* Redx cannot be used as first operand */   \
        remzx_ri(W(RG), W(IM))

#define remyx_rr(RG, RS)                 /* RG, RS no Redx, RS no Reax */   \
        remzx_rr(W(RG), W(RS))

#define remyx_ld(RG, MS, DP)   /* Redx cannot be used as first operand */   \
        remzx_ld(W(RG), W(MS), W(DP))


#define remyn_ri(RG, IM)       /* Redx cannot be used as first operand */   \
        remzn_ri(W(RG), W(IM))

#define remyn_rr(RG, RS)                 /* RG, RS no Redx, RS no Reax */   \
        remzn_rr(W(RG), W(RS))

#define remyn_ld(RG, MS, DP)   /* Redx cannot be used as first operand */   \
        remzn_ld(W(RG), W(MS), W(DP))


#define remyx_xx()          /* to be placed immediately prior divyx_x* */   \
        remzx_xx()                   /* to prepare for rem calculation */

#define remyx_xr(RS)        /* to be placed immediately after divyx_xr */   \
        remzx_xr(W(RS))              /* to produce remainder Redx<-rem */

#define remyx_xm(MS, DP)    /* to be placed immediately after divyx_xm */   \
        remzx_xm(W(MS), W(DP))       /* to produce remainder Redx<-rem */


#define remyn_xx()          /* to be placed immediately prior divyn_x* */   \
        remzn_xx()                   /* to prepare for rem calculation */

#define remyn_xr(RS)        /* to be placed immediately after divyn_xr */   \
        remzn_xr(W(RS))              /* to produce remainder Redx<-rem */

#define remyn_xm(MS, DP)    /* to be placed immediately after divyn_xm */   \
        remzn_xm(W(MS), W(DP))       /* to produce remainder Redx<-rem */

/* arj
 * set-flags: undefined
 * refer to individual instruction descriptions
 * to stay within special register limitations */

#define arjyx_rx(RG, op, cc, lb)                                            \
        arjzx_rx(W(RG), op, cc, lb)

#define arjyx_mx(MG, DP, op, cc, lb)                                        \
        arjzx_mx(W(MG), W(DP), op, cc, lb)

#define arjyx_ri(RG, IM, op, cc, lb)                                        \
        arjzx_ri(W(RG), W(IM), op, cc, lb)

#define arjyx_mi(MG, DP, IM, op, cc, lb)                                    \
        arjzx_mi(W(MG), W(DP), W(IM), op, cc, lb)

#define arjyx_rr(RG, RS, op, cc, lb)                                        \
        arjzx_rr(W(RG), W(RS), op, cc, lb)

#define arjyx_ld(RG, MS, DP, op, cc, lb)                                    \
        arjzx_ld(W(RG), W(MS), W(DP), op, cc, lb)

#define arjyx_st(RS, MG, DP, op, cc, lb)                                    \
        arjzx_st(W(RS), W(MG), W(DP), op, cc, lb)

#define arjyx_mr(MG, DP, RS, op, cc, lb)                                    \
        arjzx_mr(W(MG), W(DP), W(RS), op, cc, lb)

/* cmj
 * set-flags: undefined */

#define cmjyx_rz(RS, cc, lb)                                                \
        cmjzx_rz(W(RS), cc, lb)

#define cmjyx_mz(MS, DP, cc, lb)                                            \
        cmjzx_mz(W(MS), W(DP), cc, lb)

#define cmjyx_ri(RS, IM, cc, lb)                                            \
        cmjzx_ri(W(RS), W(IM), cc, lb)

#define cmjyx_mi(MS, DP, IM, cc, lb)                                        \
        cmjzx_mi(W(MS), W(DP), W(IM), cc, lb)

#define cmjyx_rr(RS, RT, cc, lb)                                            \
        cmjzx_rr(W(RS), W(RT), cc, lb)

#define cmjyx_rm(RS, MT, DP, cc, lb)                                        \
        cmjzx_rm(W(RS), W(MT), W(DP), cc, lb)

#define cmjyx_mr(MS, DP, RT, cc, lb)                                        \
        cmjzx_mr(W(MS), W(DP), W(RT), cc, lb)

/* cmp
 * set-flags: yes */

#define cmpyx_ri(RS, IM)                                                    \
        cmpzx_ri(W(RS), W(IM))

#define cmpyx_mi(MS, DP, IM)                                                \
        cmpzx_mi(W(MS), W(DP), W(IM))

#define cmpyx_rr(RS, RT)                                                    \
        cmpzx_rr(W(RS), W(RT))

#define cmpyx_rm(RS, MT, DP)                                                \
        cmpzx_rm(W(RS), W(MT), W(DP))

#define cmpyx_mr(MS, DP, RT)                                                \
        cmpzx_mr(W(MS), W(DP), W(RT))

#endif /* RT_ELEMENT */

#endif /* RT_RTBASE_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

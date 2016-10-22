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

/* cbr (D = cbrt S) */

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

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP == 0

#define rcpps_rr(XD, XS) /* destroys XS */                                  \
        rceps_rr(W(XD), W(XS))                                              \
        rcsps_rr(W(XD), W(XS)) /* <- not reusable without extra temp reg */

#else /* RT_SIMD_COMPAT_RCP */

#define rcpps_rr(XD, XS) /* destroys XS */                                  \
        movpx_ld(W(XD), Mebp, inf_GPC01)                                    \
        divps_rr(W(XD), W(XS))

#endif /* RT_SIMD_COMPAT_RCP */

#if RT_SIMD_COMPAT_RCP == 1

#define rceps_rr(XD, XS)                                                    \
        movpx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movpx_ld(W(XD), Mebp, inf_GPC01)                                    \
        divps_ld(W(XD), Mebp, inf_SCR02(0))

#define rcsps_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

/* rsq (D = 1.0 / sqrt S)
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

#endif /* RT_SIMD_COMPAT_RSQ */

#if RT_SIMD_COMPAT_RSQ == 1

#define rseps_rr(XD, XS)                                                    \
        sqrps_rr(W(XD), W(XS))                                              \
        movpx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movpx_ld(W(XD), Mebp, inf_GPC01)                                    \
        divps_ld(W(XD), Mebp, inf_SCR02(0))

#define rssps_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

/****************** instructions for fixed-sized 32-bit SIMD ******************/

/* cbr (D = cbrt S) */

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

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP == 0

#define rcpos_rr(XD, XS) /* destroys XS */                                  \
        rceos_rr(W(XD), W(XS))                                              \
        rcsos_rr(W(XD), W(XS)) /* <- not reusable without extra temp reg */

#else /* RT_SIMD_COMPAT_RCP */

#define rcpos_rr(XD, XS) /* destroys XS */                                  \
        movox_ld(W(XD), Mebp, inf_GPC01_32)                                 \
        divos_rr(W(XD), W(XS))

#endif /* RT_SIMD_COMPAT_RCP */

#if RT_SIMD_COMPAT_RCP == 1

#define rceos_rr(XD, XS)                                                    \
        movox_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movox_ld(W(XD), Mebp, inf_GPC01_32)                                 \
        divos_ld(W(XD), Mebp, inf_SCR02(0))

#define rcsos_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

/* rsq (D = 1.0 / sqrt S)
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

#endif /* RT_SIMD_COMPAT_RSQ */

#if RT_SIMD_COMPAT_RSQ == 1

#define rseos_rr(XD, XS)                                                    \
        sqros_rr(W(XD), W(XS))                                              \
        movox_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movox_ld(W(XD), Mebp, inf_GPC01_32)                                 \
        divos_ld(W(XD), Mebp, inf_SCR02(0))

#define rssos_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

/* fma (G = G + S * T)
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

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
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

/****************** instructions for fixed-sized 64-bit SIMD ******************/

/* cbr (D = cbrt S) */

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

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP == 0

#define rcpqs_rr(XD, XS) /* destroys XS */                                  \
        rceqs_rr(W(XD), W(XS))                                              \
        rcsqs_rr(W(XD), W(XS)) /* <- not reusable without extra temp reg */

#else /* RT_SIMD_COMPAT_RCP */

#define rcpqs_rr(XD, XS) /* destroys XS */                                  \
        movqx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divqs_rr(W(XD), W(XS))

#endif /* RT_SIMD_COMPAT_RCP */

#if RT_SIMD_COMPAT_RCP == 1

#define rceqs_rr(XD, XS)                                                    \
        movqx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movqx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divqs_ld(W(XD), Mebp, inf_SCR02(0))

#define rcsqs_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

/* rsq (D = 1.0 / sqrt S)
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

#endif /* RT_SIMD_COMPAT_RSQ */

#if RT_SIMD_COMPAT_RSQ == 1

#define rseqs_rr(XD, XS)                                                    \
        sqrqs_rr(W(XD), W(XS))                                              \
        movqx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movqx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divqs_ld(W(XD), Mebp, inf_SCR02(0))

#define rssqs_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

/* fma (G = G + S * T)
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

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
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

/****************** original FCTRL blocks (cannot be nested) ******************/

#define FCTRL_ENTER(mode) /* assumes default mode (ROUNDN) upon entry */    \
        FCTRL_SET(mode)

#define FCTRL_LEAVE(mode) /* resumes default mode (ROUNDN) upon leave */    \
        FCTRL_RESET()

/***************** instructions for element-sized 32-bit SIMD *****************/

#if   RT_ELEMENT == 32

/* mov (D = S) */

#define movpx_rr(XD, XS)                                                    \
        movox_rr(W(XD), W(XS))

#define movpx_ld(XD, MS, DS)                                                \
        movox_ld(W(XD), W(MS), W(DS))

#define movpx_st(XS, MD, DD)                                                \
        movox_st(W(XS), W(MD), W(DD))

/* and (G = G & S) */

#define andpx_rr(XG, XS)                                                    \
        andox_rr(W(XG), W(XS))

#define andpx_ld(XG, MS, DS)                                                \
        andox_ld(W(XG), W(MS), W(DS))

/* ann (G = ~G & S) */

#define annpx_rr(XG, XS)                                                    \
        annox_rr(W(XG), W(XS))

#define annpx_ld(XG, MS, DS)                                                \
        annox_ld(W(XG), W(MS), W(DS))

/* orr (G = G | S) */

#define orrpx_rr(XG, XS)                                                    \
        orrox_rr(W(XG), W(XS))

#define orrpx_ld(XG, MS, DS)                                                \
        orrox_ld(W(XG), W(MS), W(DS))

/* orn (G = ~G | S) */

#define ornpx_rr(XG, XS)                                                    \
        ornox_rr(W(XG), W(XS))

#define ornpx_ld(XG, MS, DS)                                                \
        ornox_ld(W(XG), W(MS), W(DS))

/* xor (G = G ^ S) */

#define xorpx_rr(XG, XS)                                                    \
        xorox_rr(W(XG), W(XS))

#define xorpx_ld(XG, MS, DS)                                                \
        xorox_ld(W(XG), W(MS), W(DS))

/* not (G = ~G) */

#define notpx_rx(XG)                                                        \
        notox_rx(W(XG))

/* neg (G = -G) */

#define negps_rx(XG)                                                        \
        negos_rx(W(XG))

/* add (G = G + S) */

#define addps_rr(XG, XS)                                                    \
        addos_rr(W(XG), W(XS))

#define addps_ld(XG, MS, DS)                                                \
        addos_ld(W(XG), W(MS), W(DS))

/* sub (G = G - S) */

#define subps_rr(XG, XS)                                                    \
        subos_rr(W(XG), W(XS))

#define subps_ld(XG, MS, DS)                                                \
        subos_ld(W(XG), W(MS), W(DS))

/* mul (G = G * S) */

#define mulps_rr(XG, XS)                                                    \
        mulos_rr(W(XG), W(XS))

#define mulps_ld(XG, MS, DS)                                                \
        mulos_ld(W(XG), W(MS), W(DS))

/* div (G = G / S) */

#define divps_rr(XG, XS)                                                    \
        divos_rr(W(XG), W(XS))

#define divps_ld(XG, MS, DS)                                                \
        divos_ld(W(XG), W(MS), W(DS))

/* sqr (D = sqrt S) */

#define sqrps_rr(XD, XS)                                                    \
        sqros_rr(W(XD), W(XS))

#define sqrps_ld(XD, MS, DS)                                                \
        sqros_ld(W(XD), W(MS), W(DS))

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP == 0

#define rceps_rr(XD, XS)                                                    \
        rceos_rr(W(XD), W(XS))

#define rcsps_rr(XG, XS) /* destroys RM */                                  \
        rcsos_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ == 0

#define rseps_rr(XD, XS)                                                    \
        rseos_rr(W(XD), W(XS))

#define rssps_rr(XG, XS) /* destroys RM */                                  \
        rssos_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmaps_rr(XG, XS, XT)                                                \
        fmaos_rr(W(XG), W(XS), W(XT))

#define fmaps_ld(XG, XS, MT, DT)                                            \
        fmaos_ld(W(XG), W(XS), W(MT), W(DT))

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsps_rr(XG, XS, XT)                                                \
        fmsos_rr(W(XG), W(XS), W(XT))

#define fmsps_ld(XG, XS, MT, DT)                                            \
        fmsos_ld(W(XG), W(XS), W(MT), W(DT))

/* min (G = G < S ? G : S) */

#define minps_rr(XG, XS)                                                    \
        minos_rr(W(XG), W(XS))

#define minps_ld(XG, MS, DS)                                                \
        minos_ld(W(XG), W(MS), W(DS))

/* max (G = G > S ? G : S) */

#define maxps_rr(XG, XS)                                                    \
        maxos_rr(W(XG), W(XS))

#define maxps_ld(XG, MS, DS)                                                \
        maxos_ld(W(XG), W(MS), W(DS))

/* cmp (G = G ? S) */

#define ceqps_rr(XG, XS)                                                    \
        ceqos_rr(W(XG), W(XS))

#define ceqps_ld(XG, MS, DS)                                                \
        ceqos_ld(W(XG), W(MS), W(DS))

#define cneps_rr(XG, XS)                                                    \
        cneos_rr(W(XG), W(XS))

#define cneps_ld(XG, MS, DS)                                                \
        cneos_ld(W(XG), W(MS), W(DS))

#define cltps_rr(XG, XS)                                                    \
        cltos_rr(W(XG), W(XS))

#define cltps_ld(XG, MS, DS)                                                \
        cltos_ld(W(XG), W(MS), W(DS))

#define cleps_rr(XG, XS)                                                    \
        cleos_rr(W(XG), W(XS))

#define cleps_ld(XG, MS, DS)                                                \
        cleos_ld(W(XG), W(MS), W(DS))

#define cgtps_rr(XG, XS)                                                    \
        cgtos_rr(W(XG), W(XS))

#define cgtps_ld(XG, MS, DS)                                                \
        cgtos_ld(W(XG), W(MS), W(DS))

#define cgeps_rr(XG, XS)                                                    \
        cgeos_rr(W(XG), W(XS))

#define cgeps_ld(XG, MS, DS)                                                \
        cgeos_ld(W(XG), W(MS), W(DS))

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
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
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
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
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
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
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
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

/* add (G = G + S) */

#define addpx_rr(XG, XS)                                                    \
        addox_rr(W(XG), W(XS))

#define addpx_ld(XG, MS, DS)                                                \
        addox_ld(W(XG), W(MS), W(DS))

/* sub (G = G - S) */

#define subpx_rr(XG, XS)                                                    \
        subox_rr(W(XG), W(XS))

#define subpx_ld(XG, MS, DS)                                                \
        subox_ld(W(XG), W(MS), W(DS))

/* shl (G = G << S) */

#define shlpx_ri(XG, IS)                                                    \
        shlox_ri(W(XG), W(IS))

#define shlpx_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        shlox_ld(W(XG), W(MS), W(DS))

/* shr (G = G >> S) */

#define shrpx_ri(XG, IS)                                                    \
        shrox_ri(W(XG), W(IS))

#define shrpx_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        shrox_ld(W(XG), W(MS), W(DS))

#define shrpn_ri(XG, IS)                                                    \
        shron_ri(W(XG), W(IS))

#define shrpn_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        shron_ld(W(XG), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
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
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvtpn_rr(XD, XS)                                                    \
        cvton_rr(W(XD), W(XS))

#define cvtpn_ld(XD, MS, DS)                                                \
        cvton_ld(W(XD), W(MS), W(DS))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnrps_rr(XD, XS, mode)                                              \
        rnros_rr(W(XD), W(XS), mode)

#define cvrps_rr(XD, XS, mode)                                              \
        cvros_rr(W(XD), W(XS), mode)

/* mmv (D = mask-merge S)
 * uses Xmm0 implicitly as a mask register */

#define mmvpx_ld(XD, MS, DS) /* not portable, use conditionally (on x86) */ \
        mmvox_ld(W(XD), W(MS), W(DS))

#define mmvpx_st(XS, MD, DD) /* not portable, use conditionally (on x86) */ \
        mmvox_st(W(XS), W(MD), W(DD))

/***************** instructions for element-sized 64-bit SIMD *****************/

#elif RT_ELEMENT == 64

/* mov (D = S) */

#define movpx_rr(XD, XS)                                                    \
        movqx_rr(W(XD), W(XS))

#define movpx_ld(XD, MS, DS)                                                \
        movqx_ld(W(XD), W(MS), W(DS))

#define movpx_st(XS, MD, DD)                                                \
        movqx_st(W(XS), W(MD), W(DD))

/* and (G = G & S) */

#define andpx_rr(XG, XS)                                                    \
        andqx_rr(W(XG), W(XS))

#define andpx_ld(XG, MS, DS)                                                \
        andqx_ld(W(XG), W(MS), W(DS))

/* ann (G = ~G & S) */

#define annpx_rr(XG, XS)                                                    \
        annqx_rr(W(XG), W(XS))

#define annpx_ld(XG, MS, DS)                                                \
        annqx_ld(W(XG), W(MS), W(DS))

/* orr (G = G | S) */

#define orrpx_rr(XG, XS)                                                    \
        orrqx_rr(W(XG), W(XS))

#define orrpx_ld(XG, MS, DS)                                                \
        orrqx_ld(W(XG), W(MS), W(DS))

/* orn (G = ~G | S) */

#define ornpx_rr(XG, XS)                                                    \
        ornqx_rr(W(XG), W(XS))

#define ornpx_ld(XG, MS, DS)                                                \
        ornqx_ld(W(XG), W(MS), W(DS))

/* xor (G = G ^ S) */

#define xorpx_rr(XG, XS)                                                    \
        xorqx_rr(W(XG), W(XS))

#define xorpx_ld(XG, MS, DS)                                                \
        xorqx_ld(W(XG), W(MS), W(DS))

/* not (G = ~G) */

#define notpx_rx(XG)                                                        \
        notqx_rx(W(XG))

/* neg (G = -G) */

#define negps_rx(XG)                                                        \
        negqs_rx(W(XG))

/* add (G = G + S) */

#define addps_rr(XG, XS)                                                    \
        addqs_rr(W(XG), W(XS))

#define addps_ld(XG, MS, DS)                                                \
        addqs_ld(W(XG), W(MS), W(DS))

/* sub (G = G - S) */

#define subps_rr(XG, XS)                                                    \
        subqs_rr(W(XG), W(XS))

#define subps_ld(XG, MS, DS)                                                \
        subqs_ld(W(XG), W(MS), W(DS))

/* mul (G = G * S) */

#define mulps_rr(XG, XS)                                                    \
        mulqs_rr(W(XG), W(XS))

#define mulps_ld(XG, MS, DS)                                                \
        mulqs_ld(W(XG), W(MS), W(DS))

/* div (G = G / S) */

#define divps_rr(XG, XS)                                                    \
        divqs_rr(W(XG), W(XS))

#define divps_ld(XG, MS, DS)                                                \
        divqs_ld(W(XG), W(MS), W(DS))

/* sqr (D = sqrt S) */

#define sqrps_rr(XD, XS)                                                    \
        sqrqs_rr(W(XD), W(XS))

#define sqrps_ld(XD, MS, DS)                                                \
        sqrqs_ld(W(XD), W(MS), W(DS))

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP == 0

#define rceps_rr(XD, XS)                                                    \
        rceqs_rr(W(XD), W(XS))

#define rcsps_rr(XG, XS) /* destroys RM */                                  \
        rcsqs_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ == 0

#define rseps_rr(XD, XS)                                                    \
        rseqs_rr(W(XD), W(XS))

#define rssps_rr(XG, XS) /* destroys RM */                                  \
        rssqs_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmaps_rr(XG, XS, XT)                                                \
        fmaqs_rr(W(XG), W(XS), W(XT))

#define fmaps_ld(XG, XS, MT, DT)                                            \
        fmaqs_ld(W(XG), W(XS), W(MT), W(DT))

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsps_rr(XG, XS, XT)                                                \
        fmsqs_rr(W(XG), W(XS), W(XT))

#define fmsps_ld(XG, XS, MT, DT)                                            \
        fmsqs_ld(W(XG), W(XS), W(MT), W(DT))

/* min (G = G < S ? G : S) */

#define minps_rr(XG, XS)                                                    \
        minqs_rr(W(XG), W(XS))

#define minps_ld(XG, MS, DS)                                                \
        minqs_ld(W(XG), W(MS), W(DS))

/* max (G = G > S ? G : S) */

#define maxps_rr(XG, XS)                                                    \
        maxqs_rr(W(XG), W(XS))

#define maxps_ld(XG, MS, DS)                                                \
        maxqs_ld(W(XG), W(MS), W(DS))

/* cmp (G = G ? S) */

#define ceqps_rr(XG, XS)                                                    \
        ceqqs_rr(W(XG), W(XS))

#define ceqps_ld(XG, MS, DS)                                                \
        ceqqs_ld(W(XG), W(MS), W(DS))

#define cneps_rr(XG, XS)                                                    \
        cneqs_rr(W(XG), W(XS))

#define cneps_ld(XG, MS, DS)                                                \
        cneqs_ld(W(XG), W(MS), W(DS))

#define cltps_rr(XG, XS)                                                    \
        cltqs_rr(W(XG), W(XS))

#define cltps_ld(XG, MS, DS)                                                \
        cltqs_ld(W(XG), W(MS), W(DS))

#define cleps_rr(XG, XS)                                                    \
        cleqs_rr(W(XG), W(XS))

#define cleps_ld(XG, MS, DS)                                                \
        cleqs_ld(W(XG), W(MS), W(DS))

#define cgtps_rr(XG, XS)                                                    \
        cgtqs_rr(W(XG), W(XS))

#define cgtps_ld(XG, MS, DS)                                                \
        cgtqs_ld(W(XG), W(MS), W(DS))

#define cgeps_rr(XG, XS)                                                    \
        cgeqs_rr(W(XG), W(XS))

#define cgeps_ld(XG, MS, DS)                                                \
        cgeqs_ld(W(XG), W(MS), W(DS))

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
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
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
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
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
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
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
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

/* add (G = G + S) */

#define addpx_rr(XG, XS)                                                    \
        addqx_rr(W(XG), W(XS))

#define addpx_ld(XG, MS, DS)                                                \
        addqx_ld(W(XG), W(MS), W(DS))

/* sub (G = G - S) */

#define subpx_rr(XG, XS)                                                    \
        subqx_rr(W(XG), W(XS))

#define subpx_ld(XG, MS, DS)                                                \
        subqx_ld(W(XG), W(MS), W(DS))

/* shl (G = G << S) */

#define shlpx_ri(XG, IS)                                                    \
        shlqx_ri(W(XG), W(IS))

#define shlpx_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        shlqx_ld(W(XG), W(MS), W(DS))

/* shr (G = G >> S) */

#define shrpx_ri(XG, IS)                                                    \
        shrqx_ri(W(XG), W(IS))

#define shrpx_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        shrqx_ld(W(XG), W(MS), W(DS))

#define shrpn_ri(XG, IS)                                                    \
        shrqn_ri(W(XG), W(IS))

#define shrpn_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        shrqn_ld(W(XG), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
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
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvtpn_rr(XD, XS)                                                    \
        cvtqn_rr(W(XD), W(XS))

#define cvtpn_ld(XD, MS, DS)                                                \
        cvtqn_ld(W(XD), W(MS), W(DS))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnrps_rr(XD, XS, mode)                                              \
        rnrqs_rr(W(XD), W(XS), mode)

#define cvrps_rr(XD, XS, mode)                                              \
        cvrqs_rr(W(XD), W(XS), mode)

/* mmv (D = mask-merge S)
 * uses Xmm0 implicitly as a mask register */

#define mmvpx_ld(XD, MS, DS) /* not portable, use conditionally (on x86) */ \
        mmvqx_ld(W(XD), W(MS), W(DS))

#define mmvpx_st(XS, MD, DD) /* not portable, use conditionally (on x86) */ \
        mmvqx_st(W(XS), W(MD), W(DD))

#endif /* RT_ELEMENT */

/******************************************************************************/
/************************   COMMON BASE INSTRUCTIONS   ************************/
/******************************************************************************/

/***************** original forms of deprecated cmdx* aliases *****************/

/* adr */

#define adrxx_lb(lb) /* load label to Reax */                               \
        label_ld(lb)

/* mul (G = G * S) */

#define mulxn_ri(RG, IS)                                                    \
        mulxx_ri(W(RG), W(IS))

#define mulxn_rr(RG, RS)                                                    \
        mulxx_rr(W(RG), W(RS))

#define mulxn_ld(RG, MS, DS)                                                \
        mulxx_ld(W(RG), W(MS), W(DS))

/***************** original forms of one-operand instructions *****************/

/* not (G = ~G) */

#define notxx_rr(RG)                                                        \
        notxx_rx(W(RG))

#define notxx_mm(MG, DG)                                                    \
        notxx_mx(W(MG), W(DG))

/* neg (G = -G) */

#define negxx_rr(RG)                                                        \
        negxx_rx(W(RG))

#define negxx_mm(MG, DG)                                                    \
        negxx_mx(W(MG), W(DG))

/* jmp */

#define jmpxx_rr(RS)                                                        \
        jmpxx_xr(W(RS))

#define jmpxx_mm(MS, DS)                                                    \
        jmpxx_xm(W(MS), W(DS))

/***************** address-sized instructions for 32-bit mode *****************/

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
 * set-flags: undefined (*x), yes (*z) */

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
 * set-flags: undefined (*x), yes (*z) */

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
 * set-flags: undefined (*x), yes (*z) */

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
 * 0th byte - 128-bit version, 1st byte - 256-bit version,
 * 2nd byte - 512-bit version, 3rd byte - upper, reserved */

     /* verxx_xx() in 32-bit rtarch_***.h files, destroys Reax, ... , Redi */

/************************* address-sized instructions *************************/

/* adr (D = adr S)
 * set-flags: no */

     /* adrxx_ld(RD, MS, DS) is defined in 32-bit rtarch_***.h files */

     /* adrpx_ld(RD, MS, DS) in 32-bit rtarch_***_***.h files, SIMD-aligned */

/************************* pointer-sized instructions *************************/

/* label (D = Reax = adr lb)
 * set-flags: no */

     /* label_ld(lb) is defined in rtarch.h file, loads label to Reax */

     /* label_st(lb, MD, DD) is defined in rtarch.h file, destroys Reax */

/* jmp (if unconditional jump S/lb, else if cc flags then jump lb)
 * set-flags: no
 * maximum byte-address-range for un/conditional jumps is signed 18/16-bit
 * based on minimum natively-encoded offset across supported targets (u/c)
 * MIPS:18-bit, Power:26-bit, AArch32:26-bit, AArch64:28-bit, x86:32-bit /
 * MIPS:18-bit, Power:16-bit, AArch32:26-bit, AArch64:21-bit, x86:32-bit */

     /* jccxx_** is defined in 32-bit rtarch_***.h files */

/************************* register-size instructions *************************/

/* stack (push stack = S, D = pop stack)
 * set-flags: no (sequence cmp/stack_la/jmp is not allowed on MIPS & Power)
 * adjust stack pointer with 8-byte (64-bit) steps on all current targets */

     /* stack_** is defined in 32-bit rtarch_***.h files */

/***************** address-sized instructions for 64-bit mode *****************/

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
 * set-flags: undefined (*x), yes (*z) */

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
 * set-flags: undefined (*x), yes (*z) */

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
 * set-flags: undefined (*x), yes (*z) */

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
 * 0th byte - 128-bit version, 1st byte - 256-bit version,
 * 2nd byte - 512-bit version, 3rd byte - upper, reserved */

     /* verxx_xx() in 32-bit rtarch_***.h files, destroys Reax, ... , Redi */

/************************* address-sized instructions *************************/

/* adr (D = adr S)
 * set-flags: no */

     /* adrxx_ld(RD, MS, DS) is defined in 32-bit rtarch_***.h files */

     /* adrpx_ld(RD, MS, DS) in 32-bit rtarch_***_***.h files, SIMD-aligned */

/************************* pointer-sized instructions *************************/

/* label (D = Reax = adr lb)
 * set-flags: no */

     /* label_ld(lb) is defined in rtarch.h file, loads label to Reax */

     /* label_st(lb, MD, DD) is defined in rtarch.h file, destroys Reax */

/* jmp (if unconditional jump S/lb, else if cc flags then jump lb)
 * set-flags: no
 * maximum byte-address-range for un/conditional jumps is signed 18/16-bit
 * based on minimum natively-encoded offset across supported targets (u/c)
 * MIPS:18-bit, Power:26-bit, AArch32:26-bit, AArch64:28-bit, x86:32-bit /
 * MIPS:18-bit, Power:16-bit, AArch32:26-bit, AArch64:21-bit, x86:32-bit */

     /* jccxx_** is defined in 32-bit rtarch_***.h files */

/************************* register-size instructions *************************/

/* stack (push stack = S, D = pop stack)
 * set-flags: no (sequence cmp/stack_la/jmp is not allowed on MIPS & Power)
 * adjust stack pointer with 8-byte (64-bit) steps on all current targets */

     /* stack_** is defined in 32-bit rtarch_***.h files */

#endif /* RT_ADDRESS */

/***************** element-sized instructions for 32-bit mode *****************/

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
 * set-flags: undefined (*x), yes (*z) */

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
 * set-flags: undefined (*x), yes (*z) */

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
 * set-flags: undefined (*x), yes (*z) */

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

/***************** element-sized instructions for 64-bit mode *****************/

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
 * set-flags: undefined (*x), yes (*z) */

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
 * set-flags: undefined (*x), yes (*z) */

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
 * set-flags: undefined (*x), yes (*z) */

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

#endif /* RT_RTBASE_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

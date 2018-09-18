/******************************************************************************/
/* Copyright (c) 2013-2018 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTBASE_H
#define RT_RTBASE_H

#include <math.h>
#include <float.h>
#include <stdlib.h>

#include "rtzero.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtbase.h: Base type definitions file.
 * The table of contents is provided below.
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

/*----------------------------------------------------------------------------*/

/*******************************   DEFINITIONS   ******************************/

/*************************   SIMD BACKEND STRUCTURE   *************************/

/************************   COMMON SIMD INSTRUCTIONS   ************************/

/**** var-len **** SIMD instructions with fixed-32-bit-element ****************/
/**** 256-bit **** SIMD instructions with fixed-32-bit-element ****************/
/**** 128-bit **** SIMD instructions with fixed-32-bit-element ****************/

/**** var-len **** SIMD instructions with fixed-64-bit-element ****************/
/**** 256-bit **** SIMD instructions with fixed-64-bit-element ****************/
/**** 128-bit **** SIMD instructions with fixed-64-bit-element ****************/

/************************   COMMON BASE INSTRUCTIONS   ************************/

/***************** original forms of deprecated cmdx* aliases *****************/
/***************** original forms of one-operand instructions *****************/

/*********************************   CONFIG   *********************************/

/*----------------------------------------------------------------------------*/

/******************************************************************************/
/*******************************   DEFINITIONS   ******************************/
/******************************************************************************/

/*
 * Determine mapping of vector-length-agnostic SIMD subsets: cmdo, cmdp, cmdq.
 */
#if (defined RT_SIMD)
/* RT_SIMD is already defined outside */
#elif           (RT_2K8_R8)
#define RT_SIMD 2048
#elif (RT_1K4 || RT_1K4_R8)
#define RT_SIMD 1024
#elif (RT_512 || RT_512_R8)
#define RT_SIMD 512
#elif (RT_256 || RT_256_R8)
#define RT_SIMD 256
#elif (RT_128)
#define RT_SIMD 128
#endif /* RT_SIMD: 2048, 1024, 512, 256, 128 */

/*
 * Determine SIMD total-quads for backend structs (maximal for a given build).
 */
#if (defined RT_MAXQ)
#define Q RT_MAXQ
#elif           (RT_2K8_R8)
#define Q 16
#elif (RT_1K4 || RT_1K4_R8)
#define Q 8
#elif (RT_512 || RT_512_R8)
#define Q 4
#elif (RT_256 || RT_256_R8)
#define Q 2
#elif (RT_128)
#define Q 1
#endif /* Q: 16, 8, 4, 2, 1 */

/*
 * RT_DATA determines the maximum load-level for data structures in code-base.
 * 1 - means full DP-level (12-bit displacements) is filled or exceeded (Q=1).
 * 2 - means 1/2  DP-level (11-bit displacements) has not been exceeded (Q=1).
 * 4 - means 1/4  DP-level (10-bit displacements) has not been exceeded (Q=1).
 * 8 - means 1/8  DP-level  (9-bit displacements) has not been exceeded (Q=1).
 * 16  means 1/16 DP-level  (8-bit displacements) has not been exceeded (Q=1).
 * NOTE: the built-in rt_SIMD_INFO structure is already filled at full 1/16th.
 */
#ifndef RT_DATA
#define RT_DATA 1
#endif /* RT_DATA */

#define O (Q/RT_DATA)

#if O == 0
#undef  O
#define O 1
#endif /* O */

/*
 * Determine SIMD quads scale-factor for displacements based on RT_DATA-level.
 */
#if   O == 16
#define DP(dp) _DH(dp)
#define DE(dp) _DV(dp)
#define DF(dp) _DV(dp)
#define DG(dp) _DV(dp)
#define DH(dp) _DV(dp)
#define DV(dp) _DV(dp)
#elif O == 8
#define DP(dp) _DG(dp)
#define DE(dp) _DH(dp)
#define DF(dp) _DV(dp)
#define DG(dp) _DV(dp)
#define DH(dp) _DV(dp)
#define DV(dp) _DV(dp)
#elif O == 4
#define DP(dp) _DF(dp)
#define DE(dp) _DG(dp)
#define DF(dp) _DH(dp)
#define DG(dp) _DV(dp)
#define DH(dp) _DV(dp)
#define DV(dp) _DV(dp)
#elif O == 2
#define DP(dp) _DE(dp)
#define DE(dp) _DF(dp)
#define DF(dp) _DG(dp)
#define DG(dp) _DH(dp)
#define DH(dp) _DV(dp)
#define DV(dp) _DV(dp)
#elif O == 1
#define DP(dp) _DP(dp)
#define DE(dp) _DE(dp)
#define DF(dp) _DF(dp)
#define DG(dp) _DG(dp)
#define DH(dp) _DH(dp)
#define DV(dp) _DV(dp)
#endif /* O: 16, 8, 4, 2, 1 */

/*
 * Determine SIMD properties for a given SIMD target (vector-length-agnostic).
 */
#if   (RT_SIMD == 2048)

#define RT_SIMD_ALIGN       256
#define RT_SIMD_WIDTH64     32
#define RT_SIMD_SET64(s, v) s[0]=s[1]=s[2]=s[3]=s[4]=s[5]=s[6]=s[7]=\
                            s[8]=s[9]=s[10]=s[11]=s[12]=s[13]=s[14]=s[15]=\
                            s[16]=s[17]=s[18]=s[19]=s[20]=s[21]=s[22]=s[23]=\
                            s[24]=s[25]=s[26]=s[27]=s[28]=s[29]=s[30]=s[31]=v
#define RT_SIMD_WIDTH32     64
#define RT_SIMD_SET32(s, v) s[0]=s[1]=s[2]=s[3]=s[4]=s[5]=s[6]=s[7]=\
                            s[8]=s[9]=s[10]=s[11]=s[12]=s[13]=s[14]=s[15]=\
                            s[16]=s[17]=s[18]=s[19]=s[20]=s[21]=s[22]=s[23]=\
                            s[24]=s[25]=s[26]=s[27]=s[28]=s[29]=s[30]=s[31]=\
                            s[32]=s[33]=s[34]=s[35]=s[36]=s[37]=s[38]=s[39]=\
                            s[40]=s[41]=s[42]=s[43]=s[44]=s[45]=s[46]=s[47]=\
                            s[48]=s[49]=s[50]=s[51]=s[52]=s[53]=s[54]=s[55]=\
                            s[56]=s[57]=s[58]=s[59]=s[60]=s[61]=s[62]=s[63]=v
#elif (RT_SIMD == 1024)

#define RT_SIMD_ALIGN       128
#define RT_SIMD_WIDTH64     16
#define RT_SIMD_SET64(s, v) s[0]=s[1]=s[2]=s[3]=s[4]=s[5]=s[6]=s[7]=\
                            s[8]=s[9]=s[10]=s[11]=s[12]=s[13]=s[14]=s[15]=v
#define RT_SIMD_WIDTH32     32
#define RT_SIMD_SET32(s, v) s[0]=s[1]=s[2]=s[3]=s[4]=s[5]=s[6]=s[7]=\
                            s[8]=s[9]=s[10]=s[11]=s[12]=s[13]=s[14]=s[15]=\
                            s[16]=s[17]=s[18]=s[19]=s[20]=s[21]=s[22]=s[23]=\
                            s[24]=s[25]=s[26]=s[27]=s[28]=s[29]=s[30]=s[31]=v
#elif (RT_SIMD == 512)

#define RT_SIMD_ALIGN       64
#define RT_SIMD_WIDTH64     8
#define RT_SIMD_SET64(s, v) s[0]=s[1]=s[2]=s[3]=s[4]=s[5]=s[6]=s[7]=v
#define RT_SIMD_WIDTH32     16
#define RT_SIMD_SET32(s, v) s[0]=s[1]=s[2]=s[3]=s[4]=s[5]=s[6]=s[7]=\
                            s[8]=s[9]=s[10]=s[11]=s[12]=s[13]=s[14]=s[15]=v

#elif (RT_SIMD == 256) && !(defined RT_SVEX1)

#define RT_SIMD_REGS            RT_SIMD_REGS_256
#define RT_SIMD_ALIGN           RT_SIMD_ALIGN_256
#define RT_SIMD_WIDTH64         RT_SIMD_WIDTH64_256
#define RT_SIMD_SET64(s, v)     RT_SIMD_SET64_256(s, v)
#define RT_SIMD_WIDTH32         RT_SIMD_WIDTH32_256
#define RT_SIMD_SET32(s, v)     RT_SIMD_SET32_256(s, v)

#elif (RT_SIMD == 256)

#define RT_SIMD_ALIGN           RT_SIMD_ALIGN_256
#define RT_SIMD_WIDTH64         RT_SIMD_WIDTH64_256
#define RT_SIMD_SET64(s, v)     RT_SIMD_SET64_256(s, v)
#define RT_SIMD_WIDTH32         RT_SIMD_WIDTH32_256
#define RT_SIMD_SET32(s, v)     RT_SIMD_SET32_256(s, v)

#elif (RT_SIMD == 128)

#define RT_SIMD_REGS            RT_SIMD_REGS_128
#define RT_SIMD_ALIGN           RT_SIMD_ALIGN_128
#define RT_SIMD_WIDTH64         RT_SIMD_WIDTH64_128
#define RT_SIMD_SET64(s, v)     RT_SIMD_SET64_128(s, v)
#define RT_SIMD_WIDTH32         RT_SIMD_WIDTH32_128
#define RT_SIMD_SET32(s, v)     RT_SIMD_SET32_128(s, v)

#endif /* RT_SIMD: 2048, 1024, 512, 256, 128 */

#define RT_SIMD_ALIGN_256       32
#define RT_SIMD_WIDTH64_256     4
#define RT_SIMD_SET64_256(s, v) s[0]=s[1]=s[2]=s[3]=v
#define RT_SIMD_WIDTH32_256     8
#define RT_SIMD_SET32_256(s, v) s[0]=s[1]=s[2]=s[3]=s[4]=s[5]=s[6]=s[7]=v

#define RT_SIMD_ALIGN_128       16
#define RT_SIMD_WIDTH64_128     2
#define RT_SIMD_SET64_128(s, v) s[0]=s[1]=v
#define RT_SIMD_WIDTH32_128     4
#define RT_SIMD_SET32_128(s, v) s[0]=s[1]=s[2]=s[3]=v

/*
 * SIMD total-quads (number of 128-bit chunks) for chosen SIMD target.
 * Short name Q represents maximal total-quads for given build config.
 * RT_SIMD_QUADS and Q may differ for builds with runtime SIMD target
 * selection in backend's ASM code sections, Q is used in SIMD structs.
 */
#define RT_SIMD_QUADS       (RT_SIMD_WIDTH32/4)

/*
 * SIMD width (number of scalar elements) for given SIMD element size.
 * Short names R, S, T represent maximal width for given build config.
 * RT_SIMD_WIDTH and S may differ for builds with runtime SIMD target
 * selection in backend's ASM code sections, S is used in SIMD structs.
 */
#define N   (Q*8)       /* for cmdm*_** SIMD-subset, rt_fp16 SIMD-fields */
#define R   (Q*4)       /* for cmdo*_** SIMD-subset, rt_fp32 SIMD-fields */
#define S   (Q*4/L)     /* for cmdp*_** SIMD-subset, rt_real SIMD-fields */
#define T   (Q*2)       /* for cmdq*_** SIMD-subset, rt_fp64 SIMD-fields */

/*
 * Short names for pointer, address and SIMD element sizes (in 32-bit chunks).
 */
#define P   (RT_POINTER/32)         /* short name for RT_POINTER/32 */
#define A   (RT_ADDRESS/32)         /* short name for RT_ADDRESS/32 */
#define L   (RT_ELEMENT/32)         /* short name for RT_ELEMENT/32 */

/*
 * Offset corrections for endianness (used in backend's structs and BASE ISA).
 */
#define B   (RT_ENDIAN*(2-1)*4)     /* for cmdw*_** working on 64-bit field */
#define C   (RT_ENDIAN*(2-A)*4)     /* for cmdx*_** working on 64-bit field */
#define D   (RT_ENDIAN*(P-1)*4)     /* for cmdw*_** working on P-size field */
#define E   (RT_ENDIAN*(P-A)*4)     /* for cmdx*_** working on P-size field */
#define F   (RT_ENDIAN*(A-1)*4)     /* for cmdw*_** working on A-size field */
#define G   (RT_ENDIAN*(2-P)*4)     /* for jmpxx_xm working on 64-bit field */
#define H   (RT_ENDIAN*(L-1)*4)     /* for cmdw*_** working on L-size field */
#define I   (RT_ENDIAN*(2-L)*4)     /* for cmdy*_** working on 64-bit field */

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
#if   (defined RT_WIN32) /* Win32, MSVC -------- for older versions --------- */

typedef __int64             rt_si64;
#define    PR_Z /*printf*/  "I64"

typedef unsigned __int64    rt_ui64;
#define    PRuZ /*printf*/  "I64u"

#define  LL(x)  x
#define ULL(x)  x

#else /* --- Win64, GCC --- Linux, GCC -------------------------------------- */

typedef long long           rt_si64;
#define    PR_Z /*printf*/  "ll"

typedef unsigned long long  rt_ui64;
#define    PRuZ /*printf*/  "llu"

#define  LL(x)  x##LL
#define ULL(x)  x##ULL

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
#if   (defined RT_WIN64) /* Win64, GCC -------------------------------------- */

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
 * SIMD info structure for ASM_ENTER/ASM_LEAVE contains internal variables
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

    rt_elem scr01[S];       /* scratchpad1, internal */
#define inf_SCR01(nx)       DP(Q*0x070 + nx)

    rt_elem scr02[S];       /* scratchpad2, internal */
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
    /* register file (maximum of 64 2K8-bit registers) */

    rt_ui32 file[64*64];
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
    RT_SIMD_SET64(__Info__->gpc04_64, LL(0x7FFFFFFFFFFFFFFF));              \
    RT_SIMD_SET64(__Info__->gpc05_64, LL(0x3FF0000000000000));              \
    RT_SIMD_SET64(__Info__->gpc06_64, LL(0x8000000000000000));              \
    __Info__->regs = (rt_ui64)(rt_word)__Regs__;

#define ASM_DONE(__Info__)

/*
 * Return SIMD target mask (in rt_SIMD_INFO->ver format) from "simd" parameters:
 * SIMD native-size (1,..,16) in 0th (lowest) byte  <- number of 128-bit chunks
 * SIMD type (1,2,4,8, 16,32) in 1st (higher) byte  <- in format for given size
 * SIMD size-factor (1, 2, 4) in 2nd (higher) byte  <- register = size * factor
 * SIMD regs (8, 15, 16, 30) in 3rd (highest) byte  <- logical vector registers
 * For interpretation of SIMD target mask check compatibility layer in rtzero.h
 */
static
rt_si32 mask_init(rt_si32 simd)
{
    rt_si32 n_simd = (simd >> 0) & 0xFF;
    rt_si32 s_type = (simd >> 8) & 0xFF;
    rt_si32 k_size = (simd >> 16) & 0xFF;
    rt_si32 v_regs = (simd >> 24) & 0xFF;

    rt_si32 mask = 0, s_x2r8 = 0, s_fma3 = 0;
    rt_si32 n = n_simd, k = k_size, m = 0, s = 0;

#if   (defined RT_P32) || (defined RT_P64)
    s_x2r8 = (s_type == 0 ? 0x10 : s_type & 0x10) >> 2; /* <- 128-x2r8 vmx */
#endif /* PPC targets */
#if   (defined RT_X32) || (defined RT_X64) || (defined RT_X86)
    s_fma3 = (s_type == 0 ? 0x30 : s_type & 0x30); /* <- 128-bit fma3/avx2 */
#endif /* X86 targets */

    s_type = s_type == 0 ? 0xF : s_type & 0xF;
    n_simd = n_simd == 0 ? 16 : n_simd; /* <- 16 is the maximal native-size */
    k_size = k_size == 0 ? 2 : k_size;   /* <- 2 is the optimal size-factor */

    for (; n_simd >= n && n_simd > 0; n_simd /= 2)
    {
        for (; k_size >= k && k_size > 0; k_size /= 2)
        {
#if   (defined RT_ARM) /* original legacy target, supports only 8 registers */
            if (k_size == 1 && n_simd == 1 && v_regs <= 8)
            {
                mask |= s_type;
            }
#elif !defined RT_X32  && !defined RT_X64  && !defined RT_X86 /* modern RISCs */
#if   (defined RT_SVEX1)
            if (k <= 1 && n_simd == 16 && v_regs <= 30)
            {
                mask |= s_type << 28;
            }
            if (k <= 1 && n_simd == 8 && v_regs <= 30)
            {
                mask |= s_type << 24;
            }
            if (k <= 1 && n_simd == 4 && v_regs <= 30)
            {
                mask |= s_type << 16;
            }
            if (k <= 1 && n_simd == 2 && v_regs <= 30)
            {
                mask |= s_type <<  8;
            }
#endif /* SVE targets */
            m = 2; s = 0x00030F;
#if   (defined RT_P32) || (defined RT_P64)
            m = 4; s = 0x030F0F;
            if (k <= 1 && n && n_simd == 4 && v_regs <= 15)
            {
                k_size = k = 4;
                n_simd = n = 1;
            }
            if (k == 2 && n && n_simd == 2 && v_regs <= 15)
            {
                k_size = k = 4;
                n_simd = n = 1;
            }
            if (k <= 1 && n && n_simd == 2 && v_regs <= 30)
            {
                k_size = k = 2;
                n_simd = n = 1;
            }
#endif /* PPC targets */
            if (k <= 1 && n && n_simd == 2 && v_regs <= 15)
            {
                k_size = k = 2;
                n_simd = n = 1;
            }
            if (k_size <= m && n_simd == 1 && v_regs <= 8)
            {
                mask |= s_type << (8*(k_size/2) - 4*(k_size>1)) | s_x2r8 << 4;
            }
            if (k_size <= m && n_simd == 1 && v_regs <= 15)
            {
                mask |= s_type << (8*(k_size/2)) & s;
            }
            if (k_size == 1 && n_simd == 1 && v_regs <= 30 && s_type <= 2)
            {
                mask |= s_type << (8*(k_size/2)) & s;
            }
            if (k_size == 2 && n_simd == 1 && v_regs <= 30 && s_type >= 4)
            {
                mask |= s_type << (8*(k_size/2)) & s;
            }
#elif (defined RT_X32) || (defined RT_X64)
            if (k <= 1 && n && n_simd == 16 && v_regs <= 8)
            {
                k_size = k = 4;
                n_simd = n = 4;
            }
            if (k == 2 && n && n_simd == 8 && v_regs <= 8)
            {
                k_size = k = 4;
                n_simd = n = 4;
            }
            if (k <= 1 && n && n_simd == 8 && v_regs <= 16)
            {
                k_size = k = 2;
                n_simd = n = 4;
            }
            if (k_size == 4 && n_simd == 4 && v_regs <= 8)
            {
                mask |= s_type << 28;
            }
            if (k_size == 2 && n_simd == 4 && v_regs <= 16)
            {
                mask |= s_type << 24;
            }
            if (k_size == 1 && n_simd == 4 && v_regs <= 30)
            {
                mask |= s_type << 16;
            }
            if (k_size == 2 && n_simd <= 4 && v_regs <= 8)
            {
                mask |= s_type << (8*(n_simd/2) + 4);
            }
            if (k_size == 1 && n_simd <= 4 && v_regs <= 16)
            {
                mask |= s_type << (8*(n_simd/2)) | (n_simd == 1 ? s_fma3 : 0);
            }
            if (k_size == 1 && n_simd == 1 && v_regs <= 30 && s_type == 2)
            {
                mask |= s_type << (8*(n_simd/2));
            }
            if (k_size == 1 && n_simd == 2 && v_regs <= 30 && s_type == 8)
            {
                mask |= s_type << (8*(n_simd/2));
            }
#elif (defined RT_X86) /* original legacy target, supports only 8 registers */
            if (k_size == 1 && n_simd <= 4 && v_regs <= 8)
            {
                mask |= s_type << (8*(n_simd/2)) | (n_simd == 1 ? s_fma3 : 0);
            }
#endif /* all targets */
        }

        k_size = k == 0 ? 2 : k;         /* <- 2 is the optimal size-factor */
    }

    return mask;
}

/*
 * Pack/return SIMD parameters from target "mask" (in rt_SIMD_INFO->ver format).
 * SIMD native-size (1,..,16) in 0th (lowest) byte  <- number of 128-bit chunks
 * SIMD type (1,2,4,8, 16,32) in 1st (higher) byte  <- in format for given size
 * SIMD size-factor (1, 2, 4) in 2nd (higher) byte  <- register = size * factor
 * SIMD regs (8, 15, 16, 30) in 3rd (highest) byte  <- logical vector registers
 * For interpretation of SIMD target mask check compatibility layer in rtzero.h
 */
static
rt_si32 from_mask(rt_si32 mask)
{
    rt_si32 n_simd, s_type, k_size, v_regs, n_keep = 0;

    n_simd = mask >= 0x01000000 ? 6 : mask >= 0x00010000 ? 4 :
             mask >= 0x00000100 ? 2 : mask >= 0x00000001 ? 1 : 0;

    s_type = mask >> (8*(n_simd/2));
    k_size = s_type >= 0x10 ? 2 : 1;
    s_type = s_type >> 4*(k_size-1);
    v_regs = 16 / k_size;

#if   (defined RT_ARM) /* original legacy target, supports only 8 registers */
    if (n_simd != 1 || k_size >= 2)
    {
        n_simd = s_type = k_size = v_regs = 0;
    }
    else
    {
        v_regs = 8;
    }
#elif !defined RT_X32  && !defined RT_X64  && !defined RT_X86 /* modern RISCs */
    v_regs = v_regs == 16 ? 15 : 8;
#if   (defined RT_SVEX1)
    if (n_simd == 6)
    {
        n_simd = k_size * 8;
        k_size = 1;
    }
    if (n_simd >= 2 && k_size == 1 && s_type >= 4)
    {
        v_regs = 30;
    }
    if (n_simd >= 4 && k_size == 1)
    {
        n_keep = 1;
    }
    if (n_simd == 2 && k_size == 1 && s_type >= 4)
    {
        n_keep = 1;
    }
#endif /* SVE targets */
#if  (defined RT_P32) || (defined RT_P64)
    if (n_simd == 2 && k_size == 1 && s_type >= 4)
    {
        v_regs = 30;
    }
    if (n_simd == 1 && k_size == 2 && s_type == 4)
    {
        s_type = 0x10; /* <- vmx-x2r8, 256-bit */
        v_regs = 8;
    }
#endif /* PPC targets */
    if (n_simd >= 2 && n_keep == 0)
    {
        k_size = k_size * n_simd;
        n_simd = 1;
    }
    if (n_simd == 1 && k_size == 1 && s_type <= 3)
    {
        v_regs = 30;
    }
#elif (defined RT_X32) || (defined RT_X64)
    if (n_simd == 1 && k_size == 2 && s_type <= 3)
    {
        k_size = 1;
        s_type <<= 4; /* <- fma3/avx2, 128-bit */
    }
    if (n_simd == 6)
    {
        k_size = k_size * 2;
        n_simd = 4;
    }
    if (n_simd == 4 && k_size == 1)
    {
        v_regs = 30;
    }
    if (n_simd == 2 && k_size == 1 && s_type == 8)
    {
        v_regs = 30;
    }
    if (n_simd == 1 && k_size == 1 && s_type == 2)
    {
        v_regs = 30;
    }
#elif (defined RT_X86) /* original legacy target, supports only 8 registers */
    if (n_simd == 1 && k_size == 2 && s_type <= 3)
    {
        k_size = 1;
        s_type <<= 4; /* <- fma3/avx2, 128-bit */
    }
    if (n_simd >= 6 || k_size >= 2)
    {
        n_simd = s_type = k_size = v_regs = 0;
    }
    else
    {
        v_regs = 8;
    }
#endif /* all targets */

    if (n_simd == 0 || mask == 0)
    {
        n_simd = s_type = k_size = v_regs = 0;
    }

    /* ------- v_regs ------- k_size ------- s_type ------- n_simd ------- */
    return (v_regs << 24) | (k_size << 16) | (s_type << 8) | (n_simd);
}

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

#if (defined RT_SIMD_CODE)

/****************** original adrpx instruction, SIMD-aligned ******************/

#define adrpx_ld(RD, MS, DS)                                                \
        adrxx_ld(W(RD), W(MS), W(DS))

/****************** original CHECK_MASK macro (configurable) ******************/

#define CHECK_MASK(lb, mask, XS) /* destroys Reax, jump lb if mask == S */  \
        mkjpx_rx(W(XS), mask, lb)

/****************** original FCTRL blocks (cannot be nested) ******************/

#define FCTRL_ENTER(mode) /* assumes default mode (ROUNDN) upon entry */    \
        FCTRL_SET(mode)

#define FCTRL_LEAVE(mode) /* resumes default mode (ROUNDN) upon leave */    \
        FCTRL_RESET()

/******************************************************************************/
/**** var-len **** SIMD instructions with fixed-32-bit-element ****************/
/******************************************************************************/

#if   (RT_SIMD == 2048)

#define adpos_rr(XG, XS) /* horizontal pairwise add, first 15-regs only */  \
        adpos3rr(W(XG), W(XG), W(XS))

#define adpos_ld(XG, MS, DS)                                                \
        adpos3ld(W(XG), W(XG), W(MS), W(DS))

#define adpos3rr(XD, XS, XT)                                                \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XT), Mebp, inf_SCR02(0))                                 \
        adpos_rx(W(XD))                                                     \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define adpos3ld(XD, XS, MT, DT)                                            \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XD), W(MT), W(DT))                                       \
        movox_st(W(XD), Mebp, inf_SCR02(0))                                 \
        adpos_rx(W(XD))                                                     \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define adhos_rr(XD, XS) /* horizontal reductive add, first 15-regs only */ \
        adpos3rr(W(XD), W(XS), W(XS))                                       \
        adpos3rr(W(XD), W(XD), W(XD))                                       \
        adpos3rr(W(XD), W(XD), W(XD))                                       \
        adpos3rr(W(XD), W(XD), W(XD))                                       \
        adpos3rr(W(XD), W(XD), W(XD))                                       \
        adpos3rr(W(XD), W(XD), W(XD))

#define adhos_ld(XD, MS, DS)                                                \
        movox_ld(W(XD), W(MS), W(DS))                                       \
        adhos_rr(W(XD), W(XD))

#if (defined RT_A32) || (defined RT_A64) || (defined RT_X32) || (defined RT_X64)

#define adpos_rx(XD) /* not portable, do not use outside */                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adpis_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        adpis_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        adpis_ld(W(XD), Mebp, inf_SCR01(0x50))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        adpis_ld(W(XD), Mebp, inf_SCR01(0x70))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x80))                              \
        adpis_ld(W(XD), Mebp, inf_SCR01(0x90))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0xA0))                              \
        adpis_ld(W(XD), Mebp, inf_SCR01(0xB0))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x50))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0xC0))                              \
        adpis_ld(W(XD), Mebp, inf_SCR01(0xD0))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0xE0))                              \
        adpis_ld(W(XD), Mebp, inf_SCR01(0xF0))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x70))                              \
        movix_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        adpis_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x80))                              \
        movix_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        adpis_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x90))                              \
        movix_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        adpis_ld(W(XD), Mebp, inf_SCR02(0x50))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0xA0))                              \
        movix_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        adpis_ld(W(XD), Mebp, inf_SCR02(0x70))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0xB0))                              \
        movix_ld(W(XD), Mebp, inf_SCR02(0x80))                              \
        adpis_ld(W(XD), Mebp, inf_SCR02(0x90))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0xC0))                              \
        movix_ld(W(XD), Mebp, inf_SCR02(0xA0))                              \
        adpis_ld(W(XD), Mebp, inf_SCR02(0xB0))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0xD0))                              \
        movix_ld(W(XD), Mebp, inf_SCR02(0xC0))                              \
        adpis_ld(W(XD), Mebp, inf_SCR02(0xD0))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0xE0))                              \
        movix_ld(W(XD), Mebp, inf_SCR02(0xE0))                              \
        adpis_ld(W(XD), Mebp, inf_SCR02(0xF0))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0xF0))

#else /* RT_M32, RT_M64, RT_P32, RT_P64 */

#define adpos_rx(XD) /* not portable, do not use outside */                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x24))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x28))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x2C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x34))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x38))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x3C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x1C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x44))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x48))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x4C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x24))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x50))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x54))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x28))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x58))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x5C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x2C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x64))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x68))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x6C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x34))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x70))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x74))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x38))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x78))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x7C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x3C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x80))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x84))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x88))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x8C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x44))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x90))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x94))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x48))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x98))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x9C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x4C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xA0))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0xA4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x50))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xA8))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0xAC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x54))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xB0))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0xB4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x58))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xB8))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0xBC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x5C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xC0))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0xC4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xC8))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0xCC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x64))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xD0))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0xD4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x68))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xD8))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0xDC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x6C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xE0))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0xE4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x70))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xE8))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0xEC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x74))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xF0))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0xF4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x78))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xF8))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0xFC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x7C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x80))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x84))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x88))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x8C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x24))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x90))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x28))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x2C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x94))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x34))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x98))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x38))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x3C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x9C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x44))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xA0))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x48))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x4C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xA4))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x50))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x54))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xA8))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x58))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x5C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xAC))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x64))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xB0))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x68))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x6C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xB4))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x70))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x74))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xB8))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x78))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x7C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xBC))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x80))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x84))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xC0))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x88))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x8C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xC4))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x90))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x94))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xC8))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x98))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x9C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xCC))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xA0))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0xA4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xD0))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xA8))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0xAC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xD4))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xB0))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0xB4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xD8))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xB8))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0xBC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xDC))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xC0))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0xC4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xE0))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xC8))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0xCC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xE4))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xD0))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0xD4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xE8))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xD8))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0xDC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xEC))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xE0))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0xE4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xF0))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xE8))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0xEC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xF4))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xF0))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0xF4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xF8))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xF8))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0xFC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xFC))

#endif /* target specific (ARM/x86 or MIPS/POWER) */

#define mlpos_rr(XG, XS) /* horizontal pairwise mul */                      \
        mlpos3rr(W(XG), W(XG), W(XS))

#define mlpos_ld(XG, MS, DS)                                                \
        mlpos3ld(W(XG), W(XG), W(MS), W(DS))

#define mlpos3rr(XD, XS, XT)                                                \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mlpos_rx(W(XD))                                                     \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define mlpos3ld(XD, XS, MT, DT)                                            \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XD), W(MT), W(DT))                                       \
        movox_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mlpos_rx(W(XD))                                                     \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define mlhos_rr(XD, XS) /* horizontal reductive mul */                     \
        mlpos3rr(W(XD), W(XS), W(XS))                                       \
        mlpos3rr(W(XD), W(XD), W(XD))                                       \
        mlpos3rr(W(XD), W(XD), W(XD))                                       \
        mlpos3rr(W(XD), W(XD), W(XD))                                       \
        mlpos3rr(W(XD), W(XD), W(XD))                                       \
        mlpos3rr(W(XD), W(XD), W(XD))

#define mlhos_ld(XD, MS, DS)                                                \
        movox_ld(W(XD), W(MS), W(DS))                                       \
        mlhos_rr(W(XD), W(XD))

#define mlpos_rx(XD) /* not portable, do not use outside */                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x24))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x28))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x2C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x34))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x38))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x3C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x1C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x44))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x48))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x4C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x24))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x50))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x54))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x28))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x58))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x5C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x2C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x64))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x68))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x6C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x34))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x70))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x74))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x38))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x78))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x7C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x3C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x80))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x84))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x88))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x8C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x44))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x90))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x94))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x48))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x98))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x9C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x4C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xA0))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0xA4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x50))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xA8))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0xAC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x54))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xB0))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0xB4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x58))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xB8))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0xBC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x5C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xC0))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0xC4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xC8))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0xCC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x64))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xD0))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0xD4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x68))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xD8))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0xDC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x6C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xE0))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0xE4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x70))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xE8))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0xEC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x74))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xF0))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0xF4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x78))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xF8))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0xFC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x7C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x80))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x84))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x88))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x8C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x24))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x90))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x28))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x2C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x94))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x34))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x98))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x38))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x3C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x9C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x44))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xA0))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x48))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x4C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xA4))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x50))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x54))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xA8))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x58))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x5C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xAC))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x64))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xB0))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x68))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x6C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xB4))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x70))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x74))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xB8))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x78))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x7C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xBC))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x80))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x84))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xC0))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x88))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x8C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xC4))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x90))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x94))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xC8))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x98))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x9C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xCC))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xA0))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0xA4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xD0))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xA8))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0xAC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xD4))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xB0))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0xB4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xD8))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xB8))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0xBC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xDC))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xC0))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0xC4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xE0))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xC8))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0xCC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xE4))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xD0))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0xD4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xE8))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xD8))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0xDC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xEC))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xE0))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0xE4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xF0))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xE8))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0xEC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xF4))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xF0))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0xF4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xF8))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xF8))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0xFC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xFC))

#define mnpos_rr(XG, XS) /* horizontal pairwise min */                      \
        mnpos3rr(W(XG), W(XG), W(XS))

#define mnpos_ld(XG, MS, DS)                                                \
        mnpos3ld(W(XG), W(XG), W(MS), W(DS))

#define mnpos3rr(XD, XS, XT)                                                \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mnpos_rx(W(XD))                                                     \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define mnpos3ld(XD, XS, MT, DT)                                            \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XD), W(MT), W(DT))                                       \
        movox_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mnpos_rx(W(XD))                                                     \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define mnhos_rr(XD, XS) /* horizontal reductive min */                     \
        mnpos3rr(W(XD), W(XS), W(XS))                                       \
        mnpos3rr(W(XD), W(XD), W(XD))                                       \
        mnpos3rr(W(XD), W(XD), W(XD))                                       \
        mnpos3rr(W(XD), W(XD), W(XD))                                       \
        mnpos3rr(W(XD), W(XD), W(XD))                                       \
        mnpos3rr(W(XD), W(XD), W(XD))

#define mnhos_ld(XD, MS, DS)                                                \
        movox_ld(W(XD), W(MS), W(DS))                                       \
        mnhos_rr(W(XD), W(XD))

#define mnpos_rx(XD) /* not portable, do not use outside */                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x24))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x28))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x2C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x34))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x38))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x3C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x1C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x44))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x48))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x4C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x24))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x50))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x54))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x28))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x58))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x5C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x2C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x64))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x68))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x6C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x34))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x70))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x74))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x38))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x78))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x7C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x3C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x80))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x84))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x88))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x8C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x44))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x90))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x94))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x48))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x98))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x9C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x4C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xA0))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0xA4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x50))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xA8))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0xAC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x54))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xB0))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0xB4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x58))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xB8))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0xBC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x5C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xC0))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0xC4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xC8))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0xCC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x64))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xD0))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0xD4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x68))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xD8))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0xDC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x6C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xE0))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0xE4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x70))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xE8))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0xEC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x74))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xF0))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0xF4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x78))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xF8))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0xFC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x7C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x80))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x84))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x88))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x8C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x24))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x90))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x28))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x2C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x94))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x34))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x98))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x38))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x3C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x9C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x44))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xA0))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x48))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x4C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xA4))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x50))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x54))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xA8))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x58))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x5C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xAC))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x64))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xB0))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x68))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x6C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xB4))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x70))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x74))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xB8))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x78))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x7C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xBC))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x80))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x84))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xC0))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x88))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x8C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xC4))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x90))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x94))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xC8))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x98))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x9C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xCC))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xA0))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0xA4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xD0))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xA8))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0xAC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xD4))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xB0))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0xB4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xD8))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xB8))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0xBC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xDC))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xC0))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0xC4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xE0))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xC8))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0xCC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xE4))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xD0))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0xD4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xE8))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xD8))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0xDC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xEC))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xE0))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0xE4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xF0))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xE8))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0xEC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xF4))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xF0))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0xF4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xF8))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xF8))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0xFC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xFC))

#define mxpos_rr(XG, XS) /* horizontal pairwise max */                      \
        mxpos3rr(W(XG), W(XG), W(XS))

#define mxpos_ld(XG, MS, DS)                                                \
        mxpos3ld(W(XG), W(XG), W(MS), W(DS))

#define mxpos3rr(XD, XS, XT)                                                \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mxpos_rx(W(XD))                                                     \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define mxpos3ld(XD, XS, MT, DT)                                            \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XD), W(MT), W(DT))                                       \
        movox_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mxpos_rx(W(XD))                                                     \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define mxhos_rr(XD, XS) /* horizontal reductive max */                     \
        mxpos3rr(W(XD), W(XS), W(XS))                                       \
        mxpos3rr(W(XD), W(XD), W(XD))                                       \
        mxpos3rr(W(XD), W(XD), W(XD))                                       \
        mxpos3rr(W(XD), W(XD), W(XD))                                       \
        mxpos3rr(W(XD), W(XD), W(XD))                                       \
        mxpos3rr(W(XD), W(XD), W(XD))

#define mxhos_ld(XD, MS, DS)                                                \
        movox_ld(W(XD), W(MS), W(DS))                                       \
        mxhos_rr(W(XD), W(XD))

#define mxpos_rx(XD) /* not portable, do not use outside */                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x24))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x28))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x2C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x34))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x38))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x3C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x1C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x44))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x48))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x4C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x24))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x50))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x54))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x28))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x58))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x5C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x2C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x64))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x68))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x6C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x34))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x70))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x74))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x38))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x78))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x7C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x3C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x80))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x84))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x88))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x8C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x44))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x90))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x94))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x48))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x98))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x9C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x4C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xA0))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0xA4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x50))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xA8))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0xAC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x54))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xB0))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0xB4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x58))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xB8))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0xBC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x5C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xC0))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0xC4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xC8))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0xCC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x64))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xD0))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0xD4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x68))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xD8))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0xDC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x6C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xE0))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0xE4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x70))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xE8))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0xEC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x74))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xF0))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0xF4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x78))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0xF8))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0xFC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x7C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x80))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x84))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x88))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x8C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x24))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x90))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x28))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x2C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x94))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x34))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x98))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x38))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x3C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x9C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x44))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xA0))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x48))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x4C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xA4))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x50))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x54))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xA8))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x58))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x5C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xAC))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x64))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xB0))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x68))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x6C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xB4))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x70))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x74))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xB8))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x78))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x7C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xBC))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x80))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x84))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xC0))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x88))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x8C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xC4))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x90))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x94))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xC8))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x98))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x9C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xCC))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xA0))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0xA4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xD0))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xA8))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0xAC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xD4))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xB0))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0xB4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xD8))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xB8))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0xBC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xDC))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xC0))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0xC4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xE0))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xC8))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0xCC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xE4))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xD0))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0xD4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xE8))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xD8))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0xDC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xEC))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xE0))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0xE4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xF0))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xE8))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0xEC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xF4))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xF0))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0xF4))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xF8))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0xF8))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0xFC))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0xFC))

#elif (RT_SIMD == 1024)

#define adpos_rr(XG, XS) /* horizontal pairwise add, first 15-regs only */  \
        adpos3rr(W(XG), W(XG), W(XS))

#define adpos_ld(XG, MS, DS)                                                \
        adpos3ld(W(XG), W(XG), W(MS), W(DS))

#define adpos3rr(XD, XS, XT)                                                \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XT), Mebp, inf_SCR02(0))                                 \
        adpos_rx(W(XD))                                                     \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define adpos3ld(XD, XS, MT, DT)                                            \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XD), W(MT), W(DT))                                       \
        movox_st(W(XD), Mebp, inf_SCR02(0))                                 \
        adpos_rx(W(XD))                                                     \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define adhos_rr(XD, XS) /* horizontal reductive add, first 15-regs only */ \
        adpos3rr(W(XD), W(XS), W(XS))                                       \
        adpos3rr(W(XD), W(XD), W(XD))                                       \
        adpos3rr(W(XD), W(XD), W(XD))                                       \
        adpos3rr(W(XD), W(XD), W(XD))                                       \
        adpos3rr(W(XD), W(XD), W(XD))

#define adhos_ld(XD, MS, DS)                                                \
        movox_ld(W(XD), W(MS), W(DS))                                       \
        adhos_rr(W(XD), W(XD))

#if (defined RT_A32) || (defined RT_A64) || (defined RT_X32) || (defined RT_X64)

#define adpos_rx(XD) /* not portable, do not use outside */                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adpis_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        adpis_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        adpis_ld(W(XD), Mebp, inf_SCR01(0x50))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        adpis_ld(W(XD), Mebp, inf_SCR01(0x70))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movix_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        adpis_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movix_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        adpis_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x50))                              \
        movix_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        adpis_ld(W(XD), Mebp, inf_SCR02(0x50))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movix_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        adpis_ld(W(XD), Mebp, inf_SCR02(0x70))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x70))

#else /* RT_M32, RT_M64, RT_P32, RT_P64 */

#define adpos_rx(XD) /* not portable, do not use outside */                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x24))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x28))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x2C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x34))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x38))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x3C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x1C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x44))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x48))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x4C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x24))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x50))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x54))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x28))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x58))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x5C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x2C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x64))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x68))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x6C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x34))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x70))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x74))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x38))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x78))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x7C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x3C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x44))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x48))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x4C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x24))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x50))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x28))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x2C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x54))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x34))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x58))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x38))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x3C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x5C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x44))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x48))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x4C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x64))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x50))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x54))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x68))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x58))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x5C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x6C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x64))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x70))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x68))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x6C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x74))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x70))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x74))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x78))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x78))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x7C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x7C))

#endif /* target specific (ARM/x86 or MIPS/POWER) */

#define mlpos_rr(XG, XS) /* horizontal pairwise mul */                      \
        mlpos3rr(W(XG), W(XG), W(XS))

#define mlpos_ld(XG, MS, DS)                                                \
        mlpos3ld(W(XG), W(XG), W(MS), W(DS))

#define mlpos3rr(XD, XS, XT)                                                \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mlpos_rx(W(XD))                                                     \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define mlpos3ld(XD, XS, MT, DT)                                            \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XD), W(MT), W(DT))                                       \
        movox_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mlpos_rx(W(XD))                                                     \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define mlhos_rr(XD, XS) /* horizontal reductive mul */                     \
        mlpos3rr(W(XD), W(XS), W(XS))                                       \
        mlpos3rr(W(XD), W(XD), W(XD))                                       \
        mlpos3rr(W(XD), W(XD), W(XD))                                       \
        mlpos3rr(W(XD), W(XD), W(XD))                                       \
        mlpos3rr(W(XD), W(XD), W(XD))

#define mlhos_ld(XD, MS, DS)                                                \
        movox_ld(W(XD), W(MS), W(DS))                                       \
        mlhos_rr(W(XD), W(XD))

#define mlpos_rx(XD) /* not portable, do not use outside */                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x24))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x28))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x2C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x34))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x38))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x3C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x1C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x44))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x48))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x4C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x24))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x50))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x54))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x28))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x58))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x5C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x2C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x64))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x68))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x6C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x34))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x70))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x74))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x38))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x78))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x7C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x3C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x44))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x48))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x4C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x24))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x50))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x28))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x2C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x54))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x34))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x58))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x38))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x3C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x5C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x44))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x48))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x4C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x64))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x50))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x54))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x68))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x58))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x5C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x6C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x64))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x70))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x68))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x6C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x74))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x70))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x74))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x78))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x78))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x7C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x7C))

#define mnpos_rr(XG, XS) /* horizontal pairwise min */                      \
        mnpos3rr(W(XG), W(XG), W(XS))

#define mnpos_ld(XG, MS, DS)                                                \
        mnpos3ld(W(XG), W(XG), W(MS), W(DS))

#define mnpos3rr(XD, XS, XT)                                                \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mnpos_rx(W(XD))                                                     \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define mnpos3ld(XD, XS, MT, DT)                                            \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XD), W(MT), W(DT))                                       \
        movox_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mnpos_rx(W(XD))                                                     \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define mnhos_rr(XD, XS) /* horizontal reductive min */                     \
        mnpos3rr(W(XD), W(XS), W(XS))                                       \
        mnpos3rr(W(XD), W(XD), W(XD))                                       \
        mnpos3rr(W(XD), W(XD), W(XD))                                       \
        mnpos3rr(W(XD), W(XD), W(XD))                                       \
        mnpos3rr(W(XD), W(XD), W(XD))

#define mnhos_ld(XD, MS, DS)                                                \
        movox_ld(W(XD), W(MS), W(DS))                                       \
        mnhos_rr(W(XD), W(XD))

#define mnpos_rx(XD) /* not portable, do not use outside */                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x24))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x28))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x2C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x34))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x38))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x3C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x1C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x44))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x48))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x4C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x24))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x50))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x54))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x28))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x58))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x5C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x2C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x64))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x68))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x6C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x34))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x70))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x74))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x38))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x78))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x7C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x3C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x44))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x48))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x4C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x24))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x50))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x28))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x2C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x54))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x34))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x58))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x38))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x3C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x5C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x44))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x48))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x4C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x64))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x50))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x54))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x68))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x58))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x5C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x6C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x64))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x70))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x68))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x6C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x74))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x70))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x74))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x78))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x78))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x7C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x7C))

#define mxpos_rr(XG, XS) /* horizontal pairwise max */                      \
        mxpos3rr(W(XG), W(XG), W(XS))

#define mxpos_ld(XG, MS, DS)                                                \
        mxpos3ld(W(XG), W(XG), W(MS), W(DS))

#define mxpos3rr(XD, XS, XT)                                                \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mxpos_rx(W(XD))                                                     \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define mxpos3ld(XD, XS, MT, DT)                                            \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XD), W(MT), W(DT))                                       \
        movox_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mxpos_rx(W(XD))                                                     \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define mxhos_rr(XD, XS) /* horizontal reductive max */                     \
        mxpos3rr(W(XD), W(XS), W(XS))                                       \
        mxpos3rr(W(XD), W(XD), W(XD))                                       \
        mxpos3rr(W(XD), W(XD), W(XD))                                       \
        mxpos3rr(W(XD), W(XD), W(XD))                                       \
        mxpos3rr(W(XD), W(XD), W(XD))

#define mxhos_ld(XD, MS, DS)                                                \
        movox_ld(W(XD), W(MS), W(DS))                                       \
        mxhos_rr(W(XD), W(XD))

#define mxpos_rx(XD) /* not portable, do not use outside */                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x24))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x28))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x2C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x34))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x38))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x3C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x1C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x44))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x48))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x4C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x24))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x50))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x54))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x28))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x58))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x5C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x2C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x64))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x68))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x6C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x34))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x70))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x74))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x38))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x78))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x7C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x3C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x44))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x48))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x4C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x24))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x50))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x28))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x2C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x54))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x34))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x58))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x38))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x3C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x5C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x44))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x48))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x4C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x64))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x50))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x54))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x68))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x58))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x5C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x6C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x64))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x70))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x68))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x6C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x74))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x70))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x74))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x78))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x78))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x7C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x7C))

#elif (RT_SIMD == 512)

#define adpos_rr(XG, XS) /* horizontal pairwise add, first 15-regs only */  \
        adpos3rr(W(XG), W(XG), W(XS))

#define adpos_ld(XG, MS, DS)                                                \
        adpos3ld(W(XG), W(XG), W(MS), W(DS))

#define adpos3rr(XD, XS, XT)                                                \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XT), Mebp, inf_SCR02(0))                                 \
        adpos_rx(W(XD))                                                     \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define adpos3ld(XD, XS, MT, DT)                                            \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XD), W(MT), W(DT))                                       \
        movox_st(W(XD), Mebp, inf_SCR02(0))                                 \
        adpos_rx(W(XD))                                                     \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define adhos_rr(XD, XS) /* horizontal reductive add, first 15-regs only */ \
        adpos3rr(W(XD), W(XS), W(XS))                                       \
        adpos3rr(W(XD), W(XD), W(XD))                                       \
        adpos3rr(W(XD), W(XD), W(XD))                                       \
        adpos3rr(W(XD), W(XD), W(XD))

#define adhos_ld(XD, MS, DS)                                                \
        movox_ld(W(XD), W(MS), W(DS))                                       \
        adhos_rr(W(XD), W(XD))

#if (defined RT_A32) || (defined RT_A64) || (defined RT_X32) || (defined RT_X64)

#define adpos_rx(XD) /* not portable, do not use outside */                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adpis_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        adpis_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movix_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        adpis_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movix_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        adpis_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x30))

#else /* RT_M32, RT_M64, RT_P32, RT_P64 */

#define adpos_rx(XD) /* not portable, do not use outside */                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x24))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x28))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x2C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x34))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x38))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x3C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x1C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x24))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x28))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x2C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x24))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x28))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x2C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x34))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x34))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x38))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x38))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x3C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x3C))

#endif /* target specific (ARM/x86 or MIPS/POWER) */

#define mlpos_rr(XG, XS) /* horizontal pairwise mul */                      \
        mlpos3rr(W(XG), W(XG), W(XS))

#define mlpos_ld(XG, MS, DS)                                                \
        mlpos3ld(W(XG), W(XG), W(MS), W(DS))

#define mlpos3rr(XD, XS, XT)                                                \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mlpos_rx(W(XD))                                                     \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define mlpos3ld(XD, XS, MT, DT)                                            \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XD), W(MT), W(DT))                                       \
        movox_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mlpos_rx(W(XD))                                                     \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define mlhos_rr(XD, XS) /* horizontal reductive mul */                     \
        mlpos3rr(W(XD), W(XS), W(XS))                                       \
        mlpos3rr(W(XD), W(XD), W(XD))                                       \
        mlpos3rr(W(XD), W(XD), W(XD))                                       \
        mlpos3rr(W(XD), W(XD), W(XD))

#define mlhos_ld(XD, MS, DS)                                                \
        movox_ld(W(XD), W(MS), W(DS))                                       \
        mlhos_rr(W(XD), W(XD))

#define mlpos_rx(XD) /* not portable, do not use outside */                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x24))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x28))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x2C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x34))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x38))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x3C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x1C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x24))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x28))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x2C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x24))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x28))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x2C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x34))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x34))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x38))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x38))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x3C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x3C))

#define mnpos_rr(XG, XS) /* horizontal pairwise min */                      \
        mnpos3rr(W(XG), W(XG), W(XS))

#define mnpos_ld(XG, MS, DS)                                                \
        mnpos3ld(W(XG), W(XG), W(MS), W(DS))

#define mnpos3rr(XD, XS, XT)                                                \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mnpos_rx(W(XD))                                                     \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define mnpos3ld(XD, XS, MT, DT)                                            \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XD), W(MT), W(DT))                                       \
        movox_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mnpos_rx(W(XD))                                                     \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define mnhos_rr(XD, XS) /* horizontal reductive min */                     \
        mnpos3rr(W(XD), W(XS), W(XS))                                       \
        mnpos3rr(W(XD), W(XD), W(XD))                                       \
        mnpos3rr(W(XD), W(XD), W(XD))                                       \
        mnpos3rr(W(XD), W(XD), W(XD))

#define mnhos_ld(XD, MS, DS)                                                \
        movox_ld(W(XD), W(MS), W(DS))                                       \
        mnhos_rr(W(XD), W(XD))

#define mnpos_rx(XD) /* not portable, do not use outside */                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x24))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x28))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x2C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x34))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x38))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x3C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x1C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x24))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x28))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x2C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x24))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x28))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x2C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x34))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x34))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x38))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x38))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x3C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x3C))

#define mxpos_rr(XG, XS) /* horizontal pairwise max */                      \
        mxpos3rr(W(XG), W(XG), W(XS))

#define mxpos_ld(XG, MS, DS)                                                \
        mxpos3ld(W(XG), W(XG), W(MS), W(DS))

#define mxpos3rr(XD, XS, XT)                                                \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mxpos_rx(W(XD))                                                     \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define mxpos3ld(XD, XS, MT, DT)                                            \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XD), W(MT), W(DT))                                       \
        movox_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mxpos_rx(W(XD))                                                     \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define mxhos_rr(XD, XS) /* horizontal reductive max */                     \
        mxpos3rr(W(XD), W(XS), W(XS))                                       \
        mxpos3rr(W(XD), W(XD), W(XD))                                       \
        mxpos3rr(W(XD), W(XD), W(XD))                                       \
        mxpos3rr(W(XD), W(XD), W(XD))

#define mxhos_ld(XD, MS, DS)                                                \
        movox_ld(W(XD), W(MS), W(DS))                                       \
        mxhos_rr(W(XD), W(XD))

#define mxpos_rx(XD) /* not portable, do not use outside */                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x24))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x28))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x2C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x34))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x38))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x3C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x1C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x24))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x28))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x2C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x24))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x28))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x2C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x34))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x34))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x38))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x38))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x3C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x3C))

#endif /* RT_SIMD: 2K8, 1K4, 512 */

#if   (RT_SIMD >= 512) || (RT_SIMD == 256 && defined RT_SVEX1)

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

#endif /* RT_SIMD: 2K8, 1K4, 512 */

/******************************************************************************/
/**** 256-bit **** SIMD instructions with fixed-32-bit-element ****************/
/******************************************************************************/

#define adpcs_rr(XG, XS) /* horizontal pairwise add, first 15-regs only */  \
        adpcs3rr(W(XG), W(XG), W(XS))

#define adpcs_ld(XG, MS, DS)                                                \
        adpcs3ld(W(XG), W(XG), W(MS), W(DS))

#define adpcs3rr(XD, XS, XT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        adpcs_rx(W(XD))                                                     \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

#define adpcs3ld(XD, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_ld(W(XD), W(MT), W(DT))                                       \
        movcx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        adpcs_rx(W(XD))                                                     \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

#define adhcs_rr(XD, XS) /* horizontal reductive add, first 15-regs only */ \
        adpcs3rr(W(XD), W(XS), W(XS))                                       \
        adpcs3rr(W(XD), W(XD), W(XD))                                       \
        adpcs3rr(W(XD), W(XD), W(XD))

#define adhcs_ld(XD, MS, DS)                                                \
        movcx_ld(W(XD), W(MS), W(DS))                                       \
        adhcs_rr(W(XD), W(XD))

#if (defined RT_A32) || (defined RT_A64) || (defined RT_X32) || (defined RT_X64)

#define adpcs_rx(XD) /* not portable, do not use outside */                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adpis_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        adpis_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x10))

#else /* RT_M32, RT_M64, RT_P32, RT_P64 */

#define adpcs_rx(XD) /* not portable, do not use outside */                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x1C))

#endif /* target specific (ARM/x86 or MIPS/POWER) */

#define mlpcs_rr(XG, XS) /* horizontal pairwise mul */                      \
        mlpcs3rr(W(XG), W(XG), W(XS))

#define mlpcs_ld(XG, MS, DS)                                                \
        mlpcs3ld(W(XG), W(XG), W(MS), W(DS))

#define mlpcs3rr(XD, XS, XT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mlpcs_rx(W(XD))                                                     \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

#define mlpcs3ld(XD, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_ld(W(XD), W(MT), W(DT))                                       \
        movcx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mlpcs_rx(W(XD))                                                     \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

#define mlhcs_rr(XD, XS) /* horizontal reductive mul */                     \
        mlpcs3rr(W(XD), W(XS), W(XS))                                       \
        mlpcs3rr(W(XD), W(XD), W(XD))                                       \
        mlpcs3rr(W(XD), W(XD), W(XD))

#define mlhcs_ld(XD, MS, DS)                                                \
        movcx_ld(W(XD), W(MS), W(DS))                                       \
        mlhcs_rr(W(XD), W(XD))

#define mlpcs_rx(XD) /* not portable, do not use outside */                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x1C))

#define mnpcs_rr(XG, XS) /* horizontal pairwise min */                      \
        mnpcs3rr(W(XG), W(XG), W(XS))

#define mnpcs_ld(XG, MS, DS)                                                \
        mnpcs3ld(W(XG), W(XG), W(MS), W(DS))

#define mnpcs3rr(XD, XS, XT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mnpcs_rx(W(XD))                                                     \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

#define mnpcs3ld(XD, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_ld(W(XD), W(MT), W(DT))                                       \
        movcx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mnpcs_rx(W(XD))                                                     \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

#define mnhcs_rr(XD, XS) /* horizontal reductive min */                     \
        mnpcs3rr(W(XD), W(XS), W(XS))                                       \
        mnpcs3rr(W(XD), W(XD), W(XD))                                       \
        mnpcs3rr(W(XD), W(XD), W(XD))

#define mnhcs_ld(XD, MS, DS)                                                \
        movcx_ld(W(XD), W(MS), W(DS))                                       \
        mnhcs_rr(W(XD), W(XD))

#define mnpcs_rx(XD) /* not portable, do not use outside */                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x1C))

#define mxpcs_rr(XG, XS) /* horizontal pairwise max */                      \
        mxpcs3rr(W(XG), W(XG), W(XS))

#define mxpcs_ld(XG, MS, DS)                                                \
        mxpcs3ld(W(XG), W(XG), W(MS), W(DS))

#define mxpcs3rr(XD, XS, XT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mxpcs_rx(W(XD))                                                     \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

#define mxpcs3ld(XD, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_ld(W(XD), W(MT), W(DT))                                       \
        movcx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mxpcs_rx(W(XD))                                                     \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

#define mxhcs_rr(XD, XS) /* horizontal reductive max */                     \
        mxpcs3rr(W(XD), W(XS), W(XS))                                       \
        mxpcs3rr(W(XD), W(XD), W(XD))                                       \
        mxpcs3rr(W(XD), W(XD), W(XD))

#define mxhcs_ld(XD, MS, DS)                                                \
        movcx_ld(W(XD), W(MS), W(DS))                                       \
        mxhcs_rr(W(XD), W(XD))

#define mxpcs_rx(XD) /* not portable, do not use outside */                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x14))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x14))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x1C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x1C))

/* cbr (D = cbrt S) */

/*
 * Based on the original idea by Russell Borogove (kaleja[AT]estarcion[DOT]com)
 * available at http://www.musicdsp.org/showone.php?id=206
 * converted to S-way SIMD version by VectorChief.
 */
#define cbrcs_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbecs_rr(W(XD), W(X1), W(X2), W(XS))                                \
        cbscs_rr(W(XD), W(X1), W(X2), W(XS))                                \
        cbscs_rr(W(XD), W(X1), W(X2), W(XS))                                \
        cbscs_rr(W(XD), W(X1), W(X2), W(XS))

#define cbecs_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        /* cube root estimate, the exponent is divided by three             \
         * in such a way that remainder bits get shoved into                \
         * the top of the normalized mantissa */                            \
        movcx_ld(W(X2), Mebp, inf_GPC04_32)                                 \
        movcx_rr(W(XD), W(XS))                                              \
        andcx_rr(W(XD), W(X2))   /* exponent & mantissa in biased-127 */    \
        subcx_ld(W(XD), Mebp, inf_GPC05_32) /* convert to 2's complement */ \
        shrcn_ri(W(XD), IB(10))  /* XD / 1024 */                            \
        movcx_rr(W(X1), W(XD))   /* XD * 341 (next 8 ops) */                \
        shlcx_ri(W(X1), IB(2))                                              \
        addcx_rr(W(XD), W(X1))                                              \
        shlcx_ri(W(X1), IB(2))                                              \
        addcx_rr(W(XD), W(X1))                                              \
        shlcx_ri(W(X1), IB(2))                                              \
        addcx_rr(W(XD), W(X1))                                              \
        shlcx_ri(W(X1), IB(2))                                              \
        addcx_rr(W(XD), W(X1))   /* XD * (341/1024) ~= XD * (0.333) */      \
        addcx_ld(W(XD), Mebp, inf_GPC05_32) /* back to biased-127 */        \
        andcx_rr(W(XD), W(X2))   /* remask exponent & mantissa */           \
        anncx_rr(W(X2), W(XS))   /* original sign */                        \
        orrcx_rr(W(XD), W(X2))   /* new exponent & mantissa, old sign */

#define cbscs_rr(XG, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        movcx_rr(W(X1), W(XG))                                              \
        mulcs_rr(W(X1), W(XG))                                              \
        movcx_rr(W(X2), W(X1))                                              \
        mulcs_ld(W(X1), Mebp, inf_GPC03_32)                                 \
        rcecs_rr(W(X1), W(X1))                                              \
        mulcs_rr(W(X2), W(XG))                                              \
        subcs_rr(W(X2), W(XS))                                              \
        mulcs_rr(W(X2), W(X1))                                              \
        subcs_rr(W(XG), W(X2))

/******************************************************************************/
/**** 128-bit **** SIMD instructions with fixed-32-bit-element ****************/
/******************************************************************************/

#define adpis_rr(XG, XS) /* horizontal pairwise add, first 15-regs only */  \
        adpis3rr(W(XG), W(XG), W(XS))

#define adpis_ld(XG, MS, DS)                                                \
        adpis3ld(W(XG), W(XG), W(MS), W(DS))

#define adpis3rr(XD, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_st(W(XT), Mebp, inf_SCR02(0))                                 \
        adpis_rx(W(XD))                                                     \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#define adpis3ld(XD, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR02(0))                                 \
        adpis_rx(W(XD))                                                     \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#define adhis_rr(XD, XS) /* horizontal reductive add, first 15-regs only */ \
        adpis3rr(W(XD), W(XS), W(XS))                                       \
        adpis3rr(W(XD), W(XD), W(XD))

#define adhis_ld(XD, MS, DS)                                                \
        movix_ld(W(XD), W(MS), W(DS))                                       \
        adhis_rr(W(XD), W(XD))

#define adpis_rx(XD) /* not portable, do not use outside */                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        addrs_ld(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        addrs_ld(W(XD), Mebp, inf_SCR02(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x0C))

#define mlpis_rr(XG, XS) /* horizontal pairwise mul */                      \
        mlpis3rr(W(XG), W(XG), W(XS))

#define mlpis_ld(XG, MS, DS)                                                \
        mlpis3ld(W(XG), W(XG), W(MS), W(DS))

#define mlpis3rr(XD, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mlpis_rx(W(XD))                                                     \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#define mlpis3ld(XD, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mlpis_rx(W(XD))                                                     \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#define mlhis_rr(XD, XS) /* horizontal reductive mul */                     \
        mlpis3rr(W(XD), W(XS), W(XS))                                       \
        mlpis3rr(W(XD), W(XD), W(XD))

#define mlhis_ld(XD, MS, DS)                                                \
        movix_ld(W(XD), W(MS), W(DS))                                       \
        mlhis_rr(W(XD), W(XD))

#define mlpis_rx(XD) /* not portable, do not use outside */                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        mulrs_ld(W(XD), Mebp, inf_SCR02(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x0C))

#define mnpis_rr(XG, XS) /* horizontal pairwise min */                      \
        mnpis3rr(W(XG), W(XG), W(XS))

#define mnpis_ld(XG, MS, DS)                                                \
        mnpis3ld(W(XG), W(XG), W(MS), W(DS))

#define mnpis3rr(XD, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mnpis_rx(W(XD))                                                     \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#define mnpis3ld(XD, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mnpis_rx(W(XD))                                                     \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#define mnhis_rr(XD, XS) /* horizontal reductive min */                     \
        mnpis3rr(W(XD), W(XS), W(XS))                                       \
        mnpis3rr(W(XD), W(XD), W(XD))

#define mnhis_ld(XD, MS, DS)                                                \
        movix_ld(W(XD), W(MS), W(DS))                                       \
        mnhis_rr(W(XD), W(XD))

#define mnpis_rx(XD) /* not portable, do not use outside */                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        minrs_ld(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        minrs_ld(W(XD), Mebp, inf_SCR02(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x0C))

#define mxpis_rr(XG, XS) /* horizontal pairwise max */                      \
        mxpis3rr(W(XG), W(XG), W(XS))

#define mxpis_ld(XG, MS, DS)                                                \
        mxpis3ld(W(XG), W(XG), W(MS), W(DS))

#define mxpis3rr(XD, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mxpis_rx(W(XD))                                                     \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#define mxpis3ld(XD, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mxpis_rx(W(XD))                                                     \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#define mxhis_rr(XD, XS) /* horizontal reductive max */                     \
        mxpis3rr(W(XD), W(XS), W(XS))                                       \
        mxpis3rr(W(XD), W(XD), W(XD))

#define mxhis_ld(XD, MS, DS)                                                \
        movix_ld(W(XD), W(MS), W(DS))                                       \
        mxhis_rr(W(XD), W(XD))

#define mxpis_rx(XD) /* not portable, do not use outside */                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x04))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movrs_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        maxrs_ld(W(XD), Mebp, inf_SCR02(0x0C))                              \
        movrs_st(W(XD), Mebp, inf_SCR01(0x0C))

/* cbr (D = cbrt S) */

/*
 * Based on the original idea by Russell Borogove (kaleja[AT]estarcion[DOT]com)
 * available at http://www.musicdsp.org/showone.php?id=206
 * converted to S-way SIMD version by VectorChief.
 */
#define cbris_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbeis_rr(W(XD), W(X1), W(X2), W(XS))                                \
        cbsis_rr(W(XD), W(X1), W(X2), W(XS))                                \
        cbsis_rr(W(XD), W(X1), W(X2), W(XS))                                \
        cbsis_rr(W(XD), W(X1), W(X2), W(XS))

#define cbeis_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        /* cube root estimate, the exponent is divided by three             \
         * in such a way that remainder bits get shoved into                \
         * the top of the normalized mantissa */                            \
        movix_ld(W(X2), Mebp, inf_GPC04_32)                                 \
        movix_rr(W(XD), W(XS))                                              \
        andix_rr(W(XD), W(X2))   /* exponent & mantissa in biased-127 */    \
        subix_ld(W(XD), Mebp, inf_GPC05_32) /* convert to 2's complement */ \
        shrin_ri(W(XD), IB(10))  /* XD / 1024 */                            \
        movix_rr(W(X1), W(XD))   /* XD * 341 (next 8 ops) */                \
        shlix_ri(W(X1), IB(2))                                              \
        addix_rr(W(XD), W(X1))                                              \
        shlix_ri(W(X1), IB(2))                                              \
        addix_rr(W(XD), W(X1))                                              \
        shlix_ri(W(X1), IB(2))                                              \
        addix_rr(W(XD), W(X1))                                              \
        shlix_ri(W(X1), IB(2))                                              \
        addix_rr(W(XD), W(X1))   /* XD * (341/1024) ~= XD * (0.333) */      \
        addix_ld(W(XD), Mebp, inf_GPC05_32) /* back to biased-127 */        \
        andix_rr(W(XD), W(X2))   /* remask exponent & mantissa */           \
        annix_rr(W(X2), W(XS))   /* original sign */                        \
        orrix_rr(W(XD), W(X2))   /* new exponent & mantissa, old sign */

#define cbsis_rr(XG, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        movix_rr(W(X1), W(XG))                                              \
        mulis_rr(W(X1), W(XG))                                              \
        movix_rr(W(X2), W(X1))                                              \
        mulis_ld(W(X1), Mebp, inf_GPC03_32)                                 \
        rceis_rr(W(X1), W(X1))                                              \
        mulis_rr(W(X2), W(XG))                                              \
        subis_rr(W(X2), W(XS))                                              \
        mulis_rr(W(X2), W(X1))                                              \
        subis_rr(W(XG), W(X2))

/******************************************************************************/
/**** var-len **** SIMD instructions with fixed-64-bit-element ****************/
/******************************************************************************/

#if   (RT_SIMD == 2048)

#define adpqs_rr(XG, XS) /* horizontal pairwise add, first 15-regs only */  \
        adpqs3rr(W(XG), W(XG), W(XS))

#define adpqs_ld(XG, MS, DS)                                                \
        adpqs3ld(W(XG), W(XG), W(MS), W(DS))

#define adpqs3rr(XD, XS, XT)                                                \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        adpqs_rx(W(XD))                                                     \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define adpqs3ld(XD, XS, MT, DT)                                            \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XD), W(MT), W(DT))                                       \
        movqx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        adpqs_rx(W(XD))                                                     \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define adhqs_rr(XD, XS) /* horizontal reductive add, first 15-regs only */ \
        adpqs3rr(W(XD), W(XS), W(XS))                                       \
        adpqs3rr(W(XD), W(XD), W(XD))                                       \
        adpqs3rr(W(XD), W(XD), W(XD))                                       \
        adpqs3rr(W(XD), W(XD), W(XD))                                       \
        adpqs3rr(W(XD), W(XD), W(XD))

#define adhqs_ld(XD, MS, DS)                                                \
        movqx_ld(W(XD), W(MS), W(DS))                                       \
        adhqs_rr(W(XD), W(XD))

#if (defined RT_A32) || (defined RT_A64) || (defined RT_X32) || (defined RT_X64)

#define adpqs_rx(XD) /* not portable, do not use outside */                 \
        movjx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movjx_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movjx_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR01(0x50))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movjx_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR01(0x70))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movjx_ld(W(XD), Mebp, inf_SCR01(0x80))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR01(0x90))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movjx_ld(W(XD), Mebp, inf_SCR01(0xA0))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR01(0xB0))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x50))                              \
        movjx_ld(W(XD), Mebp, inf_SCR01(0xC0))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR01(0xD0))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movjx_ld(W(XD), Mebp, inf_SCR01(0xE0))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR01(0xF0))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x70))                              \
        movjx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x80))                              \
        movjx_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x90))                              \
        movjx_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR02(0x50))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0xA0))                              \
        movjx_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR02(0x70))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0xB0))                              \
        movjx_ld(W(XD), Mebp, inf_SCR02(0x80))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR02(0x90))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0xC0))                              \
        movjx_ld(W(XD), Mebp, inf_SCR02(0xA0))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR02(0xB0))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0xD0))                              \
        movjx_ld(W(XD), Mebp, inf_SCR02(0xC0))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR02(0xD0))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0xE0))                              \
        movjx_ld(W(XD), Mebp, inf_SCR02(0xE0))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR02(0xF0))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0xF0))

#else /* RT_M32, RT_M64, RT_P32, RT_P64 */

#define adpqs_rx(XD) /* not portable, do not use outside */                 \
        movts_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0x48))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x50))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0x58))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0x68))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x70))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0x78))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x38))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x80))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0x88))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x90))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0x98))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x48))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0xA0))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0xA8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x50))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0xB0))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0xB8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x58))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0xC0))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0xC8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0xD0))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0xD8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x68))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0xE0))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0xE8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x70))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0xF0))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0xF8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x78))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x80))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x88))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x90))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x98))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0x48))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xA0))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x50))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0x58))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xA8))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0x68))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xB0))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x70))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0x78))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xB8))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x80))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0x88))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xC0))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x90))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0x98))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xC8))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0xA0))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0xA8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xD0))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0xB0))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0xB8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xD8))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0xC0))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0xC8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xE0))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0xD0))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0xD8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xE8))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0xE0))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0xE8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xF0))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0xF0))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0xF8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xF8))

#endif /* target specific (ARM/x86 or MIPS/POWER) */

#define mlpqs_rr(XG, XS) /* horizontal pairwise mul */                      \
        mlpqs3rr(W(XG), W(XG), W(XS))

#define mlpqs_ld(XG, MS, DS)                                                \
        mlpqs3ld(W(XG), W(XG), W(MS), W(DS))

#define mlpqs3rr(XD, XS, XT)                                                \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mlpqs_rx(W(XD))                                                     \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define mlpqs3ld(XD, XS, MT, DT)                                            \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XD), W(MT), W(DT))                                       \
        movqx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mlpqs_rx(W(XD))                                                     \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define mlhqs_rr(XD, XS) /* horizontal reductive mul */                     \
        mlpqs3rr(W(XD), W(XS), W(XS))                                       \
        mlpqs3rr(W(XD), W(XD), W(XD))                                       \
        mlpqs3rr(W(XD), W(XD), W(XD))                                       \
        mlpqs3rr(W(XD), W(XD), W(XD))                                       \
        mlpqs3rr(W(XD), W(XD), W(XD))

#define mlhqs_ld(XD, MS, DS)                                                \
        movqx_ld(W(XD), W(MS), W(DS))                                       \
        mlhqs_rr(W(XD), W(XD))

#define mlpqs_rx(XD) /* not portable, do not use outside */                 \
        movts_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0x48))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x50))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0x58))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0x68))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x70))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0x78))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x38))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x80))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0x88))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x90))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0x98))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x48))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0xA0))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0xA8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x50))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0xB0))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0xB8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x58))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0xC0))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0xC8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0xD0))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0xD8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x68))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0xE0))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0xE8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x70))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0xF0))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0xF8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x78))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x80))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x88))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x90))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x98))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0x48))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xA0))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x50))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0x58))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xA8))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0x68))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xB0))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x70))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0x78))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xB8))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x80))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0x88))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xC0))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x90))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0x98))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xC8))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0xA0))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0xA8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xD0))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0xB0))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0xB8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xD8))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0xC0))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0xC8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xE0))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0xD0))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0xD8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xE8))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0xE0))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0xE8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xF0))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0xF0))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0xF8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xF8))

#define mnpqs_rr(XG, XS) /* horizontal pairwise min */                      \
        mnpqs3rr(W(XG), W(XG), W(XS))

#define mnpqs_ld(XG, MS, DS)                                                \
        mnpqs3ld(W(XG), W(XG), W(MS), W(DS))

#define mnpqs3rr(XD, XS, XT)                                                \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mnpqs_rx(W(XD))                                                     \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define mnpqs3ld(XD, XS, MT, DT)                                            \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XD), W(MT), W(DT))                                       \
        movqx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mnpqs_rx(W(XD))                                                     \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define mnhqs_rr(XD, XS) /* horizontal reductive min */                     \
        mnpqs3rr(W(XD), W(XS), W(XS))                                       \
        mnpqs3rr(W(XD), W(XD), W(XD))                                       \
        mnpqs3rr(W(XD), W(XD), W(XD))                                       \
        mnpqs3rr(W(XD), W(XD), W(XD))                                       \
        mnpqs3rr(W(XD), W(XD), W(XD))

#define mnhqs_ld(XD, MS, DS)                                                \
        movqx_ld(W(XD), W(MS), W(DS))                                       \
        mnhqs_rr(W(XD), W(XD))

#define mnpqs_rx(XD) /* not portable, do not use outside */                 \
        movts_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0x48))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x50))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0x58))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0x68))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x70))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0x78))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x38))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x80))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0x88))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x90))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0x98))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x48))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0xA0))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0xA8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x50))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0xB0))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0xB8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x58))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0xC0))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0xC8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0xD0))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0xD8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x68))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0xE0))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0xE8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x70))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0xF0))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0xF8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x78))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x80))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x88))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x90))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x98))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0x48))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xA0))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x50))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0x58))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xA8))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0x68))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xB0))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x70))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0x78))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xB8))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x80))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0x88))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xC0))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x90))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0x98))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xC8))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0xA0))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0xA8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xD0))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0xB0))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0xB8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xD8))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0xC0))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0xC8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xE0))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0xD0))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0xD8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xE8))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0xE0))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0xE8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xF0))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0xF0))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0xF8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xF8))

#define mxpqs_rr(XG, XS) /* horizontal pairwise max */                      \
        mxpqs3rr(W(XG), W(XG), W(XS))

#define mxpqs_ld(XG, MS, DS)                                                \
        mxpqs3ld(W(XG), W(XG), W(MS), W(DS))

#define mxpqs3rr(XD, XS, XT)                                                \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mxpqs_rx(W(XD))                                                     \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define mxpqs3ld(XD, XS, MT, DT)                                            \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XD), W(MT), W(DT))                                       \
        movqx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mxpqs_rx(W(XD))                                                     \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define mxhqs_rr(XD, XS) /* horizontal reductive max */                     \
        mxpqs3rr(W(XD), W(XS), W(XS))                                       \
        mxpqs3rr(W(XD), W(XD), W(XD))                                       \
        mxpqs3rr(W(XD), W(XD), W(XD))                                       \
        mxpqs3rr(W(XD), W(XD), W(XD))                                       \
        mxpqs3rr(W(XD), W(XD), W(XD))

#define mxhqs_ld(XD, MS, DS)                                                \
        movqx_ld(W(XD), W(MS), W(DS))                                       \
        mxhqs_rr(W(XD), W(XD))

#define mxpqs_rx(XD) /* not portable, do not use outside */                 \
        movts_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0x48))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x50))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0x58))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0x68))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x70))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0x78))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x38))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x80))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0x88))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x90))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0x98))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x48))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0xA0))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0xA8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x50))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0xB0))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0xB8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x58))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0xC0))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0xC8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0xD0))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0xD8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x68))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0xE0))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0xE8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x70))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0xF0))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0xF8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x78))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x80))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x88))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x90))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x98))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0x48))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xA0))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x50))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0x58))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xA8))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0x68))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xB0))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x70))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0x78))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xB8))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x80))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0x88))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xC0))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x90))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0x98))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xC8))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0xA0))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0xA8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xD0))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0xB0))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0xB8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xD8))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0xC0))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0xC8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xE0))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0xD0))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0xD8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xE8))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0xE0))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0xE8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xF0))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0xF0))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0xF8))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0xF8))

#elif (RT_SIMD == 1024)

#define adpqs_rr(XG, XS) /* horizontal pairwise add, first 15-regs only */  \
        adpqs3rr(W(XG), W(XG), W(XS))

#define adpqs_ld(XG, MS, DS)                                                \
        adpqs3ld(W(XG), W(XG), W(MS), W(DS))

#define adpqs3rr(XD, XS, XT)                                                \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        adpqs_rx(W(XD))                                                     \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define adpqs3ld(XD, XS, MT, DT)                                            \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XD), W(MT), W(DT))                                       \
        movqx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        adpqs_rx(W(XD))                                                     \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define adhqs_rr(XD, XS) /* horizontal reductive add, first 15-regs only */ \
        adpqs3rr(W(XD), W(XS), W(XS))                                       \
        adpqs3rr(W(XD), W(XD), W(XD))                                       \
        adpqs3rr(W(XD), W(XD), W(XD))                                       \
        adpqs3rr(W(XD), W(XD), W(XD))

#define adhqs_ld(XD, MS, DS)                                                \
        movqx_ld(W(XD), W(MS), W(DS))                                       \
        adhqs_rr(W(XD), W(XD))

#if (defined RT_A32) || (defined RT_A64) || (defined RT_X32) || (defined RT_X64)

#define adpqs_rx(XD) /* not portable, do not use outside */                 \
        movjx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movjx_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movjx_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR01(0x50))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movjx_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR01(0x70))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movjx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movjx_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x50))                              \
        movjx_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR02(0x50))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movjx_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR02(0x70))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x70))

#else /* RT_M32, RT_M64, RT_P32, RT_P64 */

#define adpqs_rx(XD) /* not portable, do not use outside */                 \
        movts_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0x48))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x50))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0x58))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0x68))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x70))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0x78))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x38))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x48))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x50))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x58))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0x48))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x50))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0x58))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x68))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0x68))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x70))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x70))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0x78))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x78))

#endif /* target specific (ARM/x86 or MIPS/POWER) */

#define mlpqs_rr(XG, XS) /* horizontal pairwise mul */                      \
        mlpqs3rr(W(XG), W(XG), W(XS))

#define mlpqs_ld(XG, MS, DS)                                                \
        mlpqs3ld(W(XG), W(XG), W(MS), W(DS))

#define mlpqs3rr(XD, XS, XT)                                                \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mlpqs_rx(W(XD))                                                     \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define mlpqs3ld(XD, XS, MT, DT)                                            \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XD), W(MT), W(DT))                                       \
        movqx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mlpqs_rx(W(XD))                                                     \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define mlhqs_rr(XD, XS) /* horizontal reductive mul */                     \
        mlpqs3rr(W(XD), W(XS), W(XS))                                       \
        mlpqs3rr(W(XD), W(XD), W(XD))                                       \
        mlpqs3rr(W(XD), W(XD), W(XD))                                       \
        mlpqs3rr(W(XD), W(XD), W(XD))

#define mlhqs_ld(XD, MS, DS)                                                \
        movqx_ld(W(XD), W(MS), W(DS))                                       \
        mlhqs_rr(W(XD), W(XD))

#define mlpqs_rx(XD) /* not portable, do not use outside */                 \
        movts_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0x48))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x50))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0x58))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0x68))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x70))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0x78))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x38))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x48))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x50))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x58))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0x48))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x50))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0x58))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x68))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0x68))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x70))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x70))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0x78))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x78))

#define mnpqs_rr(XG, XS) /* horizontal pairwise min */                      \
        mnpqs3rr(W(XG), W(XG), W(XS))

#define mnpqs_ld(XG, MS, DS)                                                \
        mnpqs3ld(W(XG), W(XG), W(MS), W(DS))

#define mnpqs3rr(XD, XS, XT)                                                \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mnpqs_rx(W(XD))                                                     \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define mnpqs3ld(XD, XS, MT, DT)                                            \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XD), W(MT), W(DT))                                       \
        movqx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mnpqs_rx(W(XD))                                                     \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define mnhqs_rr(XD, XS) /* horizontal reductive min */                     \
        mnpqs3rr(W(XD), W(XS), W(XS))                                       \
        mnpqs3rr(W(XD), W(XD), W(XD))                                       \
        mnpqs3rr(W(XD), W(XD), W(XD))                                       \
        mnpqs3rr(W(XD), W(XD), W(XD))

#define mnhqs_ld(XD, MS, DS)                                                \
        movqx_ld(W(XD), W(MS), W(DS))                                       \
        mnhqs_rr(W(XD), W(XD))

#define mnpqs_rx(XD) /* not portable, do not use outside */                 \
        movts_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0x48))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x50))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0x58))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0x68))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x70))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0x78))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x38))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x48))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x50))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x58))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0x48))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x50))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0x58))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x68))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0x68))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x70))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x70))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0x78))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x78))

#define mxpqs_rr(XG, XS) /* horizontal pairwise max */                      \
        mxpqs3rr(W(XG), W(XG), W(XS))

#define mxpqs_ld(XG, MS, DS)                                                \
        mxpqs3ld(W(XG), W(XG), W(MS), W(DS))

#define mxpqs3rr(XD, XS, XT)                                                \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mxpqs_rx(W(XD))                                                     \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define mxpqs3ld(XD, XS, MT, DT)                                            \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XD), W(MT), W(DT))                                       \
        movqx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mxpqs_rx(W(XD))                                                     \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define mxhqs_rr(XD, XS) /* horizontal reductive max */                     \
        mxpqs3rr(W(XD), W(XS), W(XS))                                       \
        mxpqs3rr(W(XD), W(XD), W(XD))                                       \
        mxpqs3rr(W(XD), W(XD), W(XD))                                       \
        mxpqs3rr(W(XD), W(XD), W(XD))

#define mxhqs_ld(XD, MS, DS)                                                \
        movqx_ld(W(XD), W(MS), W(DS))                                       \
        mxhqs_rr(W(XD), W(XD))

#define mxpqs_rx(XD) /* not portable, do not use outside */                 \
        movts_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0x48))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x50))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0x58))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0x68))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x70))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0x78))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x38))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x48))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x50))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x58))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0x48))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x50))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0x58))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x68))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0x68))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x70))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x70))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0x78))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x78))

#elif (RT_SIMD == 512)

#define adpqs_rr(XG, XS) /* horizontal pairwise add, first 15-regs only */  \
        adpqs3rr(W(XG), W(XG), W(XS))

#define adpqs_ld(XG, MS, DS)                                                \
        adpqs3ld(W(XG), W(XG), W(MS), W(DS))

#define adpqs3rr(XD, XS, XT)                                                \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        adpqs_rx(W(XD))                                                     \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define adpqs3ld(XD, XS, MT, DT)                                            \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XD), W(MT), W(DT))                                       \
        movqx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        adpqs_rx(W(XD))                                                     \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define adhqs_rr(XD, XS) /* horizontal reductive add, first 15-regs only */ \
        adpqs3rr(W(XD), W(XS), W(XS))                                       \
        adpqs3rr(W(XD), W(XD), W(XD))                                       \
        adpqs3rr(W(XD), W(XD), W(XD))

#define adhqs_ld(XD, MS, DS)                                                \
        movqx_ld(W(XD), W(MS), W(DS))                                       \
        adhqs_rr(W(XD), W(XD))

#if (defined RT_A32) || (defined RT_A64) || (defined RT_X32) || (defined RT_X64)

#define adpqs_rx(XD) /* not portable, do not use outside */                 \
        movjx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movjx_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movjx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movjx_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x30))

#else /* RT_M32, RT_M64, RT_P32, RT_P64 */

#define adpqs_rx(XD) /* not portable, do not use outside */                 \
        movts_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x38))

#endif /* target specific (ARM/x86 or MIPS/POWER) */

#define mlpqs_rr(XG, XS) /* horizontal pairwise mul */                      \
        mlpqs3rr(W(XG), W(XG), W(XS))

#define mlpqs_ld(XG, MS, DS)                                                \
        mlpqs3ld(W(XG), W(XG), W(MS), W(DS))

#define mlpqs3rr(XD, XS, XT)                                                \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mlpqs_rx(W(XD))                                                     \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define mlpqs3ld(XD, XS, MT, DT)                                            \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XD), W(MT), W(DT))                                       \
        movqx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mlpqs_rx(W(XD))                                                     \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define mlhqs_rr(XD, XS) /* horizontal reductive mul */                     \
        mlpqs3rr(W(XD), W(XS), W(XS))                                       \
        mlpqs3rr(W(XD), W(XD), W(XD))                                       \
        mlpqs3rr(W(XD), W(XD), W(XD))

#define mlhqs_ld(XD, MS, DS)                                                \
        movqx_ld(W(XD), W(MS), W(DS))                                       \
        mlhqs_rr(W(XD), W(XD))

#define mlpqs_rx(XD) /* not portable, do not use outside */                 \
        movts_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x38))

#define mnpqs_rr(XG, XS) /* horizontal pairwise min */                      \
        mnpqs3rr(W(XG), W(XG), W(XS))

#define mnpqs_ld(XG, MS, DS)                                                \
        mnpqs3ld(W(XG), W(XG), W(MS), W(DS))

#define mnpqs3rr(XD, XS, XT)                                                \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mnpqs_rx(W(XD))                                                     \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define mnpqs3ld(XD, XS, MT, DT)                                            \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XD), W(MT), W(DT))                                       \
        movqx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mnpqs_rx(W(XD))                                                     \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define mnhqs_rr(XD, XS) /* horizontal reductive min */                     \
        mnpqs3rr(W(XD), W(XS), W(XS))                                       \
        mnpqs3rr(W(XD), W(XD), W(XD))                                       \
        mnpqs3rr(W(XD), W(XD), W(XD))

#define mnhqs_ld(XD, MS, DS)                                                \
        movqx_ld(W(XD), W(MS), W(DS))                                       \
        mnhqs_rr(W(XD), W(XD))

#define mnpqs_rx(XD) /* not portable, do not use outside */                 \
        movts_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x38))

#define mxpqs_rr(XG, XS) /* horizontal pairwise max */                      \
        mxpqs3rr(W(XG), W(XG), W(XS))

#define mxpqs_ld(XG, MS, DS)                                                \
        mxpqs3ld(W(XG), W(XG), W(MS), W(DS))

#define mxpqs3rr(XD, XS, XT)                                                \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mxpqs_rx(W(XD))                                                     \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define mxpqs3ld(XD, XS, MT, DT)                                            \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XD), W(MT), W(DT))                                       \
        movqx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mxpqs_rx(W(XD))                                                     \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define mxhqs_rr(XD, XS) /* horizontal reductive max */                     \
        mxpqs3rr(W(XD), W(XS), W(XS))                                       \
        mxpqs3rr(W(XD), W(XD), W(XD))                                       \
        mxpqs3rr(W(XD), W(XD), W(XD))

#define mxhqs_ld(XD, MS, DS)                                                \
        movqx_ld(W(XD), W(MS), W(DS))                                       \
        mxhqs_rr(W(XD), W(XD))

#define mxpqs_rx(XD) /* not portable, do not use outside */                 \
        movts_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x38))

#endif /* RT_SIMD: 2K8, 1K4, 512 */

#if   (RT_SIMD >= 512) || (RT_SIMD == 256 && defined RT_SVEX1)

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

#endif /* RT_SIMD: 2K8, 1K4, 512 */

/******************************************************************************/
/**** 256-bit **** SIMD instructions with fixed-64-bit-element ****************/
/******************************************************************************/

#define adpds_rr(XG, XS) /* horizontal pairwise add, first 15-regs only */  \
        adpds3rr(W(XG), W(XG), W(XS))

#define adpds_ld(XG, MS, DS)                                                \
        adpds3ld(W(XG), W(XG), W(MS), W(DS))

#define adpds3rr(XD, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        adpds_rx(W(XD))                                                     \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

#define adpds3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), W(MT), W(DT))                                       \
        movdx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        adpds_rx(W(XD))                                                     \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

#define adhds_rr(XD, XS) /* horizontal reductive add, first 15-regs only */ \
        adpds3rr(W(XD), W(XS), W(XS))                                       \
        adpds3rr(W(XD), W(XD), W(XD))

#define adhds_ld(XD, MS, DS)                                                \
        movdx_ld(W(XD), W(MS), W(DS))                                       \
        adhds_rr(W(XD), W(XD))

#if (defined RT_A32) || (defined RT_A64) || (defined RT_X32) || (defined RT_X64)

#define adpds_rx(XD) /* not portable, do not use outside */                 \
        movjx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movjx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x10))

#else /* RT_M32, RT_M64, RT_P32, RT_P64 */

#define adpds_rx(XD) /* not portable, do not use outside */                 \
        movts_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x18))

#endif /* target specific (ARM/x86 or MIPS/POWER) */

#define mlpds_rr(XG, XS) /* horizontal pairwise mul */                      \
        mlpds3rr(W(XG), W(XG), W(XS))

#define mlpds_ld(XG, MS, DS)                                                \
        mlpds3ld(W(XG), W(XG), W(MS), W(DS))

#define mlpds3rr(XD, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mlpds_rx(W(XD))                                                     \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

#define mlpds3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), W(MT), W(DT))                                       \
        movdx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mlpds_rx(W(XD))                                                     \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

#define mlhds_rr(XD, XS) /* horizontal reductive mul */                     \
        mlpds3rr(W(XD), W(XS), W(XS))                                       \
        mlpds3rr(W(XD), W(XD), W(XD))

#define mlhds_ld(XD, MS, DS)                                                \
        movdx_ld(W(XD), W(MS), W(DS))                                       \
        mlhds_rr(W(XD), W(XD))

#define mlpds_rx(XD) /* not portable, do not use outside */                 \
        movts_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x18))

#define mnpds_rr(XG, XS) /* horizontal pairwise min */                      \
        mnpds3rr(W(XG), W(XG), W(XS))

#define mnpds_ld(XG, MS, DS)                                                \
        mnpds3ld(W(XG), W(XG), W(MS), W(DS))

#define mnpds3rr(XD, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mnpds_rx(W(XD))                                                     \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

#define mnpds3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), W(MT), W(DT))                                       \
        movdx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mnpds_rx(W(XD))                                                     \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

#define mnhds_rr(XD, XS) /* horizontal reductive min */                     \
        mnpds3rr(W(XD), W(XS), W(XS))                                       \
        mnpds3rr(W(XD), W(XD), W(XD))

#define mnhds_ld(XD, MS, DS)                                                \
        movdx_ld(W(XD), W(MS), W(DS))                                       \
        mnhds_rr(W(XD), W(XD))

#define mnpds_rx(XD) /* not portable, do not use outside */                 \
        movts_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x18))

#define mxpds_rr(XG, XS) /* horizontal pairwise max */                      \
        mxpds3rr(W(XG), W(XG), W(XS))

#define mxpds_ld(XG, MS, DS)                                                \
        mxpds3ld(W(XG), W(XG), W(MS), W(DS))

#define mxpds3rr(XD, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mxpds_rx(W(XD))                                                     \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

#define mxpds3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), W(MT), W(DT))                                       \
        movdx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mxpds_rx(W(XD))                                                     \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

#define mxhds_rr(XD, XS) /* horizontal reductive max */                     \
        mxpds3rr(W(XD), W(XS), W(XS))                                       \
        mxpds3rr(W(XD), W(XD), W(XD))

#define mxhds_ld(XD, MS, DS)                                                \
        movdx_ld(W(XD), W(MS), W(DS))                                       \
        mxhds_rr(W(XD), W(XD))

#define mxpds_rx(XD) /* not portable, do not use outside */                 \
        movts_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x18))

/* cbr (D = cbrt S) */

/*
 * Based on the original idea by Russell Borogove (kaleja[AT]estarcion[DOT]com)
 * available at http://www.musicdsp.org/showone.php?id=206
 * converted to S-way SIMD version by VectorChief.
 */
#define cbrds_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbeds_rr(W(XD), W(X1), W(X2), W(XS))                                \
        cbsds_rr(W(XD), W(X1), W(X2), W(XS))                                \
        cbsds_rr(W(XD), W(X1), W(X2), W(XS))                                \
        cbsds_rr(W(XD), W(X1), W(X2), W(XS))

#define cbeds_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        /* cube root estimate, the exponent is divided by three             \
         * in such a way that remainder bits get shoved into                \
         * the top of the normalized mantissa */                            \
        movdx_ld(W(X2), Mebp, inf_GPC04_64)                                 \
        movdx_rr(W(XD), W(XS))                                              \
        anddx_rr(W(XD), W(X2))   /* exponent & mantissa in biased-127 */    \
        subdx_ld(W(XD), Mebp, inf_GPC05_64) /* convert to 2's complement */ \
        shrdn_ri(W(XD), IB(10))  /* XD / 1024 */                            \
        movdx_rr(W(X1), W(XD))   /* XD * 341 (next 8 ops) */                \
        shldx_ri(W(X1), IB(2))                                              \
        adddx_rr(W(XD), W(X1))                                              \
        shldx_ri(W(X1), IB(2))                                              \
        adddx_rr(W(XD), W(X1))                                              \
        shldx_ri(W(X1), IB(2))                                              \
        adddx_rr(W(XD), W(X1))                                              \
        shldx_ri(W(X1), IB(2))                                              \
        adddx_rr(W(XD), W(X1))   /* XD * (341/1024) ~= XD * (0.333) */      \
        adddx_ld(W(XD), Mebp, inf_GPC05_64) /* back to biased-127 */        \
        anddx_rr(W(XD), W(X2))   /* remask exponent & mantissa */           \
        anndx_rr(W(X2), W(XS))   /* original sign */                        \
        orrdx_rr(W(XD), W(X2))   /* new exponent & mantissa, old sign */

#define cbsds_rr(XG, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        movdx_rr(W(X1), W(XG))                                              \
        mulds_rr(W(X1), W(XG))                                              \
        movdx_rr(W(X2), W(X1))                                              \
        mulds_ld(W(X1), Mebp, inf_GPC03_64)                                 \
        rceds_rr(W(X1), W(X1))                                              \
        mulds_rr(W(X2), W(XG))                                              \
        subds_rr(W(X2), W(XS))                                              \
        mulds_rr(W(X2), W(X1))                                              \
        subds_rr(W(XG), W(X2))

/******************************************************************************/
/**** 128-bit **** SIMD instructions with fixed-64-bit-element ****************/
/******************************************************************************/

#define adpjs_rr(XG, XS) /* horizontal pairwise add, first 15-regs only */  \
        adpjs3rr(W(XG), W(XG), W(XS))

#define adpjs_ld(XG, MS, DS)                                                \
        adpjs3ld(W(XG), W(XG), W(MS), W(DS))

#define adpjs3rr(XD, XS, XT)                                                \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        adpjs_rx(W(XD))                                                     \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

#define adpjs3ld(XD, XS, MT, DT)                                            \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_ld(W(XD), W(MT), W(DT))                                       \
        movjx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        adpjs_rx(W(XD))                                                     \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

#define adhjs_rr(XD, XS) /* horizontal reductive add, first 15-regs only */ \
        adpjs3rr(W(XD), W(XS), W(XS))

#define adhjs_ld(XD, MS, DS)                                                \
        movjx_ld(W(XD), W(MS), W(DS))                                       \
        adhjs_rr(W(XD), W(XD))

#define adpjs_rx(XD) /* not portable, do not use outside */                 \
        movts_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x08))

#define mlpjs_rr(XG, XS) /* horizontal pairwise mul */                      \
        mlpjs3rr(W(XG), W(XG), W(XS))

#define mlpjs_ld(XG, MS, DS)                                                \
        mlpjs3ld(W(XG), W(XG), W(MS), W(DS))

#define mlpjs3rr(XD, XS, XT)                                                \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mlpjs_rx(W(XD))                                                     \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

#define mlpjs3ld(XD, XS, MT, DT)                                            \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_ld(W(XD), W(MT), W(DT))                                       \
        movjx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mlpjs_rx(W(XD))                                                     \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

#define mlhjs_rr(XD, XS) /* horizontal reductive mul */                     \
        mlpjs3rr(W(XD), W(XS), W(XS))

#define mlhjs_ld(XD, MS, DS)                                                \
        movjx_ld(W(XD), W(MS), W(DS))                                       \
        mlhjs_rr(W(XD), W(XD))

#define mlpjs_rx(XD) /* not portable, do not use outside */                 \
        movts_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x08))

#define mnpjs_rr(XG, XS) /* horizontal pairwise min */                      \
        mnpjs3rr(W(XG), W(XG), W(XS))

#define mnpjs_ld(XG, MS, DS)                                                \
        mnpjs3ld(W(XG), W(XG), W(MS), W(DS))

#define mnpjs3rr(XD, XS, XT)                                                \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mnpjs_rx(W(XD))                                                     \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

#define mnpjs3ld(XD, XS, MT, DT)                                            \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_ld(W(XD), W(MT), W(DT))                                       \
        movjx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mnpjs_rx(W(XD))                                                     \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

#define mnhjs_rr(XD, XS) /* horizontal reductive min */                     \
        mnpjs3rr(W(XD), W(XS), W(XS))

#define mnhjs_ld(XD, MS, DS)                                                \
        movjx_ld(W(XD), W(MS), W(DS))                                       \
        mnhjs_rr(W(XD), W(XD))

#define mnpjs_rx(XD) /* not portable, do not use outside */                 \
        movts_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x08))

#define mxpjs_rr(XG, XS) /* horizontal pairwise max */                      \
        mxpjs3rr(W(XG), W(XG), W(XS))

#define mxpjs_ld(XG, MS, DS)                                                \
        mxpjs3ld(W(XG), W(XG), W(MS), W(DS))

#define mxpjs3rr(XD, XS, XT)                                                \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mxpjs_rx(W(XD))                                                     \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

#define mxpjs3ld(XD, XS, MT, DT)                                            \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_ld(W(XD), W(MT), W(DT))                                       \
        movjx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mxpjs_rx(W(XD))                                                     \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

#define mxhjs_rr(XD, XS) /* horizontal reductive max */                     \
        mxpjs3rr(W(XD), W(XS), W(XS))

#define mxhjs_ld(XD, MS, DS)                                                \
        movjx_ld(W(XD), W(MS), W(DS))                                       \
        mxhjs_rr(W(XD), W(XD))

#define mxpjs_rx(XD) /* not portable, do not use outside */                 \
        movts_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x08))

/* cbr (D = cbrt S) */

/*
 * Based on the original idea by Russell Borogove (kaleja[AT]estarcion[DOT]com)
 * available at http://www.musicdsp.org/showone.php?id=206
 * converted to S-way SIMD version by VectorChief.
 */
#define cbrjs_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        cbejs_rr(W(XD), W(X1), W(X2), W(XS))                                \
        cbsjs_rr(W(XD), W(X1), W(X2), W(XS))                                \
        cbsjs_rr(W(XD), W(X1), W(X2), W(XS))                                \
        cbsjs_rr(W(XD), W(X1), W(X2), W(XS))

#define cbejs_rr(XD, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        /* cube root estimate, the exponent is divided by three             \
         * in such a way that remainder bits get shoved into                \
         * the top of the normalized mantissa */                            \
        movjx_ld(W(X2), Mebp, inf_GPC04_64)                                 \
        movjx_rr(W(XD), W(XS))                                              \
        andjx_rr(W(XD), W(X2))   /* exponent & mantissa in biased-127 */    \
        subjx_ld(W(XD), Mebp, inf_GPC05_64) /* convert to 2's complement */ \
        shrjn_ri(W(XD), IB(10))  /* XD / 1024 */                            \
        movjx_rr(W(X1), W(XD))   /* XD * 341 (next 8 ops) */                \
        shljx_ri(W(X1), IB(2))                                              \
        addjx_rr(W(XD), W(X1))                                              \
        shljx_ri(W(X1), IB(2))                                              \
        addjx_rr(W(XD), W(X1))                                              \
        shljx_ri(W(X1), IB(2))                                              \
        addjx_rr(W(XD), W(X1))                                              \
        shljx_ri(W(X1), IB(2))                                              \
        addjx_rr(W(XD), W(X1))   /* XD * (341/1024) ~= XD * (0.333) */      \
        addjx_ld(W(XD), Mebp, inf_GPC05_64) /* back to biased-127 */        \
        andjx_rr(W(XD), W(X2))   /* remask exponent & mantissa */           \
        annjx_rr(W(X2), W(XS))   /* original sign */                        \
        orrjx_rr(W(XD), W(X2))   /* new exponent & mantissa, old sign */

#define cbsjs_rr(XG, X1, X2, XS) /* destroys X1, X2 (temp regs) */          \
        movjx_rr(W(X1), W(XG))                                              \
        muljs_rr(W(X1), W(XG))                                              \
        movjx_rr(W(X2), W(X1))                                              \
        muljs_ld(W(X1), Mebp, inf_GPC03_64)                                 \
        rcejs_rr(W(X1), W(X1))                                              \
        muljs_rr(W(X2), W(XG))                                              \
        subjs_rr(W(X2), W(XS))                                              \
        muljs_rr(W(X2), W(X1))                                              \
        subjs_rr(W(XG), W(X2))

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
 * of higher performance on MIPS and Power use combined-compare-jump (cmj).
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
/***************** original forms of deprecated cmdx* aliases *****************/
/******************************************************************************/

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

/******************************************************************************/
/***************** original forms of one-operand instructions *****************/
/******************************************************************************/

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

/******************************************************************************/
/*********************************   CONFIG   *********************************/
/******************************************************************************/

/*
 * The following two files (rtarch.h and rtconf.h) provide configuration
 * on the architecture and the instruction subset mapping levels respectively.
 *
 * Placing rtarch.h below the common instruction definitions of rtbase.h
 * allows target-specific BASE and SIMD headers to redefine common instructions
 * in the most optimal way for that particular architecture (#undef/#define).
 *
 * Definitions in rtconf.h provide common subset mappings for all architectures
 * and cannot be redefined in a target-specific way.
 */

#include "rtarch.h"
#include "rtconf.h"

#endif /* RT_RTBASE_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTBASE_H
#define RT_RTBASE_H

/* Generic includes */

#include <math.h>
#include <float.h>

/* Generic types */

typedef float               rt_real;

typedef float               rt_vec2[2];
typedef float               rt_mat2[2][2];

typedef float               rt_vec3[3];
typedef float               rt_mat3[3][3];

typedef float               rt_vec4[4];
typedef float               rt_mat4[4][4];

typedef int                 rt_bool;

typedef char                rt_char;
typedef int                 rt_cell;
typedef long                rt_long; /* time */

typedef unsigned char       rt_byte;
typedef unsigned short      rt_half;
typedef unsigned int        rt_word;

typedef void                rt_void;
typedef void               *rt_pntr;

typedef const char          rt_astr[];
typedef const char         *rt_pstr;

/* Complex types */

struct rt_SIMD_INFO
{
    /* general purpose constants */

    rt_real gpc01[4];       /* +1.0 */
#define inf_GPC01           DP(0x000)

    rt_real gpc02[4];       /* -0.5 */
#define inf_GPC02           DP(0x010)

    rt_real gpc03[4];       /* +3.0 */
#define inf_GPC03           DP(0x020)

    rt_real pad01[28];      /* reserved, do not use! */
#define inf_PAD01           DP(0x030)

    /* internal variables */

    rt_word fctrl;
#define inf_FCTRL           DP(0x0A0)

    rt_word pad02[23];      /* reserved, do not use! */
#define inf_PAD02           DP(0x0A4)

};

typedef rt_pntr (*rt_FUNC_ALLOC)(rt_word size);
typedef rt_void (*rt_FUNC_FREE)(rt_pntr ptr);

/* Generic definitions */

#define RT_NULL             0
#define RT_ALIGN            4

#define RT_FALSE            0
#define RT_TRUE             1

/* Generic macros */

#define RT_ARR_SIZE(a)      (sizeof(a) / sizeof(a[0]))

#define RT_MIN(a, b)        ((a) < (b) ? (a) : (b))
#define RT_MAX(a, b)        ((a) > (b) ? (a) : (b))

/* Vector components */

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

/* Math definitions */

#define RT_INF              FLT_MAX

#define RT_PI               3.14159265358
#define RT_2_PI             (2.0 * RT_PI)
#define RT_PI_2             (RT_PI / 2.0)

#define RT_SIGN(a)          ((a)  <    0.0f ? -1 :                          \
                             (a)  >    0.0f ? +1 :                          \
                              0)

#define RT_FABS(a)          (fabsf(a))

#define RT_SQRT(a)          ((a)  <    0.0f ?  0.0f :                       \
                             sqrtf(a))

#define RT_ASIN(a)          ((a)  <   -1.0f ? -(rt_real)RT_PI_2 :           \
                             (a)  >   +1.0f ? +(rt_real)RT_PI_2 :           \
                             asinf(a))

#define RT_ACOS(a)          ((a)  <   -1.0f ? +(rt_real)RT_PI :             \
                             (a)  >   +1.0f ?  0.0f :                       \
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

#endif /* RT_RTBASE_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

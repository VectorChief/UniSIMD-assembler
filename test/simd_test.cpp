/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include <math.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "rtarch.h"
#include "rtbase.h"

#define RUN_LEVEL       9
#define VERBOSE         RT_FALSE
#define CYC_SIZE        1000000

#define ARR_SIZE        12 /* hardcoded in asm sections */
#define MASK            (RT_SIMD_ALIGN - 1) /* SIMD alignment mask */

#define FRK(f)          (f < 10.0       ? 1.0           :                   \
                         f < 100.0      ? 10.0          :                   \
                         f < 1000.0     ? 100.0         :                   \
                         f < 10000.0    ? 1000.0        :                   \
                         f < 100000.0   ? 10000.0       :                   \
                         f < 1000000.0  ? 100000.0      :                   \
                                          1000000.0)

#define FEQ(f1, f2)     (fabsf(f1 - f2) < 0.0002f * RT_MIN(FRK(f1), FRK(f2)))
#define IEQ(i1, i2)     (i1 == i2)

#define RT_LOGI         printf
#define RT_LOGE         printf


/* NOTE: displacements start where rt_SIMD_INFO ends (at 0x100) */

struct rt_SIMD_INFO_EXT : public rt_SIMD_INFO
{
    rt_real*far0;
#define inf_FAR0        DP(0x100)

    rt_real*fco1;
#define inf_FCO1        DP(0x104)

    rt_real*fco2;
#define inf_FCO2        DP(0x108)

    rt_real*fso1;
#define inf_FSO1        DP(0x10C)

    rt_real*fso2;
#define inf_FSO2        DP(0x110)


    rt_cell*iar0;
#define inf_IAR0        DP(0x114)

    rt_cell*ico1;
#define inf_ICO1        DP(0x118)

    rt_cell*ico2;
#define inf_ICO2        DP(0x11C)

    rt_cell*iso1;
#define inf_ISO1        DP(0x120)

    rt_cell*iso2;
#define inf_ISO2        DP(0x124)


    rt_cell cyc;
#define inf_CYC         DP(0x128)

    rt_cell size;
#define inf_SIZE        DP(0x12C)

    rt_pntr label;
#define inf_LABEL       DP(0x130)

};


#if RUN_LEVEL >=  1

rt_void C_run_level01(rt_SIMD_INFO_EXT *info)
{
    rt_cell i, j, n = info->size;
    rt_real *far0 = info->far0;
    rt_real *fco1 = info->fco1;
    rt_real *fco2 = info->fco2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            fco1[j] = far0[j] + far0[(j + 4) % n];
            fco2[j] = far0[j] - far0[(j + 4) % n];
        }
    }
}

rt_void S_run_level01(rt_SIMD_INFO_EXT *info)
{
    rt_cell i;

#define AJ0             DP(0x000)
#define AJ1             DP(0x010)
#define AJ2             DP(0x020)

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Redx, Mebp, inf_FSO1)
        movxx_ld(Rebx, Mebp, inf_FSO2)

        movpx_ld(Xmm0, Mecx, AJ0)
        movpx_ld(Xmm1, Mecx, AJ1)
        movpx_rr(Xmm2, Xmm0)
        addps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        subps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)

        movpx_ld(Xmm0, Mecx, AJ1)
        movpx_ld(Xmm1, Mecx, AJ2)
        movpx_rr(Xmm2, Xmm0)
        addps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        subps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)

        movpx_ld(Xmm0, Mecx, AJ2)
        movpx_ld(Xmm1, Mecx, AJ0)
        movpx_rr(Xmm2, Xmm0)
        addps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        subps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void P_run_level01(rt_SIMD_INFO_EXT *info, rt_long tC, rt_long tS, rt_bool v)
{
    rt_cell j, n;

    rt_real *far0 = info->far0;
    rt_real *fco1 = info->fco1;
    rt_real *fco2 = info->fco2;
    rt_real *fso1 = info->fso1;
    rt_real *fso2 = info->fso2;

    RT_LOGI("-----------------  RUN LEVEL = %2d  -----------------\n",  1);

    j = n = info->size;
    while (j-->0)
    {
        if (FEQ(fco1[j], fso1[j]) && FEQ(fco2[j], fso2[j]) && !v)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e, farr[%d] = %e\n",
                j, far0[j], (j + 4) % n, far0[(j + 4) % n]);

        RT_LOGI("C farr[%d]+farr[%d] = %e, farr[%d]-farr[%d] = %e\n",
                j, (j + 4) % n, fco1[j], j, (j + 4) % n, fco2[j]);

        RT_LOGI("S farr[%d]+farr[%d] = %e, farr[%d]-farr[%d] = %e\n",
                j, (j + 4) % n, fso1[j], j, (j + 4) % n, fso2[j]);
    }

    RT_LOGI("Time C = %d\n", (rt_cell)tC);
    RT_LOGI("Time S = %d\n", (rt_cell)tS);

    RT_LOGI("----------------------------------------------------\n");
}

#endif /* RUN_LEVEL  1 */


#if RUN_LEVEL >=  2

rt_void C_run_level02(rt_SIMD_INFO_EXT *info)
{
    rt_cell i, j, n = info->size;
    rt_real *far0 = info->far0;
    rt_real *fco1 = info->fco1;
    rt_real *fco2 = info->fco2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            fco1[j] = far0[j] * far0[(j + 4) % n];
            fco2[j] = far0[j] / far0[(j + 4) % n];
        }
    }
}

rt_void S_run_level02(rt_SIMD_INFO_EXT *info)
{
    rt_cell i;

#define AJ0             DP(0x000)
#define AJ1             DP(0x010)
#define AJ2             DP(0x020)

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Redx, Mebp, inf_FSO1)
        movxx_ld(Rebx, Mebp, inf_FSO2)

        movpx_ld(Xmm0, Mecx, AJ0)
        movpx_ld(Xmm1, Mecx, AJ1)
        movpx_rr(Xmm2, Xmm0)
        mulps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        divps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)

        movpx_ld(Xmm0, Mecx, AJ1)
        movpx_ld(Xmm1, Mecx, AJ2)
        movpx_rr(Xmm2, Xmm0)
        mulps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        divps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)

        movpx_ld(Xmm0, Mecx, AJ2)
        movpx_ld(Xmm1, Mecx, AJ0)
        movpx_rr(Xmm2, Xmm0)
        mulps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        divps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void P_run_level02(rt_SIMD_INFO_EXT *info, rt_long tC, rt_long tS, rt_bool v)
{
    rt_cell j, n;

    rt_real *far0 = info->far0;
    rt_real *fco1 = info->fco1;
    rt_real *fco2 = info->fco2;
    rt_real *fso1 = info->fso1;
    rt_real *fso2 = info->fso2;

    RT_LOGI("-----------------  RUN LEVEL = %2d  -----------------\n",  2);

    j = n = info->size;
    while (j-->0)
    {
        if (FEQ(fco1[j], fso1[j]) && FEQ(fco2[j], fso2[j]) && !v)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e, farr[%d] = %e\n",
                j, far0[j], (j + 4) % n, far0[(j + 4) % n]);

        RT_LOGI("C farr[%d]*farr[%d] = %e, farr[%d]/farr[%d] = %e\n",
                j, (j + 4) % n, fco1[j], j, (j + 4) % n, fco2[j]);

        RT_LOGI("S farr[%d]*farr[%d] = %e, farr[%d]/farr[%d] = %e\n",
                j, (j + 4) % n, fso1[j], j, (j + 4) % n, fso2[j]);

    }

    RT_LOGI("Time C = %d\n", (rt_cell)tC);
    RT_LOGI("Time S = %d\n", (rt_cell)tS);

    RT_LOGI("----------------------------------------------------\n");
}

#endif /* RUN_LEVEL  2 */


#if RUN_LEVEL >=  3

rt_void C_run_level03(rt_SIMD_INFO_EXT *info)
{
    rt_cell i, j, n = info->size;
    rt_real *far0 = info->far0;
    rt_cell *ico1 = info->ico1;
    rt_cell *ico2 = info->ico2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = (far0[j] >  far0[(j + 4) % n]) ? 0xFFFFFFFF : 0x00000000;
            ico2[j] = (far0[j] >= far0[(j + 4) % n]) ? 0xFFFFFFFF : 0x00000000;
        }
    }
}

rt_void S_run_level03(rt_SIMD_INFO_EXT *info)
{
    rt_cell i;

#define AJ0             DP(0x000)
#define AJ1             DP(0x010)
#define AJ2             DP(0x020)

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Redx, Mebp, inf_ISO1)
        movxx_ld(Rebx, Mebp, inf_ISO2)

        movpx_ld(Xmm0, Mecx, AJ0)
        movpx_ld(Xmm1, Mecx, AJ1)
        movpx_rr(Xmm2, Xmm0)
        cgtps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        cgeps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)

        movpx_ld(Xmm0, Mecx, AJ1)
        movpx_ld(Xmm1, Mecx, AJ2)
        movpx_rr(Xmm2, Xmm0)
        cgtps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        cgeps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)

        movpx_ld(Xmm0, Mecx, AJ2)
        movpx_ld(Xmm1, Mecx, AJ0)
        movpx_rr(Xmm2, Xmm0)
        cgtps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        cgeps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void P_run_level03(rt_SIMD_INFO_EXT *info, rt_long tC, rt_long tS, rt_bool v)
{
    rt_cell j, n;

    rt_real *far0 = info->far0;
    rt_cell *ico1 = info->ico1;
    rt_cell *ico2 = info->ico2;
    rt_cell *iso1 = info->iso1;
    rt_cell *iso2 = info->iso2;

    RT_LOGI("-----------------  RUN LEVEL = %2d  -----------------\n",  3);

    j = n = info->size;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e, farr[%d] = %e\n",
                j, far0[j], (j + 4) % n, far0[(j + 4) % n]);

        RT_LOGI("C (farr[%d] > farr[%d]) = %X, (farr[%d] >= farr[%d]) = %X\n",
                j, (j + 4) % n, ico1[j], j, (j + 4) % n, ico2[j]);

        RT_LOGI("S (farr[%d] > farr[%d]) = %X, (farr[%d] >= farr[%d]) = %X\n",
                j, (j + 4) % n, iso1[j], j, (j + 4) % n, iso2[j]);
    }

    RT_LOGI("Time C = %d\n", (rt_cell)tC);
    RT_LOGI("Time S = %d\n", (rt_cell)tS);

    RT_LOGI("----------------------------------------------------\n");
}

#endif /* RUN_LEVEL  3 */


#if RUN_LEVEL >=  4

rt_void C_run_level04(rt_SIMD_INFO_EXT *info)
{
    rt_cell i, j, n = info->size;
    rt_real *far0 = info->far0;
    rt_cell *ico1 = info->ico1;
    rt_cell *ico2 = info->ico2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = (far0[j] <  far0[(j + 4) % n]) ? 0xFFFFFFFF : 0x00000000;
            ico2[j] = (far0[j] <= far0[(j + 4) % n]) ? 0xFFFFFFFF : 0x00000000;
        }
    }
}

rt_void S_run_level04(rt_SIMD_INFO_EXT *info)
{
    rt_cell i;

#define AJ0             DP(0x000)
#define AJ1             DP(0x010)
#define AJ2             DP(0x020)

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Redx, Mebp, inf_ISO1)
        movxx_ld(Rebx, Mebp, inf_ISO2)

        movpx_ld(Xmm0, Mecx, AJ0)
        movpx_ld(Xmm1, Mecx, AJ1)
        movpx_rr(Xmm2, Xmm0)
        cltps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        cleps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)

        movpx_ld(Xmm0, Mecx, AJ1)
        movpx_ld(Xmm1, Mecx, AJ2)
        movpx_rr(Xmm2, Xmm0)
        cltps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        cleps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)

        movpx_ld(Xmm0, Mecx, AJ2)
        movpx_ld(Xmm1, Mecx, AJ0)
        movpx_rr(Xmm2, Xmm0)
        cltps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        cleps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void P_run_level04(rt_SIMD_INFO_EXT *info, rt_long tC, rt_long tS, rt_bool v)
{
    rt_cell j, n;

    rt_real *far0 = info->far0;
    rt_cell *ico1 = info->ico1;
    rt_cell *ico2 = info->ico2;
    rt_cell *iso1 = info->iso1;
    rt_cell *iso2 = info->iso2;

    RT_LOGI("-----------------  RUN LEVEL = %2d  -----------------\n",  4);

    j = n = info->size;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e, farr[%d] = %e\n",
                j, far0[j], (j + 4) % n, far0[(j + 4) % n]);

        RT_LOGI("C (farr[%d] < farr[%d]) = %X, (farr[%d] <= farr[%d]) = %X\n",
                j, (j + 4) % n, ico1[j], j, (j + 4) % n, ico2[j]);

        RT_LOGI("S (farr[%d] < farr[%d]) = %X, (farr[%d] <= farr[%d]) = %X\n",
                j, (j + 4) % n, iso1[j], j, (j + 4) % n, iso2[j]);
    }

    RT_LOGI("Time C = %d\n", (rt_cell)tC);
    RT_LOGI("Time S = %d\n", (rt_cell)tS);

    RT_LOGI("----------------------------------------------------\n");
}

#endif /* RUN_LEVEL  4 */


#if RUN_LEVEL >=  5

rt_void C_run_level05(rt_SIMD_INFO_EXT *info)
{
    rt_cell i, j, n = info->size;
    rt_real *far0 = info->far0;
    rt_cell *ico1 = info->ico1;
    rt_cell *ico2 = info->ico2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = (far0[j] == far0[(j + 4) % n]) ? 0xFFFFFFFF : 0x00000000;
            ico2[j] = (far0[j] != far0[(j + 4) % n]) ? 0xFFFFFFFF : 0x00000000;
        }
    }
}

rt_void S_run_level05(rt_SIMD_INFO_EXT *info)
{
    rt_cell i;

#define AJ0             DP(0x000)
#define AJ1             DP(0x010)
#define AJ2             DP(0x020)

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Redx, Mebp, inf_ISO1)
        movxx_ld(Rebx, Mebp, inf_ISO2)

        movpx_ld(Xmm0, Mecx, AJ0)
        movpx_ld(Xmm1, Mecx, AJ1)
        movpx_rr(Xmm2, Xmm0)
        ceqps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        cneps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)

        movpx_ld(Xmm0, Mecx, AJ1)
        movpx_ld(Xmm1, Mecx, AJ2)
        movpx_rr(Xmm2, Xmm0)
        ceqps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        cneps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)

        movpx_ld(Xmm0, Mecx, AJ2)
        movpx_ld(Xmm1, Mecx, AJ0)
        movpx_rr(Xmm2, Xmm0)
        ceqps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        cneps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void P_run_level05(rt_SIMD_INFO_EXT *info, rt_long tC, rt_long tS, rt_bool v)
{
    rt_cell j, n;

    rt_real *far0 = info->far0;
    rt_cell *ico1 = info->ico1;
    rt_cell *ico2 = info->ico2;
    rt_cell *iso1 = info->iso1;
    rt_cell *iso2 = info->iso2;

    RT_LOGI("-----------------  RUN LEVEL = %2d  -----------------\n",  5);

    j = n = info->size;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e, farr[%d] = %e\n",
                j, far0[j], (j + 4) % n, far0[(j + 4) % n]);

        RT_LOGI("C (farr[%d] == farr[%d]) = %X, (farr[%d] != farr[%d]) = %X\n",
                j, (j + 4) % n, ico1[j], j, (j + 4) % n, ico2[j]);

        RT_LOGI("S (farr[%d] == farr[%d]) = %X, (farr[%d] != farr[%d]) = %X\n",
                j, (j + 4) % n, iso1[j], j, (j + 4) % n, iso2[j]);
    }

    RT_LOGI("Time C = %d\n", (rt_cell)tC);
    RT_LOGI("Time S = %d\n", (rt_cell)tS);

    RT_LOGI("----------------------------------------------------\n");
}

#endif /* RUN_LEVEL  5 */


#if RUN_LEVEL >=  6

rt_void C_run_level06(rt_SIMD_INFO_EXT *info)
{
    rt_cell i, j, n = info->size;
    rt_real *far0 = info->far0;
    rt_cell *iar0 = info->iar0;
    rt_cell *ico1 = info->ico1;
    rt_real *fco2 = info->fco2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = (rt_cell)far0[j];
            fco2[j] = (rt_real)iar0[j];
        }
    }
}

rt_void S_run_level06(rt_SIMD_INFO_EXT *info)
{
    rt_cell i;

#define AJ0             DP(0x000)
#define AJ1             DP(0x010)
#define AJ2             DP(0x020)

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        FCTRL_ENTER(ROUNDM)

        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Resi, Mebp, inf_IAR0)
        movxx_ld(Redx, Mebp, inf_ISO1)
        movxx_ld(Rebx, Mebp, inf_FSO2)

        movpx_ld(Xmm0, Mecx, AJ0)
        movpx_ld(Xmm1, Mesi, AJ0)
        cvtps_rr(Xmm2, Xmm0)
        cvtpn_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)

        movpx_ld(Xmm0, Mecx, AJ1)
        movpx_ld(Xmm1, Mesi, AJ1)
        cvtps_rr(Xmm2, Xmm0)
        cvtpn_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)

        movpx_ld(Xmm0, Mecx, AJ2)
        movpx_ld(Xmm1, Mesi, AJ2)
        cvtps_rr(Xmm2, Xmm0)
        cvtpn_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)

        FCTRL_LEAVE(ROUNDM)

        ASM_LEAVE(info)
    }
}

rt_void P_run_level06(rt_SIMD_INFO_EXT *info, rt_long tC, rt_long tS, rt_bool v)
{
    rt_cell j, n;

    rt_real *far0 = info->far0;
    rt_cell *iar0 = info->iar0;
    rt_cell *ico1 = info->ico1;
    rt_real *fco2 = info->fco2;
    rt_cell *iso1 = info->iso1;
    rt_real *fso2 = info->fso2;

    RT_LOGI("-----------------  RUN LEVEL = %2d  -----------------\n",  6);

    j = n = info->size;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && FEQ(fco2[j], fso2[j]) && !v)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e, iarr[%d] = %d\n",
                j, far0[j], j, iar0[j]);

        RT_LOGI("C (rt_cell)farr[%d] = %d, (rt_real)iarr[%d] = %e\n",
                j, ico1[j], j, fco2[j]);

        RT_LOGI("S (rt_cell)farr[%d] = %d, (rt_real)iarr[%d] = %e\n",
                j, iso1[j], j, fso2[j]);
    }

    RT_LOGI("Time C = %d\n", (rt_cell)tC);
    RT_LOGI("Time S = %d\n", (rt_cell)tS);

    RT_LOGI("----------------------------------------------------\n");
}

#endif /* RUN_LEVEL  6 */


#if RUN_LEVEL >=  7

rt_void C_run_level07(rt_SIMD_INFO_EXT *info)
{
    rt_cell i, j, n = info->size;
    rt_real *far0 = info->far0;
    rt_real *fco1 = info->fco1;
    rt_real *fco2 = info->fco2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            fco1[j] = sqrtf(far0[j]);
            fco2[j] = 1.0f / far0[j];
        }
    }
}

rt_void S_run_level07(rt_SIMD_INFO_EXT *info)
{
    rt_cell i;

#define AJ0             DP(0x000)
#define AJ1             DP(0x010)
#define AJ2             DP(0x020)

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Redx, Mebp, inf_FSO1)
        movxx_ld(Rebx, Mebp, inf_FSO2)

        movpx_ld(Xmm0, Mecx, AJ0)
        movpx_rr(Xmm1, Xmm0)
        rsqps_rr(Xmm2, Xmm0) /* destroys Xmm0 */
        mulps_rr(Xmm2, Xmm1)
        rcpps_rr(Xmm3, Xmm1) /* destroys Xmm1 */
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)

        movpx_ld(Xmm0, Mecx, AJ1)
        movpx_rr(Xmm1, Xmm0)
        rsqps_rr(Xmm2, Xmm0) /* destroys Xmm0 */
        mulps_rr(Xmm2, Xmm1)
        rcpps_rr(Xmm3, Xmm1) /* destroys Xmm1 */
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)

        movpx_ld(Xmm0, Mecx, AJ2)
        movpx_rr(Xmm1, Xmm0)
        rsqps_rr(Xmm2, Xmm0) /* destroys Xmm0 */
        mulps_rr(Xmm2, Xmm1)
        rcpps_rr(Xmm3, Xmm1) /* destroys Xmm1 */
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void P_run_level07(rt_SIMD_INFO_EXT *info, rt_long tC, rt_long tS, rt_bool v)
{
    rt_cell j, n;

    rt_real *far0 = info->far0;
    rt_real *fco1 = info->fco1;
    rt_real *fco2 = info->fco2;
    rt_real *fso1 = info->fso1;
    rt_real *fso2 = info->fso2;

    RT_LOGI("-----------------  RUN LEVEL = %2d  -----------------\n",  7);

    j = n = info->size;
    while (j-->0)
    {
        if (FEQ(fco1[j], fso1[j]) && FEQ(fco2[j], fso2[j]) && !v)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e\n",
                j, far0[j]);

        RT_LOGI("C sqrt(farr[%d]) = %e, 1.0/farr[%d] = %e\n",
                j, fco1[j], j, fco2[j]);

        RT_LOGI("S sqrt(farr[%d]) = %e, 1.0/farr[%d] = %e\n",
                j, fso1[j], j, fso2[j]);
    }

    RT_LOGI("Time C = %d\n", (rt_cell)tC);
    RT_LOGI("Time S = %d\n", (rt_cell)tS);

    RT_LOGI("----------------------------------------------------\n");
}

#endif /* RUN_LEVEL  7 */


#if RUN_LEVEL >=  8

rt_void C_run_level08(rt_SIMD_INFO_EXT *info)
{
    rt_cell i, j, n = info->size;
    rt_cell *iar0 = info->iar0;
    rt_cell *ico1 = info->ico1;
    rt_cell *ico2 = info->ico2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = iar0[j] + (iar0[j] << 1);
            ico2[j] = iar0[j] + (iar0[j] >> 2);
        }
    }
}

rt_void S_run_level08(rt_SIMD_INFO_EXT *info)
{
    rt_cell i;

#define AJ0             DP(0x000)
#define AJ1             DP(0x010)
#define AJ2             DP(0x020)

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Resi, Mebp, inf_IAR0)
        movxx_ld(Redx, Mebp, inf_ISO1)
        movxx_ld(Rebx, Mebp, inf_ISO2)

        movpx_ld(Xmm0, Mesi, AJ0)
        movpx_ld(Xmm1, Mesi, AJ0)
        movpx_rr(Xmm2, Xmm0)
        shlpx_ri(Xmm0, IB(1))
        addpx_rr(Xmm2, Xmm0)
        movpx_rr(Xmm3, Xmm1)
        shrpx_ri(Xmm1, IB(2))
        addpx_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)

        movpx_ld(Xmm0, Mesi, AJ1)
        movpx_ld(Xmm1, Mesi, AJ1)
        movpx_rr(Xmm2, Xmm0)
        shlpx_ri(Xmm0, IB(1))
        addpx_rr(Xmm2, Xmm0)
        movpx_rr(Xmm3, Xmm1)
        shrpx_ri(Xmm1, IB(2))
        addpx_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)

        movpx_ld(Xmm0, Mesi, AJ2)
        movpx_ld(Xmm1, Mesi, AJ2)
        movpx_rr(Xmm2, Xmm0)
        shlpx_ri(Xmm0, IB(1))
        addpx_rr(Xmm2, Xmm0)
        movpx_rr(Xmm3, Xmm1)
        shrpx_ri(Xmm1, IB(2))
        addpx_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void P_run_level08(rt_SIMD_INFO_EXT *info, rt_long tC, rt_long tS, rt_bool v)
{
    rt_cell j, n;

    rt_cell *iar0 = info->iar0;
    rt_cell *ico1 = info->ico1;
    rt_cell *ico2 = info->ico2;
    rt_cell *iso1 = info->iso1;
    rt_cell *iso2 = info->iso2;

    RT_LOGI("-----------------  RUN LEVEL = %2d  -----------------\n",  8);

    j = n = info->size;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v)
        {
            continue;
        }

        RT_LOGI("iarr[%d] = %d\n",
                j, iar0[j]);

        RT_LOGI("C iarr[%d]+(iarr[%d]<<1) = %d, iarr[%d]+(iarr[%d]>>2) = %d\n",
                j, j, ico1[j], j, j, ico2[j]);

        RT_LOGI("S iarr[%d]+(iarr[%d]<<1) = %d, iarr[%d]+(iarr[%d]>>2) = %d\n",
                j, j, iso1[j], j, j, iso2[j]);

    }

    RT_LOGI("Time C = %d\n", (rt_cell)tC);
    RT_LOGI("Time S = %d\n", (rt_cell)tS);

    RT_LOGI("----------------------------------------------------\n");
}

#endif /* RUN_LEVEL  8 */


#if RUN_LEVEL >=  9

rt_void C_run_level09(rt_SIMD_INFO_EXT *info)
{
    rt_cell i, j, n = info->size;
    rt_cell *iar0 = info->iar0;
    rt_cell *ico1 = info->ico1;
    rt_cell *ico2 = info->ico2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = iar0[j] * iar0[(j + 4) % n];
            ico2[j] = iar0[j] / iar0[(j + 4) % n];
        }
    }
}

rt_void S_run_level09(rt_SIMD_INFO_EXT *info)
{
    ASM_ENTER(info)

        label_ld(cyc_beg) /* load to Reax */
        movxx_st(Reax, Mebp, inf_LABEL)

    LBL(cyc_beg)

        movxx_ld(Recx, Mebp, inf_IAR0)
        movxx_ld(Rebx, Mebp, inf_ISO1)
        movxx_ld(Resi, Mebp, inf_ISO2)
        movxx_ld(Redi, Mebp, inf_SIZE)

    LBL(loc_beg)

        movxx_ld(Reax, Mecx, DP(0x00))
        mulxn_xm(Mecx, DP(0x10))
        movxx_st(Reax, Mebx, DP(0x00))
        movxx_ld(Reax, Mecx, DP(0x00))
        movxx_ri(Redx, IB(0))
        divxn_xm(Mecx, DP(0x10))
        movxx_st(Reax, Mesi, DP(0x00))

        addxx_ri(Recx, IB(4))
        addxx_ri(Rebx, IB(4))
        addxx_ri(Resi, IB(4))
        subxx_ri(Redi, IB(1))
        cmpxx_ri(Redi, IB(4))
        jgtxx_lb(loc_beg)

        movxx_ld(Redi, Mebp, inf_IAR0)

        movxx_ld(Reax, Mecx, DP(0x00))
        mulxn_xm(Medi, DP(0x00))
        movxx_st(Reax, Mebx, DP(0x00))
        movxx_ld(Reax, Mecx, DP(0x00))
        movxx_ri(Redx, IB(0))
        divxn_xm(Medi, DP(0x00))
        movxx_st(Reax, Mesi, DP(0x00))

        movxx_ld(Reax, Mecx, DP(0x04))
        mulxn_xm(Medi, DP(0x04))
        movxx_st(Reax, Mebx, DP(0x04))
        movxx_ld(Reax, Mecx, DP(0x04))
        movxx_ri(Redx, IB(0))
        divxn_xm(Medi, DP(0x04))
        movxx_st(Reax, Mesi, DP(0x04))

        movxx_ld(Reax, Mecx, DP(0x08))
        mulxn_xm(Medi, DP(0x08))
        movxx_st(Reax, Mebx, DP(0x08))
        movxx_ld(Reax, Mecx, DP(0x08))
        movxx_ri(Redx, IB(0))
        divxn_xm(Medi, DP(0x08))
        movxx_st(Reax, Mesi, DP(0x08))

        movxx_ld(Reax, Mecx, DP(0x0C))
        mulxn_xm(Medi, DP(0x0C))
        movxx_st(Reax, Mebx, DP(0x0C))
        movxx_ld(Reax, Mecx, DP(0x0C))
        movxx_ri(Redx, IB(0))
        divxn_xm(Medi, DP(0x0C))
        movxx_st(Reax, Mesi, DP(0x0C))

        subxx_mi(Mebp, inf_CYC, IB(1))
        cmpxx_mi(Mebp, inf_CYC, IB(0))
        jeqxx_lb(cyc_end)
        jmpxx_mm(Mebp, inf_LABEL)
        jmpxx_lb(cyc_beg) /* the same jump as above */

    LBL(cyc_end)

    ASM_LEAVE(info)
}

rt_void P_run_level09(rt_SIMD_INFO_EXT *info, rt_long tC, rt_long tS, rt_bool v)
{
    rt_cell j, n;

    rt_cell *iar0 = info->iar0;
    rt_cell *ico1 = info->ico1;
    rt_cell *ico2 = info->ico2;
    rt_cell *iso1 = info->iso1;
    rt_cell *iso2 = info->iso2;

    RT_LOGI("-----------------  RUN LEVEL = %2d  -----------------\n",  9);

    j = n = info->size;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v)
        {
            continue;
        }

        RT_LOGI("iarr[%d] = %d, iarr[%d] = %d\n",
                j, iar0[j], (j + 4) % n, iar0[(j + 4) % n]);

        RT_LOGI("C iarr[%d]*iarr[%d] = %d, iarr[%d]/iarr[%d] = %d\n",
                j, (j + 4) % n, ico1[j], j, (j + 4) % n, ico2[j]);

        RT_LOGI("S iarr[%d]*iarr[%d] = %d, iarr[%d]/iarr[%d] = %d\n",
                j, (j + 4) % n, iso1[j], j, (j + 4) % n, iso2[j]);

    }

    RT_LOGI("Time C = %d\n", (rt_cell)tC);
    RT_LOGI("Time S = %d\n", (rt_cell)tS);

    RT_LOGI("----------------------------------------------------\n");
}

#endif /* RUN_LEVEL  9 */


typedef rt_void (*C_run_levelXX)(rt_SIMD_INFO_EXT *);
typedef rt_void (*S_run_levelXX)(rt_SIMD_INFO_EXT *);
typedef rt_void (*P_run_levelXX)(rt_SIMD_INFO_EXT *, rt_long, rt_long, rt_bool);

C_run_levelXX Carr[RUN_LEVEL] =
{
#if RUN_LEVEL >=  1
    C_run_level01,
#endif /* RUN_LEVEL  1 */

#if RUN_LEVEL >=  2
    C_run_level02,
#endif /* RUN_LEVEL  2 */

#if RUN_LEVEL >=  3
    C_run_level03,
#endif /* RUN_LEVEL  3 */

#if RUN_LEVEL >=  4
    C_run_level04,
#endif /* RUN_LEVEL  4 */

#if RUN_LEVEL >=  5
    C_run_level05,
#endif /* RUN_LEVEL  5 */

#if RUN_LEVEL >=  6
    C_run_level06,
#endif /* RUN_LEVEL  6 */

#if RUN_LEVEL >=  7
    C_run_level07,
#endif /* RUN_LEVEL  7 */

#if RUN_LEVEL >=  8
    C_run_level08,
#endif /* RUN_LEVEL  8 */

#if RUN_LEVEL >=  9
    C_run_level09,
#endif /* RUN_LEVEL  9 */
};

S_run_levelXX Sarr[RUN_LEVEL] =
{
#if RUN_LEVEL >=  1
    S_run_level01,
#endif /* RUN_LEVEL  1 */

#if RUN_LEVEL >=  2
    S_run_level02,
#endif /* RUN_LEVEL  2 */

#if RUN_LEVEL >=  3
    S_run_level03,
#endif /* RUN_LEVEL  3 */

#if RUN_LEVEL >=  4
    S_run_level04,
#endif /* RUN_LEVEL  4 */

#if RUN_LEVEL >=  5
    S_run_level05,
#endif /* RUN_LEVEL  5 */

#if RUN_LEVEL >=  6
    S_run_level06,
#endif /* RUN_LEVEL  6 */

#if RUN_LEVEL >=  7
    S_run_level07,
#endif /* RUN_LEVEL  7 */

#if RUN_LEVEL >=  8
    S_run_level08,
#endif /* RUN_LEVEL  8 */

#if RUN_LEVEL >=  9
    S_run_level09,
#endif /* RUN_LEVEL  9 */
};

P_run_levelXX Parr[RUN_LEVEL] =
{
#if RUN_LEVEL >=  1
    P_run_level01,
#endif /* RUN_LEVEL  1 */

#if RUN_LEVEL >=  2
    P_run_level02,
#endif /* RUN_LEVEL  2 */

#if RUN_LEVEL >=  3
    P_run_level03,
#endif /* RUN_LEVEL  3 */

#if RUN_LEVEL >=  4
    P_run_level04,
#endif /* RUN_LEVEL  4 */

#if RUN_LEVEL >=  5
    P_run_level05,
#endif /* RUN_LEVEL  5 */

#if RUN_LEVEL >=  6
    P_run_level06,
#endif /* RUN_LEVEL  6 */

#if RUN_LEVEL >=  7
    P_run_level07,
#endif /* RUN_LEVEL  7 */

#if RUN_LEVEL >=  8
    P_run_level08,
#endif /* RUN_LEVEL  8 */

#if RUN_LEVEL >=  9
    P_run_level09,
#endif /* RUN_LEVEL  9 */
};


rt_long get_time();

/*
 * info - info original pointer
 * inf0 - info aligned pointer
 * marr - memory original pointer
 * mar0 - memory aligned pointer
 *
 * farr - float original array
 * far0 - float aligned array 0
 * fco1 - float aligned C out 1
 * fco2 - float aligned C out 2
 * fso1 - float aligned S out 1
 * fso2 - float aligned S out 2
 *
 * iarr - int original array
 * iar0 - int aligned array 0
 * ico1 - int aligned C out 1
 * ico2 - int aligned C out 2
 * iso1 - int aligned S out 1
 * iso2 - int aligned S out 2
 */
int main ()
{
    rt_pntr marr = malloc(10 * ARR_SIZE * sizeof(rt_word) + MASK);
    rt_pntr mar0 = (rt_pntr)(((rt_word)marr + MASK) & ~MASK);

    rt_real farr[ARR_SIZE] =
    {
        34.2785,
        113.98764,
        0.65434,
        1.4687656,
        0.0032467,
        2.175953,
        0.65434,
        578986.23,
        8764.7534,
        113.98764,
        0.0765376,
        43187.487,
    };

    rt_real *far0 = (rt_real *)mar0 + ARR_SIZE * 0;
    rt_real *fco1 = (rt_real *)mar0 + ARR_SIZE * 1;
    rt_real *fco2 = (rt_real *)mar0 + ARR_SIZE * 2;
    rt_real *fso1 = (rt_real *)mar0 + ARR_SIZE * 3;
    rt_real *fso2 = (rt_real *)mar0 + ARR_SIZE * 4;

    memcpy(far0, farr, sizeof(farr));

    rt_cell iarr[ARR_SIZE] =
    {
        285,
        113,
        65,
        14,
        3,
        1,
        7,
        57896,
        2347875,
        87647531,
        7665,
        318773,
    };

    rt_cell *iar0 = (rt_cell *)mar0 + ARR_SIZE * 5;
    rt_cell *ico1 = (rt_cell *)mar0 + ARR_SIZE * 6;
    rt_cell *ico2 = (rt_cell *)mar0 + ARR_SIZE * 7;
    rt_cell *iso1 = (rt_cell *)mar0 + ARR_SIZE * 8;
    rt_cell *iso2 = (rt_cell *)mar0 + ARR_SIZE * 9;

    memcpy(iar0, iarr, sizeof(iarr));

    rt_pntr info = malloc(sizeof(rt_SIMD_INFO_EXT) + MASK);
    rt_SIMD_INFO_EXT *inf0 = (rt_SIMD_INFO_EXT *)(((rt_word)info + MASK) & ~MASK);

    RT_SIMD_SET(inf0->gpc01, +1.0f);
    RT_SIMD_SET(inf0->gpc02, -0.5f);
    RT_SIMD_SET(inf0->gpc03, +3.0f);

    inf0->far0 = far0;
    inf0->fco1 = fco1;
    inf0->fco2 = fco2;
    inf0->fso1 = fso1;
    inf0->fso2 = fso2;

    inf0->iar0 = iar0;
    inf0->ico1 = ico1;
    inf0->ico2 = ico2;
    inf0->iso1 = iso1;
    inf0->iso2 = iso2;

    inf0->cyc  = CYC_SIZE;
    inf0->size = ARR_SIZE;

    rt_long time1 = 0;
    rt_long time2 = 0;
    rt_long tC = 0;
    rt_long tS = 0;

    rt_cell i;

    for (i = 0; i < RUN_LEVEL; i++)
    {
        time1 = get_time();

        Carr[i](inf0);

        time2 = get_time();
        tC = time2 - time1;

        time1 = get_time();

        Sarr[i](inf0);

        time2 = get_time();
        tS = time2 - time1;

        Parr[i](inf0, tC, tS, VERBOSE);
    }

    free(info);
    free(marr);

#if   defined (WIN32) /* Win32, MSVC ---------------------------------------- */

    RT_LOGI("Type any letter and press ENTER to exit:");
    rt_char str[256]; /* not secure, do not inherit this practice */
    scanf("%s", str); /* not secure, do not inherit this practice */

#endif /* ------------- OS specific ----------------------------------------- */

    return 0;
}

#if   defined (WIN32) /* Win32, MSVC ---------------------------------------- */

#include <windows.h>

rt_long get_time()
{
    LARGE_INTEGER fr;
    QueryPerformanceFrequency(&fr);
    LARGE_INTEGER tm;
    QueryPerformanceCounter(&tm);
    return (rt_long)(tm.QuadPart * 1000 / fr.QuadPart);
}

#elif defined (linux) /* Linux, GCC ----------------------------------------- */

#include <sys/time.h>

rt_long get_time()
{
    timeval tm;
    gettimeofday(&tm, NULL);
    return (rt_long)(tm.tv_sec * 1000 + tm.tv_usec / 1000);
}

#endif /* ------------- OS specific ----------------------------------------- */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

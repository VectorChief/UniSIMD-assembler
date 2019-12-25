/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define RT_SIMD_CODE /* enable SIMD instructions definitions */
#define RT_BASE_TEST /* enable BASE instructions sub-tests */

#include "rtarch.h"
#include "rtbase.h"

/******************************************************************************/
/*******************************   DEFINITIONS   ******************************/
/******************************************************************************/

#define RUN_LEVEL           18
#define CYC_SIZE            1000000

#define ARR_SIZE            S*3 /* hardcoded in asm sections, S = SIMD width */
#define MASK                (RT_SIMD_ALIGN - 1) /* SIMD alignment mask */

/* NOTE: floating point values are not tested for equality precisely due to
 * the slight difference in SIMD/FPU implementations across supported targets */
#define FRK(f)              (f < 10.0       ?    0.0001     :               \
                             f < 100.0      ?    0.001      :               \
                             f < 1000.0     ?    0.01       :               \
                             f < 10000.0    ?    0.1        :               \
                             f < 100000.0   ?    1.0        :               \
                             f < 1000000.0  ?   10.0        :  100.0)

#define IEQ(i1, i2)         (i1 == i2)

#define FEQ(f1, f2)         (RT_FABS((f1) - (f2)) <= t_diff *               \
                             RT_MIN(FRK(f1), FRK(f2)))

#define RT_LOGI             printf
#define RT_LOGE             printf

/******************************************************************************/
/***************************   VARS, FUNCS, TYPES   ***************************/
/******************************************************************************/

static rt_si32 t_diff = 2;
static rt_bool v_mode = RT_FALSE;

/*
 * Get system time in milliseconds.
 */
rt_time get_time();

/*
 * Allocate memory from system heap.
 */
rt_pntr sys_alloc(rt_size size);

/*
 * Free memory from system heap.
 */
rt_void sys_free(rt_pntr ptr, rt_size size);

/*
 * Extended SIMD info structure for asm enter/leave
 * serves as a container for test arrays and internal variables.
 * Note that DP offsets below start where rt_SIMD_INFO ends (at Q*0x100).
 * SIMD width is taken into account via S and Q from rtarch.h
 */
struct rt_SIMD_INFOX : public rt_SIMD_INFO
{
    /* internal variables */

    rt_si32 cyc;
#define inf_CYC             DP(Q*0x100+0x000)

    rt_si32 loc;
#define inf_LOC             DP(Q*0x100+0x004)

    rt_si32 size;
#define inf_SIZE            DP(Q*0x100+0x008)

    rt_si32 simd;
#define inf_SIMD            DP(Q*0x100+0x00C)

    rt_pntr label;
#define inf_LABEL           DP(Q*0x100+0x010+0x000*P)

    rt_pntr tail;
#define inf_TAIL            DP(Q*0x100+0x010+0x004*P)

    /* floating point arrays */

    rt_real*far0;
#define inf_FAR0            DP(Q*0x100+0x010+0x008*P+E)

    rt_real*fco1;
#define inf_FCO1            DP(Q*0x100+0x010+0x00C*P+E)

    rt_real*fco2;
#define inf_FCO2            DP(Q*0x100+0x010+0x010*P+E)

    rt_real*fso1;
#define inf_FSO1            DP(Q*0x100+0x010+0x014*P+E)

    rt_real*fso2;
#define inf_FSO2            DP(Q*0x100+0x010+0x018*P+E)

    /* integer arrays */

    rt_elem*iar0;
#define inf_IAR0            DP(Q*0x100+0x010+0x01C*P+E)

    rt_elem*ico1;
#define inf_ICO1            DP(Q*0x100+0x010+0x020*P+E)

    rt_elem*ico2;
#define inf_ICO2            DP(Q*0x100+0x010+0x024*P+E)

    rt_elem*iso1;
#define inf_ISO1            DP(Q*0x100+0x010+0x028*P+E)

    rt_elem*iso2;
#define inf_ISO2            DP(Q*0x100+0x010+0x02C*P+E)

};

/*
 * SIMD offsets within array.
 */
#define AJ0                 DP(Q*0x000)
#define AJ1                 DP(Q*0x010)
#define AJ2                 DP(Q*0x020)

/******************************************************************************/
/******************************   RUN LEVEL  1   ******************************/
/******************************************************************************/

#if RUN_LEVEL >=  1

rt_void c_test01(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_real *far0 = info->far0;
    rt_real *fco1 = info->fco1;
    rt_real *fco2 = info->fco2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            fco1[j] = far0[j] + far0[(j + S) % n];
            fco2[j] = far0[j] - far0[(j + S) % n];
        }
    }
}

rt_void s_test01(rt_SIMD_INFOX *info)
{
    rt_si32 i;

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

rt_void p_test01(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_real *far0 = info->far0;
    rt_real *fco1 = info->fco1;
    rt_real *fco2 = info->fco2;
    rt_real *fso1 = info->fso1;
    rt_real *fso2 = info->fso2;

    j = n;
    while (j-->0)
    {
        if (FEQ(fco1[j], fso1[j]) && FEQ(fco2[j], fso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e, farr[%d] = %e\n",
                j, far0[j], (j + S) % n, far0[(j + S) % n]);

        RT_LOGI("C farr[%d]+farr[%d] = %e, farr[%d]-farr[%d] = %e\n",
                j, (j + S) % n, fco1[j], j, (j + S) % n, fco2[j]);

        RT_LOGI("S farr[%d]+farr[%d] = %e, farr[%d]-farr[%d] = %e\n",
                j, (j + S) % n, fso1[j], j, (j + S) % n, fso2[j]);
    }
}

#endif /* RUN_LEVEL  1 */

/******************************************************************************/
/******************************   RUN LEVEL  2   ******************************/
/******************************************************************************/

#if RUN_LEVEL >=  2

rt_void c_test02(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_real *far0 = info->far0;
    rt_real *fco1 = info->fco1;
    rt_real *fco2 = info->fco2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            fco1[j] = far0[j] * far0[(j + S) % n];
            fco2[j] = far0[j] / far0[(j + S) % n];
        }
    }
}

rt_void s_test02(rt_SIMD_INFOX *info)
{
    rt_si32 i;

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

rt_void p_test02(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_real *far0 = info->far0;
    rt_real *fco1 = info->fco1;
    rt_real *fco2 = info->fco2;
    rt_real *fso1 = info->fso1;
    rt_real *fso2 = info->fso2;

    j = n;
    while (j-->0)
    {
        if (FEQ(fco1[j], fso1[j]) && FEQ(fco2[j], fso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e, farr[%d] = %e\n",
                j, far0[j], (j + S) % n, far0[(j + S) % n]);

        RT_LOGI("C farr[%d]*farr[%d] = %e, farr[%d]/farr[%d] = %e\n",
                j, (j + S) % n, fco1[j], j, (j + S) % n, fco2[j]);

        RT_LOGI("S farr[%d]*farr[%d] = %e, farr[%d]/farr[%d] = %e\n",
                j, (j + S) % n, fso1[j], j, (j + S) % n, fso2[j]);
    }
}

#endif /* RUN_LEVEL  2 */

/******************************************************************************/
/******************************   RUN LEVEL  3   ******************************/
/******************************************************************************/

#if RUN_LEVEL >=  3

rt_void c_test03(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_real *far0 = info->far0;
    rt_elem *ico1 = info->ico1;
    rt_elem *ico2 = info->ico2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = (far0[j] >  far0[(j + S) % n]) ? -1 : 0;
            ico2[j] = (far0[j] >= far0[(j + S) % n]) ? -1 : 0;
        }
    }
}

rt_void s_test03(rt_SIMD_INFOX *info)
{
    rt_si32 i;

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

rt_void p_test03(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_real *far0 = info->far0;
    rt_elem *ico1 = info->ico1;
    rt_elem *ico2 = info->ico2;
    rt_elem *iso1 = info->iso1;
    rt_elem *iso2 = info->iso2;

    j = n;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e, farr[%d] = %e\n",
                j, far0[j], (j + S) % n, far0[(j + S) % n]);

        RT_LOGI("C (farr[%d]>!farr[%d]) = %"PR_L"X, "
                  "(farr[%d]>=farr[%d]) = %"PR_L"X\n",
                j, (j + S) % n, ico1[j], j, (j + S) % n, ico2[j]);

        RT_LOGI("S (farr[%d]>!farr[%d]) = %"PR_L"X, "
                  "(farr[%d]>=farr[%d]) = %"PR_L"X\n",
                j, (j + S) % n, iso1[j], j, (j + S) % n, iso2[j]);
    }
}

#endif /* RUN_LEVEL  3 */

/******************************************************************************/
/******************************   RUN LEVEL  4   ******************************/
/******************************************************************************/

#if RUN_LEVEL >=  4

rt_void c_test04(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_real *far0 = info->far0;
    rt_elem *ico1 = info->ico1;
    rt_elem *ico2 = info->ico2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = (far0[j] <  far0[(j + S) % n]) ? -1 : 0;
            ico2[j] = (far0[j] <= far0[(j + S) % n]) ? -1 : 0;
        }
    }
}

rt_void s_test04(rt_SIMD_INFOX *info)
{
    rt_si32 i;

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

rt_void p_test04(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_real *far0 = info->far0;
    rt_elem *ico1 = info->ico1;
    rt_elem *ico2 = info->ico2;
    rt_elem *iso1 = info->iso1;
    rt_elem *iso2 = info->iso2;

    j = n;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e, farr[%d] = %e\n",
                j, far0[j], (j + S) % n, far0[(j + S) % n]);

        RT_LOGI("C (farr[%d]<!farr[%d]) = %"PR_L"X, "
                  "(farr[%d]<=farr[%d]) = %"PR_L"X\n",
                j, (j + S) % n, ico1[j], j, (j + S) % n, ico2[j]);

        RT_LOGI("S (farr[%d]<!farr[%d]) = %"PR_L"X, "
                  "(farr[%d]<=farr[%d]) = %"PR_L"X\n",
                j, (j + S) % n, iso1[j], j, (j + S) % n, iso2[j]);
    }
}

#endif /* RUN_LEVEL  4 */

/******************************************************************************/
/******************************   RUN LEVEL  5   ******************************/
/******************************************************************************/

#if RUN_LEVEL >=  5

rt_void c_test05(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_real *far0 = info->far0;
    rt_elem *ico1 = info->ico1;
    rt_elem *ico2 = info->ico2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = (far0[j] == far0[(j + S) % n]) ? -1 : 0;
            ico2[j] = (far0[j] != far0[(j + S) % n]) ? -1 : 0;
        }
    }
}

rt_void s_test05(rt_SIMD_INFOX *info)
{
    rt_si32 i;

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

rt_void p_test05(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_real *far0 = info->far0;
    rt_elem *ico1 = info->ico1;
    rt_elem *ico2 = info->ico2;
    rt_elem *iso1 = info->iso1;
    rt_elem *iso2 = info->iso2;

    j = n;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e, farr[%d] = %e\n",
                j, far0[j], (j + S) % n, far0[(j + S) % n]);

        RT_LOGI("C (farr[%d]==farr[%d]) = %"PR_L"X, "
                  "(farr[%d]!=farr[%d]) = %"PR_L"X\n",
                j, (j + S) % n, ico1[j], j, (j + S) % n, ico2[j]);

        RT_LOGI("S (farr[%d]==farr[%d]) = %"PR_L"X, "
                  "(farr[%d]!=farr[%d]) = %"PR_L"X\n",
                j, (j + S) % n, iso1[j], j, (j + S) % n, iso2[j]);
    }
}

#endif /* RUN_LEVEL  5 */

/******************************************************************************/
/******************************   RUN LEVEL  6   ******************************/
/******************************************************************************/

#if RUN_LEVEL >=  6

rt_void c_test06(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_real *far0 = info->far0;
    rt_elem *iar0 = info->iar0;
    rt_elem *ico1 = info->ico1;
    rt_real *fco2 = info->fco2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = (rt_elem)far0[j];
            fco2[j] = (rt_real)iar0[j];
        }
    }
}

rt_void s_test06(rt_SIMD_INFOX *info)
{
    rt_si32 i;

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Resi, Mebp, inf_IAR0)
        movxx_ld(Redx, Mebp, inf_ISO1)
        movxx_ld(Rebx, Mebp, inf_FSO2)

        movpx_ld(Xmm0, Mecx, AJ0)
        movpx_ld(Xmm1, Mesi, AJ0)
        cvzps_rr(Xmm2, Xmm0)
        cvnpn_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)

        movpx_ld(Xmm0, Mecx, AJ1)
        movpx_ld(Xmm1, Mesi, AJ1)
        cvzps_rr(Xmm2, Xmm0)
        cvnpn_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)

        movpx_ld(Xmm0, Mecx, AJ2)
        movpx_ld(Xmm1, Mesi, AJ2)
        cvzps_rr(Xmm2, Xmm0)
        cvnpn_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void p_test06(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_real *far0 = info->far0;
    rt_elem *iar0 = info->iar0;
    rt_elem *ico1 = info->ico1;
    rt_real *fco2 = info->fco2;
    rt_elem *iso1 = info->iso1;
    rt_real *fso2 = info->fso2;

    j = n;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && FEQ(fco2[j], fso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e, iarr[%d] = %"PR_L"d\n",
                j, far0[j], j, iar0[j]);

        RT_LOGI("C (rt_elem)farr[%d] = %"PR_L"d, (rt_real)iarr[%d] = %e\n",
                j, ico1[j], j, fco2[j]);

        RT_LOGI("S (rt_elem)farr[%d] = %"PR_L"d, (rt_real)iarr[%d] = %e\n",
                j, iso1[j], j, fso2[j]);
    }
}

#endif /* RUN_LEVEL  6 */

/******************************************************************************/
/******************************   RUN LEVEL  7   ******************************/
/******************************************************************************/

#if RUN_LEVEL >=  7

rt_void c_test07(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_real *far0 = info->far0;
    rt_real *fco1 = info->fco1;
    rt_real *fco2 = info->fco2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            fco1[j] = RT_SQRT(far0[j]);
            fco2[j] = 1.0 / far0[j];
        }
    }
}

rt_void s_test07(rt_SIMD_INFOX *info)
{
    rt_si32 i;

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Redx, Mebp, inf_FSO1)
        movxx_ld(Rebx, Mebp, inf_FSO2)

        movpx_ld(Xmm0, Mecx, AJ0)
        sqrps_rr(Xmm2, Xmm0)
        rcpps_rr(Xmm3, Xmm0) /* destroys Xmm0 */
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)

        movpx_ld(Xmm0, Mecx, AJ1)
        sqrps_rr(Xmm2, Xmm0)
        rcpps_rr(Xmm3, Xmm0) /* destroys Xmm0 */
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)

        movpx_ld(Xmm0, Mecx, AJ2)
        sqrps_rr(Xmm2, Xmm0)
        rcpps_rr(Xmm3, Xmm0) /* destroys Xmm0 */
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void p_test07(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_real *far0 = info->far0;
    rt_real *fco1 = info->fco1;
    rt_real *fco2 = info->fco2;
    rt_real *fso1 = info->fso1;
    rt_real *fso2 = info->fso2;

    j = n;
    while (j-->0)
    {
        if (FEQ(fco1[j], fso1[j]) && FEQ(fco2[j], fso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e\n",
                j, far0[j]);

        RT_LOGI("C RT_SQRT(farr[%d]) = %e, 1.0/farr[%d] = %e\n",
                j, fco1[j], j, fco2[j]);

        RT_LOGI("S RT_SQRT(farr[%d]) = %e, 1.0/farr[%d] = %e\n",
                j, fso1[j], j, fso2[j]);
    }
}

#endif /* RUN_LEVEL  7 */

/******************************************************************************/
/******************************   RUN LEVEL  8   ******************************/
/******************************************************************************/

#if RUN_LEVEL >=  8

rt_void c_test08(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_elem *iar0 = info->iar0;
    rt_elem *ico1 = info->ico1;
    rt_elem *ico2 = info->ico2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = iar0[j] + ((rt_uelm)+iar0[j] << 1);
            ico2[j] = iar0[j] - ((rt_uelm)-iar0[j] >> 2);
        }
    }
}

rt_void s_test08(rt_SIMD_INFOX *info)
{
    rt_si32 i;

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Resi, Mebp, inf_IAR0)
        movxx_ld(Redx, Mebp, inf_ISO1)
        movxx_ld(Rebx, Mebp, inf_ISO2)

        movpx_ld(Xmm0, Mesi, AJ0)
        movpx_ld(Xmm3, Mesi, AJ0)
        movpx_rr(Xmm2, Xmm0)
        shlpx_ri(Xmm0, IB(1))
        addpx_rr(Xmm2, Xmm0)
        xorpx_rr(Xmm0, Xmm0)
        subpx_rr(Xmm0, Xmm3)
        shrpx_ri(Xmm0, IB(2))
        subpx_rr(Xmm3, Xmm0)
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ0)
        movyx_ld(Recx, Mesi, AJ0)
        movyx_rr(Redi, Reax)
        mulyx_ri(Redi, IB(2))
        movyx_rr(Reax, Redi)
        addyx_rr(Reax, Recx)
        movyx_st(Reax, Medx, AJ0)
        movyx_rr(Reax, Recx)
        negyx_rx(Reax)
        shryx_ri(Reax, IB(2))
        subyx_rr(Recx, Reax)
        movyx_st(Recx, Mebx, AJ0)
#endif /* RT_BASE_TEST */

        movpx_ld(Xmm0, Mesi, AJ1)
        movpx_ld(Xmm3, Mesi, AJ1)
        movpx_rr(Xmm2, Xmm0)
        shlpx_ri(Xmm0, IB(1))
        addpx_rr(Xmm2, Xmm0)
        xorpx_rr(Xmm0, Xmm0)
        subpx_rr(Xmm0, Xmm3)
        shrpx_ri(Xmm0, IB(2))
        subpx_rr(Xmm3, Xmm0)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ1)
        movyx_ld(Recx, Mesi, AJ1)
        movyx_st(Reax, Medx, AJ1)
        shlyx_mi(Medx, AJ1, IB(1))
        addyx_st(Recx, Medx, AJ1)
        movyx_st(Recx, Mebx, AJ1)
        negyx_mx(Mebx, AJ1)
        shryx_mi(Mebx, AJ1, IB(2))
        movyx_ld(Reax, Mebx, AJ1)
        movyx_st(Recx, Mebx, AJ1)
        subyx_st(Reax, Mebx, AJ1)
#endif /* RT_BASE_TEST */

        movpx_ld(Xmm0, Mesi, AJ2)
        movpx_ld(Xmm3, Mesi, AJ2)
        movpx_rr(Xmm2, Xmm0)
        shlpx_ri(Xmm0, IB(1))
        addpx_rr(Xmm2, Xmm0)
        xorpx_rr(Xmm0, Xmm0)
        subpx_rr(Xmm0, Xmm3)
        shrpx_ri(Xmm0, IB(2))
        subpx_rr(Xmm3, Xmm0)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ2)
        movyx_ld(Recx, Mesi, AJ2)
        shlyx_ri(Reax, IB(1))
        addyx_rr(Reax, Recx)
        movyx_st(Reax, Medx, AJ2)
        movyx_rr(Reax, Recx)
        negyx_rx(Reax)
        shryx_ri(Reax, IB(2))
        subyx_rr(Recx, Reax)
        movyx_st(Recx, Mebx, AJ2)
#endif /* RT_BASE_TEST */

        ASM_LEAVE(info)
    }
}

rt_void p_test08(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_elem *iar0 = info->iar0;
    rt_elem *ico1 = info->ico1;
    rt_elem *ico2 = info->ico2;
    rt_elem *iso1 = info->iso1;
    rt_elem *iso2 = info->iso2;

    j = n;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("iarr[%d] = %"PR_L"d\n",
                j, iar0[j]);

        RT_LOGI("C iarr[%d]+((rt_uelm)+iarr[%d]<<1) = %"PR_L"d, "
                  "iarr[%d]-((rt_uelm)-iarr[%d]>>2) = %"PR_L"d\n",
                j, j, ico1[j], j, j, ico2[j]);

        RT_LOGI("S iarr[%d]+((rt_uelm)+iarr[%d]<<1) = %"PR_L"d, "
                  "iarr[%d]-((rt_uelm)-iarr[%d]>>2) = %"PR_L"d\n",
                j, j, iso1[j], j, j, iso2[j]);
    }
}

#endif /* RUN_LEVEL  8 */

/******************************************************************************/
/******************************   RUN LEVEL  9   ******************************/
/******************************************************************************/

#if RUN_LEVEL >=  9

rt_void c_test09(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_elem *iar0 = info->iar0;
    rt_elem *ico1 = info->ico1;
    rt_elem *ico2 = info->ico2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = iar0[j] * iar0[(j + S) % n];
            ico2[j] = iar0[j] / iar0[(j + S) % n];
        }
    }
}

rt_void s_test09(rt_SIMD_INFOX *info)
{
    ASM_ENTER(info)

        label_st(cyc_beg,
                 Mebp, inf_LABEL)

        movwx_ld(Reax, Mebp, inf_CYC)
        movwx_st(Reax, Mebp, inf_LOC)

    LBL(cyc_beg)

        movxx_ld(Recx, Mebp, inf_IAR0)
        movxx_ld(Rebx, Mebp, inf_ISO1)
        movxx_ld(Resi, Mebp, inf_ISO2)
        movwx_ld(Redi, Mebp, inf_SIZE)

    LBL(loc_beg)

        movyx_ld(Reax, Mecx, DP(Q*0x000))
        mulyn_xm(Mecx, DP(Q*0x010))
        movyx_st(Reax, Mebx, DP(Q*0x000))
        movyx_ld(Reax, Mecx, DP(Q*0x000))
        preyn_xx()
        divyn_xm(Mecx, DP(Q*0x010))
        movyx_st(Reax, Mesi, DP(Q*0x000))

        addxx_ri(Recx, IB(4*L))
        addxx_ri(Rebx, IB(4*L))
        addxx_ri(Resi, IB(4*L))
        subwx_ri(Redi, IB(1))
        cmjwx_ri(Redi, IB(S),
        /* if */ GT_x, loc_beg)

        movxx_ld(Redi, Mebp, inf_IAR0)
        movwx_mi(Mebp, inf_SIMD, IB(S))

    LBL(smd_beg)

        movyx_ld(Reax, Mecx, DP(Q*0x000))
        mulyn_xm(Medi, DP(Q*0x000))
        movyx_st(Reax, Mebx, DP(Q*0x000))
        movyx_ld(Reax, Mecx, DP(Q*0x000))
        preyn_xx()
        divyn_xm(Medi, DP(Q*0x000))
        movyx_st(Reax, Mesi, DP(Q*0x000))

        addxx_ri(Recx, IB(4*L))
        addxx_ri(Rebx, IB(4*L))
        addxx_ri(Resi, IB(4*L))
        addxx_ri(Redi, IB(4*L))
        subwx_mi(Mebp, inf_SIMD, IB(1))
        cmjwx_mz(Mebp, inf_SIMD,
        /* if */ GT_x, smd_beg)

        subwx_mi(Mebp, inf_LOC, IB(1))
        cmjwx_mz(Mebp, inf_LOC,
        /* if */ EQ_x, cyc_end)
        jmpxx_mm(Mebp, inf_LABEL)
        jmpxx_lb(cyc_beg) /* the same jump as above */

    LBL(cyc_end)

    ASM_LEAVE(info)
}

rt_void p_test09(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_elem *iar0 = info->iar0;
    rt_elem *ico1 = info->ico1;
    rt_elem *ico2 = info->ico2;
    rt_elem *iso1 = info->iso1;
    rt_elem *iso2 = info->iso2;

    j = n;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("iarr[%d] = %"PR_L"d, iarr[%d] = %"PR_L"d\n",
                j, iar0[j], (j + S) % n, iar0[(j + S) % n]);

        RT_LOGI("C iarr[%d]*iarr[%d] = %"PR_L"d, "
                  "iarr[%d]/iarr[%d] = %"PR_L"d\n",
                j, (j + S) % n, ico1[j], j, (j + S) % n, ico2[j]);

        RT_LOGI("S iarr[%d]*iarr[%d] = %"PR_L"d, "
                  "iarr[%d]/iarr[%d] = %"PR_L"d\n",
                j, (j + S) % n, iso1[j], j, (j + S) % n, iso2[j]);
    }
}

#endif /* RUN_LEVEL  9 */

/******************************************************************************/
/******************************   RUN LEVEL 10   ******************************/
/******************************************************************************/

#if RUN_LEVEL >= 10

rt_void c_test10(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_real *far0 = info->far0;
    rt_real *fco1 = info->fco1;
    rt_real *fco2 = info->fco2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            fco1[j] = RT_MIN(far0[j], far0[(j + S) % n]);
            fco2[j] = RT_MAX(far0[j], far0[(j + S) % n]);
        }
    }
}

rt_void s_test10(rt_SIMD_INFOX *info)
{
    rt_si32 i;

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
        minps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        maxps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)

        movpx_ld(Xmm0, Mecx, AJ1)
        movpx_ld(Xmm1, Mecx, AJ2)
        movpx_rr(Xmm2, Xmm0)
        minps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        maxps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)

        movpx_ld(Xmm0, Mecx, AJ2)
        movpx_ld(Xmm1, Mecx, AJ0)
        movpx_rr(Xmm2, Xmm0)
        minps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        maxps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void p_test10(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_real *far0 = info->far0;
    rt_real *fco1 = info->fco1;
    rt_real *fco2 = info->fco2;
    rt_real *fso1 = info->fso1;
    rt_real *fso2 = info->fso2;

    j = n;
    while (j-->0)
    {
        if (FEQ(fco1[j], fso1[j]) && FEQ(fco2[j], fso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e, farr[%d] = %e\n",
                j, far0[j], (j + S) % n, far0[(j + S) % n]);

        RT_LOGI("C MIN(farr[%d],farr[%d]) = %e, MAX(farr[%d],farr[%d]) = %e\n",
                j, (j + S) % n, fco1[j], j, (j + S) % n, fco2[j]);

        RT_LOGI("S MIN(farr[%d],farr[%d]) = %e, MAX(farr[%d],farr[%d]) = %e\n",
                j, (j + S) % n, fso1[j], j, (j + S) % n, fso2[j]);
    }
}

#endif /* RUN_LEVEL 10 */

/******************************************************************************/
/******************************   RUN LEVEL 11   ******************************/
/******************************************************************************/

#if RUN_LEVEL >= 11

rt_void c_test11(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_elem *iar0 = info->iar0;
    rt_elem *ico1 = info->ico1;
    rt_elem *ico2 = info->ico2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = iar0[j] | (iar0[j] << 7);
            ico2[j] = iar0[j] ^ (iar0[j] >> 3);
        }
    }
}

rt_void s_test11(rt_SIMD_INFOX *info)
{
    rt_si32 i;

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
        shlpx_ri(Xmm0, IB(7))
        orrpx_rr(Xmm2, Xmm0)
        movpx_rr(Xmm3, Xmm1)
        shrpx_ri(Xmm1, IB(3))
        xorpx_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ0)
        movyx_ld(Recx, Mesi, AJ0)
        shlyx_ri(Reax, IB(7))
        orryx_rr(Reax, Recx)
        movyx_st(Reax, Medx, AJ0)
        movyx_rr(Reax, Recx)
        shryx_ri(Reax, IB(3))
        xoryx_rr(Reax, Recx)
        movyx_st(Reax, Mebx, AJ0)
#endif /* RT_BASE_TEST */

        movpx_ld(Xmm0, Mesi, AJ1)
        movpx_ld(Xmm1, Mesi, AJ1)
        movpx_rr(Xmm2, Xmm0)
        shlpx_ri(Xmm0, IB(7))
        orrpx_rr(Xmm2, Xmm0)
        movpx_rr(Xmm3, Xmm1)
        shrpx_ri(Xmm1, IB(3))
        xorpx_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ1)
        movyx_ld(Recx, Mesi, AJ1)
        movyx_st(Reax, Medx, AJ1)
        shlyx_mi(Medx, AJ1, IB(7))
        orryx_st(Recx, Medx, AJ1)
        movyx_st(Recx, Mebx, AJ1)
        shryx_mi(Mebx, AJ1, IB(3))
        xoryx_st(Recx, Mebx, AJ1)
#endif /* RT_BASE_TEST */

        movpx_ld(Xmm0, Mesi, AJ2)
        movpx_ld(Xmm1, Mesi, AJ2)
        movpx_rr(Xmm2, Xmm0)
        shlpx_ri(Xmm0, IB(7))
        orrpx_rr(Xmm2, Xmm0)
        movpx_rr(Xmm3, Xmm1)
        shrpx_ri(Xmm1, IB(3))
        xorpx_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ2)
        movyx_ld(Recx, Mesi, AJ2)
        shlyx_ri(Reax, IB(7))
        orryx_rr(Reax, Recx)
        movyx_st(Reax, Medx, AJ2)
        movyx_st(Recx, Mebx, AJ2)
        shryx_mi(Mebx, AJ2, IB(3))
        xoryx_ld(Recx, Mebx, AJ2)
        movyx_st(Recx, Mebx, AJ2)
#endif /* RT_BASE_TEST */

        ASM_LEAVE(info)
    }
}

rt_void p_test11(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_elem *iar0 = info->iar0;
    rt_elem *ico1 = info->ico1;
    rt_elem *ico2 = info->ico2;
    rt_elem *iso1 = info->iso1;
    rt_elem *iso2 = info->iso2;

    j = n;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("iarr[%d] = %"PR_L"d\n",
                j, iar0[j]);

        RT_LOGI("C iarr[%d]|(iarr[%d]<<7) = %"PR_L"d, "
                  "iarr[%d]^(iarr[%d]>>3) = %"PR_L"d\n",
                j, j, ico1[j], j, j, ico2[j]);

        RT_LOGI("S iarr[%d]|(iarr[%d]<<7) = %"PR_L"d, "
                  "iarr[%d]^(iarr[%d]>>3) = %"PR_L"d\n",
                j, j, iso1[j], j, j, iso2[j]);
    }
}

#endif /* RUN_LEVEL 11 */

/******************************************************************************/
/******************************   RUN LEVEL 12   ******************************/
/******************************************************************************/

#if RUN_LEVEL >= 12

rt_void c_test12(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_elem *iar0 = info->iar0;
    rt_elem *ico1 = info->ico1;
    rt_elem *ico2 = info->ico2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] =  iar0[j] & (iar0[j] << 7);
            ico2[j] = ~iar0[j] & (iar0[j] >> 3);
        }
    }
}

rt_void s_test12(rt_SIMD_INFOX *info)
{
    rt_si32 i;

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
        shlpx_ri(Xmm0, IB(7))
        andpx_rr(Xmm2, Xmm0)
        movpx_rr(Xmm3, Xmm1)
        shrpx_ri(Xmm1, IB(3))
        annpx_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ0)
        movyx_ld(Recx, Mesi, AJ0)
        shlyx_ri(Reax, IB(7))
        andyx_rr(Reax, Recx)
        movyx_st(Reax, Medx, AJ0)
        movyx_rr(Reax, Recx)
        shryx_ri(Recx, IB(3))
        notyx_rx(Reax)
        andyx_rr(Recx, Reax)
        movyx_st(Recx, Mebx, AJ0)
#endif /* RT_BASE_TEST */

        movpx_ld(Xmm0, Mesi, AJ1)
        movpx_ld(Xmm1, Mesi, AJ1)
        movpx_rr(Xmm2, Xmm0)
        shlpx_ri(Xmm0, IB(7))
        andpx_rr(Xmm2, Xmm0)
        movpx_rr(Xmm3, Xmm1)
        shrpx_ri(Xmm1, IB(3))
        annpx_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ1)
        movyx_ld(Recx, Mesi, AJ1)
        movyx_st(Reax, Medx, AJ1)
        shlyx_mi(Medx, AJ1, IB(7))
        andyx_st(Recx, Medx, AJ1)
        movyx_st(Recx, Mebx, AJ1)
        notyx_mx(Mebx, AJ1)
        movyx_ld(Reax, Mebx, AJ1)
        movyx_st(Recx, Mebx, AJ1)
        shryx_mi(Mebx, AJ1, IB(3))
        andyx_st(Reax, Mebx, AJ1)
#endif /* RT_BASE_TEST */

        movpx_ld(Xmm0, Mesi, AJ2)
        movpx_ld(Xmm1, Mesi, AJ2)
        movpx_rr(Xmm2, Xmm0)
        shlpx_ri(Xmm0, IB(7))
        andpx_rr(Xmm2, Xmm0)
        movpx_rr(Xmm3, Xmm1)
        shrpx_ri(Xmm1, IB(3))
        annpx_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ2)
        movyx_ld(Recx, Mesi, AJ2)
        shlyx_ri(Reax, IB(7))
        andyx_rr(Reax, Recx)
        movyx_st(Reax, Medx, AJ2)
        movyx_rr(Reax, Recx)
        shryx_ri(Recx, IB(3))
        notyx_rx(Reax)
        andyx_rr(Recx, Reax)
        movyx_st(Recx, Mebx, AJ2)
#endif /* RT_BASE_TEST */

        ASM_LEAVE(info)
    }
}

rt_void p_test12(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_elem *iar0 = info->iar0;
    rt_elem *ico1 = info->ico1;
    rt_elem *ico2 = info->ico2;
    rt_elem *iso1 = info->iso1;
    rt_elem *iso2 = info->iso2;

    j = n;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("iarr[%d] = %"PR_L"d\n",
                j, iar0[j]);

        RT_LOGI("C iarr[%d]&(iarr[%d]<<7) = %"PR_L"d, "
                 "~iarr[%d]&(iarr[%d]>>3) = %"PR_L"d\n",
                j, j, ico1[j], j, j, ico2[j]);

        RT_LOGI("S iarr[%d]&(iarr[%d]<<7) = %"PR_L"d, "
                 "~iarr[%d]&(iarr[%d]>>3) = %"PR_L"d\n",
                j, j, iso1[j], j, j, iso2[j]);
    }
}

#endif /* RUN_LEVEL 12 */

/******************************************************************************/
/******************************   RUN LEVEL 13   ******************************/
/******************************************************************************/

#if RUN_LEVEL >= 13

rt_void c_test13(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_real *far0 = info->far0;
    rt_real *fco1 = info->fco1;
    rt_real *fco2 = info->fco2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            fco1[j] = RT_POW(far0[j], 1.0 / 3.0);
            fco2[j] = 1.0 / RT_SQRT(far0[j]);
        }
    }
}

rt_void s_test13(rt_SIMD_INFOX *info)
{
    rt_si32 i;

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Redx, Mebp, inf_FSO1)
        movxx_ld(Rebx, Mebp, inf_FSO2)

        movpx_ld(Xmm0, Mecx, AJ0)
        cbrps_rr(Xmm2, Xmm5, Xmm6, Xmm0) /* destroys Xmm5, Xmm6 */
        rsqps_rr(Xmm3, Xmm0) /* destroys Xmm0 */
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)

        movpx_ld(Xmm0, Mecx, AJ1)
        cbrps_rr(Xmm2, Xmm5, Xmm6, Xmm0) /* destroys Xmm5, Xmm6 */
        rsqps_rr(Xmm3, Xmm0) /* destroys Xmm0 */
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)

        movpx_ld(Xmm0, Mecx, AJ2)
        cbrps_rr(Xmm2, Xmm5, Xmm6, Xmm0) /* destroys Xmm5, Xmm6 */
        rsqps_rr(Xmm3, Xmm0) /* destroys Xmm0 */
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void p_test13(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_real *far0 = info->far0;
    rt_real *fco1 = info->fco1;
    rt_real *fco2 = info->fco2;
    rt_real *fso1 = info->fso1;
    rt_real *fso2 = info->fso2;

    j = n;
    while (j-->0)
    {
        if (FEQ(fco1[j], fso1[j]) && FEQ(fco2[j], fso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e\n",
                j, far0[j]);

        RT_LOGI("C RT_POW(farr[%d],1.0/3.0) = %e, 1.0/RT_SQRT(farr[%d]) = %e\n",
                j, fco1[j], j, fco2[j]);

        RT_LOGI("S RT_POW(farr[%d],1.0/3.0) = %e, 1.0/RT_SQRT(farr[%d]) = %e\n",
                j, fso1[j], j, fso2[j]);
    }
}

#endif /* RUN_LEVEL 13 */

/******************************************************************************/
/******************************   RUN LEVEL 14   ******************************/
/******************************************************************************/

#if RUN_LEVEL >= 14

rt_void c_test14(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, k, n = info->size;

    rt_real *far0 = info->far0;
    rt_elem *ico1 = info->ico1;
    rt_elem *ico2 = info->ico2;

    i = info->cyc;
    while (i-->0)
    {
        j = n / S;
        while (j-->0)
        {
            rt_si32 e = 0;

            k = S;
            while (k-->0)
            {
                e += (far0[j*S + k] == far0[((j+1)*S + k) % n]) ? 1 : 0;
            }

            k = S;
            while (k-->0)
            {
                ico1[j*S + k] = (e == 0) ? 0 : -1;
                ico2[j*S + k] = (e != 0) ? 0 : -1;
            }
        }
    }
}

rt_void s_test14(rt_SIMD_INFOX *info)
{
    rt_si32 i;

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Redx, Mebp, inf_ISO1)
        movxx_ld(Rebx, Mebp, inf_ISO2)

        /* 0th section */
        movpx_ld(Xmm0, Mecx, AJ0)
        movpx_ld(Xmm1, Mecx, AJ1)

        movpx_rr(Xmm2, Xmm0)
        ceqps_rr(Xmm2, Xmm1)
        CHECK_MASK(eq0_out, NONE, Xmm2)

        xorpx_rr(Xmm2, Xmm2)
        ceqps_rr(Xmm2, Xmm2)

    LBL(eq0_out)

        movpx_st(Xmm2, Medx, AJ0)

        movpx_rr(Xmm3, Xmm0)
        cneps_rr(Xmm3, Xmm1)
        CHECK_MASK(ne0_out, FULL, Xmm3)

        xorpx_rr(Xmm3, Xmm3)

    LBL(ne0_out)

        movpx_st(Xmm3, Mebx, AJ0)

        /* 1st section */
        movpx_ld(Xmm0, Mecx, AJ1)
        movpx_ld(Xmm1, Mecx, AJ2)

        movpx_rr(Xmm2, Xmm0)
        ceqps_rr(Xmm2, Xmm1)
        CHECK_MASK(eq1_out, NONE, Xmm2)

        xorpx_rr(Xmm2, Xmm2)
        ceqps_rr(Xmm2, Xmm2)

    LBL(eq1_out)

        movpx_st(Xmm2, Medx, AJ1)

        movpx_rr(Xmm3, Xmm0)
        cneps_rr(Xmm3, Xmm1)
        CHECK_MASK(ne1_out, FULL, Xmm3)

        xorpx_rr(Xmm3, Xmm3)

    LBL(ne1_out)

        movpx_st(Xmm3, Mebx, AJ1)

        /* 2nd section */
        movpx_ld(Xmm0, Mecx, AJ2)
        movpx_ld(Xmm1, Mecx, AJ0)

        movpx_rr(Xmm2, Xmm0)
        ceqps_rr(Xmm2, Xmm1)
        CHECK_MASK(eq2_out, NONE, Xmm2)

        xorpx_rr(Xmm2, Xmm2)
        ceqps_rr(Xmm2, Xmm2)

    LBL(eq2_out)

        movpx_st(Xmm2, Medx, AJ2)

        movpx_rr(Xmm3, Xmm0)
        cneps_rr(Xmm3, Xmm1)
        CHECK_MASK(ne2_out, FULL, Xmm3)

        xorpx_rr(Xmm3, Xmm3)

    LBL(ne2_out)

        movpx_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void p_test14(rt_SIMD_INFOX *info)
{
    rt_si32 j, k, n = info->size;

    rt_real *far0 = info->far0;
    rt_elem *ico1 = info->ico1;
    rt_elem *ico2 = info->ico2;
    rt_elem *iso1 = info->iso1;
    rt_elem *iso2 = info->iso2;

    j = n / S;
    while (j-->0)
    {
        rt_si32 e = 0;

        k = S;
        while (k-->0)
        {
            e += IEQ(ico1[j*S + k], iso1[j*S + k]) ? 1 : 0;
            e += IEQ(ico2[j*S + k], iso2[j*S + k]) ? 1 : 0;
        }

        if (e == 2*S && !v_mode)
        {
            continue;
        }

        k = S;
        while (k-->0)
        {
            RT_LOGI("farr[%d] = %e, farr[%d] = %e\n",
                    j*S + k, far0[j*S + k],
                    ((j+1)*S + k) % n, far0[((j+1)*S + k) % n]);
        }

        k = S;
        while (k-->0)
        {
            RT_LOGI("C (farr[%d]==farr[%d]) = %"PR_L"X, "
                      "(farr[%d]!=farr[%d]) = %"PR_L"X\n",
                    j*S + k, ((j+1)*S + k) % n, ico1[j*S + k],
                    j*S + k, ((j+1)*S + k) % n, ico2[j*S + k]);
        }

        k = S;
        while (k-->0)
        {
            RT_LOGI("S (farr[%d]==farr[%d]) = %"PR_L"X, "
                      "(farr[%d]!=farr[%d]) = %"PR_L"X\n",
                    j*S + k, ((j+1)*S + k) % n, iso1[j*S + k],
                    j*S + k, ((j+1)*S + k) % n, iso2[j*S + k]);
        }
    }
}

#endif /* RUN_LEVEL 14 */

/******************************************************************************/
/******************************   RUN LEVEL 15   ******************************/
/******************************************************************************/

#if RUN_LEVEL >= 15

rt_void c_test15(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_elem *iar0 = info->iar0;
    rt_elem *ico1 = info->ico1;
    rt_elem *ico2 = info->ico2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = +iar0[j] >> 3;
            ico2[j] = -iar0[j] >> 5;
        }
    }
}

rt_void s_test15(rt_SIMD_INFOX *info)
{
    rt_si32 i;

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Resi, Mebp, inf_IAR0)
        movxx_ld(Redx, Mebp, inf_ISO1)
        movxx_ld(Rebx, Mebp, inf_ISO2)

        movpx_ld(Xmm2, Mesi, AJ0)
        xorpx_rr(Xmm3, Xmm3)
        subpx_rr(Xmm3, Xmm2)
        shrpn_ri(Xmm2, IB(3))
        shrpn_ri(Xmm3, IB(5))
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ0)
        xoryx_rr(Recx, Recx)
        subyx_ld(Recx, Mesi, AJ0)
        shryn_ri(Reax, IB(3))
        shryn_ri(Recx, IB(5))
        movyx_st(Reax, Medx, AJ0)
        movyx_st(Recx, Mebx, AJ0)
#endif /* RT_BASE_TEST */

        movpx_ld(Xmm2, Mesi, AJ1)
        xorpx_rr(Xmm3, Xmm3)
        subpx_rr(Xmm3, Xmm2)
        shrpn_ri(Xmm2, IB(3))
        shrpn_ri(Xmm3, IB(5))
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ1)
        xoryx_rr(Recx, Recx)
        subyx_ld(Recx, Mesi, AJ1)
        movyx_st(Reax, Medx, AJ1)
        shryn_mi(Medx, AJ1, IB(3))
        movyx_st(Recx, Mebx, AJ1)
        shryn_mi(Mebx, AJ1, IB(5))
#endif /* RT_BASE_TEST */

        movpx_ld(Xmm2, Mesi, AJ2)
        xorpx_rr(Xmm3, Xmm3)
        subpx_rr(Xmm3, Xmm2)
        shrpn_ri(Xmm2, IB(3))
        shrpn_ri(Xmm3, IB(5))
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ2)
        xoryx_rr(Recx, Recx)
        subyx_ld(Recx, Mesi, AJ2)
        shryn_ri(Reax, IB(3))
        shryn_ri(Recx, IB(5))
        movyx_st(Reax, Medx, AJ2)
        movyx_st(Recx, Mebx, AJ2)
#endif /* RT_BASE_TEST */

        ASM_LEAVE(info)
    }
}

rt_void p_test15(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_elem *iar0 = info->iar0;
    rt_elem *ico1 = info->ico1;
    rt_elem *ico2 = info->ico2;
    rt_elem *iso1 = info->iso1;
    rt_elem *iso2 = info->iso2;

    j = n;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("iarr[%d] = %"PR_L"d\n",
                j, iar0[j]);

        RT_LOGI("C +iarr[%d]>>3 = %"PR_L"d, -iarr[%d]>>5 = %"PR_L"d\n",
                j, ico1[j], j, ico2[j]);

        RT_LOGI("S +iarr[%d]>>3 = %"PR_L"d, -iarr[%d]>>5 = %"PR_L"d\n",
                j, iso1[j], j, iso2[j]);
    }
}

#endif /* RUN_LEVEL 15 */

/******************************************************************************/
/******************************   RUN LEVEL 16   ******************************/
/******************************************************************************/

#if RUN_LEVEL >= 16

rt_void c_test16(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_elem *iar0 = info->iar0;
    rt_elem *ico1 = info->ico1;
    rt_elem *ico2 = info->ico2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = iar0[j] >> (iar0[(j/S)*S] & 0x1F);
            ico2[j] = iar0[j] << (iar0[(j/S)*S] & 0x1F);
        }
    }
}

rt_void s_test16(rt_SIMD_INFOX *info)
{
    rt_si32 i;

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Resi, Mebp, inf_IAR0)
        movxx_ld(Redx, Mebp, inf_ISO1)
        movxx_ld(Rebx, Mebp, inf_ISO2)

        movpx_ld(Xmm0, Mesi, AJ0)
        movpx_rr(Xmm2, Xmm0)
        movpx_rr(Xmm3, Xmm0)
        movyx_ld(Recx, Mesi, AJ0)
        andyx_ri(Recx, IB(0x1F))
        xorpx_rr(Xmm1, Xmm1)
        movpx_st(Xmm1, Mesi, AJ0)
        movyx_st(Recx, Mesi, AJ0)
        shrpn_ld(Xmm2, Mesi, AJ0)
        shlpx_ld(Xmm3, Mesi, AJ0)
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)
        movpx_st(Xmm0, Mesi, AJ0)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ0)
        shryn_rx(Reax)
        movyx_st(Reax, Medx, AJ0)
        movyx_ld(Reax, Mesi, AJ0)
        shlyx_rx(Reax)
        movyx_st(Reax, Mebx, AJ0)
#endif /* RT_BASE_TEST */

        movpx_ld(Xmm0, Mesi, AJ1)
        movpx_rr(Xmm2, Xmm0)
        movpx_rr(Xmm3, Xmm0)
        movyx_ld(Recx, Mesi, AJ1)
        andyx_ri(Recx, IB(0x1F))
        xorpx_rr(Xmm1, Xmm1)
        movpx_st(Xmm1, Mesi, AJ1)
        movyx_st(Recx, Mesi, AJ1)
        shrpn_ld(Xmm2, Mesi, AJ1)
        shlpx_ld(Xmm3, Mesi, AJ1)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)
        movpx_st(Xmm0, Mesi, AJ1)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ1)
        movyx_st(Reax, Medx, AJ1)
        shryn_mx(Medx, AJ1)
        movyx_ld(Reax, Mesi, AJ1)
        movyx_st(Reax, Mebx, AJ1)
        shlyx_mx(Mebx, AJ1)
#endif /* RT_BASE_TEST */

        movpx_ld(Xmm0, Mesi, AJ2)
        movpx_rr(Xmm2, Xmm0)
        movpx_rr(Xmm3, Xmm0)
        movyx_ld(Recx, Mesi, AJ2)
        andyx_ri(Recx, IB(0x1F))
        xorpx_rr(Xmm1, Xmm1)
        movpx_st(Xmm1, Mesi, AJ2)
        movyx_st(Recx, Mesi, AJ2)
        shrpn_ld(Xmm2, Mesi, AJ2)
        shlpx_ld(Xmm3, Mesi, AJ2)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)
        movpx_st(Xmm0, Mesi, AJ2)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ2)
        shryn_rx(Reax)
        movyx_st(Reax, Medx, AJ2)
        movyx_ld(Reax, Mesi, AJ2)
        shlyx_rx(Reax)
        movyx_st(Reax, Mebx, AJ2)
#endif /* RT_BASE_TEST */

        ASM_LEAVE(info)
    }
}

rt_void p_test16(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_elem *iar0 = info->iar0;
    rt_elem *ico1 = info->ico1;
    rt_elem *ico2 = info->ico2;
    rt_elem *iso1 = info->iso1;
    rt_elem *iso2 = info->iso2;

    j = n;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("iarr[%d] = %"PR_L"d, iarr[%d] = %"PR_L"d\n",
                j, iar0[j], (j/S)*S, iar0[(j/S)*S]);

        RT_LOGI("C iarr[%d]>>iarr[%d] = %"PR_L"d, "
                  "iarr[%d]<<iarr[%d] = %"PR_L"d\n",
                j, (j/S)*S, ico1[j], j, (j/S)*S, ico2[j]);

        RT_LOGI("S iarr[%d]>>iarr[%d] = %"PR_L"d, "
                  "iarr[%d]<<iarr[%d] = %"PR_L"d\n",
                j, (j/S)*S, iso1[j], j, (j/S)*S, iso2[j]);
    }
}

#endif /* RUN_LEVEL 16 */

/******************************************************************************/
/******************************   RUN LEVEL 17   ******************************/
/******************************************************************************/

#if RUN_LEVEL >= 17

rt_void c_test17(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_real *far0 = info->far0;
    rt_real *fco1 = info->fco1;
    rt_real *fco2 = info->fco2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            fco1[j] = RT_CEIL(far0[j]);
            fco2[j] = RT_FLOOR(far0[j]);
        }
    }
}

rt_void s_test17(rt_SIMD_INFOX *info)
{
    rt_si32 i;

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Redx, Mebp, inf_FSO1)
        movxx_ld(Rebx, Mebp, inf_FSO2)

        movpx_ld(Xmm0, Mecx, AJ0)
        rnpps_rr(Xmm2, Xmm0)
        rnmps_rr(Xmm3, Xmm0)
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)

        movpx_ld(Xmm0, Mecx, AJ1)
        rnpps_rr(Xmm2, Xmm0)
        rnmps_rr(Xmm3, Xmm0)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)

        movpx_ld(Xmm0, Mecx, AJ2)
        rnpps_rr(Xmm2, Xmm0)
        rnmps_rr(Xmm3, Xmm0)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void p_test17(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_real *far0 = info->far0;
    rt_real *fco1 = info->fco1;
    rt_real *fco2 = info->fco2;
    rt_real *fso1 = info->fso1;
    rt_real *fso2 = info->fso2;

    j = n;
    while (j-->0)
    {
        if (FEQ(fco1[j], fso1[j]) && FEQ(fco2[j], fso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e\n",
                j, far0[j]);

        RT_LOGI("C RT_CEIL(farr[%d]) = %e, RT_FLOOR(farr[%d]) = %e\n",
                j, fco1[j], j, fco2[j]);

        RT_LOGI("S RT_CEIL(farr[%d]) = %e, RT_FLOOR(farr[%d]) = %e\n",
                j, fso1[j], j, fso2[j]);
    }
}

#endif /* RUN_LEVEL 17 */

/******************************************************************************/
/******************************   RUN LEVEL 18   ******************************/
/******************************************************************************/

#if RUN_LEVEL >= 18

rt_void c_test18(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_elem *iar0 = info->iar0;
    rt_elem *ico1 = info->ico1;
    rt_elem *ico2 = info->ico2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = iar0[j] * iar0[(j + S) % n];
            ico2[j] = iar0[j] % iar0[(j + S) % n];
        }
    }
}

rt_void s_test18(rt_SIMD_INFOX *info)
{
    ASM_ENTER(info)

        label_st(cyc_ini,
                 Mebp, inf_LABEL)

        movwx_ld(Reax, Mebp, inf_CYC)
        movwx_st(Reax, Mebp, inf_LOC)

    LBL(cyc_ini)

        movxx_ld(Recx, Mebp, inf_IAR0)
        movxx_ld(Rebx, Mebp, inf_ISO1)
        movxx_ld(Resi, Mebp, inf_ISO2)
        movwx_ld(Redi, Mebp, inf_SIZE)

    LBL(loc_ini)

        movyx_ld(Reax, Mecx, DP(Q*0x000))
        mulyn_xm(Mecx, DP(Q*0x010))
        movyx_st(Reax, Mebx, DP(Q*0x000))
        movyx_ld(Reax, Mecx, DP(Q*0x000))
        preyn_xx()
        remyn_xx()
        divyn_xm(Mecx, DP(Q*0x010))
        remyn_xm(Mecx, DP(Q*0x010))
        movyx_st(Redx, Mesi, DP(Q*0x000))

        addxx_ri(Recx, IB(4*L))
        addxx_ri(Rebx, IB(4*L))
        addxx_ri(Resi, IB(4*L))
        subwx_ri(Redi, IB(1))
        cmjwx_ri(Redi, IB(S),
        /* if */ GT_x, loc_ini)

        movxx_ld(Redi, Mebp, inf_IAR0)
        movwx_mi(Mebp, inf_SIMD, IB(S))

    LBL(smd_ini)

        movyx_ld(Reax, Mecx, DP(Q*0x000))
        mulyn_xm(Medi, DP(Q*0x000))
        movyx_st(Reax, Mebx, DP(Q*0x000))
        movyx_ld(Reax, Mecx, DP(Q*0x000))
        preyn_xx()
        remyn_xx()
        divyn_xm(Medi, DP(Q*0x000))
        remyn_xm(Mecx, DP(Q*0x010))
        movyx_st(Redx, Mesi, DP(Q*0x000))

        addxx_ri(Recx, IB(4*L))
        addxx_ri(Rebx, IB(4*L))
        addxx_ri(Resi, IB(4*L))
        addxx_ri(Redi, IB(4*L))
        subwx_mi(Mebp, inf_SIMD, IB(1))
        cmjwx_mz(Mebp, inf_SIMD,
        /* if */ GT_x, smd_ini)

        subwx_mi(Mebp, inf_LOC, IB(1))
        cmjwx_mz(Mebp, inf_LOC,
        /* if */ EQ_x, cyc_fin)
        jmpxx_mm(Mebp, inf_LABEL)
        jmpxx_lb(cyc_ini) /* the same jump as above */

    LBL(cyc_fin)

    ASM_LEAVE(info)
}

rt_void p_test18(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_elem *iar0 = info->iar0;
    rt_elem *ico1 = info->ico1;
    rt_elem *ico2 = info->ico2;
    rt_elem *iso1 = info->iso1;
    rt_elem *iso2 = info->iso2;

    j = n;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("iarr[%d] = %"PR_L"d, iarr[%d] = %"PR_L"d\n",
                j, iar0[j], (j + S) % n, iar0[(j + S) % n]);

        RT_LOGI("C iarr[%d]*iarr[%d] = %"PR_L"d, "
                  "iarr[%d]%%iarr[%d] = %"PR_L"d\n",
                j, (j + S) % n, ico1[j], j, (j + S) % n, ico2[j]);

        RT_LOGI("S iarr[%d]*iarr[%d] = %"PR_L"d, "
                  "iarr[%d]%%iarr[%d] = %"PR_L"d\n",
                j, (j + S) % n, iso1[j], j, (j + S) % n, iso2[j]);
    }
}

#endif /* RUN_LEVEL 18 */

/******************************************************************************/
/*********************************   TABLES   *********************************/
/******************************************************************************/

typedef rt_void (*testXX)(rt_SIMD_INFOX *);

testXX c_test[RUN_LEVEL] =
{
#if RUN_LEVEL >=  1
    c_test01,
#endif /* RUN_LEVEL  1 */

#if RUN_LEVEL >=  2
    c_test02,
#endif /* RUN_LEVEL  2 */

#if RUN_LEVEL >=  3
    c_test03,
#endif /* RUN_LEVEL  3 */

#if RUN_LEVEL >=  4
    c_test04,
#endif /* RUN_LEVEL  4 */

#if RUN_LEVEL >=  5
    c_test05,
#endif /* RUN_LEVEL  5 */

#if RUN_LEVEL >=  6
    c_test06,
#endif /* RUN_LEVEL  6 */

#if RUN_LEVEL >=  7
    c_test07,
#endif /* RUN_LEVEL  7 */

#if RUN_LEVEL >=  8
    c_test08,
#endif /* RUN_LEVEL  8 */

#if RUN_LEVEL >=  9
    c_test09,
#endif /* RUN_LEVEL  9 */

#if RUN_LEVEL >= 10
    c_test10,
#endif /* RUN_LEVEL 10 */

#if RUN_LEVEL >= 11
    c_test11,
#endif /* RUN_LEVEL 11 */

#if RUN_LEVEL >= 12
    c_test12,
#endif /* RUN_LEVEL 12 */

#if RUN_LEVEL >= 13
    c_test13,
#endif /* RUN_LEVEL 13 */

#if RUN_LEVEL >= 14
    c_test14,
#endif /* RUN_LEVEL 14 */

#if RUN_LEVEL >= 15
    c_test15,
#endif /* RUN_LEVEL 15 */

#if RUN_LEVEL >= 16
    c_test16,
#endif /* RUN_LEVEL 16 */

#if RUN_LEVEL >= 17
    c_test17,
#endif /* RUN_LEVEL 17 */

#if RUN_LEVEL >= 18
    c_test18,
#endif /* RUN_LEVEL 18 */
};

testXX s_test[RUN_LEVEL] =
{
#if RUN_LEVEL >=  1
    s_test01,
#endif /* RUN_LEVEL  1 */

#if RUN_LEVEL >=  2
    s_test02,
#endif /* RUN_LEVEL  2 */

#if RUN_LEVEL >=  3
    s_test03,
#endif /* RUN_LEVEL  3 */

#if RUN_LEVEL >=  4
    s_test04,
#endif /* RUN_LEVEL  4 */

#if RUN_LEVEL >=  5
    s_test05,
#endif /* RUN_LEVEL  5 */

#if RUN_LEVEL >=  6
    s_test06,
#endif /* RUN_LEVEL  6 */

#if RUN_LEVEL >=  7
    s_test07,
#endif /* RUN_LEVEL  7 */

#if RUN_LEVEL >=  8
    s_test08,
#endif /* RUN_LEVEL  8 */

#if RUN_LEVEL >=  9
    s_test09,
#endif /* RUN_LEVEL  9 */

#if RUN_LEVEL >= 10
    s_test10,
#endif /* RUN_LEVEL 10 */

#if RUN_LEVEL >= 11
    s_test11,
#endif /* RUN_LEVEL 11 */

#if RUN_LEVEL >= 12
    s_test12,
#endif /* RUN_LEVEL 12 */

#if RUN_LEVEL >= 13
    s_test13,
#endif /* RUN_LEVEL 13 */

#if RUN_LEVEL >= 14
    s_test14,
#endif /* RUN_LEVEL 14 */

#if RUN_LEVEL >= 15
    s_test15,
#endif /* RUN_LEVEL 15 */

#if RUN_LEVEL >= 16
    s_test16,
#endif /* RUN_LEVEL 16 */

#if RUN_LEVEL >= 17
    s_test17,
#endif /* RUN_LEVEL 17 */

#if RUN_LEVEL >= 18
    s_test18,
#endif /* RUN_LEVEL 18 */
};

testXX p_test[RUN_LEVEL] =
{
#if RUN_LEVEL >=  1
    p_test01,
#endif /* RUN_LEVEL  1 */

#if RUN_LEVEL >=  2
    p_test02,
#endif /* RUN_LEVEL  2 */

#if RUN_LEVEL >=  3
    p_test03,
#endif /* RUN_LEVEL  3 */

#if RUN_LEVEL >=  4
    p_test04,
#endif /* RUN_LEVEL  4 */

#if RUN_LEVEL >=  5
    p_test05,
#endif /* RUN_LEVEL  5 */

#if RUN_LEVEL >=  6
    p_test06,
#endif /* RUN_LEVEL  6 */

#if RUN_LEVEL >=  7
    p_test07,
#endif /* RUN_LEVEL  7 */

#if RUN_LEVEL >=  8
    p_test08,
#endif /* RUN_LEVEL  8 */

#if RUN_LEVEL >=  9
    p_test09,
#endif /* RUN_LEVEL  9 */

#if RUN_LEVEL >= 10
    p_test10,
#endif /* RUN_LEVEL 10 */

#if RUN_LEVEL >= 11
    p_test11,
#endif /* RUN_LEVEL 11 */

#if RUN_LEVEL >= 12
    p_test12,
#endif /* RUN_LEVEL 12 */

#if RUN_LEVEL >= 13
    p_test13,
#endif /* RUN_LEVEL 13 */

#if RUN_LEVEL >= 14
    p_test14,
#endif /* RUN_LEVEL 14 */

#if RUN_LEVEL >= 15
    p_test15,
#endif /* RUN_LEVEL 15 */

#if RUN_LEVEL >= 16
    p_test16,
#endif /* RUN_LEVEL 16 */

#if RUN_LEVEL >= 17
    p_test17,
#endif /* RUN_LEVEL 17 */

#if RUN_LEVEL >= 18
    p_test18,
#endif /* RUN_LEVEL 18 */
};

/******************************************************************************/
/**********************************   MAIN   **********************************/
/******************************************************************************/

rt_time get_time();

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
rt_si32 main(rt_si32 argc, rt_char *argv[])
{
    rt_si32 k;

    if (argc >= 2)
    {
        RT_LOGI("---------------------------------------------------------\n");
        RT_LOGI("Usage options are given below:\n");
        RT_LOGI(" -d n, override diff threshold, where n is new diff 0..9\n");
        RT_LOGI(" -v, enable verbose mode\n");
        RT_LOGI("options -d, -v can be combined\n");
        RT_LOGI("---------------------------------------------------------\n");
    }

    for (k = 1; k < argc; k++)
    {
        if (strcmp(argv[k], "-d") == 0 && ++k < argc)
        {
            t_diff = argv[k][0] - '0';
            if (strlen(argv[k]) == 1 && t_diff >= 0 && t_diff <= 9)
            {
                RT_LOGI("Diff threshold overriden: %d\n", t_diff);
            }
            else
            {
                RT_LOGI("Diff threshold value out of range\n");
                return 0;
            }
        }
        if (strcmp(argv[k], "-v") == 0 && !v_mode)
        {
            v_mode = RT_TRUE;
            RT_LOGI("Verbose mode enabled\n");
        }
    }

    rt_pntr marr = sys_alloc(10 * ARR_SIZE * sizeof(rt_elem) + MASK);
    memset(marr, 0, 10 * ARR_SIZE * sizeof(rt_elem) + MASK);
    rt_pntr mar0 = (rt_pntr)(((rt_full)marr + MASK) & ~MASK);

#if   RT_ELEMENT == 32
    rt_real farr[4*3] =
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
#elif RT_ELEMENT == 64
    rt_real farr[2*3] =
    {
        34.278543634635234534,
        0.6543485436532716935,
        0.0032467678456476578,
        0.6543485436532716935,
        8764.7534912638712638,
        0.0765376764546510983,
    };
#endif /* RT_ELEMENT */

    rt_real *far0 = (rt_real *)mar0 + ARR_SIZE * 0;
    rt_real *fco1 = (rt_real *)mar0 + ARR_SIZE * 1;
    rt_real *fco2 = (rt_real *)mar0 + ARR_SIZE * 2;
    rt_real *fso1 = (rt_real *)mar0 + ARR_SIZE * 3;
    rt_real *fso2 = (rt_real *)mar0 + ARR_SIZE * 4;

    for (k = 0; k < Q; k++)
    {
        memcpy(far0 + RT_ARR_SIZE(farr) * k, farr, sizeof(farr));
    }

#if   RT_ELEMENT == 32
    rt_elem iarr[4*3] =
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
#elif RT_ELEMENT == 64
    rt_elem iarr[2*3] =
    {
        28598736572364876L,
        65,
        38723648235427834L,
        71273568176523765L,
        2347875,
        76657654198239823L,
    };
#endif /* RT_ELEMENT */

    rt_elem *iar0 = (rt_elem *)mar0 + ARR_SIZE * 5;
    rt_elem *ico1 = (rt_elem *)mar0 + ARR_SIZE * 6;
    rt_elem *ico2 = (rt_elem *)mar0 + ARR_SIZE * 7;
    rt_elem *iso1 = (rt_elem *)mar0 + ARR_SIZE * 8;
    rt_elem *iso2 = (rt_elem *)mar0 + ARR_SIZE * 9;

    for (k = 0; k < Q; k++)
    {
        memcpy(iar0 + RT_ARR_SIZE(iarr) * k, iarr, sizeof(iarr));
    }

    rt_pntr info = sys_alloc(sizeof(rt_SIMD_INFOX) + MASK);
    rt_SIMD_INFOX *inf0 = (rt_SIMD_INFOX *)(((rt_full)info + MASK) & ~MASK);

    rt_pntr regs = sys_alloc(sizeof(rt_SIMD_REGS) + MASK);
    rt_SIMD_REGS *reg0 = (rt_SIMD_REGS *)(((rt_full)regs + MASK) & ~MASK);

    ASM_INIT(inf0, reg0)

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
    inf0->tail = (rt_pntr)0xABCDEF01;

    rt_si32 run_level = RUN_LEVEL;
    rt_si32 simd = 0;

    ASM_ENTER(inf0)
        verxx_xx()
    ASM_LEAVE(inf0)

#if defined (RT_256) && (RT_256 != 0)
    if ((inf0->ver & (RT_256 << 8)) == 0)
    {
        RT_LOGI("Chosen SIMD target not supported, check build flags\n");
        run_level = 0;
    }
    simd = simd == 0 ? (RT_256 << 8) | 8 : simd;
#endif /* RT_256 */

#if defined (RT_128) && (RT_128 != 0)
    if ((inf0->ver & (RT_128 << 0)) == 0)
    {
        RT_LOGI("Chosen SIMD target not supported, check build flags\n");
        run_level = 0;
    }
    simd = simd == 0 ? (RT_128 << 8) | 4 : simd;
#endif /* RT_128 */

    rt_time time1 = 0;
    rt_time time2 = 0;
    rt_time tC = 0;
    rt_time tS = 0;

    rt_si32 i;

    for (i = 0; i < run_level; i++)
    {
        RT_LOGI("-----------------  RUN LEVEL = %2d  -----------------\n", i+1);

        time1 = get_time();

        c_test[i](inf0);

        time2 = get_time();
        tC = time2 - time1;
        RT_LOGI("Time C = %d\n", (rt_si32)tC);

        /* --------------------------------- */

        time1 = get_time();

        s_test[i](inf0);

        time2 = get_time();
        tS = time2 - time1;
        RT_LOGI("Time S = %d\n", (rt_si32)tS);

        /* --------------------------------- */

        p_test[i](inf0);

        RT_LOGI("----------------------------------- simd = %dv%d ---\n",
                                                (simd & 0xFF) * 32, simd >> 8);
    }

    ASM_DONE(inf0)

    sys_free(regs, sizeof(rt_SIMD_REGS) + MASK);
    sys_free(info, sizeof(rt_SIMD_INFOX) + MASK);
    sys_free(marr, 10 * ARR_SIZE * sizeof(rt_ui32) + MASK);

#if defined (RT_WIN32) || defined (RT_WIN64) /* Win32, MSVC -- Win64, GCC --- */

    RT_LOGI("Type any letter and press ENTER to exit:");
    rt_char str[80];
    scanf("%79s", str);

#endif /* ----------------- OS specific ------------------------------------- */

    return 0;
}

/******************************************************************************/
/**********************************   UTILS   *********************************/
/******************************************************************************/

#undef Q /* short name for SIMD-quads in structs (number of 128-bit chunks) */
#undef N /* short name for SIMD-width in structs (with rt_fp16 SIMD-fields) */
#undef R /* short name for SIMD-width in structs (with rt_fp32 SIMD-fields) */
#undef S /* short name for SIMD-width in structs (with rt_real SIMD-fields) */
#undef T /* short name for SIMD-width in structs (with rt_fp64 SIMD-fields) */

#undef W /* triplet pass-through wrapper */

#undef P /* short name for RT_POINTER/32 */
#undef A /* short name for RT_ADDRESS/32 */
#undef L /* short name for RT_ELEMENT/32 */

#undef B /* short name for RT_ENDIAN*(2-1)*4 */
#undef C /* short name for RT_ENDIAN*(2-A)*4 */
#undef D /* short name for RT_ENDIAN*(P-1)*4 */
#undef E /* short name for RT_ENDIAN*(P-A)*4 */
#undef F /* short name for RT_ENDIAN*(A-1)*4 */
#undef G /* short name for RT_ENDIAN*(P-2)*4 */
#undef H /* short name for RT_ENDIAN*(L-1)*4 */
#undef I /* short name for RT_ENDIAN*(2-L)*4 */

#undef Xmm0 /* external name for SIMD register */
#undef Xmm1 /* external name for SIMD register */
#undef Xmm2 /* external name for SIMD register */
#undef Xmm3 /* external name for SIMD register */
#undef Xmm4 /* external name for SIMD register */
#undef Xmm5 /* external name for SIMD register */
#undef Xmm6 /* external name for SIMD register */
#undef Xmm7 /* external name for SIMD register */
#undef Xmm8 /* external name for SIMD register */
#undef Xmm9 /* external name for SIMD register */
#undef XmmA /* external name for SIMD register */
#undef XmmB /* external name for SIMD register */
#undef XmmC /* external name for SIMD register */
#undef XmmD /* external name for SIMD register */
#undef XmmE /* external name for SIMD register */
#undef XmmF /* external name for SIMD register */


#if RT_POINTER == 64
#if RT_ADDRESS == 32

#define RT_ADDRESS_MIN      ((rt_byte *)0x0000000040000000)
#define RT_ADDRESS_MAX      ((rt_byte *)0x0000000080000000)

#else /* RT_ADDRESS == 64 */

#define RT_ADDRESS_MIN      ((rt_byte *)0x0000000140000000)
#define RT_ADDRESS_MAX      ((rt_byte *)0x0000080000000000)

#endif /* RT_ADDRESS */

static
rt_byte *s_ptr = RT_ADDRESS_MIN;

#endif /* RT_POINTER */


#if defined (RT_WIN32) || defined (RT_WIN64) /* Win32, MSVC -- Win64, GCC --- */

#include <windows.h>

/*
 * Get system time in milliseconds.
 */
rt_time get_time()
{
    LARGE_INTEGER fr;
    QueryPerformanceFrequency(&fr);
    LARGE_INTEGER tm;
    QueryPerformanceCounter(&tm);
    return (rt_time)(tm.QuadPart * 1000 / fr.QuadPart);
}

static
DWORD s_step = 0;

static
SYSTEM_INFO s_sys = {0};

/*
 * Allocate memory from system heap.
 * Not thread-safe due to common static ptr.
 */
rt_pntr sys_alloc(rt_size size)
{
#if RT_POINTER == 64

    /* loop around RT_ADDRESS_MAX boundary */
    if (s_ptr >= RT_ADDRESS_MAX - size)
    {
        s_ptr  = RT_ADDRESS_MIN;
    }

    if (s_step == 0)
    {
        GetSystemInfo(&s_sys);
        s_step = s_sys.dwAllocationGranularity;
    }

    rt_pntr ptr = VirtualAlloc(s_ptr, size, MEM_COMMIT | MEM_RESERVE,
                  PAGE_READWRITE);

    /* advance with allocation granularity */
    s_ptr = (rt_byte *)ptr + ((size + s_step - 1) / s_step) * s_step;

#else /* RT_POINTER == 32 */

    rt_pntr ptr = malloc(size);

#endif /* RT_POINTER */

#if RT_DEBUG >= 1

    RT_LOGI("ALLOC PTR = %016"PR_Z"X, size = %ld\n", (rt_full)ptr, size);

#endif /* RT_DEBUG */

#if (RT_POINTER - RT_ADDRESS) != 0

    if ((rt_byte *)ptr >= RT_ADDRESS_MAX - size)
    {
        RT_LOGE("address exceeded allowed range, exiting...\n");
        exit(EXIT_FAILURE);
    }

#endif /* (RT_POINTER - RT_ADDRESS) */

    if (ptr == RT_NULL)
    {
        RT_LOGE("alloc failed with NULL address, exiting...\n");
        exit(EXIT_FAILURE);
    }

    return ptr;
}

/*
 * Free memory from system heap.
 */
rt_void sys_free(rt_pntr ptr, rt_size size)
{
#if RT_POINTER == 64

    VirtualFree(ptr, 0, MEM_RELEASE);

#else /* RT_POINTER == 32 */

    free(ptr);

#endif /* RT_POINTER */

#if RT_DEBUG >= 1

    RT_LOGI("FREED PTR = %016"PR_Z"X, size = %ld\n", (rt_full)ptr, size);

#endif /* RT_DEBUG */
}

#elif defined (RT_LINUX) /* Linux, GCC -------------------------------------- */

#include <sys/time.h>

/*
 * Get system time in milliseconds.
 */
rt_time get_time()
{
    timeval tm;
    gettimeofday(&tm, NULL);
    return (rt_time)(tm.tv_sec * 1000 + tm.tv_usec / 1000);
}

#if RT_POINTER == 64

#include <sys/mman.h>

#endif /* RT_POINTER */

/*
 * Allocate memory from system heap.
 * Not thread-safe due to common static ptr.
 */
rt_pntr sys_alloc(rt_size size)
{
#if RT_POINTER == 64

    /* loop around RT_ADDRESS_MAX boundary */
    /* in 64/32-bit hybrid mode addresses can't have sign bit
     * as MIPS64 sign-extends all 32-bit mem-loads by default */
    if (s_ptr >= RT_ADDRESS_MAX - size)
    {
        s_ptr  = RT_ADDRESS_MIN;
    }

    rt_pntr ptr = mmap(s_ptr, size, PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    /* advance with allocation granularity */
    /* in case when page-size differs from default 4096 bytes
     * mmap should round toward closest correct page boundary */
    s_ptr = (rt_byte *)ptr + ((size + 4095) / 4096) * 4096;

#else /* RT_POINTER == 32 */

    rt_pntr ptr = malloc(size);

#endif /* RT_POINTER */

#if RT_DEBUG >= 1

    RT_LOGI("ALLOC PTR = %016"PR_Z"X, size = %ld\n", (rt_full)ptr, size);

#endif /* RT_DEBUG */

#if (RT_POINTER - RT_ADDRESS) != 0

    if ((rt_byte *)ptr >= RT_ADDRESS_MAX - size)
    {
        RT_LOGE("address exceeded allowed range, exiting...\n");
        exit(EXIT_FAILURE);
    }

#endif /* (RT_POINTER - RT_ADDRESS) */

    if (ptr == RT_NULL)
    {
        RT_LOGE("alloc failed with NULL address, exiting...\n");
        exit(EXIT_FAILURE);
    }

    return ptr;
}

/*
 * Free memory from system heap.
 */
rt_void sys_free(rt_pntr ptr, rt_size size)
{
#if RT_POINTER == 64

    munmap(ptr, size);

#else /* RT_POINTER == 32 */

    free(ptr);

#endif /* RT_POINTER */

#if RT_DEBUG >= 1

    RT_LOGI("FREED PTR = %016"PR_Z"X, size = %ld\n", (rt_full)ptr, size);

#endif /* RT_DEBUG */
}

#endif /* ----------------- OS specific ------------------------------------- */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

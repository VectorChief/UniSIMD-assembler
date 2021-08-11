/******************************************************************************/
/* Copyright (c) 2013-2021 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define RT_SIMD_CODE /* enable SIMD instruction definitions */
#define RT_BASE_TEST /* enable BASE instruction sub-tests */
#define RT_ELEM_TEST /* enable ELEM instruction sub-tests (scalar SIMD) */

/*
 * RT_OFFS_DATA is intended for testing different displacement levels locally.
 * 0x060 - 1/8  DP-level  (9-bit displacements) has not been exceeded (Q=1).
 * 0x260 - 1/4  DP-level (10-bit displacements) has not been exceeded (Q=1).
 * 0x660 - 1/2  DP-level (11-bit displacements) has not been exceeded (Q=1).
 * 0xE60 - full DP-level (12-bit displacements) has not been exceeded (Q=1).
 * NOTE: the offset value must be divisible by 16 in order for code to work.
 * NOTE: the built-in rt_SIMD_INFO structure is already filled at full 1/16th.
 */
#define RT_OFFS_DATA        0x060 /* test different displacement levels */
#define RT_OFFS_SIMD        (RT_OFFS_DATA/16) /* number of quads in offset */

/*
 * RT_DATA determines the maximum load-level for data structures in code-base.
 * 1 - means full DP-level (12-bit displacements) is filled or exceeded (Q=1).
 * 2 - means 1/2  DP-level (11-bit displacements) has not been exceeded (Q=1).
 * 4 - means 1/4  DP-level (10-bit displacements) has not been exceeded (Q=1).
 * 8 - means 1/8  DP-level  (9-bit displacements) has not been exceeded (Q=1).
 * 16  means 1/16 DP-level  (8-bit displacements) has not been exceeded (Q=1).
 * NOTE: the built-in rt_SIMD_INFO structure is already filled at full 1/16th.
 */
#if     RT_OFFS_DATA <= 0x060
#define RT_DATA 8
#elif   RT_OFFS_DATA <= 0x260
#define RT_DATA 4
#elif   RT_OFFS_DATA <= 0x660
#define RT_DATA 2
#elif   RT_OFFS_DATA <= 0xE60
#define RT_DATA 1
#endif /* RT_OFFS_DATA */

#include "rtbase.h"

/******************************************************************************/
/*******************************   DEFINITIONS   ******************************/
/******************************************************************************/

#define SUB_TEST            29
#define CYC_SIZE            1000000

#define ARR_SIZE            S*3 /* hardcoded in ASM sections, S = SIMD elems */
#define MASK                (RT_SIMD_ALIGN - 1) /* SIMD alignment mask */

/* NOTE: floating point values are not tested for equality precisely due to
 * the slight difference in SIMD/FPU implementations across supported targets */
#define FRK(f)              (RT_FABS(f) < 10.0       ?   0.0001   :         \
                             RT_FABS(f) < 100.0      ?   0.001    :         \
                             RT_FABS(f) < 1000.0     ?   0.01     :         \
                             RT_FABS(f) < 10000.0    ?   0.1      :         \
                             RT_FABS(f) < 100000.0   ?   1.0      :         \
                             RT_FABS(f) < 1000000.0  ?  10.0      :  100.0)

#define IEQ(i1, i2)         (i1 == i2)

#define FEQ(f1, f2)         (RT_FABS((f1) - (f2)) <= t_diff *               \
                             RT_MIN(FRK(f1), FRK(f2)))

#define RT_LOGI             printf
#define RT_LOGE             printf

/******************************************************************************/
/***************************   VARS, FUNCS, TYPES   ***************************/
/******************************************************************************/

rt_si32     n_init      = 0;            /* subtest-init (from command-line) */
rt_si32     n_done      = SUB_TEST-1;   /* subtest-done (from command-line) */
rt_si32     t_diff      = 2;          /* diff-threshold (from command-line) */
rt_si32     r_test      = CYC_SIZE;   /* test-redundant (from command-line) */
rt_bool     v_mode      = RT_FALSE;     /* verbose mode (from command-line) */

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
 * Extended SIMD info structure for ASM_ENTER/ASM_LEAVE
 * serves as a container for test arrays and internal variables.
 * Note that DP offsets below start where rt_SIMD_INFO ends (at Q*0x100).
 * SIMD width is taken into account via S and Q from rtbase.h
 */
struct rt_SIMD_INFOX : public rt_SIMD_INFO
{
    rt_elem pad01[S*RT_OFFS_SIMD];
#define inf_PAD01           DP(Q*0x100)

    /* internal variables */

    rt_si32 cyc;
#define inf_CYC             DP(Q*0x100 + Q*RT_OFFS_DATA + 0x000)

    rt_si32 loc;
#define inf_LOC             DP(Q*0x100 + Q*RT_OFFS_DATA + 0x004)

    rt_si32 size;
#define inf_SIZE            DP(Q*0x100 + Q*RT_OFFS_DATA + 0x008)

    rt_si32 simd;
#define inf_SIMD            DP(Q*0x100 + Q*RT_OFFS_DATA + 0x00C)

    rt_pntr label;
#define inf_LABEL           DP(Q*0x100 + Q*RT_OFFS_DATA + 0x010+0x000*P)

    rt_pntr tail;
#define inf_TAIL            DP(Q*0x100 + Q*RT_OFFS_DATA + 0x010+0x004*P)

    /* floating point arrays */

    rt_real*far0;
#define inf_FAR0            DP(Q*0x100 + Q*RT_OFFS_DATA + 0x010+0x008*P+E)

    rt_real*fco1;
#define inf_FCO1            DP(Q*0x100 + Q*RT_OFFS_DATA + 0x010+0x00C*P+E)

    rt_real*fco2;
#define inf_FCO2            DP(Q*0x100 + Q*RT_OFFS_DATA + 0x010+0x010*P+E)

    rt_real*fso1;
#define inf_FSO1            DP(Q*0x100 + Q*RT_OFFS_DATA + 0x010+0x014*P+E)

    rt_real*fso2;
#define inf_FSO2            DP(Q*0x100 + Q*RT_OFFS_DATA + 0x010+0x018*P+E)

    /* integer arrays */

    rt_elem*iar0;
#define inf_IAR0            DP(Q*0x100 + Q*RT_OFFS_DATA + 0x010+0x01C*P+E)

    rt_elem*ico1;
#define inf_ICO1            DP(Q*0x100 + Q*RT_OFFS_DATA + 0x010+0x020*P+E)

    rt_elem*ico2;
#define inf_ICO2            DP(Q*0x100 + Q*RT_OFFS_DATA + 0x010+0x024*P+E)

    rt_elem*iso1;
#define inf_ISO1            DP(Q*0x100 + Q*RT_OFFS_DATA + 0x010+0x028*P+E)

    rt_elem*iso2;
#define inf_ISO2            DP(Q*0x100 + Q*RT_OFFS_DATA + 0x010+0x02C*P+E)

};

/*
 * SIMD offsets within array (j-index below).
 */
#define AJ0                 DP(Q*0x000 + Q*RT_OFFS_DATA)
#define AJ1                 DP(Q*0x010 + Q*RT_OFFS_DATA)
#define AJ2                 DP(Q*0x020 + Q*RT_OFFS_DATA)

/******************************************************************************/
/*******************************   SUB TEST  1   ******************************/
/******************************************************************************/

#if SUB_TEST >=  1

rt_void c_test01(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_real *fco1 = info->fco1 + S*RT_OFFS_SIMD;
    rt_real *fco2 = info->fco2 + S*RT_OFFS_SIMD;

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

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The test code below was designed mainly for assembler validation purposes
 * and therefore may not fully represent its unlocked performance potential.
 * For optimal results keep ASM sections in separate functions away from
 * complex C/C++ logic, while making sure those functions are not inlined.
 * This is needed for better compatibility with modern optimizing compilers.
 */
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
#ifdef RT_ELEM_TEST
        movss_ld(Xmm0, Mecx, AJ0)
        movss_ld(Xmm1, Mecx, AJ1)
        movss_rr(Xmm2, Xmm0)
        addss_rr(Xmm2, Xmm1)
        movss_rr(Xmm3, Xmm0)
        subss_rr(Xmm3, Xmm1)
        movss_st(Xmm2, Medx, AJ0)
        movss_st(Xmm3, Mebx, AJ0)
#endif /* RT_ELEM_TEST */

        movpx_ld(Xmm0, Mecx, AJ1)
        movpx_rr(Xmm2, Xmm0)
        addps_ld(Xmm2, Mecx, AJ2)
        movpx_rr(Xmm3, Xmm0)
        subps_ld(Xmm3, Mecx, AJ2)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)
#ifdef RT_ELEM_TEST
        movss_ld(Xmm0, Mecx, AJ1)
        movss_rr(Xmm2, Xmm0)
        addss_ld(Xmm2, Mecx, AJ2)
        movss_rr(Xmm3, Xmm0)
        subss_ld(Xmm3, Mecx, AJ2)
        movss_st(Xmm2, Medx, AJ1)
        movss_st(Xmm3, Mebx, AJ1)
#endif /* RT_ELEM_TEST */

        movpx_ld(Xmm0, Mecx, AJ2)
        movpx_ld(Xmm1, Mecx, AJ0)
        movpx_rr(Xmm2, Xmm0)
        addps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        subps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)
#ifdef RT_ELEM_TEST
        movss_ld(Xmm0, Mecx, AJ2)
        movss_ld(Xmm1, Mecx, AJ0)
        movss_rr(Xmm2, Xmm0)
        addss_rr(Xmm2, Xmm1)
        movss_rr(Xmm3, Xmm0)
        subss_rr(Xmm3, Xmm1)
        movss_st(Xmm2, Medx, AJ2)
        movss_st(Xmm3, Mebx, AJ2)
#endif /* RT_ELEM_TEST */

        ASM_LEAVE(info)
    }
}

rt_void p_test01(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_real *fco1 = info->fco1 + S*RT_OFFS_SIMD;
    rt_real *fco2 = info->fco2 + S*RT_OFFS_SIMD;
    rt_real *fso1 = info->fso1 + S*RT_OFFS_SIMD;
    rt_real *fso2 = info->fso2 + S*RT_OFFS_SIMD;

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

#endif /* SUB_TEST  1 */

/******************************************************************************/
/*******************************   SUB TEST  2   ******************************/
/******************************************************************************/

#if SUB_TEST >=  2

rt_void c_test02(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_real *fco1 = info->fco1 + S*RT_OFFS_SIMD;
    rt_real *fco2 = info->fco2 + S*RT_OFFS_SIMD;

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

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The test code below was designed mainly for assembler validation purposes
 * and therefore may not fully represent its unlocked performance potential.
 * For optimal results keep ASM sections in separate functions away from
 * complex C/C++ logic, while making sure those functions are not inlined.
 * This is needed for better compatibility with modern optimizing compilers.
 */
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
#ifdef RT_ELEM_TEST
        movss_ld(Xmm0, Mecx, AJ0)
        movss_ld(Xmm1, Mecx, AJ1)
        movss_rr(Xmm2, Xmm0)
        mulss_rr(Xmm2, Xmm1)
        movss_rr(Xmm3, Xmm0)
        divss_rr(Xmm3, Xmm1)
        movss_st(Xmm2, Medx, AJ0)
        movss_st(Xmm3, Mebx, AJ0)
#endif /* RT_ELEM_TEST */

        movpx_ld(Xmm0, Mecx, AJ1)
        movpx_rr(Xmm2, Xmm0)
        mulps_ld(Xmm2, Mecx, AJ2)
        movpx_rr(Xmm3, Xmm0)
        divps_ld(Xmm3, Mecx, AJ2)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)
#ifdef RT_ELEM_TEST
        movss_ld(Xmm0, Mecx, AJ1)
        movss_rr(Xmm2, Xmm0)
        mulss_ld(Xmm2, Mecx, AJ2)
        movss_rr(Xmm3, Xmm0)
        divss_ld(Xmm3, Mecx, AJ2)
        movss_st(Xmm2, Medx, AJ1)
        movss_st(Xmm3, Mebx, AJ1)
#endif /* RT_ELEM_TEST */

        movpx_ld(Xmm0, Mecx, AJ2)
        movpx_ld(Xmm1, Mecx, AJ0)
        movpx_rr(Xmm2, Xmm0)
        mulps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        divps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)
#ifdef RT_ELEM_TEST
        movss_ld(Xmm0, Mecx, AJ2)
        movss_ld(Xmm1, Mecx, AJ0)
        movss_rr(Xmm2, Xmm0)
        mulss_rr(Xmm2, Xmm1)
        movss_rr(Xmm3, Xmm0)
        divss_rr(Xmm3, Xmm1)
        movss_st(Xmm2, Medx, AJ2)
        movss_st(Xmm3, Mebx, AJ2)
#endif /* RT_ELEM_TEST */

        ASM_LEAVE(info)
    }
}

rt_void p_test02(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_real *fco1 = info->fco1 + S*RT_OFFS_SIMD;
    rt_real *fco2 = info->fco2 + S*RT_OFFS_SIMD;
    rt_real *fso1 = info->fso1 + S*RT_OFFS_SIMD;
    rt_real *fso2 = info->fso2 + S*RT_OFFS_SIMD;

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

#endif /* SUB_TEST  2 */

/******************************************************************************/
/*******************************   SUB TEST  3   ******************************/
/******************************************************************************/

#if SUB_TEST >=  3

rt_void c_test03(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;

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

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The test code below was designed mainly for assembler validation purposes
 * and therefore may not fully represent its unlocked performance potential.
 * For optimal results keep ASM sections in separate functions away from
 * complex C/C++ logic, while making sure those functions are not inlined.
 * This is needed for better compatibility with modern optimizing compilers.
 */
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
#ifdef RT_ELEM_TEST
        movss_ld(Xmm0, Mecx, AJ0)
        movss_ld(Xmm1, Mecx, AJ1)
        movss_rr(Xmm2, Xmm0)
        cgtss_rr(Xmm2, Xmm1)
        movss_rr(Xmm3, Xmm0)
        cgess_rr(Xmm3, Xmm1)
        movss_st(Xmm2, Medx, AJ0)
        movss_st(Xmm3, Mebx, AJ0)
#endif /* RT_ELEM_TEST */

        movpx_ld(Xmm0, Mecx, AJ1)
        movpx_rr(Xmm2, Xmm0)
        cgtps_ld(Xmm2, Mecx, AJ2)
        movpx_rr(Xmm3, Xmm0)
        cgeps_ld(Xmm3, Mecx, AJ2)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)
#ifdef RT_ELEM_TEST
        movss_ld(Xmm0, Mecx, AJ1)
        movss_rr(Xmm2, Xmm0)
        cgtss_ld(Xmm2, Mecx, AJ2)
        movss_rr(Xmm3, Xmm0)
        cgess_ld(Xmm3, Mecx, AJ2)
        movss_st(Xmm2, Medx, AJ1)
        movss_st(Xmm3, Mebx, AJ1)
#endif /* RT_ELEM_TEST */

        movpx_ld(Xmm0, Mecx, AJ2)
        movpx_ld(Xmm1, Mecx, AJ0)
        movpx_rr(Xmm2, Xmm0)
        cgtps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        cgeps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)
#ifdef RT_ELEM_TEST
        movss_ld(Xmm0, Mecx, AJ2)
        movss_ld(Xmm1, Mecx, AJ0)
        movss_rr(Xmm2, Xmm0)
        cgtss_rr(Xmm2, Xmm1)
        movss_rr(Xmm3, Xmm0)
        cgess_rr(Xmm3, Xmm1)
        movss_st(Xmm2, Medx, AJ2)
        movss_st(Xmm3, Mebx, AJ2)
#endif /* RT_ELEM_TEST */

        ASM_LEAVE(info)
    }
}

rt_void p_test03(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;
    rt_elem *iso1 = info->iso1 + S*RT_OFFS_SIMD;
    rt_elem *iso2 = info->iso2 + S*RT_OFFS_SIMD;

    j = n;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e, farr[%d] = %e\n",
                j, far0[j], (j + S) % n, far0[(j + S) % n]);

        RT_LOGI("C (farr[%d]>!farr[%d]) = %" PR_L "X, "
                  "(farr[%d]>=farr[%d]) = %" PR_L "X\n",
                j, (j + S) % n, ico1[j], j, (j + S) % n, ico2[j]);

        RT_LOGI("S (farr[%d]>!farr[%d]) = %" PR_L "X, "
                  "(farr[%d]>=farr[%d]) = %" PR_L "X\n",
                j, (j + S) % n, iso1[j], j, (j + S) % n, iso2[j]);
    }
}

#endif /* SUB_TEST  3 */

/******************************************************************************/
/*******************************   SUB TEST  4   ******************************/
/******************************************************************************/

#if SUB_TEST >=  4

rt_void c_test04(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;

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

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The test code below was designed mainly for assembler validation purposes
 * and therefore may not fully represent its unlocked performance potential.
 * For optimal results keep ASM sections in separate functions away from
 * complex C/C++ logic, while making sure those functions are not inlined.
 * This is needed for better compatibility with modern optimizing compilers.
 */
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
#ifdef RT_ELEM_TEST
        movss_ld(Xmm0, Mecx, AJ0)
        movss_ld(Xmm1, Mecx, AJ1)
        movss_rr(Xmm2, Xmm0)
        cltss_rr(Xmm2, Xmm1)
        movss_rr(Xmm3, Xmm0)
        cless_rr(Xmm3, Xmm1)
        movss_st(Xmm2, Medx, AJ0)
        movss_st(Xmm3, Mebx, AJ0)
#endif /* RT_ELEM_TEST */

        movpx_ld(Xmm0, Mecx, AJ1)
        movpx_rr(Xmm2, Xmm0)
        cltps_ld(Xmm2, Mecx, AJ2)
        movpx_rr(Xmm3, Xmm0)
        cleps_ld(Xmm3, Mecx, AJ2)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)
#ifdef RT_ELEM_TEST
        movss_ld(Xmm0, Mecx, AJ1)
        movss_rr(Xmm2, Xmm0)
        cltss_ld(Xmm2, Mecx, AJ2)
        movss_rr(Xmm3, Xmm0)
        cless_ld(Xmm3, Mecx, AJ2)
        movss_st(Xmm2, Medx, AJ1)
        movss_st(Xmm3, Mebx, AJ1)
#endif /* RT_ELEM_TEST */

        movpx_ld(Xmm0, Mecx, AJ2)
        movpx_ld(Xmm1, Mecx, AJ0)
        movpx_rr(Xmm2, Xmm0)
        cltps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        cleps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)
#ifdef RT_ELEM_TEST
        movss_ld(Xmm0, Mecx, AJ2)
        movss_ld(Xmm1, Mecx, AJ0)
        movss_rr(Xmm2, Xmm0)
        cltss_rr(Xmm2, Xmm1)
        movss_rr(Xmm3, Xmm0)
        cless_rr(Xmm3, Xmm1)
        movss_st(Xmm2, Medx, AJ2)
        movss_st(Xmm3, Mebx, AJ2)
#endif /* RT_ELEM_TEST */

        ASM_LEAVE(info)
    }
}

rt_void p_test04(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;
    rt_elem *iso1 = info->iso1 + S*RT_OFFS_SIMD;
    rt_elem *iso2 = info->iso2 + S*RT_OFFS_SIMD;

    j = n;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e, farr[%d] = %e\n",
                j, far0[j], (j + S) % n, far0[(j + S) % n]);

        RT_LOGI("C (farr[%d]<!farr[%d]) = %" PR_L "X, "
                  "(farr[%d]<=farr[%d]) = %" PR_L "X\n",
                j, (j + S) % n, ico1[j], j, (j + S) % n, ico2[j]);

        RT_LOGI("S (farr[%d]<!farr[%d]) = %" PR_L "X, "
                  "(farr[%d]<=farr[%d]) = %" PR_L "X\n",
                j, (j + S) % n, iso1[j], j, (j + S) % n, iso2[j]);
    }
}

#endif /* SUB_TEST  4 */

/******************************************************************************/
/*******************************   SUB TEST  5   ******************************/
/******************************************************************************/

#if SUB_TEST >=  5

rt_void c_test05(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;

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

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The test code below was designed mainly for assembler validation purposes
 * and therefore may not fully represent its unlocked performance potential.
 * For optimal results keep ASM sections in separate functions away from
 * complex C/C++ logic, while making sure those functions are not inlined.
 * This is needed for better compatibility with modern optimizing compilers.
 */
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
#ifdef RT_ELEM_TEST
        movss_ld(Xmm0, Mecx, AJ0)
        movss_ld(Xmm1, Mecx, AJ1)
        movss_rr(Xmm2, Xmm0)
        ceqss_rr(Xmm2, Xmm1)
        movss_rr(Xmm3, Xmm0)
        cness_rr(Xmm3, Xmm1)
        movss_st(Xmm2, Medx, AJ0)
        movss_st(Xmm3, Mebx, AJ0)
#endif /* RT_ELEM_TEST */

        movpx_ld(Xmm0, Mecx, AJ1)
        movpx_rr(Xmm2, Xmm0)
        ceqps_ld(Xmm2, Mecx, AJ2)
        movpx_rr(Xmm3, Xmm0)
        cneps_ld(Xmm3, Mecx, AJ2)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)
#ifdef RT_ELEM_TEST
        movss_ld(Xmm0, Mecx, AJ1)
        movss_rr(Xmm2, Xmm0)
        ceqss_ld(Xmm2, Mecx, AJ2)
        movss_rr(Xmm3, Xmm0)
        cness_ld(Xmm3, Mecx, AJ2)
        movss_st(Xmm2, Medx, AJ1)
        movss_st(Xmm3, Mebx, AJ1)
#endif /* RT_ELEM_TEST */

        movpx_ld(Xmm0, Mecx, AJ2)
        movpx_ld(Xmm1, Mecx, AJ0)
        movpx_rr(Xmm2, Xmm0)
        ceqps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        cneps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)
#ifdef RT_ELEM_TEST
        movss_ld(Xmm0, Mecx, AJ2)
        movss_ld(Xmm1, Mecx, AJ0)
        movss_rr(Xmm2, Xmm0)
        ceqss_rr(Xmm2, Xmm1)
        movss_rr(Xmm3, Xmm0)
        cness_rr(Xmm3, Xmm1)
        movss_st(Xmm2, Medx, AJ2)
        movss_st(Xmm3, Mebx, AJ2)
#endif /* RT_ELEM_TEST */

        ASM_LEAVE(info)
    }
}

rt_void p_test05(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;
    rt_elem *iso1 = info->iso1 + S*RT_OFFS_SIMD;
    rt_elem *iso2 = info->iso2 + S*RT_OFFS_SIMD;

    j = n;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e, farr[%d] = %e\n",
                j, far0[j], (j + S) % n, far0[(j + S) % n]);

        RT_LOGI("C (farr[%d]==farr[%d]) = %" PR_L "X, "
                  "(farr[%d]!=farr[%d]) = %" PR_L "X\n",
                j, (j + S) % n, ico1[j], j, (j + S) % n, ico2[j]);

        RT_LOGI("S (farr[%d]==farr[%d]) = %" PR_L "X, "
                  "(farr[%d]!=farr[%d]) = %" PR_L "X\n",
                j, (j + S) % n, iso1[j], j, (j + S) % n, iso2[j]);
    }
}

#endif /* SUB_TEST  5 */

/******************************************************************************/
/*******************************   SUB TEST  6   ******************************/
/******************************************************************************/

#if SUB_TEST >=  6

rt_void c_test06(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_elem *iar0 = info->iar0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_real *fco2 = info->fco2 + S*RT_OFFS_SIMD;

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

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The test code below was designed mainly for assembler validation purposes
 * and therefore may not fully represent its unlocked performance potential.
 * For optimal results keep ASM sections in separate functions away from
 * complex C/C++ logic, while making sure those functions are not inlined.
 * This is needed for better compatibility with modern optimizing compilers.
 */
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

        cvzps_ld(Xmm2, Mecx, AJ1)
        cvnpn_ld(Xmm3, Mesi, AJ1)
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

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_elem *iar0 = info->iar0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_real *fco2 = info->fco2 + S*RT_OFFS_SIMD;
    rt_elem *iso1 = info->iso1 + S*RT_OFFS_SIMD;
    rt_real *fso2 = info->fso2 + S*RT_OFFS_SIMD;

    j = n;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && FEQ(fco2[j], fso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e, iarr[%d] = %" PR_L "d\n",
                j, far0[j], j, iar0[j]);

        RT_LOGI("C (rt_elem)farr[%d] = %" PR_L "d, (rt_real)iarr[%d] = %e\n",
                j, ico1[j], j, fco2[j]);

        RT_LOGI("S (rt_elem)farr[%d] = %" PR_L "d, (rt_real)iarr[%d] = %e\n",
                j, iso1[j], j, fso2[j]);
    }
}

#endif /* SUB_TEST  6 */

/******************************************************************************/
/*******************************   SUB TEST  7   ******************************/
/******************************************************************************/

#if SUB_TEST >=  7

rt_void c_test07(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_real *fco1 = info->fco1 + S*RT_OFFS_SIMD;
    rt_real *fco2 = info->fco2 + S*RT_OFFS_SIMD;

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

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The test code below was designed mainly for assembler validation purposes
 * and therefore may not fully represent its unlocked performance potential.
 * For optimal results keep ASM sections in separate functions away from
 * complex C/C++ logic, while making sure those functions are not inlined.
 * This is needed for better compatibility with modern optimizing compilers.
 */
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
#ifdef RT_ELEM_TEST
        movss_ld(Xmm0, Mecx, AJ0)
        sqrss_rr(Xmm2, Xmm0)
        rcpss_rr(Xmm3, Xmm0) /* destroys Xmm0 */
        movss_st(Xmm2, Medx, AJ0)
        movss_st(Xmm3, Mebx, AJ0)
#endif /* RT_ELEM_TEST */

        movpx_ld(Xmm0, Mecx, AJ1)
        sqrps_ld(Xmm2, Mecx, AJ1)
        rcpps_rr(Xmm3, Xmm0) /* destroys Xmm0 */
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)
#ifdef RT_ELEM_TEST
        movss_ld(Xmm0, Mecx, AJ1)
        sqrss_ld(Xmm2, Mecx, AJ1)
        rcpss_rr(Xmm3, Xmm0) /* destroys Xmm0 */
        movss_st(Xmm2, Medx, AJ1)
        movss_st(Xmm3, Mebx, AJ1)
#endif /* RT_ELEM_TEST */

        movpx_ld(Xmm0, Mecx, AJ2)
        sqrps_rr(Xmm2, Xmm0)
        rcpps_rr(Xmm3, Xmm0) /* destroys Xmm0 */
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)
#ifdef RT_ELEM_TEST
        movss_ld(Xmm0, Mecx, AJ2)
        sqrss_rr(Xmm2, Xmm0)
        rcpss_rr(Xmm3, Xmm0) /* destroys Xmm0 */
        movss_st(Xmm2, Medx, AJ2)
        movss_st(Xmm3, Mebx, AJ2)
#endif /* RT_ELEM_TEST */

        ASM_LEAVE(info)
    }
}

rt_void p_test07(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_real *fco1 = info->fco1 + S*RT_OFFS_SIMD;
    rt_real *fco2 = info->fco2 + S*RT_OFFS_SIMD;
    rt_real *fso1 = info->fso1 + S*RT_OFFS_SIMD;
    rt_real *fso2 = info->fso2 + S*RT_OFFS_SIMD;

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

#endif /* SUB_TEST  7 */

/******************************************************************************/
/*******************************   SUB TEST  8   ******************************/
/******************************************************************************/

#if SUB_TEST >=  8

rt_void c_test08(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_elem *iar0 = info->iar0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = iar0[j] + ((rt_uelm)+iar0[j] << (1+32*(L-1)));
            ico2[j] = iar0[j] - ((rt_uelm)-iar0[j] >> (2+32*(L-1)));
        }
    }
}

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The test code below was designed mainly for assembler validation purposes
 * and therefore may not fully represent its unlocked performance potential.
 * For optimal results keep ASM sections in separate functions away from
 * complex C/C++ logic, while making sure those functions are not inlined.
 * This is needed for better compatibility with modern optimizing compilers.
 */
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
        movpx_rr(Xmm3, Xmm0)
        movpx_rr(Xmm2, Xmm0)
        shlpx_ri(Xmm0, IB((1+32*(L-1))))
        addpx_rr(Xmm2, Xmm0)
        xorpx_rr(Xmm0, Xmm0)
        subpx_rr(Xmm0, Xmm3)
        shrpx_ri(Xmm0, IB((2+32*(L-1))))
        subpx_rr(Xmm3, Xmm0)
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ0)
        movyx_rr(Recx, Reax)
        shlyx_ri(Reax, IB((1+32*(L-1))))
        addyx_rr(Reax, Recx)
        movyx_st(Reax, Medx, AJ0)
        movyx_rr(Reax, Recx)
        negyx_rx(Reax)
        shryx_ri(Reax, IB((2+32*(L-1))))
        subyx_rr(Recx, Reax)
        movyx_st(Recx, Mebx, AJ0)
#endif /* RT_BASE_TEST */

        movpx_ld(Xmm0, Mesi, AJ1)
        movpx_rr(Xmm3, Xmm0)
        movpx_rr(Xmm2, Xmm0)
        xorpx_rr(Xmm1, Xmm1)
        movpx_st(Xmm1, Medx, AJ1)
        movyx_mi(Medx, AJ1, IB((1+32*(L-1))))
        shlpx_ld(Xmm0, Medx, AJ1)
        movpx_st(Xmm0, Medx, AJ1)
        addpx_ld(Xmm2, Medx, AJ1)
        movpx_st(Xmm0, Mebx, AJ1)
        xorpx_ld(Xmm0, Mebx, AJ1)
        subpx_ld(Xmm0, Mesi, AJ1)
        movpx_st(Xmm1, Mebx, AJ1)
        movyx_mi(Mebx, AJ1, IB((2+32*(L-1))))
        shrpx_ld(Xmm0, Mebx, AJ1)
        movpx_st(Xmm0, Mebx, AJ1)
        subpx_ld(Xmm3, Mebx, AJ1)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ1)
        movyx_rr(Recx, Reax)
        movyx_st(Reax, Medx, AJ1)
        shlyx_mi(Medx, AJ1, IB((1+32*(L-1))))
        addyx_st(Recx, Medx, AJ1)
        movyx_st(Recx, Mebx, AJ1)
        negyx_mx(Mebx, AJ1)
        shryx_mi(Mebx, AJ1, IB((2+32*(L-1))))
        movyx_ld(Reax, Mebx, AJ1)
        movyx_st(Recx, Mebx, AJ1)
        subyx_st(Reax, Mebx, AJ1)
#endif /* RT_BASE_TEST */

        movpx_ld(Xmm0, Mesi, AJ2)
        movpx_rr(Xmm3, Xmm0)
        movpx_rr(Xmm2, Xmm0)
        shlpx_ri(Xmm0, IB((1+32*(L-1))))
        addpx_rr(Xmm2, Xmm0)
        xorpx_rr(Xmm0, Xmm0)
        subpx_rr(Xmm0, Xmm3)
        shrpx_ri(Xmm0, IB((2+32*(L-1))))
        subpx_rr(Xmm3, Xmm0)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ2)
        movyx_rr(Recx, Reax)
        shlyx_ri(Reax, IB((1+32*(L-1))))
        addyx_ld(Reax, Mesi, AJ2)
        movyx_st(Reax, Medx, AJ2)
        movyx_rr(Reax, Recx)
        negyx_rx(Reax)
        shryx_ri(Reax, IB((2+32*(L-1))))
        movyx_st(Reax, Mebx, AJ2)
        subyx_ld(Recx, Mebx, AJ2)
        movyx_st(Recx, Mebx, AJ2)
#endif /* RT_BASE_TEST */

        ASM_LEAVE(info)
    }
}

rt_void p_test08(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_elem *iar0 = info->iar0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;
    rt_elem *iso1 = info->iso1 + S*RT_OFFS_SIMD;
    rt_elem *iso2 = info->iso2 + S*RT_OFFS_SIMD;

    j = n;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("iarr[%d] = %" PR_L "d\n",
                j, iar0[j]);

        RT_LOGI("C iarr[%d]+((rt_uelm)+iarr[%d]<<1) = %" PR_L "d, "
                  "iarr[%d]-((rt_uelm)-iarr[%d]>>2) = %" PR_L "d\n",
                j, j, ico1[j], j, j, ico2[j]);

        RT_LOGI("S iarr[%d]+((rt_uelm)+iarr[%d]<<1) = %" PR_L "d, "
                  "iarr[%d]-((rt_uelm)-iarr[%d]>>2) = %" PR_L "d\n",
                j, j, iso1[j], j, j, iso2[j]);
    }
}

#endif /* SUB_TEST  8 */

/******************************************************************************/
/*******************************   SUB TEST  9   ******************************/
/******************************************************************************/

#if SUB_TEST >=  9

rt_void c_test09(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_elem *iar0 = info->iar0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;

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

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The test code below was designed mainly for assembler validation purposes
 * and therefore may not fully represent its unlocked performance potential.
 * For optimal results keep ASM sections in separate functions away from
 * complex C/C++ logic, while making sure those functions are not inlined.
 * This is needed for better compatibility with modern optimizing compilers.
 */
rt_void s_test09(rt_SIMD_INFOX *info)
{
    ASM_ENTER(info)

        movwx_ld(Reax, Mebp, inf_CYC)
        movwx_st(Reax, Mebp, inf_LOC)

    LBL(cyc_beg)

        movxx_ld(Recx, Mebp, inf_IAR0)
        movxx_ld(Rebx, Mebp, inf_ISO1)
        movxx_ld(Resi, Mebp, inf_ISO2)
        movwx_ld(Redi, Mebp, inf_SIZE)

    LBL(loc_beg)

        movyx_ld(Reax, Mecx, AJ0)
        mulyn_xm(Mecx, AJ1)
        movyx_st(Reax, Mebx, AJ0)
        movyx_ld(Reax, Mecx, AJ0)
        preyn_xx()
        divyn_xm(Mecx, AJ1)
        movyx_st(Reax, Mesi, AJ0)

        addxx_ri(Recx, IB(4*L))
        addxx_ri(Rebx, IB(4*L))
        addxx_ri(Resi, IB(4*L))
        subwx_ri(Redi, IB(1))
        cmjwx_ri(Redi, IB(S),
        /* if */ GT_x, loc_beg)

        movxx_ld(Redi, Mebp, inf_IAR0)
        movwx_mi(Mebp, inf_SIMD, IB(S))

    LBL(smd_beg)

        movyx_ld(Reax, Mecx, AJ0)
        mulyn_xm(Medi, AJ0)
        movyx_st(Reax, Mebx, AJ0)
        movyx_ld(Reax, Mecx, AJ0)
        preyn_xx()
        divyn_xm(Medi, AJ0)
        movyx_st(Reax, Mesi, AJ0)

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

        jmpxx_lb(cyc_beg)

    LBL(cyc_end)

    ASM_LEAVE(info)
}

rt_void p_test09(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_elem *iar0 = info->iar0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;
    rt_elem *iso1 = info->iso1 + S*RT_OFFS_SIMD;
    rt_elem *iso2 = info->iso2 + S*RT_OFFS_SIMD;

    j = n;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("iarr[%d] = %" PR_L "d, iarr[%d] = %" PR_L "d\n",
                j, iar0[j], (j + S) % n, iar0[(j + S) % n]);

        RT_LOGI("C iarr[%d]*iarr[%d] = %" PR_L "d, "
                  "iarr[%d]/iarr[%d] = %" PR_L "d\n",
                j, (j + S) % n, ico1[j], j, (j + S) % n, ico2[j]);

        RT_LOGI("S iarr[%d]*iarr[%d] = %" PR_L "d, "
                  "iarr[%d]/iarr[%d] = %" PR_L "d\n",
                j, (j + S) % n, iso1[j], j, (j + S) % n, iso2[j]);
    }
}

#endif /* SUB_TEST  9 */

/******************************************************************************/
/*******************************   SUB TEST 10   ******************************/
/******************************************************************************/

#if SUB_TEST >= 10

rt_void c_test10(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_real *fco1 = info->fco1 + S*RT_OFFS_SIMD;
    rt_real *fco2 = info->fco2 + S*RT_OFFS_SIMD;

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

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The test code below was designed mainly for assembler validation purposes
 * and therefore may not fully represent its unlocked performance potential.
 * For optimal results keep ASM sections in separate functions away from
 * complex C/C++ logic, while making sure those functions are not inlined.
 * This is needed for better compatibility with modern optimizing compilers.
 */
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
#ifdef RT_ELEM_TEST
        movss_ld(Xmm0, Mecx, AJ0)
        movss_ld(Xmm1, Mecx, AJ1)
        movss_rr(Xmm2, Xmm0)
        minss_rr(Xmm2, Xmm1)
        movss_rr(Xmm3, Xmm0)
        maxss_rr(Xmm3, Xmm1)
        movss_st(Xmm2, Medx, AJ0)
        movss_st(Xmm3, Mebx, AJ0)
#endif /* RT_ELEM_TEST */

        movpx_ld(Xmm0, Mecx, AJ1)
        movpx_rr(Xmm2, Xmm0)
        minps_ld(Xmm2, Mecx, AJ2)
        movpx_rr(Xmm3, Xmm0)
        maxps_ld(Xmm3, Mecx, AJ2)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)
#ifdef RT_ELEM_TEST
        movss_ld(Xmm0, Mecx, AJ1)
        movss_rr(Xmm2, Xmm0)
        minss_ld(Xmm2, Mecx, AJ2)
        movss_rr(Xmm3, Xmm0)
        maxss_ld(Xmm3, Mecx, AJ2)
        movss_st(Xmm2, Medx, AJ1)
        movss_st(Xmm3, Mebx, AJ1)
#endif /* RT_ELEM_TEST */

        movpx_ld(Xmm0, Mecx, AJ2)
        movpx_ld(Xmm1, Mecx, AJ0)
        movpx_rr(Xmm2, Xmm0)
        minps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        maxps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)
#ifdef RT_ELEM_TEST
        movss_ld(Xmm0, Mecx, AJ2)
        movss_ld(Xmm1, Mecx, AJ0)
        movss_rr(Xmm2, Xmm0)
        minss_rr(Xmm2, Xmm1)
        movss_rr(Xmm3, Xmm0)
        maxss_rr(Xmm3, Xmm1)
        movss_st(Xmm2, Medx, AJ2)
        movss_st(Xmm3, Mebx, AJ2)
#endif /* RT_ELEM_TEST */

        ASM_LEAVE(info)
    }
}

rt_void p_test10(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_real *fco1 = info->fco1 + S*RT_OFFS_SIMD;
    rt_real *fco2 = info->fco2 + S*RT_OFFS_SIMD;
    rt_real *fso1 = info->fso1 + S*RT_OFFS_SIMD;
    rt_real *fso2 = info->fso2 + S*RT_OFFS_SIMD;

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

#endif /* SUB_TEST 10 */

/******************************************************************************/
/*******************************   SUB TEST 11   ******************************/
/******************************************************************************/

#if SUB_TEST >= 11

rt_void c_test11(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_elem *iar0 = info->iar0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;

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

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The test code below was designed mainly for assembler validation purposes
 * and therefore may not fully represent its unlocked performance potential.
 * For optimal results keep ASM sections in separate functions away from
 * complex C/C++ logic, while making sure those functions are not inlined.
 * This is needed for better compatibility with modern optimizing compilers.
 */
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
        movpx_rr(Xmm1, Xmm0)
        movpx_rr(Xmm2, Xmm0)
        shlpx_ri(Xmm0, IB(7))
        orrpx_rr(Xmm2, Xmm0)
        movpx_rr(Xmm3, Xmm1)
        shrpn_ri(Xmm1, IB(3))
        xorpx_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ0)
        movyx_rr(Recx, Reax)
        shlyx_ri(Reax, IB(7))
        orryx_rr(Reax, Recx)
        movyx_st(Reax, Medx, AJ0)
        movyx_rr(Reax, Recx)
        shryn_ri(Reax, IB(3))
        xoryx_rr(Reax, Recx)
        movyx_st(Reax, Mebx, AJ0)
#endif /* RT_BASE_TEST */

        movpx_ld(Xmm0, Mesi, AJ1)
        movpx_rr(Xmm1, Xmm0)
        movpx_rr(Xmm2, Xmm0)
        shlpx_ri(Xmm0, IB(7))
        movpx_st(Xmm0, Medx, AJ1)
        orrpx_ld(Xmm2, Medx, AJ1)
        movpx_rr(Xmm3, Xmm1)
        shrpn_ri(Xmm1, IB(3))
        movpx_st(Xmm1, Mebx, AJ1)
        xorpx_ld(Xmm3, Mebx, AJ1)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ1)
        movyx_rr(Recx, Reax)
        movyx_st(Reax, Medx, AJ1)
        shlyx_mi(Medx, AJ1, IB(7))
        orryx_st(Recx, Medx, AJ1)
        movyx_st(Recx, Mebx, AJ1)
        shryn_mi(Mebx, AJ1, IB(3))
        xoryx_st(Recx, Mebx, AJ1)
#endif /* RT_BASE_TEST */

        movpx_ld(Xmm0, Mesi, AJ2)
        movpx_rr(Xmm1, Xmm0)
        movpx_rr(Xmm2, Xmm0)
        shlpx_ri(Xmm0, IB(7))
        orrpx_rr(Xmm2, Xmm0)
        movpx_rr(Xmm3, Xmm1)
        shrpn_ri(Xmm1, IB(3))
        xorpx_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ2)
        movyx_rr(Recx, Reax)
        movyx_rr(Redi, Reax)
        mulyx_ri(Redi, IB(1 << 7))
        movyx_rr(Reax, Redi)
        orryx_ld(Reax, Mesi, AJ2)
        movyx_st(Reax, Medx, AJ2)
        movyx_st(Recx, Mebx, AJ2)
        shryn_mi(Mebx, AJ2, IB(3))
        xoryx_ld(Recx, Mebx, AJ2)
        movyx_st(Recx, Mebx, AJ2)
#endif /* RT_BASE_TEST */

        ASM_LEAVE(info)
    }
}

rt_void p_test11(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_elem *iar0 = info->iar0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;
    rt_elem *iso1 = info->iso1 + S*RT_OFFS_SIMD;
    rt_elem *iso2 = info->iso2 + S*RT_OFFS_SIMD;

    j = n;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("iarr[%d] = %" PR_L "d\n",
                j, iar0[j]);

        RT_LOGI("C iarr[%d]|(iarr[%d]<<7) = %" PR_L "d, "
                  "iarr[%d]^(iarr[%d]>>3) = %" PR_L "d\n",
                j, j, ico1[j], j, j, ico2[j]);

        RT_LOGI("S iarr[%d]|(iarr[%d]<<7) = %" PR_L "d, "
                  "iarr[%d]^(iarr[%d]>>3) = %" PR_L "d\n",
                j, j, iso1[j], j, j, iso2[j]);
    }
}

#endif /* SUB_TEST 11 */

/******************************************************************************/
/*******************************   SUB TEST 12   ******************************/
/******************************************************************************/

#if SUB_TEST >= 12

rt_void c_test12(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_elem *iar0 = info->iar0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] =  iar0[j] & (iar0[j] << 17);
            ico2[j] = ~iar0[j] & (iar0[j] >> 13);
        }
    }
}

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The test code below was designed mainly for assembler validation purposes
 * and therefore may not fully represent its unlocked performance potential.
 * For optimal results keep ASM sections in separate functions away from
 * complex C/C++ logic, while making sure those functions are not inlined.
 * This is needed for better compatibility with modern optimizing compilers.
 */
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
        movpx_rr(Xmm1, Xmm0)
        movpx_rr(Xmm2, Xmm0)
        shlpx_ri(Xmm0, IB(17))
        andpx_rr(Xmm2, Xmm0)
        movpx_rr(Xmm3, Xmm1)
        shrpn_ri(Xmm1, IB(13))
        annpx_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ0)
        movyx_rr(Recx, Reax)
        shlyx_ri(Reax, IB(17))
        andyx_rr(Reax, Recx)
        movyx_st(Reax, Medx, AJ0)
        movyx_rr(Reax, Recx)
        shryn_ri(Recx, IB(13))
        annyx_rr(Reax, Recx)
        movyx_st(Reax, Mebx, AJ0)
#endif /* RT_BASE_TEST */

        movpx_ld(Xmm0, Mesi, AJ1)
        movpx_rr(Xmm1, Xmm0)
        movpx_rr(Xmm2, Xmm0)
        shlpx_ri(Xmm0, IB(17))
        movpx_st(Xmm0, Medx, AJ1)
        andpx_ld(Xmm2, Medx, AJ1)
        movpx_rr(Xmm3, Xmm1)
        shrpn_ri(Xmm1, IB(13))
        movpx_st(Xmm1, Mebx, AJ1)
        annpx_ld(Xmm3, Mebx, AJ1)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ1)
        movyx_rr(Recx, Reax)
        movyx_st(Recx, Medx, AJ1)
        shlyx_mi(Medx, AJ1, IB(17))
        andyx_st(Reax, Medx, AJ1)
        movyx_st(Reax, Mebx, AJ1)
        shryn_ri(Recx, IB(13))
        annyx_st(Recx, Mebx, AJ1)
#endif /* RT_BASE_TEST */

        movpx_ld(Xmm0, Mesi, AJ2)
        movpx_rr(Xmm1, Xmm0)
        movpx_rr(Xmm2, Xmm0)
        shlpx_ri(Xmm0, IB(17))
        andpx_rr(Xmm2, Xmm0)
        movpx_rr(Xmm3, Xmm1)
        shrpn_ri(Xmm1, IB(13))
        annpx_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ2)
        movyx_rr(Recx, Reax)
        movyx_rr(Redi, Reax)
        mulyx_ri(Redi, IV(1 << 17))
        movyx_rr(Reax, Redi)
        andyx_ld(Reax, Mesi, AJ2)
        movyx_st(Reax, Medx, AJ2)
        movyx_rr(Reax, Recx)
        shryn_ri(Recx, IB(13))
        movyx_st(Recx, Mebx, AJ2)
        annyx_ld(Reax, Mebx, AJ2)
        movyx_st(Reax, Mebx, AJ2)
#endif /* RT_BASE_TEST */

        ASM_LEAVE(info)
    }
}

rt_void p_test12(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_elem *iar0 = info->iar0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;
    rt_elem *iso1 = info->iso1 + S*RT_OFFS_SIMD;
    rt_elem *iso2 = info->iso2 + S*RT_OFFS_SIMD;

    j = n;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("iarr[%d] = %" PR_L "d\n",
                j, iar0[j]);

        RT_LOGI("C iarr[%d]&(iarr[%d]<<7) = %" PR_L "d, "
                 "~iarr[%d]&(iarr[%d]>>3) = %" PR_L "d\n",
                j, j, ico1[j], j, j, ico2[j]);

        RT_LOGI("S iarr[%d]&(iarr[%d]<<7) = %" PR_L "d, "
                 "~iarr[%d]&(iarr[%d]>>3) = %" PR_L "d\n",
                j, j, iso1[j], j, j, iso2[j]);
    }
}

#endif /* SUB_TEST 12 */

/******************************************************************************/
/*******************************   SUB TEST 13   ******************************/
/******************************************************************************/

#if SUB_TEST >= 13

rt_void c_test13(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_real *fco1 = info->fco1 + S*RT_OFFS_SIMD;
    rt_real *fco2 = info->fco2 + S*RT_OFFS_SIMD;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            fco1[j] = RT_POW(far0[j], 1.0 / 3.0);
            fco2[j] = -1.0 / RT_SQRT(far0[j]);
        }
    }
}

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The test code below was designed mainly for assembler validation purposes
 * and therefore may not fully represent its unlocked performance potential.
 * For optimal results keep ASM sections in separate functions away from
 * complex C/C++ logic, while making sure those functions are not inlined.
 * This is needed for better compatibility with modern optimizing compilers.
 */
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
        negps_rx(Xmm3)
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)

        movpx_ld(Xmm0, Mecx, AJ1)
        cbrps_rr(Xmm2, Xmm5, Xmm6, Xmm0) /* destroys Xmm5, Xmm6 */
        rsqps_rr(Xmm3, Xmm0) /* destroys Xmm0 */
        negps_rx(Xmm3)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)

        movpx_ld(Xmm0, Mecx, AJ2)
        cbrps_rr(Xmm2, Xmm5, Xmm6, Xmm0) /* destroys Xmm5, Xmm6 */
        rsqps_rr(Xmm3, Xmm0) /* destroys Xmm0 */
        negps_rx(Xmm3)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void p_test13(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_real *fco1 = info->fco1 + S*RT_OFFS_SIMD;
    rt_real *fco2 = info->fco2 + S*RT_OFFS_SIMD;
    rt_real *fso1 = info->fso1 + S*RT_OFFS_SIMD;
    rt_real *fso2 = info->fso2 + S*RT_OFFS_SIMD;

    j = n;
    while (j-->0)
    {
        if (FEQ(fco1[j], fso1[j]) && FEQ(fco2[j], fso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e\n",
                j, far0[j]);

        RT_LOGI("C RT_POW(farr[%d],1.0/3.0) = %e, "
                    "-1.0/RT_SQRT(farr[%d]) = %e\n",
                j, fco1[j], j, fco2[j]);

        RT_LOGI("S RT_POW(farr[%d],1.0/3.0) = %e, "
                    "-1.0/RT_SQRT(farr[%d]) = %e\n",
                j, fso1[j], j, fso2[j]);
    }
}

#endif /* SUB_TEST 13 */

/******************************************************************************/
/*******************************   SUB TEST 14   ******************************/
/******************************************************************************/

#if SUB_TEST >= 14

rt_void c_test14(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, k, n = info->size;

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;

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

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The test code below was designed mainly for assembler validation purposes
 * and therefore may not fully represent its unlocked performance potential.
 * For optimal results keep ASM sections in separate functions away from
 * complex C/C++ logic, while making sure those functions are not inlined.
 * This is needed for better compatibility with modern optimizing compilers.
 */
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

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;
    rt_elem *iso1 = info->iso1 + S*RT_OFFS_SIMD;
    rt_elem *iso2 = info->iso2 + S*RT_OFFS_SIMD;

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
            RT_LOGI("C (farr[%d]==farr[%d]) = %" PR_L "X, "
                      "(farr[%d]!=farr[%d]) = %" PR_L "X\n",
                    j*S + k, ((j+1)*S + k) % n, ico1[j*S + k],
                    j*S + k, ((j+1)*S + k) % n, ico2[j*S + k]);
        }

        k = S;
        while (k-->0)
        {
            RT_LOGI("S (farr[%d]==farr[%d]) = %" PR_L "X, "
                      "(farr[%d]!=farr[%d]) = %" PR_L "X\n",
                    j*S + k, ((j+1)*S + k) % n, iso1[j*S + k],
                    j*S + k, ((j+1)*S + k) % n, iso2[j*S + k]);
        }
    }
}

#endif /* SUB_TEST 14 */

/******************************************************************************/
/*******************************   SUB TEST 15   ******************************/
/******************************************************************************/

#if SUB_TEST >= 15

rt_void c_test15(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_elem *iar0 = info->iar0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = +iar0[j] >> (3+32*(L-1));
            ico2[j] = -iar0[j] >> (5+32*(L-1));
        }
    }
}

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The test code below was designed mainly for assembler validation purposes
 * and therefore may not fully represent its unlocked performance potential.
 * For optimal results keep ASM sections in separate functions away from
 * complex C/C++ logic, while making sure those functions are not inlined.
 * This is needed for better compatibility with modern optimizing compilers.
 */
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
        shrpn_ri(Xmm2, IB((3+32*(L-1))))
        shrpn_ri(Xmm3, IB((5+32*(L-1))))
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ0)
        xoryx_rr(Recx, Recx)
        subyx_rr(Recx, Reax)
        shryn_ri(Reax, IB((3+32*(L-1))))
        shryn_ri(Recx, IB((5+32*(L-1))))
        movyx_st(Reax, Medx, AJ0)
        movyx_st(Recx, Mebx, AJ0)
#endif /* RT_BASE_TEST */

        movpx_ld(Xmm2, Mesi, AJ1)
        xorpx_rr(Xmm3, Xmm3)
        subpx_ld(Xmm3, Mesi, AJ1)
        shrpn_ri(Xmm2, IB((3+32*(L-1))))
        shrpn_ri(Xmm3, IB((5+32*(L-1))))
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ1)
        xoryx_rr(Recx, Recx)
        subyx_ld(Recx, Mesi, AJ1)
        movyx_st(Reax, Medx, AJ1)
        shryn_mi(Medx, AJ1, IB((3+32*(L-1))))
        movyx_st(Recx, Mebx, AJ1)
        shryn_mi(Mebx, AJ1, IB((5+32*(L-1))))
#endif /* RT_BASE_TEST */

        movpx_ld(Xmm2, Mesi, AJ2)
        xorpx_rr(Xmm3, Xmm3)
        subpx_rr(Xmm3, Xmm2)
        shrpn_ri(Xmm2, IB((3+32*(L-1))))
        shrpn_ri(Xmm3, IB((5+32*(L-1))))
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ2)
        xoryx_rr(Recx, Recx)
        subyx_rr(Recx, Reax)
        shryn_ri(Reax, IB((3+32*(L-1))))
        shryn_ri(Recx, IB((5+32*(L-1))))
        movyx_st(Reax, Medx, AJ2)
        movyx_st(Recx, Mebx, AJ2)
#endif /* RT_BASE_TEST */

        ASM_LEAVE(info)
    }
}

rt_void p_test15(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_elem *iar0 = info->iar0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;
    rt_elem *iso1 = info->iso1 + S*RT_OFFS_SIMD;
    rt_elem *iso2 = info->iso2 + S*RT_OFFS_SIMD;

    j = n;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("iarr[%d] = %" PR_L "d\n",
                j, iar0[j]);

        RT_LOGI("C +iarr[%d]>>3 = %" PR_L "d, -iarr[%d]>>5 = %" PR_L "d\n",
                j, ico1[j], j, ico2[j]);

        RT_LOGI("S +iarr[%d]>>3 = %" PR_L "d, -iarr[%d]>>5 = %" PR_L "d\n",
                j, iso1[j], j, iso2[j]);
    }
}

#endif /* SUB_TEST 15 */

/******************************************************************************/
/*******************************   SUB TEST 16   ******************************/
/******************************************************************************/

#if SUB_TEST >= 16

rt_void c_test16(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_elem *iar0 = info->iar0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;

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

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The test code below was designed mainly for assembler validation purposes
 * and therefore may not fully represent its unlocked performance potential.
 * For optimal results keep ASM sections in separate functions away from
 * complex C/C++ logic, while making sure those functions are not inlined.
 * This is needed for better compatibility with modern optimizing compilers.
 */
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

    rt_elem *iar0 = info->iar0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;
    rt_elem *iso1 = info->iso1 + S*RT_OFFS_SIMD;
    rt_elem *iso2 = info->iso2 + S*RT_OFFS_SIMD;

    j = n;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("iarr[%d] = %" PR_L "d, iarr[%d] = %" PR_L "d\n",
                j, iar0[j], (j/S)*S, iar0[(j/S)*S]);

        RT_LOGI("C iarr[%d]>>iarr[%d] = %" PR_L "d, "
                  "iarr[%d]<<iarr[%d] = %" PR_L "d\n",
                j, (j/S)*S, ico1[j], j, (j/S)*S, ico2[j]);

        RT_LOGI("S iarr[%d]>>iarr[%d] = %" PR_L "d, "
                  "iarr[%d]<<iarr[%d] = %" PR_L "d\n",
                j, (j/S)*S, iso1[j], j, (j/S)*S, iso2[j]);
    }
}

#endif /* SUB_TEST 16 */

/******************************************************************************/
/*******************************   SUB TEST 17   ******************************/
/******************************************************************************/

#if SUB_TEST >= 17

rt_void c_test17(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_real *fco1 = info->fco1 + S*RT_OFFS_SIMD;
    rt_real *fco2 = info->fco2 + S*RT_OFFS_SIMD;

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

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The test code below was designed mainly for assembler validation purposes
 * and therefore may not fully represent its unlocked performance potential.
 * For optimal results keep ASM sections in separate functions away from
 * complex C/C++ logic, while making sure those functions are not inlined.
 * This is needed for better compatibility with modern optimizing compilers.
 */
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

        rnpps_ld(Xmm2, Mecx, AJ1)
        rnmps_ld(Xmm3, Mecx, AJ1)
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

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_real *fco1 = info->fco1 + S*RT_OFFS_SIMD;
    rt_real *fco2 = info->fco2 + S*RT_OFFS_SIMD;
    rt_real *fso1 = info->fso1 + S*RT_OFFS_SIMD;
    rt_real *fso2 = info->fso2 + S*RT_OFFS_SIMD;

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

#endif /* SUB_TEST 17 */

/******************************************************************************/
/*******************************   SUB TEST 18   ******************************/
/******************************************************************************/

#if SUB_TEST >= 18

rt_void c_test18(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_elem *iar0 = info->iar0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;

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

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The test code below was designed mainly for assembler validation purposes
 * and therefore may not fully represent its unlocked performance potential.
 * For optimal results keep ASM sections in separate functions away from
 * complex C/C++ logic, while making sure those functions are not inlined.
 * This is needed for better compatibility with modern optimizing compilers.
 */
rt_void s_test18(rt_SIMD_INFOX *info)
{
    ASM_ENTER(info)

        movwx_ld(Reax, Mebp, inf_CYC)
        movwx_st(Reax, Mebp, inf_LOC)

    LBL(cyc_ini)

        movxx_ld(Recx, Mebp, inf_IAR0)
        movxx_ld(Rebx, Mebp, inf_ISO1)
        movxx_ld(Resi, Mebp, inf_ISO2)
        movwx_ld(Redi, Mebp, inf_SIZE)

    LBL(loc_ini)

        movyx_ld(Reax, Mecx, AJ0)
        mulyn_xm(Mecx, AJ1)
        movyx_st(Reax, Mebx, AJ0)
        movyx_ld(Reax, Mecx, AJ0)
        preyn_xx()
        remyn_xx()
        divyn_xm(Mecx, AJ1)
        remyn_xm(Mecx, AJ1)
        movyx_st(Redx, Mesi, AJ0)

        addxx_ri(Recx, IB(4*L))
        addxx_ri(Rebx, IB(4*L))
        addxx_ri(Resi, IB(4*L))
        subwx_ri(Redi, IB(1))
        cmjwx_ri(Redi, IB(S),
        /* if */ GT_x, loc_ini)

        movxx_ld(Redi, Mebp, inf_IAR0)
        movwx_mi(Mebp, inf_SIMD, IB(S))

    LBL(smd_ini)

        movyx_ld(Reax, Mecx, AJ0)
        mulyn_xm(Medi, AJ0)
        movyx_st(Reax, Mebx, AJ0)
        movyx_ld(Reax, Mecx, AJ0)
        preyn_xx()
        remyn_xx()
        divyn_xm(Medi, AJ0)
        remyn_xm(Mecx, AJ1)
        movyx_st(Redx, Mesi, AJ0)

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

        jmpxx_lb(cyc_ini)

    LBL(cyc_fin)

    ASM_LEAVE(info)
}

rt_void p_test18(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_elem *iar0 = info->iar0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;
    rt_elem *iso1 = info->iso1 + S*RT_OFFS_SIMD;
    rt_elem *iso2 = info->iso2 + S*RT_OFFS_SIMD;

    j = n;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("iarr[%d] = %" PR_L "d, iarr[%d] = %" PR_L "d\n",
                j, iar0[j], (j + S) % n, iar0[(j + S) % n]);

        RT_LOGI("C iarr[%d]*iarr[%d] = %" PR_L "d, "
                  "iarr[%d]%%iarr[%d] = %" PR_L "d\n",
                j, (j + S) % n, ico1[j], j, (j + S) % n, ico2[j]);

        RT_LOGI("S iarr[%d]*iarr[%d] = %" PR_L "d, "
                  "iarr[%d]%%iarr[%d] = %" PR_L "d\n",
                j, (j + S) % n, iso1[j], j, (j + S) % n, iso2[j]);
    }
}

#endif /* SUB_TEST 18 */

/******************************************************************************/
/*******************************   SUB TEST 19   ******************************/
/******************************************************************************/

#if SUB_TEST >= 19

rt_void c_test19(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_elem *iar0 = info->iar0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = ~iar0[j] | ((rt_uelm)iar0[j] >> 7);
            ico2[j] = ~iar0[j];
        }
    }
}

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The test code below was designed mainly for assembler validation purposes
 * and therefore may not fully represent its unlocked performance potential.
 * For optimal results keep ASM sections in separate functions away from
 * complex C/C++ logic, while making sure those functions are not inlined.
 * This is needed for better compatibility with modern optimizing compilers.
 */
rt_void s_test19(rt_SIMD_INFOX *info)
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
        movpx_rr(Xmm1, Xmm0)
        movpx_rr(Xmm2, Xmm0)
        shrpx_ri(Xmm0, IB(7))
        ornpx_rr(Xmm1, Xmm0)
        notpx_rx(Xmm2)
        movpx_st(Xmm1, Medx, AJ0)
        movpx_st(Xmm2, Mebx, AJ0)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ0)
        movyx_rr(Recx, Reax)
        movyx_rr(Redi, Reax)
        shryx_ri(Reax, IB(7))
        ornyx_rr(Recx, Reax)
        notyx_rx(Redi)
        movyx_st(Recx, Medx, AJ0)
        movyx_st(Redi, Mebx, AJ0)
#endif /* RT_BASE_TEST */

        movpx_ld(Xmm0, Mesi, AJ1)
        movpx_rr(Xmm1, Xmm0)
        movpx_rr(Xmm2, Xmm0)
        shrpx_ri(Xmm0, IB(7))
        movpx_st(Xmm0, Medx, AJ1)
        ornpx_ld(Xmm1, Medx, AJ1)
        notpx_rx(Xmm2)
        movpx_st(Xmm1, Medx, AJ1)
        movpx_st(Xmm2, Mebx, AJ1)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ1)
        movyx_rr(Recx, Reax)
        movyx_rr(Redi, Reax)
        movyx_st(Recx, Medx, AJ1)
        movyx_st(Redi, Mebx, AJ1)
        shryx_ri(Reax, IB(7))
        ornyx_st(Reax, Medx, AJ1)
        notyx_mx(Mebx, AJ1)
#endif /* RT_BASE_TEST */

        movpx_ld(Xmm0, Mesi, AJ2)
        movpx_rr(Xmm1, Xmm0)
        movpx_rr(Xmm2, Xmm0)
        shrpx_ri(Xmm0, IB(7))
        ornpx_rr(Xmm1, Xmm0)
        notpx_rx(Xmm2)
        movpx_st(Xmm1, Medx, AJ2)
        movpx_st(Xmm2, Mebx, AJ2)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ2)
        movyx_rr(Recx, Reax)
        movyx_rr(Redi, Reax)
        shryx_ri(Reax, IB(7))
        movyx_st(Reax, Medx, AJ2)
        ornyx_ld(Recx, Medx, AJ2)
        notyx_rx(Redi)
        movyx_st(Recx, Medx, AJ2)
        movyx_st(Redi, Mebx, AJ2)
#endif /* RT_BASE_TEST */

        ASM_LEAVE(info)
    }
}

rt_void p_test19(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_elem *iar0 = info->iar0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;
    rt_elem *iso1 = info->iso1 + S*RT_OFFS_SIMD;
    rt_elem *iso2 = info->iso2 + S*RT_OFFS_SIMD;

    j = n;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("iarr[%d] = %" PR_L "d\n",
                j, iar0[j]);

        RT_LOGI("C ~iarr[%d]|(iarr[%d]<<7) = %" PR_L "d, "
                  "~iarr[%d] = %" PR_L "d\n",
                j, j, ico1[j], j, ico2[j]);

        RT_LOGI("S ~iarr[%d]|(iarr[%d]<<7) = %" PR_L "d, "
                  "~iarr[%d] = %" PR_L "d\n",
                j, j, iso1[j], j, iso2[j]);
    }
}

#endif /* SUB_TEST 19 */

/******************************************************************************/
/*******************************   SUB TEST 20   ******************************/
/******************************************************************************/

#if SUB_TEST >= 20

rt_void c_test20(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_real *fco1 = info->fco1 + S*RT_OFFS_SIMD;
    rt_real *fco2 = info->fco2 + S*RT_OFFS_SIMD;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            fco1[j] = far0[j] + far0[(j + S) % n] * far0[(j + 2*S) % n];
            fco2[j] = far0[j] - far0[(j + S) % n] * far0[(j + 2*S) % n];
        }
    }
}

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The test code below was designed mainly for assembler validation purposes
 * and therefore may not fully represent its unlocked performance potential.
 * For optimal results keep ASM sections in separate functions away from
 * complex C/C++ logic, while making sure those functions are not inlined.
 * This is needed for better compatibility with modern optimizing compilers.
 */
rt_void s_test20(rt_SIMD_INFOX *info)
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
        movpx_ld(Xmm2, Mecx, AJ2)
        movpx_rr(Xmm3, Xmm0)
        fmaps_rr(Xmm0, Xmm1, Xmm2)
        fmsps_rr(Xmm3, Xmm1, Xmm2)
        movpx_st(Xmm0, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)
#ifdef RT_ELEM_TEST
        movss_ld(Xmm0, Mecx, AJ0)
        movss_ld(Xmm1, Mecx, AJ1)
        movss_ld(Xmm2, Mecx, AJ2)
        movss_rr(Xmm3, Xmm0)
        fmass_rr(Xmm0, Xmm1, Xmm2)
        fmsss_rr(Xmm3, Xmm1, Xmm2)
        movss_st(Xmm0, Medx, AJ0)
        movss_st(Xmm3, Mebx, AJ0)
#endif /* RT_ELEM_TEST */

        movpx_ld(Xmm0, Mecx, AJ1)
        movpx_ld(Xmm1, Mecx, AJ2)
        movpx_rr(Xmm3, Xmm0)
        fmaps_ld(Xmm0, Xmm1, Mecx, AJ0)
        fmsps_ld(Xmm3, Xmm1, Mecx, AJ0)
        movpx_st(Xmm0, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)
#ifdef RT_ELEM_TEST
        movss_ld(Xmm0, Mecx, AJ1)
        movss_ld(Xmm1, Mecx, AJ2)
        movss_rr(Xmm3, Xmm0)
        fmass_ld(Xmm0, Xmm1, Mecx, AJ0)
        fmsss_ld(Xmm3, Xmm1, Mecx, AJ0)
        movss_st(Xmm0, Medx, AJ1)
        movss_st(Xmm3, Mebx, AJ1)
#endif /* RT_ELEM_TEST */

        movpx_ld(Xmm0, Mecx, AJ2)
        movpx_ld(Xmm1, Mecx, AJ0)
        movpx_ld(Xmm2, Mecx, AJ1)
        movpx_rr(Xmm3, Xmm0)
        fmaps_rr(Xmm0, Xmm1, Xmm2)
        fmsps_rr(Xmm3, Xmm1, Xmm2)
        movpx_st(Xmm0, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)
#ifdef RT_ELEM_TEST
        movss_ld(Xmm0, Mecx, AJ2)
        movss_ld(Xmm1, Mecx, AJ0)
        movss_ld(Xmm2, Mecx, AJ1)
        movss_rr(Xmm3, Xmm0)
        fmass_rr(Xmm0, Xmm1, Xmm2)
        fmsss_rr(Xmm3, Xmm1, Xmm2)
        movss_st(Xmm0, Medx, AJ2)
        movss_st(Xmm3, Mebx, AJ2)
#endif /* RT_ELEM_TEST */

        ASM_LEAVE(info)
    }
}

rt_void p_test20(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_real *fco1 = info->fco1 + S*RT_OFFS_SIMD;
    rt_real *fco2 = info->fco2 + S*RT_OFFS_SIMD;
    rt_real *fso1 = info->fso1 + S*RT_OFFS_SIMD;
    rt_real *fso2 = info->fso2 + S*RT_OFFS_SIMD;

    j = n;
    while (j-->0)
    {
        if (FEQ(fco1[j], fso1[j]) && FEQ(fco2[j], fso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e, farr[%d] = %e, farr[%d] = %e\n",
                j, far0[j], (j + S) % n, far0[(j + S) % n],
                (j + 2*S) % n, far0[(j + 2*S) % n]);

        RT_LOGI("C farr[%d]+farr[%d]*farr[%d] = %+.25e, "
                  "farr[%d]-farr[%d]*farr[%d] = %+.25e\n",
                j, (j + S) % n, (j + 2*S) % n, fco1[j],
                j, (j + S) % n, (j + 2*S) % n, fco2[j]);

        RT_LOGI("S farr[%d]+farr[%d]*farr[%d] = %+.25e, "
                  "farr[%d]-farr[%d]*farr[%d] = %+.25e\n",
                j, (j + S) % n, (j + 2*S) % n, fso1[j],
                j, (j + S) % n, (j + 2*S) % n, fso2[j]);
    }
}

#endif /* SUB_TEST 20 */

/******************************************************************************/
/*******************************   SUB TEST 21   ******************************/
/******************************************************************************/

#if SUB_TEST >= 21

rt_void c_test21(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_elem *iar0 = info->iar0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] =  iar0[j] / 117;
            ico2[j] = -iar0[j] / 289;
        }
    }
}

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The test code below was designed mainly for assembler validation purposes
 * and therefore may not fully represent its unlocked performance potential.
 * For optimal results keep ASM sections in separate functions away from
 * complex C/C++ logic, while making sure those functions are not inlined.
 * This is needed for better compatibility with modern optimizing compilers.
 */
rt_void s_test21(rt_SIMD_INFOX *info)
{
    rt_si32 i;

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Resi, Mebp, inf_IAR0)
        movxx_ld(Redx, Mebp, inf_ISO1)
        movxx_ld(Rebx, Mebp, inf_ISO2)

        movwx_mi(Mebp, inf_SIMD, IB(S))

    LBL(div_ini)

        movyx_ld(Reax, Mesi, AJ0)
        movyx_rr(Recx, Reax)
        divyx_ri(Recx, IB(117))
        movyx_st(Recx, Medx, AJ0)
        movyx_rr(Recx, Reax)
        negyx_rx(Recx)
        divyn_ri(Recx, IH(289))
        movyx_st(Recx, Mebx, AJ0)

        movyx_ld(Reax, Mesi, AJ1)
        movyx_rr(Recx, Reax)
        movyx_ri(Redi, IB(117))
        divyx_rr(Recx, Redi)
        movyx_st(Recx, Medx, AJ1)
        movyx_rr(Recx, Reax)
        negyx_rx(Recx)
        movyx_ri(Redi, IH(289))
        divyn_rr(Recx, Redi)
        movyx_st(Recx, Mebx, AJ1)

        movyx_ld(Reax, Mesi, AJ2)
        movyx_rr(Recx, Reax)
        movxx_rr(Redi, Redx)
        movyx_mi(Medi, AJ2, IB(117))
        divyx_ld(Recx, Medi, AJ2)
        movyx_st(Recx, Medi, AJ2)
        movyx_rr(Recx, Reax)
        negyx_rx(Recx)
        movyx_mi(Mebx, AJ2, IH(289))
        divyn_ld(Recx, Mebx, AJ2)
        movyx_st(Recx, Mebx, AJ2)

        addxx_ri(Resi, IB(L*4))
        addxx_ri(Redx, IB(L*4))
        addxx_ri(Rebx, IB(L*4))

        arjwx_mi(Mebp, inf_SIMD, IB(1),
        sub_x,   NZ_x, div_ini)

        ASM_LEAVE(info)
    }
}

rt_void p_test21(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_elem *iar0 = info->iar0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;
    rt_elem *iso1 = info->iso1 + S*RT_OFFS_SIMD;
    rt_elem *iso2 = info->iso2 + S*RT_OFFS_SIMD;

    j = n;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("iarr[%d] = %" PR_L "d\n",
                j, iar0[j]);

        RT_LOGI("C iarr[%d]/117 = %" PR_L "d, "
                 "-iarr[%d]/289 = %" PR_L "d\n",
                j, ico1[j], j, ico2[j]);

        RT_LOGI("S iarr[%d]/117 = %" PR_L "d, "
                 "-iarr[%d]/289 = %" PR_L "d\n",
                j, iso1[j], j, iso2[j]);
    }
}

#endif /* SUB_TEST 21 */

/******************************************************************************/
/*******************************   SUB TEST 22   ******************************/
/******************************************************************************/

#if SUB_TEST >= 22

rt_void c_test22(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_elem *iar0 = info->iar0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] =  iar0[j] % 117;
            ico2[j] = -iar0[j] % 289;
        }
    }
}

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The test code below was designed mainly for assembler validation purposes
 * and therefore may not fully represent its unlocked performance potential.
 * For optimal results keep ASM sections in separate functions away from
 * complex C/C++ logic, while making sure those functions are not inlined.
 * This is needed for better compatibility with modern optimizing compilers.
 */
rt_void s_test22(rt_SIMD_INFOX *info)
{
    rt_si32 i;

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Resi, Mebp, inf_IAR0)
        movxx_ld(Redx, Mebp, inf_ISO1)
        movxx_ld(Rebx, Mebp, inf_ISO2)

        movwx_mi(Mebp, inf_SIMD, IB(S))

    LBL(rem_ini)

        movyx_ld(Reax, Mesi, AJ0)
        movyx_rr(Recx, Reax)
        remyx_ri(Recx, IB(117))
        movyx_st(Recx, Medx, AJ0)
        movyx_rr(Recx, Reax)
        negyx_rx(Recx)
        remyn_ri(Recx, IH(289))
        movyx_st(Recx, Mebx, AJ0)

        movyx_ld(Reax, Mesi, AJ1)
        movyx_rr(Recx, Reax)
        movyx_ri(Redi, IB(117))
        remyx_rr(Recx, Redi)
        movyx_st(Recx, Medx, AJ1)
        movyx_rr(Recx, Reax)
        negyx_rx(Recx)
        movyx_ri(Redi, IH(289))
        remyn_rr(Recx, Redi)
        movyx_st(Recx, Mebx, AJ1)

        movyx_ld(Reax, Mesi, AJ2)
        movyx_rr(Recx, Reax)
        movxx_rr(Redi, Redx)
        movyx_mi(Medi, AJ2, IB(117))
        remyx_ld(Recx, Medi, AJ2)
        movyx_st(Recx, Medi, AJ2)
        movyx_rr(Recx, Reax)
        negyx_rx(Recx)
        movyx_mi(Mebx, AJ2, IH(289))
        remyn_ld(Recx, Mebx, AJ2)
        movyx_st(Recx, Mebx, AJ2)

        addxx_ri(Resi, IB(L*4))
        addxx_ri(Redx, IB(L*4))
        addxx_ri(Rebx, IB(L*4))

        arjwx_mi(Mebp, inf_SIMD, IB(1),
        sub_x,   NZ_x, rem_ini)

        ASM_LEAVE(info)
    }
}

rt_void p_test22(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_elem *iar0 = info->iar0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;
    rt_elem *iso1 = info->iso1 + S*RT_OFFS_SIMD;
    rt_elem *iso2 = info->iso2 + S*RT_OFFS_SIMD;

    j = n;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("iarr[%d] = %" PR_L "d\n",
                j, iar0[j]);

        RT_LOGI("C iarr[%d]%%117 = %" PR_L "d, "
                 "-iarr[%d]%%289 = %" PR_L "d\n",
                j, ico1[j], j, ico2[j]);

        RT_LOGI("S iarr[%d]%%117 = %" PR_L "d, "
                 "-iarr[%d]%%289 = %" PR_L "d\n",
                j, iso1[j], j, iso2[j]);
    }
}

#endif /* SUB_TEST 22 */

/******************************************************************************/
/*******************************   SUB TEST 23   ******************************/
/******************************************************************************/

#if SUB_TEST >= 23

rt_void c_test23(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_elem *iar0 = info->iar0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = (iar0[j] >> 5) | (iar0[j] << (32 * L - 5));
            ico2[j] = (iar0[j] >> 31) | (iar0[j] << (32 * L - 31));
        }
    }
}

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The test code below was designed mainly for assembler validation purposes
 * and therefore may not fully represent its unlocked performance potential.
 * For optimal results keep ASM sections in separate functions away from
 * complex C/C++ logic, while making sure those functions are not inlined.
 * This is needed for better compatibility with modern optimizing compilers.
 */
rt_void s_test23(rt_SIMD_INFOX *info)
{
    rt_si32 i;

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Resi, Mebp, inf_IAR0)
        movxx_ld(Redx, Mebp, inf_ISO1)
        movxx_ld(Rebx, Mebp, inf_ISO2)

        movwx_mi(Mebp, inf_SIMD, IB(S))

    LBL(ror_ini)

        movyx_ld(Reax, Mesi, AJ0)
        movyx_rr(Recx, Reax)
        roryx_ri(Recx, IB(5))
        movyx_st(Recx, Medx, AJ0)
        movyx_st(Reax, Mebx, AJ0)
        roryx_mi(Mebx, AJ0, IB(31))

        movyx_ld(Reax, Mesi, AJ1)
        movyx_rr(Redi, Reax)
        movyx_ri(Recx, IB(5))
        roryx_rx(Redi)
        movyx_st(Redi, Medx, AJ1)
        movyx_rr(Redi, Reax)
        movyx_ri(Reax, IB(31))
        roryx_rr(Redi, Reax)
        movyx_st(Redi, Mebx, AJ1)

        movyx_ld(Reax, Mesi, AJ2)
        movyx_rr(Redi, Reax)
        movyx_mi(Medx, AJ2, IB(5))
        roryx_ld(Redi, Medx, AJ2)
        movyx_st(Redi, Medx, AJ2)
        movyx_st(Reax, Mebx, AJ2)
        movyx_ri(Reax, IB(31))
        roryx_st(Reax, Mebx, AJ2)

        addxx_ri(Resi, IB(L*4))
        addxx_ri(Redx, IB(L*4))
        addxx_ri(Rebx, IB(L*4))

        arjwx_mi(Mebp, inf_SIMD, IB(1),
        sub_x,   NZ_x, ror_ini)

        ASM_LEAVE(info)
    }
}

rt_void p_test23(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_elem *iar0 = info->iar0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;
    rt_elem *iso1 = info->iso1 + S*RT_OFFS_SIMD;
    rt_elem *iso2 = info->iso2 + S*RT_OFFS_SIMD;

    j = n;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("iarr[%d] = %" PR_L "d\n",
                j, iar0[j]);

        RT_LOGI("C iarr[%d] ror 5 = %" PR_L "d, "
                 " iarr[%d] ror 31 = %" PR_L "d\n",
                j, ico1[j], j, ico2[j]);

        RT_LOGI("S iarr[%d] ror 5 = %" PR_L "d, "
                 " iarr[%d] ror 31 = %" PR_L "d\n",
                j, iso1[j], j, iso2[j]);
    }
}

#endif /* SUB_TEST 23 */

/******************************************************************************/
/*******************************   SUB TEST 24   ******************************/
/******************************************************************************/

#if SUB_TEST >= 24

rt_void c_test24(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_elem *iar0 = info->iar0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = iar0[j] << (iar0[j] & ((16 << L) - 1));
            ico2[j] = iar0[j] >> (iar0[j] & ((16 << L) - 1));
        }
    }
}

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The test code below was designed mainly for assembler validation purposes
 * and therefore may not fully represent its unlocked performance potential.
 * For optimal results keep ASM sections in separate functions away from
 * complex C/C++ logic, while making sure those functions are not inlined.
 * This is needed for better compatibility with modern optimizing compilers.
 */
rt_void s_test24(rt_SIMD_INFOX *info)
{
    rt_si32 i;

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Resi, Mebp, inf_IAR0)
        movxx_ld(Redx, Mebp, inf_ISO1)
        movxx_ld(Rebx, Mebp, inf_ISO2)

        movpx_ld(Xmm7, Mebp, inf_GPC07)
        shrpx_ri(Xmm7, IB(31*L-4))

        movpx_ld(Xmm0, Mesi, AJ0)
        movpx_rr(Xmm1, Xmm0)
        movpx_rr(Xmm2, Xmm0)
        andpx_rr(Xmm0, Xmm7)
        svlpx_rr(Xmm1, Xmm0)
        svrpn_rr(Xmm2, Xmm0)
        movpx_st(Xmm1, Medx, AJ0)
        movpx_st(Xmm2, Mebx, AJ0)

        movpx_ld(Xmm0, Mesi, AJ1)
        movpx_rr(Xmm1, Xmm0)
        movpx_rr(Xmm2, Xmm0)
        andpx_rr(Xmm0, Xmm7)
        movpx_st(Xmm0, Medx, AJ1)
        svlpx_ld(Xmm1, Medx, AJ1)
        svrpn_ld(Xmm2, Medx, AJ1)
        movpx_st(Xmm1, Medx, AJ1)
        movpx_st(Xmm2, Mebx, AJ1)

        movpx_ld(Xmm0, Mesi, AJ2)
        movpx_rr(Xmm1, Xmm0)
        movpx_rr(Xmm2, Xmm0)
        andpx_rr(Xmm0, Xmm7)
        svlpx_rr(Xmm1, Xmm0)
        svrpn_rr(Xmm2, Xmm0)
        movpx_st(Xmm1, Medx, AJ2)
        movpx_st(Xmm2, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void p_test24(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_elem *iar0 = info->iar0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;
    rt_elem *iso1 = info->iso1 + S*RT_OFFS_SIMD;
    rt_elem *iso2 = info->iso2 + S*RT_OFFS_SIMD;

    j = n;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("iarr[%d] = %" PR_L "d\n",
                j, iar0[j]);

        RT_LOGI("C iarr[%d]<<(iarr[%d]&((16<<L)-1)) = %" PR_L "d, "
                  "iarr[%d]>>(iarr[%d]&((16<<L)-1)) = %" PR_L "d\n",
                j, j, ico1[j], j, j, ico2[j]);

        RT_LOGI("S iarr[%d]<<(iarr[%d]&((16<<L)-1)) = %" PR_L "d, "
                  "iarr[%d]>>(iarr[%d]&((16<<L)-1)) = %" PR_L "d\n",
                j, j, iso1[j], j, j, iso2[j]);
    }
}

#endif /* SUB_TEST 24 */

/******************************************************************************/
/*******************************   SUB TEST 25   ******************************/
/******************************************************************************/

#if SUB_TEST >= 25

rt_void c_test25(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_real *fco1 = info->fco1 + S*RT_OFFS_SIMD;
    rt_real *fco2 = info->fco2 + S*RT_OFFS_SIMD;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            fco1[j] = j < n / 2 ? far0[j*2+0] + far0[j*2+1] : 0.0f;
            fco2[j] = j < n / 2 ? far0[j*2+0] * far0[j*2+1] : 0.0f;
        }
#ifdef RT_ELEM_TEST
        j = n/3;
        fco1[n/2+0] = 0.0f;
        fco2[n/2+0] = 1.0f;
        while (j-->0)
        {
            fco1[n/2+0] += RT_SQRT(RT_SQRT(RT_SQRT(RT_SQRT(far0[(n/3)*0+j]))));
            fco2[n/2+0] *= RT_SQRT(RT_SQRT(RT_SQRT(RT_SQRT(far0[(n/3)*0+j]))));
        }
        j = n/3;
        fco1[n/2+1] = 0.0f;
        fco2[n/2+1] = 1.0f;
        while (j-->0)
        {
            fco1[n/2+1] += RT_SQRT(RT_SQRT(RT_SQRT(RT_SQRT(far0[(n/3)*1+j]))));
            fco2[n/2+1] *= RT_SQRT(RT_SQRT(RT_SQRT(RT_SQRT(far0[(n/3)*1+j]))));
        }
        j = n/3;
        fco1[n/2+2] = 0.0f;
        fco2[n/2+2] = 1.0f;
        while (j-->0)
        {
            fco1[n/2+2] += RT_SQRT(RT_SQRT(RT_SQRT(RT_SQRT(far0[(n/3)*2+j]))));
            fco2[n/2+2] *= RT_SQRT(RT_SQRT(RT_SQRT(RT_SQRT(far0[(n/3)*2+j]))));
        }
#endif /* RT_ELEM_TEST */
    }
}

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The test code below was designed mainly for assembler validation purposes
 * and therefore may not fully represent its unlocked performance potential.
 * For optimal results keep ASM sections in separate functions away from
 * complex C/C++ logic, while making sure those functions are not inlined.
 * This is needed for better compatibility with modern optimizing compilers.
 */
rt_void s_test25(rt_SIMD_INFOX *info)
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
        movpx_rr(Xmm2, Xmm0)
        addxx_ri(Recx, IM(16*Q))
        adpps_ld(Xmm0, Mecx, AJ0)
        mlpps_ld(Xmm2, Mecx, AJ0)
        addxx_ri(Recx, IM(16*Q))
        movpx_st(Xmm0, Medx, AJ0)
        movpx_st(Xmm2, Mebx, AJ0)
        addxx_ri(Redx, IM(16*Q))
        addxx_ri(Rebx, IM(16*Q))

        xorpx_rr(Xmm1, Xmm1)
        movpx_ld(Xmm0, Mecx, AJ0)
        movpx_rr(Xmm2, Xmm0)
        adpps_rr(Xmm0, Xmm1)
        mlpps_rr(Xmm2, Xmm1)
        movpx_st(Xmm0, Medx, AJ0)
        movpx_st(Xmm2, Mebx, AJ0)
        addxx_ri(Redx, IM(16*Q))
        addxx_ri(Rebx, IM(16*Q))
        movpx_st(Xmm1, Medx, AJ0)
        movpx_st(Xmm1, Mebx, AJ0)

#ifdef RT_ELEM_TEST
        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Redx, Mebp, inf_FSO1)
        movxx_ld(Rebx, Mebp, inf_FSO2)

        addxx_ri(Redx, IM(24*Q))
        addxx_ri(Rebx, IM(24*Q))

        movpx_ld(Xmm0, Mecx, AJ0)
        sqrps_rr(Xmm0, Xmm0)
        sqrps_rr(Xmm0, Xmm0)
        sqrps_rr(Xmm0, Xmm0)
        sqrps_rr(Xmm0, Xmm0)
        adhps_rr(Xmm1, Xmm0)
        elmpx_st(Xmm1, Medx, AJ0)
        addxx_ri(Redx, IB(4*L))
        mlhps_rr(Xmm4, Xmm0)
        elmpx_st(Xmm4, Mebx, AJ0)
        addxx_ri(Rebx, IB(4*L))

        movpx_ld(Xmm0, Mecx, AJ1)
        sqrps_rr(Xmm0, Xmm0)
        sqrps_rr(Xmm0, Xmm0)
        sqrps_rr(Xmm0, Xmm0)
        sqrps_rr(Xmm0, Xmm0)
        adhps_rr(Xmm2, Xmm0)
        elmpx_st(Xmm2, Medx, AJ0)
        addxx_ri(Redx, IB(4*L))
        mlhps_rr(Xmm5, Xmm0)
        elmpx_st(Xmm5, Mebx, AJ0)
        addxx_ri(Rebx, IB(4*L))

        movpx_ld(Xmm0, Mecx, AJ2)
        sqrps_rr(Xmm0, Xmm0)
        sqrps_rr(Xmm0, Xmm0)
        sqrps_rr(Xmm0, Xmm0)
        sqrps_rr(Xmm0, Xmm0)
        adhps_rr(Xmm3, Xmm0)
        elmpx_st(Xmm3, Medx, AJ0)
        addxx_ri(Redx, IB(4*L))
        mlhps_rr(Xmm6, Xmm0)
        elmpx_st(Xmm6, Mebx, AJ0)
        addxx_ri(Rebx, IB(4*L))
#endif /* RT_ELEM_TEST */

        ASM_LEAVE(info)
    }
}

rt_void p_test25(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_real *fco1 = info->fco1 + S*RT_OFFS_SIMD;
    rt_real *fco2 = info->fco2 + S*RT_OFFS_SIMD;
    rt_real *fso1 = info->fso1 + S*RT_OFFS_SIMD;
    rt_real *fso2 = info->fso2 + S*RT_OFFS_SIMD;

    j = n;
    while (j-->0)
    {
        if (FEQ(fco1[j], fso1[j]) && FEQ(fco2[j], fso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e, farr[%d] = %e\n",
                2*j+0, 2*j+0 < n ? far0[2*j+0] : 0.0f,
                2*j+1, 2*j+1 < n ? far0[2*j+1] : 0.0f);

        RT_LOGI("C farr[%d]+farr[%d] = %e, farr[%d]*farr[%d] = %e\n",
                2*j+0, 2*j+1, fco1[j], 2*j+0, 2*j+1, fco2[j]);

        RT_LOGI("S farr[%d]+farr[%d] = %e, farr[%d]*farr[%d] = %e\n",
                2*j+0, 2*j+1, fso1[j], 2*j+0, 2*j+1, fso2[j]);
    }
}

#endif /* SUB_TEST 25 */

/******************************************************************************/
/*******************************   SUB TEST 26   ******************************/
/******************************************************************************/

#if SUB_TEST >= 26

rt_void c_test26(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_real *fco1 = info->fco1 + S*RT_OFFS_SIMD;
    rt_real *fco2 = info->fco2 + S*RT_OFFS_SIMD;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            fco1[j] = j < n / 2 ? RT_MIN(far0[j*2+0], far0[j*2+1]) : 0.0f;
            fco2[j] = j < n / 2 ? RT_MAX(far0[j*2+0], far0[j*2+1]) : 0.0f;
        }
#ifdef RT_ELEM_TEST
        j = n/3;
        fco1[n/2+0] = +RT_INF;
        fco2[n/2+0] = -RT_INF;
        while (j-->0)
        {
            fco1[n/2+0] = RT_MIN(fco1[n/2+0], far0[(n/3)*0+j]);
            fco2[n/2+0] = RT_MAX(fco2[n/2+0], far0[(n/3)*0+j]);
        }
        j = n/3;
        fco1[n/2+1] = +RT_INF;
        fco2[n/2+1] = -RT_INF;
        while (j-->0)
        {
            fco1[n/2+1] = RT_MIN(fco1[n/2+1], far0[(n/3)*1+j]);
            fco2[n/2+1] = RT_MAX(fco2[n/2+1], far0[(n/3)*1+j]);
        }
        j = n/3;
        fco1[n/2+2] = +RT_INF;
        fco2[n/2+2] = -RT_INF;
        while (j-->0)
        {
            fco1[n/2+2] = RT_MIN(fco1[n/2+2], far0[(n/3)*2+j]);
            fco2[n/2+2] = RT_MAX(fco2[n/2+2], far0[(n/3)*2+j]);
        }
#endif /* RT_ELEM_TEST */
    }
}

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The test code below was designed mainly for assembler validation purposes
 * and therefore may not fully represent its unlocked performance potential.
 * For optimal results keep ASM sections in separate functions away from
 * complex C/C++ logic, while making sure those functions are not inlined.
 * This is needed for better compatibility with modern optimizing compilers.
 */
rt_void s_test26(rt_SIMD_INFOX *info)
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
        movpx_rr(Xmm2, Xmm0)
        addxx_ri(Recx, IM(16*Q))
        mnpps_ld(Xmm0, Mecx, AJ0)
        mxpps_ld(Xmm2, Mecx, AJ0)
        addxx_ri(Recx, IM(16*Q))
        movpx_st(Xmm0, Medx, AJ0)
        movpx_st(Xmm2, Mebx, AJ0)
        addxx_ri(Redx, IM(16*Q))
        addxx_ri(Rebx, IM(16*Q))

        xorpx_rr(Xmm1, Xmm1)
        movpx_ld(Xmm0, Mecx, AJ0)
        movpx_rr(Xmm2, Xmm0)
        mnpps_rr(Xmm0, Xmm1)
        mxpps_rr(Xmm2, Xmm1)
        movpx_st(Xmm0, Medx, AJ0)
        movpx_st(Xmm2, Mebx, AJ0)
        addxx_ri(Redx, IM(16*Q))
        addxx_ri(Rebx, IM(16*Q))
        movpx_st(Xmm1, Medx, AJ0)
        movpx_st(Xmm1, Mebx, AJ0)

#ifdef RT_ELEM_TEST
        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Redx, Mebp, inf_FSO1)
        movxx_ld(Rebx, Mebp, inf_FSO2)

        addxx_ri(Redx, IM(24*Q))
        addxx_ri(Rebx, IM(24*Q))

        movpx_ld(Xmm0, Mecx, AJ0)
        mnhps_rr(Xmm1, Xmm0)
        elmpx_st(Xmm1, Medx, AJ0)
        addxx_ri(Redx, IB(4*L))
        mxhps_rr(Xmm4, Xmm0)
        elmpx_st(Xmm4, Mebx, AJ0)
        addxx_ri(Rebx, IB(4*L))

        mnhps_ld(Xmm2, Mecx, AJ1)
        elmpx_st(Xmm2, Medx, AJ0)
        addxx_ri(Redx, IB(4*L))
        mxhps_ld(Xmm5, Mecx, AJ1)
        elmpx_st(Xmm5, Mebx, AJ0)
        addxx_ri(Rebx, IB(4*L))

        movpx_ld(Xmm0, Mecx, AJ2)
        mnhps_rr(Xmm3, Xmm0)
        elmpx_st(Xmm3, Medx, AJ0)
        addxx_ri(Redx, IB(4*L))
        mxhps_rr(Xmm6, Xmm0)
        elmpx_st(Xmm6, Mebx, AJ0)
        addxx_ri(Rebx, IB(4*L))
#endif /* RT_ELEM_TEST */

        ASM_LEAVE(info)
    }
}

rt_void p_test26(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_real *fco1 = info->fco1 + S*RT_OFFS_SIMD;
    rt_real *fco2 = info->fco2 + S*RT_OFFS_SIMD;
    rt_real *fso1 = info->fso1 + S*RT_OFFS_SIMD;
    rt_real *fso2 = info->fso2 + S*RT_OFFS_SIMD;

    j = n;
    while (j-->0)
    {
        if (FEQ(fco1[j], fso1[j]) && FEQ(fco2[j], fso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e, farr[%d] = %e\n",
                2*j+0, 2*j+0 < n ? far0[2*j+0] : 0.0f,
                2*j+1, 2*j+1 < n ? far0[2*j+1] : 0.0f);

        RT_LOGI("C MIN(farr[%d],farr[%d]) = %e, MAX(farr[%d],farr[%d]) = %e\n",
                2*j+0, 2*j+1, fco1[j], 2*j+0, 2*j+1, fco2[j]);

        RT_LOGI("S MIN(farr[%d],farr[%d]) = %e, MAX(farr[%d],farr[%d]) = %e\n",
                2*j+0, 2*j+1, fso1[j], 2*j+0, 2*j+1, fso2[j]);
    }
}

#endif /* SUB_TEST 26 */

/******************************************************************************/
/*******************************   SUB TEST 27   ******************************/
/******************************************************************************/

#if SUB_TEST >= 27

rt_void c_test27(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_real *fco1 = info->fco1 + S*RT_OFFS_SIMD;
    rt_real *fco2 = info->fco2 + S*RT_OFFS_SIMD;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            fco1[j] = 0.0f;
            if (far0[j] > far0[(j + S) % n])
            {
                fco2[j] = far0[(j + S) % n];
            }
            else
            {
                fco2[j] = far0[(j + 0) % n];
            }
        }
        fco1[0*S+(1%S)] = far0[0*S];
        fco1[1*S+(2%S)] = far0[1*S];
        fco1[2*S+(3%S)] = far0[2*S];
    }
}

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The test code below was designed mainly for assembler validation purposes
 * and therefore may not fully represent its unlocked performance potential.
 * For optimal results keep ASM sections in separate functions away from
 * complex C/C++ logic, while making sure those functions are not inlined.
 * This is needed for better compatibility with modern optimizing compilers.
 */
rt_void s_test27(rt_SIMD_INFOX *info)
{
    rt_si32 i;

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Redx, Mebp, inf_FSO1)
        movxx_ld(Rebx, Mebp, inf_FSO2)

        xorpx_rr(Xmm0, Xmm0)

        movpx_ld(Xmm1, Mecx, AJ0)
        movpx_st(Xmm0, Medx, AJ0)
        elmpx_st(Xmm1, Medx, DP(Q*0x000 + Q*RT_OFFS_DATA + (1&(S-1))*4*L))

        movpx_ld(Xmm2, Mecx, AJ1)
        movpx_st(Xmm0, Medx, AJ1)
        elmpx_st(Xmm2, Medx, DP(Q*0x010 + Q*RT_OFFS_DATA + (2&(S-1))*4*L))

        movpx_ld(Xmm3, Mecx, AJ2)
        movpx_st(Xmm0, Medx, AJ2)
        elmpx_st(Xmm3, Medx, DP(Q*0x020 + Q*RT_OFFS_DATA + (3&(S-1))*4*L))

        movpx_ld(Xmm0, Mecx, AJ0)
        cgtps_ld(Xmm0, Mecx, AJ1)
        movpx_ld(Xmm1, Mecx, AJ0)
        movpx_ld(Xmm2, Mecx, AJ1)
        mmvpx_rr(Xmm1, Xmm2)
        movpx_st(Xmm1, Mebx, AJ0)

        movpx_ld(Xmm0, Mecx, AJ1)
        cgtps_ld(Xmm0, Mecx, AJ2)
        movpx_ld(Xmm1, Mecx, AJ1)
        mmvpx_ld(Xmm1, Mecx, AJ2)
        movpx_st(Xmm1, Mebx, AJ1)

        movpx_ld(Xmm0, Mecx, AJ2)
        cgtps_ld(Xmm0, Mecx, AJ0)
        movpx_ld(Xmm1, Mecx, AJ2)
        movpx_st(Xmm1, Mebx, AJ2)
        movpx_ld(Xmm1, Mecx, AJ0)
        mmvpx_st(Xmm1, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void p_test27(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_real *far0 = info->far0 + S*RT_OFFS_SIMD;
    rt_real *fco1 = info->fco1 + S*RT_OFFS_SIMD;
    rt_real *fco2 = info->fco2 + S*RT_OFFS_SIMD;
    rt_real *fso1 = info->fso1 + S*RT_OFFS_SIMD;
    rt_real *fso2 = info->fso2 + S*RT_OFFS_SIMD;

    j = n;
    while (j-->0)
    {
        if (FEQ(fco1[j], fso1[j]) && FEQ(fco2[j], fso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e, farr[%d] = %e\n",
                j, far0[j], (j + S) % n, far0[(j + S) % n]);

        RT_LOGI("C fout[%d] = %e, MIN(farr[%d],farr[%d]) = %e\n",
                j, fco1[j], j, (j + S) % n, fco2[j]);

        RT_LOGI("S fout[%d] = %e, MIN(farr[%d],farr[%d]) = %e\n",
                j, fso1[j], j, (j + S) % n, fso2[j]);
    }
}

#endif /* SUB_TEST 27 */

/******************************************************************************/
/*******************************   SUB TEST 28   ******************************/
/******************************************************************************/

#if SUB_TEST >= 28

rt_void c_test28(rt_SIMD_INFOX *info)
{
    rt_si32 i;

    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_real *fco2 = info->fco2 + S*RT_OFFS_SIMD;

    i = info->cyc;
    while (i-->0)
    {
#if RT_REGS >= 8
        ico1[0] = 21;
        fco2[0] = 36.0f;
#if RT_REGS >= 16
        ico1[0] = 91;
        fco2[0] = 120.0f;
#if RT_REGS >= 32
        fco2[0] = 465.0f;
#endif /* RT_REGS >= 32 */
#endif /* RT_REGS >= 16 */
#endif /* RT_REGS >= 8 */
    }
}

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The test code below was designed mainly for assembler validation purposes
 * and therefore may not fully represent its unlocked performance potential.
 * For optimal results keep ASM sections in separate functions away from
 * complex C/C++ logic, while making sure those functions are not inlined.
 * This is needed for better compatibility with modern optimizing compilers.
 */
rt_void s_test28(rt_SIMD_INFOX *info)
{
    rt_si32 i;

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

#if RT_REGS >= 8

        /* BASE regs */
        movyx_ri(Reax, IB(1))

        movyx_rr(Rebx, Reax)
        addyx_rr(Rebx, Reax)

        movyx_rr(Recx, Rebx)
        addyx_rr(Recx, Reax)

        movyx_rr(Redx, Recx)
        addyx_rr(Redx, Reax)

        movyx_rr(Resi, Redx)
        addyx_rr(Resi, Reax)

        movyx_rr(Redi, Resi)
        addyx_rr(Redi, Reax)

        /* SIMD regs */
        movpx_ld(Xmm0, Mebp, inf_GPC01)

        movpx_rr(Xmm1, Xmm0)
        addps_rr(Xmm1, Xmm0)

        movpx_rr(Xmm2, Xmm1)
        addps_rr(Xmm2, Xmm0)

        movpx_rr(Xmm3, Xmm2)
        addps_rr(Xmm3, Xmm0)

        movpx_rr(Xmm4, Xmm3)
        addps_rr(Xmm4, Xmm0)

        movpx_rr(Xmm5, Xmm4)
        addps_rr(Xmm5, Xmm0)

        movpx_rr(Xmm6, Xmm5)
        addps_rr(Xmm6, Xmm0)

        movpx_rr(Xmm7, Xmm6)
        addps_rr(Xmm7, Xmm0)

#if RT_REGS >= 16

        /* SIMD regs */
        movpx_rr(Xmm8, Xmm7)
        addps_rr(Xmm8, Xmm0)

        movpx_rr(Xmm9, Xmm8)
        addps_rr(Xmm9, Xmm0)

        movpx_rr(XmmA, Xmm9)
        addps_rr(XmmA, Xmm0)

        movpx_rr(XmmB, XmmA)
        addps_rr(XmmB, Xmm0)

        movpx_rr(XmmC, XmmB)
        addps_rr(XmmC, Xmm0)

        movpx_rr(XmmD, XmmC)
        addps_rr(XmmD, Xmm0)

        movpx_rr(XmmE, XmmD)
        addps_rr(XmmE, Xmm0)

        /* BASE regs */
        movyx_rr(Reg8, Redi)
        addyx_rr(Reg8, Reax)

        movyx_rr(Reg9, Reg8)
        addyx_rr(Reg9, Reax)

        movyx_rr(RegA, Reg9)
        addyx_rr(RegA, Reax)

        movyx_rr(RegB, RegA)
        addyx_rr(RegB, Reax)

        movyx_rr(RegC, RegB)
        addyx_rr(RegC, Reax)

        movyx_rr(RegD, RegC)
        addyx_rr(RegD, Reax)

        movyx_rr(RegE, RegD)
        addyx_rr(RegE, Reax)

#if RT_REGS >= 32

        /* SIMD regs */
        movpx_rr(XmmF, XmmE)
        addps_rr(XmmF, Xmm0)

        movpx_rr(XmmG, XmmF)
        addps_rr(XmmG, Xmm0)

        movpx_rr(XmmH, XmmG)
        addps_rr(XmmH, Xmm0)

        movpx_rr(XmmI, XmmH)
        addps_rr(XmmI, Xmm0)

        movpx_rr(XmmJ, XmmI)
        addps_rr(XmmJ, Xmm0)

        movpx_rr(XmmK, XmmJ)
        addps_rr(XmmK, Xmm0)

        movpx_rr(XmmL, XmmK)
        addps_rr(XmmL, Xmm0)

        movpx_rr(XmmM, XmmL)
        addps_rr(XmmM, Xmm0)

        movpx_rr(XmmN, XmmM)
        addps_rr(XmmN, Xmm0)

        movpx_rr(XmmO, XmmN)
        addps_rr(XmmO, Xmm0)

        movpx_rr(XmmP, XmmO)
        addps_rr(XmmP, Xmm0)

        movpx_rr(XmmQ, XmmP)
        addps_rr(XmmQ, Xmm0)

        movpx_rr(XmmR, XmmQ)
        addps_rr(XmmR, Xmm0)

        movpx_rr(XmmS, XmmR)
        addps_rr(XmmS, Xmm0)

        movpx_rr(XmmT, XmmS)
        addps_rr(XmmT, Xmm0)

        /* SIMD regs */
        addps_rr(Xmm0, XmmT)
        addps_rr(Xmm0, XmmS)
        addps_rr(Xmm0, XmmR)
        addps_rr(Xmm0, XmmQ)
        addps_rr(Xmm0, XmmP)
        addps_rr(Xmm0, XmmO)
        addps_rr(Xmm0, XmmN)

        addps_rr(Xmm0, XmmM)
        addps_rr(Xmm0, XmmL)
        addps_rr(Xmm0, XmmK)
        addps_rr(Xmm0, XmmJ)
        addps_rr(Xmm0, XmmI)
        addps_rr(Xmm0, XmmH)
        addps_rr(Xmm0, XmmG)

        addps_rr(Xmm0, XmmF)

#endif /* RT_REGS >= 32 */

        /* BASE regs */
        addyx_rr(Reax, RegE)
        addyx_rr(Reax, RegD)
        addyx_rr(Reax, RegC)
        addyx_rr(Reax, RegB)
        addyx_rr(Reax, RegA)
        addyx_rr(Reax, Reg9)
        addyx_rr(Reax, Reg8)

        /* SIMD regs */
        addps_rr(Xmm0, XmmE)
        addps_rr(Xmm0, XmmD)
        addps_rr(Xmm0, XmmC)
        addps_rr(Xmm0, XmmB)
        addps_rr(Xmm0, XmmA)
        addps_rr(Xmm0, Xmm9)
        addps_rr(Xmm0, Xmm8)

#endif /* RT_REGS >= 16 */

        /* SIMD regs */
        addps_rr(Xmm0, Xmm7)
        addps_rr(Xmm0, Xmm6)
        addps_rr(Xmm0, Xmm5)
        addps_rr(Xmm0, Xmm4)
        addps_rr(Xmm0, Xmm3)
        addps_rr(Xmm0, Xmm2)
        addps_rr(Xmm0, Xmm1)

        /* BASE regs */
        addyx_rr(Reax, Redi)
        addyx_rr(Reax, Resi)
        addyx_rr(Reax, Redx)
        addyx_rr(Reax, Recx)
        addyx_rr(Reax, Rebx)

#endif /* RT_REGS >= 8 */

        movxx_ld(Redx, Mebp, inf_ISO1)
        movxx_ld(Rebx, Mebp, inf_FSO2)

        movyx_st(Reax, Medx, AJ0)
        movpx_st(Xmm0, Mebx, AJ0)

        ASM_LEAVE(info)
    }
}

rt_void p_test28(rt_SIMD_INFOX *info)
{
    rt_si32 j;

    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *iso1 = info->iso1 + S*RT_OFFS_SIMD;
    rt_real *fco2 = info->fco2 + S*RT_OFFS_SIMD;
    rt_real *fso2 = info->fso2 + S*RT_OFFS_SIMD;

    j = 1;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && FEQ(fco2[j], fso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("C iout[%d] = %" PR_L "d, fout[%d] = %e\n",
                j, ico1[j], j, fco2[j]);

        RT_LOGI("S iout[%d] = %" PR_L "d, fout[%d] = %e\n",
                j, iso1[j], j, fso2[j]);
    }
}

#endif /* SUB_TEST 28 */

/******************************************************************************/
/*******************************   SUB TEST 29   ******************************/
/******************************************************************************/

#if SUB_TEST >= 29

rt_void c_test29(rt_SIMD_INFOX *info)
{
    rt_si32 i, j, n = info->size;

    rt_elem *iar0 = info->iar0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = iar0[j] * iar0[(j + S) % n];
            ico2[j] = (rt_uelm)iar0[j] >> (iar0[j] & ((16 << L) - 1));
        }
    }
}

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The test code below was designed mainly for assembler validation purposes
 * and therefore may not fully represent its unlocked performance potential.
 * For optimal results keep ASM sections in separate functions away from
 * complex C/C++ logic, while making sure those functions are not inlined.
 * This is needed for better compatibility with modern optimizing compilers.
 */
rt_void s_test29(rt_SIMD_INFOX *info)
{
    rt_si32 i;

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Resi, Mebp, inf_IAR0)
        movxx_ld(Redx, Mebp, inf_ISO1)
        movxx_ld(Rebx, Mebp, inf_ISO2)

        movpx_ld(Xmm7, Mebp, inf_GPC07)
        shrpx_ri(Xmm7, IB(31*L-4))

        movpx_ld(Xmm0, Mesi, AJ0)
        movpx_ld(Xmm1, Mesi, AJ1)
        mulpx_rr(Xmm0, Xmm1)
        movpx_st(Xmm0, Medx, AJ0)
        movpx_ld(Xmm0, Mesi, AJ0)
        movpx_rr(Xmm2, Xmm0)
        andpx_rr(Xmm0, Xmm7)
        svrpx_rr(Xmm2, Xmm0)
        movpx_st(Xmm2, Mebx, AJ0)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ0)
        movyx_ld(Recx, Mesi, AJ1)
        mulyx_rr(Reax, Recx)
        movyx_st(Reax, Medx, AJ0)
#endif /* RT_BASE_TEST */

        movpx_ld(Xmm1, Mesi, AJ1)
        mulpx_ld(Xmm1, Mesi, AJ2)
        movpx_st(Xmm1, Medx, AJ1)
        movpx_ld(Xmm0, Mesi, AJ1)
        movpx_rr(Xmm2, Xmm0)
        andpx_rr(Xmm0, Xmm7)
        movpx_st(Xmm0, Mebx, AJ1)
        svrpx_ld(Xmm2, Mebx, AJ1)
        movpx_st(Xmm2, Mebx, AJ1)
#ifdef RT_BASE_TEST
        movyx_ld(Reax, Mesi, AJ1)
        mulyx_ld(Reax, Mesi, AJ2)
        movyx_st(Reax, Medx, AJ1)
#endif /* RT_BASE_TEST */

        movpx_ld(Xmm2, Mesi, AJ2)
        movpx_ld(Xmm0, Mesi, AJ0)
        mulpx_rr(Xmm2, Xmm0)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_ld(Xmm0, Mesi, AJ2)
        movpx_rr(Xmm2, Xmm0)
        andpx_rr(Xmm0, Xmm7)
        svrpx_rr(Xmm2, Xmm0)
        movpx_st(Xmm2, Mebx, AJ2)
#ifdef RT_BASE_TEST
        movyx_ld(Recx, Mesi, AJ2)
        movyx_ld(Reax, Mesi, AJ0)
        mulyx_rr(Recx, Reax)
        movyx_st(Recx, Medx, AJ2)
#endif /* RT_BASE_TEST */

        ASM_LEAVE(info)
    }
}

rt_void p_test29(rt_SIMD_INFOX *info)
{
    rt_si32 j, n = info->size;

    rt_elem *iar0 = info->iar0 + S*RT_OFFS_SIMD;
    rt_elem *ico1 = info->ico1 + S*RT_OFFS_SIMD;
    rt_elem *ico2 = info->ico2 + S*RT_OFFS_SIMD;
    rt_elem *iso1 = info->iso1 + S*RT_OFFS_SIMD;
    rt_elem *iso2 = info->iso2 + S*RT_OFFS_SIMD;

    j = n;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !v_mode)
        {
            continue;
        }

        RT_LOGI("iarr[%d] = %" PR_L "d, iarr[%d] = %" PR_L "d\n",
                j, iar0[j], (j + S) % n, iar0[(j + S) % n]);

        RT_LOGI("C iarr[%d]*iarr[%d] = %" PR_L "d, "
                  "(rt_uelm)iarr[%d]>>(iarr[%d]&((16<<L)-1)) = %" PR_L "d\n",
                j, (j + S) % n, ico1[j], j, j, ico2[j]);

        RT_LOGI("S iarr[%d]*iarr[%d] = %" PR_L "d, "
                  "(rt_uelm)iarr[%d]>>(iarr[%d]&((16<<L)-1)) = %" PR_L "d\n",
                j, (j + S) % n, iso1[j], j, j, iso2[j]);
    }
}

#endif /* SUB_TEST 29 */

/******************************************************************************/
/*********************************   TABLES   *********************************/
/******************************************************************************/

typedef rt_void (*testXX)(rt_SIMD_INFOX *);

testXX c_test[SUB_TEST] =
{
#if SUB_TEST >=  1
    c_test01,
#endif /* SUB_TEST  1 */

#if SUB_TEST >=  2
    c_test02,
#endif /* SUB_TEST  2 */

#if SUB_TEST >=  3
    c_test03,
#endif /* SUB_TEST  3 */

#if SUB_TEST >=  4
    c_test04,
#endif /* SUB_TEST  4 */

#if SUB_TEST >=  5
    c_test05,
#endif /* SUB_TEST  5 */

#if SUB_TEST >=  6
    c_test06,
#endif /* SUB_TEST  6 */

#if SUB_TEST >=  7
    c_test07,
#endif /* SUB_TEST  7 */

#if SUB_TEST >=  8
    c_test08,
#endif /* SUB_TEST  8 */

#if SUB_TEST >=  9
    c_test09,
#endif /* SUB_TEST  9 */

#if SUB_TEST >= 10
    c_test10,
#endif /* SUB_TEST 10 */

#if SUB_TEST >= 11
    c_test11,
#endif /* SUB_TEST 11 */

#if SUB_TEST >= 12
    c_test12,
#endif /* SUB_TEST 12 */

#if SUB_TEST >= 13
    c_test13,
#endif /* SUB_TEST 13 */

#if SUB_TEST >= 14
    c_test14,
#endif /* SUB_TEST 14 */

#if SUB_TEST >= 15
    c_test15,
#endif /* SUB_TEST 15 */

#if SUB_TEST >= 16
    c_test16,
#endif /* SUB_TEST 16 */

#if SUB_TEST >= 17
    c_test17,
#endif /* SUB_TEST 17 */

#if SUB_TEST >= 18
    c_test18,
#endif /* SUB_TEST 18 */

#if SUB_TEST >= 19
    c_test19,
#endif /* SUB_TEST 19 */

#if SUB_TEST >= 20
    c_test20,
#endif /* SUB_TEST 20 */

#if SUB_TEST >= 21
    c_test21,
#endif /* SUB_TEST 21 */

#if SUB_TEST >= 22
    c_test22,
#endif /* SUB_TEST 22 */

#if SUB_TEST >= 23
    c_test23,
#endif /* SUB_TEST 23 */

#if SUB_TEST >= 24
    c_test24,
#endif /* SUB_TEST 24 */

#if SUB_TEST >= 25
    c_test25,
#endif /* SUB_TEST 25 */

#if SUB_TEST >= 26
    c_test26,
#endif /* SUB_TEST 26 */

#if SUB_TEST >= 27
    c_test27,
#endif /* SUB_TEST 27 */

#if SUB_TEST >= 28
    c_test28,
#endif /* SUB_TEST 28 */

#if SUB_TEST >= 29
    c_test29,
#endif /* SUB_TEST 29 */
};

testXX s_test[SUB_TEST] =
{
#if SUB_TEST >=  1
    s_test01,
#endif /* SUB_TEST  1 */

#if SUB_TEST >=  2
    s_test02,
#endif /* SUB_TEST  2 */

#if SUB_TEST >=  3
    s_test03,
#endif /* SUB_TEST  3 */

#if SUB_TEST >=  4
    s_test04,
#endif /* SUB_TEST  4 */

#if SUB_TEST >=  5
    s_test05,
#endif /* SUB_TEST  5 */

#if SUB_TEST >=  6
    s_test06,
#endif /* SUB_TEST  6 */

#if SUB_TEST >=  7
    s_test07,
#endif /* SUB_TEST  7 */

#if SUB_TEST >=  8
    s_test08,
#endif /* SUB_TEST  8 */

#if SUB_TEST >=  9
    s_test09,
#endif /* SUB_TEST  9 */

#if SUB_TEST >= 10
    s_test10,
#endif /* SUB_TEST 10 */

#if SUB_TEST >= 11
    s_test11,
#endif /* SUB_TEST 11 */

#if SUB_TEST >= 12
    s_test12,
#endif /* SUB_TEST 12 */

#if SUB_TEST >= 13
    s_test13,
#endif /* SUB_TEST 13 */

#if SUB_TEST >= 14
    s_test14,
#endif /* SUB_TEST 14 */

#if SUB_TEST >= 15
    s_test15,
#endif /* SUB_TEST 15 */

#if SUB_TEST >= 16
    s_test16,
#endif /* SUB_TEST 16 */

#if SUB_TEST >= 17
    s_test17,
#endif /* SUB_TEST 17 */

#if SUB_TEST >= 18
    s_test18,
#endif /* SUB_TEST 18 */

#if SUB_TEST >= 19
    s_test19,
#endif /* SUB_TEST 19 */

#if SUB_TEST >= 20
    s_test20,
#endif /* SUB_TEST 20 */

#if SUB_TEST >= 21
    s_test21,
#endif /* SUB_TEST 21 */

#if SUB_TEST >= 22
    s_test22,
#endif /* SUB_TEST 22 */

#if SUB_TEST >= 23
    s_test23,
#endif /* SUB_TEST 23 */

#if SUB_TEST >= 24
    s_test24,
#endif /* SUB_TEST 24 */

#if SUB_TEST >= 25
    s_test25,
#endif /* SUB_TEST 25 */

#if SUB_TEST >= 26
    s_test26,
#endif /* SUB_TEST 26 */

#if SUB_TEST >= 27
    s_test27,
#endif /* SUB_TEST 27 */

#if SUB_TEST >= 28
    s_test28,
#endif /* SUB_TEST 28 */

#if SUB_TEST >= 29
    s_test29,
#endif /* SUB_TEST 29 */
};

testXX p_test[SUB_TEST] =
{
#if SUB_TEST >=  1
    p_test01,
#endif /* SUB_TEST  1 */

#if SUB_TEST >=  2
    p_test02,
#endif /* SUB_TEST  2 */

#if SUB_TEST >=  3
    p_test03,
#endif /* SUB_TEST  3 */

#if SUB_TEST >=  4
    p_test04,
#endif /* SUB_TEST  4 */

#if SUB_TEST >=  5
    p_test05,
#endif /* SUB_TEST  5 */

#if SUB_TEST >=  6
    p_test06,
#endif /* SUB_TEST  6 */

#if SUB_TEST >=  7
    p_test07,
#endif /* SUB_TEST  7 */

#if SUB_TEST >=  8
    p_test08,
#endif /* SUB_TEST  8 */

#if SUB_TEST >=  9
    p_test09,
#endif /* SUB_TEST  9 */

#if SUB_TEST >= 10
    p_test10,
#endif /* SUB_TEST 10 */

#if SUB_TEST >= 11
    p_test11,
#endif /* SUB_TEST 11 */

#if SUB_TEST >= 12
    p_test12,
#endif /* SUB_TEST 12 */

#if SUB_TEST >= 13
    p_test13,
#endif /* SUB_TEST 13 */

#if SUB_TEST >= 14
    p_test14,
#endif /* SUB_TEST 14 */

#if SUB_TEST >= 15
    p_test15,
#endif /* SUB_TEST 15 */

#if SUB_TEST >= 16
    p_test16,
#endif /* SUB_TEST 16 */

#if SUB_TEST >= 17
    p_test17,
#endif /* SUB_TEST 17 */

#if SUB_TEST >= 18
    p_test18,
#endif /* SUB_TEST 18 */

#if SUB_TEST >= 19
    p_test19,
#endif /* SUB_TEST 19 */

#if SUB_TEST >= 20
    p_test20,
#endif /* SUB_TEST 20 */

#if SUB_TEST >= 21
    p_test21,
#endif /* SUB_TEST 21 */

#if SUB_TEST >= 22
    p_test22,
#endif /* SUB_TEST 22 */

#if SUB_TEST >= 23
    p_test23,
#endif /* SUB_TEST 23 */

#if SUB_TEST >= 24
    p_test24,
#endif /* SUB_TEST 24 */

#if SUB_TEST >= 25
    p_test25,
#endif /* SUB_TEST 25 */

#if SUB_TEST >= 26
    p_test26,
#endif /* SUB_TEST 26 */

#if SUB_TEST >= 27
    p_test27,
#endif /* SUB_TEST 27 */

#if SUB_TEST >= 28
    p_test28,
#endif /* SUB_TEST 28 */

#if SUB_TEST >= 29
    p_test29,
#endif /* SUB_TEST 29 */
};

/******************************************************************************/
/**********************************   MAIN   **********************************/
/******************************************************************************/

#undef sregs_sa /* turn off SIMD-regs instruction definitions */
#undef sregs_la /* turn off SIMD-regs instruction definitions */

#define sregs_sa() /* empty SIMD-regs instruction definitions */
#define sregs_la() /* empty SIMD-regs instruction definitions */

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
    rt_si32 k, l, r, t;

    if (argc >= 2)
    {
        RT_LOGI("--------------------------------------------------------\n");
        RT_LOGI("Usage options are given below:\n");
        RT_LOGI(" -b n, specify subtest # at which testing begins, n >= 1\n");
        RT_LOGI(" -e n, specify subtest # at which testing ends, n <= max\n");
        RT_LOGI(" -d n, override diff-threshold for qualification, n >= 0\n");
        RT_LOGI(" -c n, override counter of redundant test cycles, n >= 1\n");
        RT_LOGI(" -v, enable verbose mode, always print values from tests\n");
        RT_LOGI("all options can be used together\n");
        RT_LOGI("--------------------------------------------------------\n");
    }

    for (k = 1; k < argc; k++)
    {
        if (k < argc && strcmp(argv[k], "-b") == 0 && ++k < argc)
        {
            for (l = strlen(argv[k]), r = 1, t = 0; l > 0; l--, r *= 10)
            {
                t += (argv[k][l-1] - '0') * r;
            }
            if (t >= 1 && t <= SUB_TEST)
            {
                RT_LOGI("Subtest-index-init overridden: %d\n", t);
                n_init = t-1;
            }
            else
            {
                RT_LOGI("Subtest-index-init value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-e") == 0 && ++k < argc)
        {
            for (l = strlen(argv[k]), r = 1, t = 0; l > 0; l--, r *= 10)
            {
                t += (argv[k][l-1] - '0') * r;
            }
            if (t >= 1 && t <= SUB_TEST)
            {
                RT_LOGI("Subtest-index-done overridden: %d\n", t);
                n_done = t-1;
            }
            else
            {
                RT_LOGI("Subtest-index-done value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-d") == 0 && ++k < argc)
        {
            for (l = strlen(argv[k]), r = 1, t = 0; l > 0; l--, r *= 10)
            {
                t += (argv[k][l-1] - '0') * r;
            }
            if (t >= 0)
            {
                RT_LOGI("Diff-threshold overridden: %d\n", t);
                t_diff = t;
            }
            else
            {
                RT_LOGI("Diff-threshold value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-c") == 0 && ++k < argc)
        {
            for (l = strlen(argv[k]), r = 1, t = 0; l > 0; l--, r *= 10)
            {
                t += (argv[k][l-1] - '0') * r;
            }
            if (t >= 1)
            {
                RT_LOGI("Test-redundant overridden: %d\n", t);
                r_test = t;
            }
            else
            {
                RT_LOGI("Test-redundant value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-v") == 0 && !v_mode)
        {
            v_mode = RT_TRUE;
            RT_LOGI("Verbose mode enabled\n");
        }
    }

    rt_pntr marr = sys_alloc(10*ARR_SIZE*sizeof(rt_elem)+Q*RT_OFFS_DATA*2+MASK);
    memset(marr, 0, 10*ARR_SIZE*sizeof(rt_elem)+Q*RT_OFFS_DATA*2+MASK);
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

    rt_real *far0 = (rt_real *)mar0 + S*RT_OFFS_SIMD*0x0 + ARR_SIZE*0x0;
    rt_real *fco1 = (rt_real *)mar0 + S*RT_OFFS_SIMD*0x0 + ARR_SIZE*0x1;
    rt_real *fco2 = (rt_real *)mar0 + S*RT_OFFS_SIMD*0x0 + ARR_SIZE*0x2;
    rt_real *fso1 = (rt_real *)mar0 + S*RT_OFFS_SIMD*0x0 + ARR_SIZE*0x3;
    rt_real *fso2 = (rt_real *)mar0 + S*RT_OFFS_SIMD*0x0 + ARR_SIZE*0x4;

    for (k = 0; k < Q; k++)
    {
        memcpy(far0 + S*RT_OFFS_SIMD + RT_ARR_SIZE(farr)*k, farr, sizeof(farr));
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

    rt_elem *iar0 = (rt_elem *)mar0 + S*RT_OFFS_SIMD*0x1 + ARR_SIZE*0x5;
    rt_elem *ico1 = (rt_elem *)mar0 + S*RT_OFFS_SIMD*0x1 + ARR_SIZE*0x6;
    rt_elem *ico2 = (rt_elem *)mar0 + S*RT_OFFS_SIMD*0x1 + ARR_SIZE*0x7;
    rt_elem *iso1 = (rt_elem *)mar0 + S*RT_OFFS_SIMD*0x1 + ARR_SIZE*0x8;
    rt_elem *iso2 = (rt_elem *)mar0 + S*RT_OFFS_SIMD*0x1 + ARR_SIZE*0x9;

    for (k = 0; k < Q; k++)
    {
        memcpy(iar0 + S*RT_OFFS_SIMD + RT_ARR_SIZE(iarr)*k, iarr, sizeof(iarr));
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

    inf0->cyc  = r_test;
    inf0->size = ARR_SIZE;
    inf0->tail = (rt_pntr)0xABCDEF01;

    rt_si32 simd = 0;

    ASM_ENTER(inf0)
        verxx_xx()
    ASM_LEAVE(inf0)

    if (RT_FALSE
#if   (RT_2K8_R8) && (RT_SIMD == 2048)
    ||  (inf0->ver & (RT_2K8_R8 << 0x1C)) == 0
#elif (RT_1K4)    && (RT_SIMD == 1024)
    ||  (inf0->ver & (RT_1K4 << 0x18)) == 0
#elif (RT_1K4_R8) && (RT_SIMD == 1024)
    ||  (inf0->ver & (RT_1K4_R8 << 0x14)) == 0
#elif (RT_512)    && (RT_SIMD == 512)
    ||  (inf0->ver & (RT_512 << 0x10)) == 0
#elif (RT_512_R8) && (RT_SIMD == 512)
    ||  (inf0->ver & (RT_512_R8 << 0x0C)) == 0
#elif (RT_256)    && (RT_SIMD == 256)
    ||  (inf0->ver & (RT_256 << 0x08)) == 0
#elif (RT_256_R8) && (RT_SIMD == 256)
    ||  (inf0->ver & (RT_256_R8 << 0x04)) == 0
#elif (RT_128)    && (RT_SIMD == 128)
    ||  (inf0->ver & (RT_128 << 0x00)) == 0
#endif /* RT_128 */
       )
    {
        RT_LOGI("Chosen SIMD target is not supported, check build flags\n");
        n_done = -1;
    }

#if   (RT_2K8X1)  && (RT_SIMD == 2048)
    simd = (1 << 16) | (RT_2K8X1 << 8) | 16;
#elif (RT_1K4X2)  && (RT_SIMD == 2048)
    simd = (2 << 16) | (RT_1K4X2 << 8) | 8;
#elif (RT_512X4)  && (RT_SIMD == 2048)
    simd = (4 << 16) | (RT_512X4 << 8) | 4;
#elif (RT_1K4X1)  && (RT_SIMD == 1024)
    simd = (1 << 16) | (RT_1K4X1 << 8) | 8;
#elif (RT_512X2)  && (RT_SIMD == 1024)
    simd = (2 << 16) | (RT_512X2 << 8) | 4;
#elif (RT_512X1)  && (RT_SIMD == 512)
    simd = (1 << 16) | (RT_512X1 << 8) | 4;
#elif (RT_256X2)  && (RT_SIMD == 512)
    simd = (2 << 16) | (RT_256X2 << 8) | 2;
#elif (RT_128X4)  && (RT_SIMD == 512)
    simd = (4 << 16) | (RT_128X4 << 8) | 1;
#elif (RT_256X1)  && (RT_SIMD == 256)
    simd = (1 << 16) | (RT_256X1 << 8) | 2;
#elif (RT_128X2)  && (RT_SIMD == 256)
    simd = (2 << 16) | (RT_128X2 << 8) | 1;
#elif (RT_128X1)  && (RT_SIMD == 128)
    simd = (1 << 16) | (RT_128X1 << 8) | 1;
#endif /* RT_128 */

    rt_time time1 = 0;
    rt_time time2 = 0;
    rt_time tC = 0;
    rt_time tS = 0;

    rt_si32 i;

    for (i = n_init; i <= n_done; i++)
    {
        RT_LOGI("--------------------  SUB TEST = %2d  - ptr/fp = %d%s%d --\n",
                    i+1, RT_POINTER, RT_ADDRESS == 32 ? "_" : "f", RT_ELEMENT);

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

        RT_LOGI("-------------------------------------- simd = %4dx%dv%d -\n",
                (simd & 0xFF) * 128, (simd >> 16) & 0xFF, (simd >> 8) & 0xFF);
    }

    ASM_DONE(inf0)

    sys_free(regs, sizeof(rt_SIMD_REGS) + MASK);
    sys_free(info, sizeof(rt_SIMD_INFOX) + MASK);
    sys_free(marr, 10 * ARR_SIZE * sizeof(rt_ui32) + MASK);

#if (defined RT_WIN32) || (defined RT_WIN64) /* Win32, MSVC -- Win64, GCC --- */

    RT_LOGI("Type any letter and press ENTER to exit:");
    rt_char str[80];
    scanf("%79s", str);

#endif /* ------------- OS specific ----------------------------------------- */

    return 0;
}

/******************************************************************************/
/**********************************   UTILS   *********************************/
/******************************************************************************/

#include "rtzero.h"

#if RT_POINTER == 64
#if RT_ADDRESS == 32

#define RT_ADDRESS_MIN      ((rt_byte *)0x0000000040000000)
#define RT_ADDRESS_MAX      ((rt_byte *)0x0000000080000000)

#else /* RT_ADDRESS == 64 */

#define RT_ADDRESS_MIN      ((rt_byte *)0x0000000140000000)
#define RT_ADDRESS_MAX      ((rt_byte *)0x0000080000000000)

#endif /* RT_ADDRESS */

rt_byte *s_ptr = RT_ADDRESS_MIN;

#endif /* RT_POINTER */


#if (defined RT_WIN32) || (defined RT_WIN64) /* Win32, MSVC -- Win64, GCC --- */

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

DWORD s_step = 0;

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

#if RT_DEBUG >= 2

    RT_LOGI("ALLOC PTR = %016" PR_Z "X, size = %ld\n", (rt_full)ptr, size);

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

#if RT_DEBUG >= 2

    RT_LOGI("FREED PTR = %016" PR_Z "X, size = %ld\n", (rt_full)ptr, size);

#endif /* RT_DEBUG */
}

#elif (defined RT_LINUX) /* Linux, GCC -------------------------------------- */

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

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON  /* workaround for macOS compilation */
#endif /* macOS still cannot allocate with mmap within 32-bit range */

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

#if RT_DEBUG >= 2

    RT_LOGI("ALLOC PTR = %016" PR_Z "X, size = %ld\n", (rt_full)ptr, size);

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

#if RT_DEBUG >= 2

    RT_LOGI("FREED PTR = %016" PR_Z "X, size = %ld\n", (rt_full)ptr, size);

#endif /* RT_DEBUG */
}

#endif /* ------------- OS specific ----------------------------------------- */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

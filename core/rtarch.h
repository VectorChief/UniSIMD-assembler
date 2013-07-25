/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_H
#define RT_RTARCH_H

/******************************************************************************/
/***************************   OS, COMPILER, ARCH   ***************************/
/******************************************************************************/

/*******************************   WIN32, MSVC   ******************************/

#if   defined (WIN32)

/* ---------------------------------   X86   -------------------------------- */

#if   defined (RT_X86)

#define ASM_OP0(op)             op
#define ASM_OP1(op, p1)         op  p1
#define ASM_OP2(op, p1, p2)     op  p1, p2

#define ASM_BEG /*internal*/    __asm {
#define ASM_END /*internal*/    }

#define EMITB(b)                ASM_BEG ASM_OP1(_emit, b) ASM_END
#define label_ld(lb)/*Reax*/    ASM_BEG ASM_OP2(lea, eax, lb) ASM_END

#include "rtarch_x86_sse.h"

#define ASM_ENTER(info)     __asm                                           \
                            {                                               \
                                stack_sa()                                  \
                                label_ld(info)                              \
                                movxx_ld(Rebp, Oeax, PLAIN)
#define ASM_LEAVE(info)         stack_la()                                  \
                            }

/* ---------------------------------   ARM   -------------------------------- */

#elif defined (RT_ARM)



#endif /* RT_X86, RT_ARM */

/*******************************   LINUX, GCC   *******************************/

#elif defined (linux)

/* ---------------------------------   X86   -------------------------------- */

#if   defined (RT_X86)

#define ASM_OP0(op)             #op
#define ASM_OP1(op, p1)         #op"  "#p1
#define ASM_OP2(op, p1, p2)     #op"  "#p2", "#p1

#define ASM_BEG /*internal*/    ""
#define ASM_END /*internal*/    "\n"

#define EMITB(b)                ASM_BEG ASM_OP1(.byte, b) ASM_END
#define label_ld(lb)/*Reax*/    ASM_BEG ASM_OP2(leal, %%eax, lb) ASM_END

#include "rtarch_x86_sse.h"

#define ASM_ENTER(info)     asm volatile                                    \
                            (                                               \
                                stack_sa()                                  \
                                movxx_ld(Rebp, Oeax, PLAIN)
#define ASM_LEAVE(info)         stack_la()                                  \
                                :                                           \
                                : "a" (&info)                               \
                                : "cc",  "memory"                           \
                            );

/* ---------------------------------   ARM   -------------------------------- */

#elif defined (RT_ARM)

#define ASM_OP0(op)             #op
#define ASM_OP1(op, p1)         #op"  "#p1
#define ASM_OP2(op, p1, p2)     #op"  "#p1", "#p2

#define ASM_BEG /*internal*/    ""
#define ASM_END /*internal*/    "\n"

#define EMITB(b)                ASM_BEG ASM_OP1(.byte, b) ASM_END
#define label_ld(lb)/*Reax*/    ASM_BEG ASM_OP2(adr, r0, lb) ASM_END

#include "rtarch_arm_mpe.h"

#define ASM_ENTER(info)     asm volatile                                    \
                            (                                               \
                                stack_sa()                                  \
                                ASM_BEG ASM_OP2(mov, r0, %[info]) ASM_END   \
                                movxx_ld(Rebp, Oeax, PLAIN)
#define ASM_LEAVE(info)         stack_la()                                  \
                                :                                           \
                                : [info] "r" (&info)                        \
                                : "cc",  "memory",                          \
                                  "d0",  "d1",  "d2",  "d3",                \
                                  "d4",  "d5",  "d6",  "d7",                \
                                  "d8",  "d9",  "d10", "d11",               \
                                  "d12", "d13", "d14", "d15",               \
                                  "d16", "d17", "d18", "d19",               \
                                  "d20", "d21"                              \
                            );

#endif /* RT_X86, RT_ARM */

#endif /* OS, COMPILER, ARCH */

#endif /* RT_RTARCH_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

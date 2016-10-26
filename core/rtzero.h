/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtzero.h: Clean up of internal preprocessor short names.
 * Can be used to avoid collisions with system headers (mostly windows.h).
 */

/******************************************************************************/
/*******************************   DEFINITIONS   ******************************/
/******************************************************************************/

/*
 * Alphabetical view of current/future instruction namespaces:
 *
 * cmda*_** - SIMD-data args, SIMD ISA (always fixed at 16-bit, packed-256-bit)
 * cmdb*_** - byte-size args, BASE ISA (displacement/alignment may differ)
 * cmdc*_** - SIMD-data args, SIMD ISA (always fixed at 32-bit, packed-256-bit)
 * cmdd*_** - SIMD-data args, SIMD ISA (always fixed at 64-bit, packed-256-bit)
 * cmde*_** - extd-size args, extd ISA (for 80-bit extended double, x87)
 * cmdf*_** - SIMD-data args, SIMD ISA (32/64-bit configurable, packed-256-bit)
 * cmdg*_** - SIMD-data args, SIMD ISA (always fixed at 16-bit, packed-128-bit)
 * cmdh*_** - half-size args, BASE ISA (displacement/alignment may differ)
 * cmdi*_** - SIMD-data args, SIMD ISA (always fixed at 32-bit, packed-128-bit)
 * cmdj*_** - SIMD-data args, SIMD ISA (always fixed at 64-bit, packed-128-bit)
 * cmdk*_** - king-kong args, BASE ISA (for 128-bit BASE subset, RISC-V)
 * cmdl*_** - SIMD-data args, SIMD ISA (32/64-bit configurable, packed-128-bit)
 * cmdm*_** - SIMD-data args, SIMD ISA (packed fp16/int subset, half-precision)
 * cmdn*_** - SIMD-elem args, SIMD ISA (scalar fp16/int subset, half-precision)
 * cmdo*_** - SIMD-data args, SIMD ISA (always fixed at 32-bit, packed)
 * cmdp*_** - SIMD-data args, SIMD ISA (32/64-bit configurable, packed)
 * cmdq*_** - SIMD-data args, SIMD ISA (always fixed at 64-bit, packed)
 * cmdr*_** - SIMD-elem args, SIMD ISA (always fixed at 32-bit, scalar)
 * cmds*_** - SIMD-elem args, SIMD ISA (32/64-bit configurable, scalar)
 * cmdt*_** - SIMD-elem args, SIMD ISA (always fixed at 64-bit, scalar)
 * cmdu*_** - SIMD-data args, SIMD ISA (packed f128/int subset, quad-precision)
 * cmdv*_** - SIMD-elem args, SIMD ISA (scalar f128/int subset, quad-precision)
 * cmdw*_** - word-size args, BASE ISA (data-element is always fixed at 32-bit)
 * cmdx*_** - addr-size args, BASE ISA (32/64-bit configurable with RT_ADDRESS)
 * cmdy*_** - elem-size args, BASE ISA (32/64-bit configurable with RT_ELEMENT)
 * cmdz*_** - full-size args, BASE ISA (data-element is always fixed at 64-bit)
 *
 * More detailed description of the above is given in the rtarch.h file.
 */

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
#undef XmmE /* external name for SIMD register, may be reserved in some cases */
#undef XmmF /* external name for SIMD register, may be reserved in some cases */

/* The last two SIMD registers can be reserved by the assembler when building
 * RISC targets with SIMD wider than natively supported 128-bit, in which case
 * they will be occupied by temporary data. Two hidden registers may also come
 * in handy when implementing elaborate register-spill techniques in the future
 * for current targets with less native registers than architecturally exposed.
 * Neither of the above is currently supported by the assembler, but is being
 * considered as a potential optimization/compatibility option going forward. */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

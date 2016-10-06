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

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

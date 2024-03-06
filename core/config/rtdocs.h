/******************************************************************************/
/* Copyright (c) 2013-2025 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtdocs.h: Documentation on how to get started using the assembler.
 * Table of contents is provided below.
 *
 * Chapter 1 - Overview
 * Chapter 2 - Introduction
 * Chapter 3 - Application types
 * Chapter 4 - Initialization
 * Chapter 5 - Configuration
 *
 * It is recommended to read all chapters at least once
 * before getting started with the code.
 */

/******************************************************************************/
/**************************   CHAPTER 1 - OVERVIEW  ***************************/
/******************************************************************************/

/*
 * The general structure of the application using UniSIMD is given below.
 * It is usually a combination of standard C/C++ code with some inline assembler
 * parts. Something like this:
 *
 * void func(rt_SIMD_INFOX *inf)
 * {
 *     ASM_ENTER(inf)
 *     ..
 *     ASM_LEAVE(inf)
 * }
 *
 * The code above shows a C/C++ function with a parameter and ASM code section
 * within it. The parameter is a pointer to a SIMD-aligned structure
 * that is used to pass all the data to the ASM section and back if needed.
 * ASM section can read and write fields of that structure.
 *
 * There can be two types of instructions within the ASM section: BASE and SIMD.
 * UniSIMD also defines register sets that are common for all architectures
 * including variants of a single architecture.
 *
 * So, with UniSIMD there will always be: Reax/Rebx/Recx/... for BASE and
 * Xmm0/Xmm1/Xmm2/... for SIMD. However, these common definitions are then
 * mapped to actual architectural registers of ARM/MIPS/POWER and x86.
 *
 * Both BASE and SIMD register sizes depend on the type of instruction used.
 * UniSIMD defines a number of instruction subsets that are again common
 * across all architectures.
 *
 * Some instructions work with fixed register sizes, others have it configurable
 * with a flag. The full list of instruction subsets along with registers
 * and various addressing modes can be found in "core/config/rtzero.h",
 * while "test/simd_test.cpp" shows how these definitions can be used.
 * Refer to "c_test01" and "s_test01" for a start.
 *
 * Once the program is expressed with UniSIMD's syntax (C/C++ with ASM sections)
 * it can then be built for any supported architecture without having a need
 * to modify the source code again. Just pick the right makefile.
 */

/******************************************************************************/
/************************   CHAPTER 2 - INTRODUCTION  *************************/
/******************************************************************************/

/*
 * In order for UniSIMD to work as intended application source files need
 * to do a few things first.
 *
 * #define RT_SIMD_CODE // enable SIMD instruction definitions
 * #define RT_DATA 8 // define data load-level for backend structures
 *
 * #include "rtbase.h" // include UniSIMD's base header after the 2 flags above
 *
 * As some SIMD widths (128/256/512-bit) are limited to specific CPU generations
 * (SSE/AVX/AVX-512) the use of SIMD is turned off by default and needs to be
 * explicitly enabled with a flag to make the generic ASM sections (no SIMD)
 * portable across generations. This is mostly relevant when implementing
 * runtime detection of SIMD and subsequent multi-targeting.
 *
 * Once SIMD instructions are enabled ASM sections will preserve/restore full
 * SIMD registers they are configured to work with. The maximal SIMD width
 * for a build is set in a makefile with RT_128=a/RT_256=b/RT_512=c/... and
 * is defined internally as Q (=1/2/4/...), while a/b/c define variant
 * within a given SIMD width.
 *
 * As UniSIMD needs to adjust for changing SIMD widths when working with
 * backend structures it needs to know how much they are filled.
 * The next (second) flag then defines the data load-level common for all
 * backend structures and ASM sections within its scope:
 *
 * 1 - means full DP-level (12-bit displacements) is filled or exceeded (Q=1).
 * 2 - means 1/2  DP-level (11-bit displacements) has not been exceeded (Q=1).
 * 4 - means 1/4  DP-level (10-bit displacements) has not been exceeded (Q=1).
 * 8 - means 1/8  DP-level  (9-bit displacements) has not been exceeded (Q=1).
 * 16  means 1/16 DP-level  (8-bit displacements) has not been exceeded (Q=1).
 * NOTE: the built-in rt_SIMD_INFO structure is already filled at full 1/16th.
 *
 * The load-level is measured at Q equal to 1 and UniSIMD then adjusts internal
 * displacement values as Q scales up.
 *
 * UniSIMD defines a lot of simple single-letter internal values which can
 * interfere with program's own variables, especially when adding UniSIMD to
 * an existing project. It is therefore recommended using a separate file
 * for ASM header and sections or adding them at the end of an existing file,
 * while keeping function declarations to be used in the program at the top.
 *
 * All applications need to include a single root header with base types and
 * definitions so that UniSIMD can do the rest of configuration based on
 * makefile flags. Depending on where the source files are located makefile
 * should specify a relative path to "core/config/" in order for UniSIMD headers
 * to become available.
 */

/******************************************************************************/
/**********************   CHAPTER 3 - APPLICATION TYPES  **********************/
/******************************************************************************/

/*
 * There can be two types of applications written with UniSIMD - single-target
 * and multi-target. In the first case the binary is configured and carries
 * the code for just one target (CPU generation or SIMD width and variant).
 * In the second case the binary carries multiple code sections for different
 * targets (CPU generations or SIMD widths and variants).
 *
 * Some architectures like x86 allow for runtime target detection on
 * the application level (user-space), others like ARM/MIPS/POWER only provide
 * that information to an operating system (priveleged), which makes producing
 * multi-target binaries for those architectures a bit cumbersome as
 * they become OS-specific.
 *
 * The test framework within UniSIMD is a single-target application, which means
 * only one CPU generation or SIMD width and variant per build. However, its use
 * of portable instruction subsets (cmdx*, cmdy* for BASE and cmdp*, cmds* for
 * SIMD and scalar) allows it to configure the same code-base for many different
 * targets and produce a separate binary for each target from a single source.
 *
 * Creating a proper multi-target binary requires use of C++ namespaces and
 * additional source-level target files, which would then include the same
 * portable code-base and wrap it into a target-specific namespace with a set of
 * flags for that target. In addition to that a generic ASM section should
 * determine the target at runtime and select appropriate code-path in a switch.
 *
 * A good example of how to build a multi-target binary with UniSIMD is provided
 * in the QuadRay engine (core/tracer). In that example backend structures are
 * always defined for the maximnal SIMD width (Q internally) configured in
 * makefiles (RT_128/RT_256/RT_512/...). However, the portable ASM code-base
 * needs to be aware of the actual SIMD width selected at runtime
 * and currently running.
 *
 * This is handled with a target-specific RT_SIMD_QUADS definition, which is
 * expressed in the same terms as Q (1/2/4/8/16), but is different from Q as it
 * always reflects the currently active SIMD width and not the maximal
 * SIMD width defined for the build.
 */

/******************************************************************************/
/************************   CHAPTER 4 - INITIALIZATION  ***********************/
/******************************************************************************/

/*
 * Some emulated instructions within ASM sections rely on general purpose
 * constants in rt_SIMD_INFO structure defined in "core/config/rtbase.h".
 * They need to be initialized before the pointer to this structure is passed to
 * the first ASM section and deinitialized after the last one. It's done with:
 *
 * ASM_INIT(inf, reg)
 *
 * ..
 *
 * ASM_DONE(inf)
 *
 * Here "reg" is a pointer to SIMD-aligned structure rt_SIMD_REGS intended
 * to keep the state of all SIMD registers (from C/C++ code) while ASM section
 * is doing some processing. It can be allocated separately or as a part of a
 * larger combined "inf+reg" structure. In any case both pointers should end up
 * SIMD-aligned (divisible by full SIMD-width they are pointing at in bytes).
 *
 * As was mentioned previously "inf" is a pointer to rt_SIMD_INFOX structure,
 * which is usually an extension of rt_SIMD_INFO. The extension of the initial
 * built-in rt_SIMD_INFO structure can be done with inheritance (in C++) or
 * embedding (in C). This step (extension) is necessary in order to pass
 * application-specific parameters into application-defined ASM sections,
 * something that generic rt_SIMD_INFO cannot provide.
 *
 * Once "inf" pointer of initialized structure is passed to the ASM section
 * it shows up as Rebp register and can be accessed via Mebp addressing mode
 * with corresponding displacements (offsets) defined in rt_SIMD_INFO and
 * rt_SIMD_INFOX (by extension).
 *
 * Potential future improvement is to use an array instead of structure to avoid
 * possible paddings that compiler may introduce for its own needs (alignment),
 * in which case some parts of the assembler will need to be redesigned.
 * ASM_ENTER/LEAVE macros can be converted into just-in-time compilation along
 * with EMITW/EMITH/EMITB/LBL to avoid possible compiler issues with inline ASM.
 * The order of arithmetic and shifts within internal definitions can be
 * hardened by using extra parentheses (in a form of round brackets).
 *
 * Right shifts on signed/unsigned data types in C/C++ are not guaranteed by
 * the standard to produce arithmetic/logical shift instructions respectively,
 * therefore some tests within SIMD test framework may need to be rewritten.
 * Modern open-source compilers produce consistent results only with data sizes
 * above 8-bit (char). With 8-bit signed/unsigned char ARM and POWER compilers
 * show discrepancy in right shifts behavior relative to MIPS and x86.
 *
 * For every BASE register starting with R*** (like Rebx, Recx, Redx, ...)
 * there is a corresponding addressing mode starting with M*** (like Mebx, Mecx,
 * Medx, ...), which treats the register as a pointer and dereferences it with
 * additional displacement (offset) given as a separate parameter to cmd**_ld/st
 * instructions.
 *
 * The use of Reax is reserved for indexed addressing mode in the form of I***
 * (like Iebx, Iecx, Iedx, ...) in which case the address is calculated as a sum
 * of R*** + Reax + displacement, where R*** is the BASE register encoded in the
 * addressing mode. Scaled indexed addressing modes are supported as J***, K***,
 * L*** (with Reax), while S***, T***, U***, V*** accept any BASE register index
 * maintaining the same built-in scaling factors 1x/2x/4x/8x respectively.
 * Fully configurable N*** takes index register and scale (1,2,3) for 2x/4x/8x.
 * Reax is also used for plain addressing mode (Oeax) without displacement
 * in which case PLAIN is passed as a displacement to cmd**_ld/st instructions.
 */

/******************************************************************************/
/************************   CHAPTER 5 - CONFIGURATION  ************************/
/******************************************************************************/

/*
 * The initialization of SIMD fields within SIMD-aligned backend structures
 * can be streamlined with RT_SIMD_SET(s, v) macros used from within C/C++ code.
 * In this case "s" represents SIMD field (usually an array of elements) and "v"
 * represents scalar value that is going to be replicated across all elements.
 *
 * The RT_SIMD_SET macro is the most generic form which is then mapped to
 * target-specific form depending on the configured SIMD element size and
 * the maximal SIMD width. The RT_SIMD_SET32 and RT_SIMD_SET64 always work with
 * 32-bit and 64-bit SIMD elements respectively regardless of configuration,
 * but they both still respect maximal SIMD width.
 *
 * The element size is configured with RT_ELEMENT=32/64 definition from within
 * makefiles, while RT_ADDRESS, RT_POINTER define the respective address and
 * pointer sizes. These definitions affect the size of configurable scalar and
 * vector types used within backend structures and throughout C/C++ code.
 * For example, rt_elem/rt_uelm, rt_real depend on RT_ELEMENT, rt_addr/rt_uadr
 * depend on RT_ADDRESS, while rt_pntr/rt_uptr and rt_cell/rt_word depend on
 * RT_POINTER (which is fixed for the chosen target and cannot be changed).
 *
 * In addition to already mentioned flags and definitions "core/config/rtbase.h"
 * defines other useful constants, like R, T and S to configure the size of
 * SIMD fields depending on the chosen SIMD element size and maximal SIMD width.
 * Short names P, A and L represent RT_POINTER, RT_ADDRESS and RT_ELEMENT
 * in base units: 1 for 32-bit, 2 for 64-bit.
 *
 * Note that logical cmdpx instructions are configured for floating-point
 * SIMD pipeline on x86 where applicable, while logical cmdmx instructions
 * are better suited for integer SIMD workloads.
 *
 * Constants like B/C/D/.../I define various displacement corrections for
 * endianness, when C/C++ and ASM sections work on different data sizes packed
 * within a single larger field.
 *
 * Similar to how displacements are defined and then passed to BASE and SIMD
 * cmd**_ld/st instructions, immediate values of various sizes can be passed to
 * BASE cmd**_ri/rj instructions. The assembler defines the following immediate
 * and displacement types: IC/IB/IM/IG/IH/IV/IW as 7/8/12/15/16/31/32-bit values
 * and DP/DE/DF/DG/DH/DV as 12/13/14/15/16/31-bit values respectively.
 *
 * Both displacement and immediate common types are defined in corresponding
 * "core/config/rtarch_*32.h" files for each architecture individually.
 * Displacements are then additionally scaled with Q and RT_DATA expressed via O
 * definition in "core/config/rtbase.h". Immediate arguments only apply to BASE
 * instructions and don't need any additional SIMD scaling. All displacement and
 * immediate values are always unsigned within the assembler.
 */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

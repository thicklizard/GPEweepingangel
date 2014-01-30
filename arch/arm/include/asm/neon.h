/*
 * linux/arch/arm/include/asm/neon.h
 *
 * Copyright (C) 2013 Linaro Ltd <ard.biesheuvel at linaro.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _ASM_NEON_H
#define _ASM_NEON_H

/*
 * The GCC support header file for NEON intrinsics, <arm_neon.h>, does an
 * unconditional #include of <stdint.h>, assuming it will never be used outside
 * a C99 conformant environment. Sadly, this is not the case for the kernel.
 * The only dependencies <arm_neon.h> has on <stdint.h> are the
 * uint[8|16|32|64]_t types, which the kernel defines in <linux/types.h>.
 */
#include <linux/types.h>

/*
 * The GCC option -ffreestanding prevents GCC's internal <stdint.h> from
 * including the <stdint.h> system header, it will #include "stdint-gcc.h"
 * instead.
 */
#if __STDC_HOSTED__ != 0
#error You must compile with -ffreestanding to use NEON intrinsics
#endif

/*
 * The type uintptr_t is typedef'ed to __UINTPTR_TYPE__ by "stdint-gcc.h".
 * However, the bare metal and GLIBC versions of GCC don't agree on the
 * definition of __UINTPTR_TYPE__. Bare metal agrees with the kernel
 * (unsigned long), but GCC for GLIBC uses 'unsigned int' instead.
 */
#ifdef __linux__
#undef __UINTPTR_TYPE__
#endif

#include <arm_neon.h>

#endif

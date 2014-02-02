/*
 * linux/arch/arm/lib/xor-neon.c
 *
 * Copyright (C) 2013 Linaro Ltd <ard.biesheuvel at linaro.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/raid/xor.h>

#ifndef __ARM_NEON__
#error You should compile this file with '-mfloat-abi=softfp -mfpu=neon'
#endif

/*
 * Pull in the reference implementations while instructing GCC to attempt to
 * exploit implicit parallelism and emit NEON instructions.
 */
#pragma GCC optimize "tree-vectorize"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include <asm-generic/xor.h>

struct xor_block_template const xor_block_neon_inner = {
	.name	= "__inner_neon__",
	.do_2	= xor_8regs_2,
	.do_3	= xor_8regs_3,
	.do_4	= xor_8regs_4,
	.do_5	= xor_8regs_5,
};

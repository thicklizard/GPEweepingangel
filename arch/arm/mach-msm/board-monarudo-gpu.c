/* Copyright (c) 2012, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/init.h>
#include <linux/platform_device.h>
#include <mach/kgsl.h>
#include <mach/msm_bus_board.h>
#include <mach/board.h>
#include <mach/msm_dcvs.h>
#include <mach/socinfo.h>
#include "devices-msm8x60.h"
#include "devices.h"
#include "board-monarudo.h"

/* cmdline_gpu */
#ifdef CONFIG_CMDLINE_OPTIONS

unsigned int cmdline_3dgpu[2] = {CMDLINE_3DGPU_DEFKHZ_0, CMDLINE_3DGPU_DEFKHZ_1};
static int __init devices_read_3dgpu_cmdline(char *khz)
{
	unsigned long ui_khz;
	unsigned long *f;
	unsigned long valid_freq[9] = {266667000, 300000000, 320000000, 320000000, 400000000, 450000000, 500000000, 550000000, 0};
	int err;

	err = strict_strtoul(khz, 0, &ui_khz);
	if (err) {
		cmdline_3dgpu[1] = CMDLINE_3DGPU_DEFKHZ_1;
		cmdline_3dgpu[0] = CMDLINE_3DGPU_DEFKHZ_0;
		printk(KERN_INFO "[cmdline_3dgpu]: ERROR while converting! using default value!");
		printk(KERN_INFO "[cmdline_3dgpu]: 3dgpukhz_0='%i' & 3dgpukhz_1='%i'\n",
		       cmdline_3dgpu[0], cmdline_3dgpu[1]);
		return 1;
	}

	/* Check if parsed value is valid */
	if (ui_khz > 550000000)
		cmdline_3dgpu[1] = CMDLINE_3DGPU_DEFKHZ_1;
		cmdline_3dgpu[0] = CMDLINE_3DGPU_DEFKHZ_0;

	if (ui_khz < 266667000)
		cmdline_3dgpu[1] = CMDLINE_3DGPU_DEFKHZ_1;
		cmdline_3dgpu[0] = CMDLINE_3DGPU_DEFKHZ_0;

	for (f = valid_freq; f != 0; f++) {
		if (*f == ui_khz) {
			cmdline_3dgpu[0] = ui_khz;
			if (*f == valid_freq[0]) {
				cmdline_3dgpu[1] = CMDLINE_3DGPU_DEFKHZ_1;
			} else {
				f--;
				cmdline_3dgpu[1] = *f;
				f++;
			}
			printk(KERN_INFO "[cmdline_3dgpu]: 3dgpukhz_0='%i' & 3dgpukhz_1='%i'\n",
			       cmdline_3dgpu[0], cmdline_3dgpu[1]);
			return 1;
		}
		if (ui_khz > *f) {
			f++;
			if (ui_khz < *f) {
				f--;
				cmdline_3dgpu[0] = *f;
				if (*f == valid_freq[0]) {
					cmdline_3dgpu[1] = CMDLINE_3DGPU_DEFKHZ_1;
				} else {
					f--;
					cmdline_3dgpu[1] = *f;
					f++;
				}
				printk(KERN_INFO "[cmdline_3dgpu]: AUTOCORRECT! Couldn't find entered value");
				printk(KERN_INFO "[cmdline_3dgpu]: 3dgpukhz_0='%i' & 3dgpukhz_1='%i'\n",
				       cmdline_3dgpu[0], cmdline_3dgpu[1]);
				return 1;
			}
			f--;
		}
	}
	/* if we are still in here then something went wrong. Use defaults */
	cmdline_3dgpu[1] = CMDLINE_3DGPU_DEFKHZ_1;
	cmdline_3dgpu[0] = CMDLINE_3DGPU_DEFKHZ_0;
	printk(KERN_INFO "[cmdline_3dgpu]: ERROR! using default value!");
	printk(KERN_INFO "[cmdline_3dgpu]: 3dgpukhz_0='%i' & 3dgpukhz_1='%i'\n",
	       cmdline_3dgpu[0], cmdline_3dgpu[1]);
        return 1;
}
__setup("3dgpu=", devices_read_3dgpu_cmdline);

#endif

#ifdef CONFIG_MSM_DCVS
static struct msm_dcvs_freq_entry grp3d_freq[] = {
       {0, 0, 333932},
       {0, 0, 497532},
       {0, 0, 707610},
       {0, 0, 844545},
};

static struct msm_dcvs_core_info grp3d_core_info = {
       .freq_tbl = &grp3d_freq[0],
       .core_param = {
               .max_time_us = 100000,
               .num_freq = ARRAY_SIZE(grp3d_freq),
       },
       .algo_param = {
               .slack_time_us = 39000,
               .disable_pc_threshold = 86000,
               .ss_window_size = 1000000,
               .ss_util_pct = 95,
               .em_max_util_pct = 97,
               .ss_iobusy_conv = 100,
       },
};
#endif 

#ifdef CONFIG_MSM_BUS_SCALING
static struct msm_bus_vectors grp3d_init_vectors[] = {
	{
		.src = MSM_BUS_MASTER_GRAPHICS_3D,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = 0,
	},
	{
		.src = MSM_BUS_MASTER_GRAPHICS_3D_PORT1,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = 0,
	},
};

static struct msm_bus_vectors grp3d_low_vectors[] = {
	{
		.src = MSM_BUS_MASTER_GRAPHICS_3D,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = KGSL_CONVERT_TO_MBPS(2000),
	},
	{
		.src = MSM_BUS_MASTER_GRAPHICS_3D_PORT1,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = KGSL_CONVERT_TO_MBPS(2000),
	},
};

static struct msm_bus_vectors grp3d_nominal_low_vectors[] = {
	{
		.src = MSM_BUS_MASTER_GRAPHICS_3D,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = KGSL_CONVERT_TO_MBPS(2656),
	},
	{
		.src = MSM_BUS_MASTER_GRAPHICS_3D_PORT1,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = KGSL_CONVERT_TO_MBPS(2656),
	},
};

static struct msm_bus_vectors grp3d_nominal_high_vectors[] = {
	{
		.src = MSM_BUS_MASTER_GRAPHICS_3D,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = KGSL_CONVERT_TO_MBPS(4264),
	},
	{
		.src = MSM_BUS_MASTER_GRAPHICS_3D_PORT1,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = KGSL_CONVERT_TO_MBPS(4264),
	},
};

static struct msm_bus_vectors grp3d_max_vectors[] = {
	{
		.src = MSM_BUS_MASTER_GRAPHICS_3D,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = KGSL_CONVERT_TO_MBPS(5872),
	},
	{
		.src = MSM_BUS_MASTER_GRAPHICS_3D_PORT1,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = KGSL_CONVERT_TO_MBPS(5872),
	},
};

static struct msm_bus_paths grp3d_bus_scale_usecases[] = {
	{
		ARRAY_SIZE(grp3d_init_vectors),
		grp3d_init_vectors,
	},
	{
		ARRAY_SIZE(grp3d_low_vectors),
		grp3d_low_vectors,
	},
	{
		ARRAY_SIZE(grp3d_nominal_low_vectors),
		grp3d_nominal_low_vectors,
	},
	{
		ARRAY_SIZE(grp3d_nominal_high_vectors),
		grp3d_nominal_high_vectors,
	},
	{
		ARRAY_SIZE(grp3d_max_vectors),
		grp3d_max_vectors,
	},
};

static struct msm_bus_scale_pdata grp3d_bus_scale_pdata = {
	grp3d_bus_scale_usecases,
	ARRAY_SIZE(grp3d_bus_scale_usecases),
	.name = "grp3d",
};
#endif

static struct resource kgsl_3d0_resources[] = {
	{
		.name = KGSL_3D0_REG_MEMORY,
		.start = 0x04300000, 
		.end = 0x0431ffff,
		.flags = IORESOURCE_MEM,
	},
	{
		.name = KGSL_3D0_IRQ,
		.start = GFX3D_IRQ,
		.end = GFX3D_IRQ,
		.flags = IORESOURCE_IRQ,
	},
};

static const struct kgsl_iommu_ctx kgsl_3d0_iommu0_ctxs[] = {
	{ "gfx3d_user", 0 },
	{ "gfx3d_priv", 1 },
};

static const struct kgsl_iommu_ctx kgsl_3d0_iommu1_ctxs[] = {
	{ "gfx3d1_user", 0 },
	{ "gfx3d1_priv", 1 },
};

static struct kgsl_device_iommu_data kgsl_3d0_iommu_data[] = {
	{
		.iommu_ctxs = kgsl_3d0_iommu0_ctxs,
		.iommu_ctx_count = ARRAY_SIZE(kgsl_3d0_iommu0_ctxs),
		.physstart = 0x07C00000,
		.physend = 0x07C00000 + SZ_1M - 1,
	},
	{
		.iommu_ctxs = kgsl_3d0_iommu1_ctxs,
		.iommu_ctx_count = ARRAY_SIZE(kgsl_3d0_iommu1_ctxs),
		.physstart = 0x07D00000,
		.physend = 0x07D00000 + SZ_1M - 1,
	},
};

static struct kgsl_device_platform_data kgsl_3d0_pdata = {
	.pwrlevel = {
		{
			.gpu_freq = 450000000,
			.bus_freq = 4,
			.io_fraction = 0,
		},
		{
			.gpu_freq = 400000000,
			.bus_freq = 3,
			.io_fraction = 33,
		},
		{
			.gpu_freq = 320000000,
			.bus_freq = 2,
			.io_fraction = 100,
		},
		{
			.gpu_freq = 27000000,
			.bus_freq = 0,
		},
	},
	.init_level = 2,
	.num_levels = 4,
	.set_grp_async = NULL,
	.idle_timeout = HZ/10,
	.nap_allowed = true,
	.strtstp_sleepwake = true,
	.clk_map = KGSL_CLK_CORE | KGSL_CLK_IFACE | KGSL_CLK_MEM_IFACE,
#ifdef CONFIG_MSM_BUS_SCALING
	.bus_scale_table = &grp3d_bus_scale_pdata,
#endif
	.iommu_data = kgsl_3d0_iommu_data,
	.iommu_count = ARRAY_SIZE(kgsl_3d0_iommu_data),
#ifdef CONFIG_MSM_DCVS
	.core_info = &grp3d_core_info,
#endif
};

struct platform_device device_kgsl_3d0 = {
	.name = "kgsl-3d0",
	.id = 0,
	.num_resources = ARRAY_SIZE(kgsl_3d0_resources),
	.resource = kgsl_3d0_resources,
	.dev = {
		.platform_data = &kgsl_3d0_pdata,
	},
};

#ifdef CONFIG_CMDLINE_OPTIONS
/* setters for cmdline_gpu */
extern int set_kgsl_3d0_freq(unsigned int freq0, unsigned int freq1)
{
	kgsl_3d0_pdata.pwrlevel[0].gpu_freq = freq0;
	kgsl_3d0_pdata.pwrlevel[1].gpu_freq = freq1;
	return 0;
}
#endif

void __init monarudo_init_gpu(void)
{
	unsigned int version = socinfo_get_version();

	if (cpu_is_apq8064())
		kgsl_3d0_pdata.pwrlevel[0].gpu_freq = 450000000;
	if (SOCINFO_VERSION_MAJOR(version) == 2) {
		kgsl_3d0_pdata.chipid = ADRENO_CHIPID(3, 2, 0, 2);
	} else {
		if ((SOCINFO_VERSION_MAJOR(version) == 1) &&
				(SOCINFO_VERSION_MINOR(version) == 1))
			kgsl_3d0_pdata.chipid = ADRENO_CHIPID(3, 2, 0, 1);
		else
			kgsl_3d0_pdata.chipid = ADRENO_CHIPID(3, 2, 0, 0);
	}

	platform_device_register(&device_kgsl_3d0);
}

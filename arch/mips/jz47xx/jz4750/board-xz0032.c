/*
 *  Copyright (C) 2011 Peter Zotov <whitequark@whitequark.org>
 *  XZ0032 JZ4725B board support
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>

#include <jz4750/platform.h>

#include "../clock.h"

#include <jz4750_nand.h>
#include <jz4740_fb.h>

/* NAND */
static struct nand_ecclayout xz0032_ecclayout = {
	.eccbytes = 52,
	.eccpos = {
		3,  4,  5,  6,  7,  8,  9,  10,
		11, 12, 13, 14, 15, 16, 17, 18,
		19, 20, 21, 22, 23, 24, 25, 26,
		27, 28, 29, 30, 31, 32, 33, 34,
		35, 36, 37, 38, 39, 40, 41, 42,
		43, 44, 45, 46, 47, 48, 49, 50,
		51, 52, 53, 54, 55,
	},
	.oobfree = {
		{ .offset = 0,  .length = 3 },
		{ .offset = 56, .length = 8 },
	},
};

static struct mtd_partition xz0032_partitions[] = {
	{
		.name   = "u-boot",
		.offset = 0,
		.size   = 1  * 0x100000,
	},
	{
		.name   = "kernel",
		.offset = 1  * 0x100000,
		.size   = 15 * 0x100000,
	},
	{
		.name   = "rootfs",
		.offset = 16 * 0x100000,
		.size   = (1024 - 16) * 0x100000,
	},
};

static struct jz_nand_platform_data xz0032_nand_pdata = {
	.bch_8bit       = 1,
	.ecc_layout     = &xz0032_ecclayout,
	.partitions     = xz0032_partitions,
	.num_partitions = ARRAY_SIZE(xz0032_partitions),
	.busy_gpio      = 91,
};

/* LCD */

static struct fb_videomode xz0032_video_modes[] = {
	{
		.name = "480x272",
		.xres = 480,
		.yres = 272,
		.refresh = 60,
		.left_margin = 2,
		.right_margin = 2,
		.upper_margin = 2,
		.lower_margin = 2,
		.hsync_len = 41,
		.vsync_len = 10,
		.sync = 0,
		.vmode = FB_VMODE_NONINTERLACED,
	},
};

static struct jz4740_fb_platform_data xz0032_fb_pdata = {
	.width          = 90,
	.height         = 52,
	.num_modes      = ARRAY_SIZE(xz0032_video_modes),
	.modes          = xz0032_video_modes,
	.bpp            = 16,
	.lcd_type       = JZ_LCD_TYPE_GENERIC_16_BIT,
	.pixclk_falling_edge = 1,
};

static struct platform_device *jz_platform_devices[] __initdata = {
	&jz4750_rtc_device,
	&jz4750_udc_device,
	&jz4750_nand_device,
	&jz4750_framebuffer_device,
};

static int __init xz0032_init_platform_devices(void)
{
	jz4750_nand_device.dev.platform_data = &xz0032_nand_pdata;
	jz4750_framebuffer_device.dev.platform_data = &xz0032_fb_pdata;

	jz4750_serial_device_register();

	return platform_add_devices(jz_platform_devices,
					ARRAY_SIZE(jz_platform_devices));

}

struct jz47xx_clock_board_data jz47xx_clock_bdata = {
	.ext_rate = 12000000,
	.rtc_rate = 32768,
};

static int __init xz0032_board_setup(void)
{
	printk(KERN_INFO "XZ0032 (JZ4725B) setup\n");

	if (xz0032_init_platform_devices())
		panic("Failed to initalize platform devices\n");

	return 0;
}
arch_initcall(xz0032_board_setup);

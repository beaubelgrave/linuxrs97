/*
 *  Copyright (C) 2011 Peter Zotov <whitequark@whitequark.org>
 *  JZ4750 platform devices
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

#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/resource.h>

#include <linux/dma-mapping.h>

#include <jz4750/platform.h>
#include <jz4750/irq.h>

#include <asm/mach-jz47xx/base.h>

#include <linux/serial_core.h>
#include <linux/serial_8250.h>

#include "../serial.h"
#include "../clock.h"

/* UDC (USB gadget controller) */
static struct resource jz4750_usb_gdt_resources[] = {
	{
		.start  = JZ47XX_UDC_BASE_ADDR,
		.end    = JZ47XX_UDC_BASE_ADDR + 0x1000 - 1,
		.flags  = IORESOURCE_MEM,
	},
	{
		.start  = JZ4750_IRQ_UDC,
		.end    = JZ4750_IRQ_UDC,
		.flags  = IORESOURCE_IRQ,
        },
};

struct platform_device jz4750_udc_device = {
	.name           = "jz-udc",
	.id             = -1,
	.dev = {
		.dma_mask = &jz4750_udc_device.dev.coherent_dma_mask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	},
	.num_resources  = ARRAY_SIZE(jz4750_usb_gdt_resources),
	.resource       = jz4750_usb_gdt_resources,
};

/* NAND controller */
static struct resource jz4750_nand_resources[] = {
	{
		.name   = "mmio",
		.start  = JZ47XX_EMC_BASE_ADDR,
		.end    = JZ47XX_EMC_BASE_ADDR + 0x1000 - 1,
		.flags  = IORESOURCE_MEM,
	},
	{
		.name   = "bch",
		.start  = JZ47XX_BCH_BASE_ADDR,
		.end    = JZ47XX_BCH_BASE_ADDR + 0x40 - 1,
		.flags  = IORESOURCE_MEM,
	},
	{
		.name   = "bank",
		.start  = 0x18000000,
		.end    = 0x180C0000 - 1,
		.flags  = IORESOURCE_MEM,
	},
};

struct platform_device jz4750_nand_device = {
	.name = "jz4750-nand",
	.num_resources = ARRAY_SIZE(jz4750_nand_resources),
	.resource = jz4750_nand_resources,
};

/* LCD controller */
static struct resource jz4750_framebuffer_resources[] = {
	{
		.start  = JZ47XX_LCD_BASE_ADDR,
		.end    = JZ47XX_LCD_BASE_ADDR + 0x1000 - 1,
		.flags  = IORESOURCE_MEM,
	},
};

struct platform_device jz4750_framebuffer_device = {
	.name           = "jz4740-fb",
	.id             = -1,
	.num_resources  = ARRAY_SIZE(jz4750_framebuffer_resources),
	.resource       = jz4750_framebuffer_resources,
	.dev = {
		.dma_mask = &jz4750_framebuffer_device.dev.coherent_dma_mask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	},
};

/* RTC controller */
static struct resource jz4750_rtc_resources[] = {
	{
		.start  = JZ47XX_RTC_BASE_ADDR,
		.end    = JZ47XX_RTC_BASE_ADDR + 0x38 - 1,
		.flags  = IORESOURCE_MEM,
	},
	{
		.start  = JZ4750_IRQ_RTC,
		.end    = JZ4750_IRQ_RTC,
		.flags  = IORESOURCE_IRQ,
	},
};

struct platform_device jz4750_rtc_device = {
	.name           = "jz4740-rtc",
	.id             = -1,
	.num_resources  = ARRAY_SIZE(jz4750_rtc_resources),
	.resource       = jz4750_rtc_resources,
};

/* Serial */
#define JZ4750_UART_DATA(_id) \
	{ \
		.flags = UPF_SKIP_TEST | UPF_IOREMAP | UPF_FIXED_TYPE, \
		.iotype = UPIO_MEM, \
		.regshift = 2, \
		.serial_out = jz4740_serial_out, \
		.type = PORT_16550, \
		.mapbase = JZ47XX_UART ## _id ## _BASE_ADDR, \
		.irq = JZ4750_IRQ_UART ## _id, \
	}

static struct plat_serial8250_port jz4750_uart_data[] = {
	JZ4750_UART_DATA(0),
	{},
};

static struct platform_device jz4740_uart_device = {
	.name = "serial8250",
	.id = 0,
	.dev = {
		.platform_data = jz4750_uart_data,
	},
};

void jz4750_serial_device_register(void)
{
	struct plat_serial8250_port *p;

	for (p = jz4750_uart_data; p->flags != 0; ++p)
		p->uartclk = jz47xx_clock_bdata.ext_rate;

	platform_device_register(&jz4740_uart_device);
}

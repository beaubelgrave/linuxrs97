/*
 *  Copyright (C) 2009-2010, Peter Zotov <whitequark@whitequark.org>
 *  JZ4750 platform device definitions
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

#ifndef __JZ4750_PLATFORM_H
#define __JZ4750_PLATFORM_H

#include <linux/platform_device.h>

extern struct platform_device jz4750_rtc_device;
extern struct platform_device jz4750_udc_device;
extern struct platform_device jz4750_nand_device;
extern struct platform_device jz4750_framebuffer_device;

void jz4750_serial_device_register(void);

#endif

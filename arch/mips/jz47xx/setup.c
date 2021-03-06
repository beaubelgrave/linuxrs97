/*
 *  Copyright (C) 2009-2010, Lars-Peter Clausen <lars@metafoo.de>
 *  JZ4740 setup code
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

#include <linux/init.h>
#include <linux/kernel.h>

#include <asm/mach-jz47xx/soc.h>

#include "reset.h"

void __init plat_mem_setup(void)
{
	jz4740_reset_init();
}

const char *get_system_type(void)
{
	if (soc_is_jz4740())
		return "JZ4740";
	else if(soc_is_jz4750())
		return "JZ4750";
	else if(soc_is_jz4760())
		return "JZ4760";
	else
		return "unknown";
}

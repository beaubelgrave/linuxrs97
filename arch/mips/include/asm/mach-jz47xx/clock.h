/*
 *  Copyright (C) 2010, Lars-Peter Clausen <lars@metafoo.de>
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

#ifndef __ASM_JZ47XX_CLOCK_H__
#define __ASM_JZ47XX_CLOCK_H__

enum jz47xx_wait_mode {
	JZ47XX_WAIT_MODE_IDLE,
	JZ47XX_WAIT_MODE_SLEEP,
};

void jz47xx_clock_set_wait_mode(enum jz47xx_wait_mode mode);

#endif

/*
 *  Copyright (C) 2010, Lars-Peter Clausen <lars@metafoo.de>
 *  JZ4740 platform time support
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

#include <linux/clocksource.h>

#include <asm/time.h>

#include "clock.h"
#include "timer.h"
#include "timer-csrc.h"

static unsigned int jz47xx_timer_clocksource;

static cycle_t jz47xx_clocksource_read(struct clocksource *cs)
{
	return jz4740_timer_get_count(jz47xx_timer_clocksource);
}

static struct clocksource jz47xx_clocksource = {
	.name = "jz47xx-timer",
	.rating = 200,
	.read = jz47xx_clocksource_read,
	.mask = CLOCKSOURCE_MASK(16),
	.flags = CLOCK_SOURCE_IS_CONTINUOUS,
};

int __init jz47xx_timer_csrc_init(unsigned int timer_id)
{
	int ret;
	unsigned int clk_rate;

	jz47xx_timer_clocksource = timer_id;

	clk_rate = jz47xx_clock_bdata.ext_rate >> 4;

	clocksource_set_clock(&jz47xx_clocksource, clk_rate);
	ret = clocksource_register(&jz47xx_clocksource);

	if (ret) {
		printk(KERN_ERR "Failed to register clocksource: %d\n", ret);
		return ret;
	}

	jz4740_timer_start(jz47xx_timer_clocksource);

	jz4740_timer_set_ctrl(jz47xx_timer_clocksource,
		JZ_TIMER_CTRL_PRESCALE_16 | JZ_TIMER_CTRL_SRC_EXT);

	jz4740_timer_set_period(jz47xx_timer_clocksource, 0xffff);
	jz4740_timer_enable(jz47xx_timer_clocksource);

	return 0;
}

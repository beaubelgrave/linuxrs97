/*
 *  Copyright (C) 2010, Lars-Peter Clausen <lars@metafoo.de>
 *  jz47xx platform time support
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

#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/time.h>

#include <linux/clockchips.h>

#include <asm/time.h>

#include "clock.h"
#include "timer.h"
#include "timer-cevt.h"

static unsigned int jz47xx_timer_clockevent;

static uint16_t jz47xx_ticks_per_jiffy;

static irqreturn_t jz47xx_clockevent_irq(int irq, void *devid)
{
	struct clock_event_device *cd = devid;

	jz4740_timer_ack_full(jz47xx_timer_clockevent);

	if (cd->mode != CLOCK_EVT_MODE_PERIODIC)
		jz4740_timer_disable(jz47xx_timer_clockevent);

	cd->event_handler(cd);

	return IRQ_HANDLED;
}

static void jz47xx_clockevent_set_mode(enum clock_event_mode mode,
	struct clock_event_device *cd)
{
	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		jz4740_timer_set_count(jz47xx_timer_clockevent, 0);
		jz4740_timer_set_period(jz47xx_timer_clockevent, jz47xx_ticks_per_jiffy);
	case CLOCK_EVT_MODE_RESUME:
		jz4740_timer_irq_full_enable(jz47xx_timer_clockevent);
		jz4740_timer_enable(jz47xx_timer_clockevent);
		break;
	case CLOCK_EVT_MODE_ONESHOT:
	case CLOCK_EVT_MODE_SHUTDOWN:
		jz4740_timer_disable(jz47xx_timer_clockevent);
		break;
	default:
		break;
	}
}

static int jz47xx_clockevent_set_next(unsigned long evt,
	struct clock_event_device *cd)
{
	jz4740_timer_set_count(jz47xx_timer_clockevent, 0);
	jz4740_timer_set_period(jz47xx_timer_clockevent, evt);
	jz4740_timer_enable(jz47xx_timer_clockevent);

	return 0;
}

static struct clock_event_device jz47xx_clockevent = {
	.name = "jz47xx-timer",
	.features = CLOCK_EVT_FEAT_PERIODIC,
	.set_next_event = jz47xx_clockevent_set_next,
	.set_mode = jz47xx_clockevent_set_mode,
	.rating = 200,
};

static struct irqaction timer_irqaction = {
	.handler	= jz47xx_clockevent_irq,
	.flags		= IRQF_PERCPU | IRQF_TIMER,
	.name		= "jz47xx-cevt",
	.dev_id		= &jz47xx_clockevent,
};

void __init jz47xx_timer_cevt_init(unsigned int irq, unsigned int timer_id)
{
	unsigned int clk_rate;

	jz47xx_timer_clockevent = timer_id;

	jz47xx_clockevent.irq = irq;

	clk_rate = jz47xx_clock_bdata.ext_rate >> 4;
	jz47xx_ticks_per_jiffy = DIV_ROUND_CLOSEST(clk_rate, HZ);

	clockevent_set_clock(&jz47xx_clockevent, clk_rate);
	jz47xx_clockevent.min_delta_ns = clockevent_delta2ns(100, &jz47xx_clockevent);
	jz47xx_clockevent.max_delta_ns = clockevent_delta2ns(0xffff, &jz47xx_clockevent);
	jz47xx_clockevent.cpumask = cpumask_of(0);

	clockevents_register_device(&jz47xx_clockevent);

	setup_irq(irq, &timer_irqaction);

	jz4740_timer_start(jz47xx_timer_clockevent);

	jz4740_timer_set_ctrl(jz47xx_timer_clockevent,
		JZ_TIMER_CTRL_PRESCALE_16 | JZ_TIMER_CTRL_SRC_EXT);

	jz4740_timer_set_period(jz47xx_timer_clockevent, jz47xx_ticks_per_jiffy);
	jz4740_timer_irq_full_enable(jz47xx_timer_clockevent);
	jz4740_timer_enable(jz47xx_timer_clockevent);
}

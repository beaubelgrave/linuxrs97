/*
 *  Copyright (C) 2011, Lars-Peter Clausen <lars@metafoo.de>
 *  jz47xx OST support
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
#include <linux/clocksource.h>

#include <asm/mach-jz47xx/base.h>

#include <asm/time.h>

#include "clock.h"
#include "timer.h"

#define JZ_REG_OST_COMPARE	0x0
#define JZ_REG_OST_COUNT	0x8
#define JZ_REG_OST_CTRL		0xC

/* 0: Counter will be set to 0 if it is equal to the compare value
 * 1: Counter will only be set to 0 when it wraps around */
#define JZ_OST_CTRL_COUNT_MODE BIT(15)

#define JZ47XX_TIMER_OST 15

static void __iomem *jz47xx_ost_base;

static irqreturn_t jz47xx_ost_clockevent_irq(int irq, void *devid)
{
	struct clock_event_device *cd = devid;

	jz4740_timer_irq_full_disable(JZ47XX_TIMER_OST);
	jz4740_timer_ack_full(JZ47XX_TIMER_OST);

	cd->event_handler(cd);

	return IRQ_HANDLED;
}

static void jz47xx_ost_clockevent_set_mode(enum clock_event_mode mode,
	struct clock_event_device *cd)
{
	switch (mode) {
	case CLOCK_EVT_MODE_RESUME:
		jz4740_timer_irq_full_enable(JZ47XX_TIMER_OST);
		break;
	case CLOCK_EVT_MODE_ONESHOT:
	case CLOCK_EVT_MODE_SHUTDOWN:
		jz4740_timer_irq_full_disable(JZ47XX_TIMER_OST);
		break;
	default:
		break;
	}
}

static int jz47xx_ost_clockevent_set_next(unsigned long evt,
	struct clock_event_device *cd)
{
	uint32_t count = readl(jz47xx_ost_base + JZ_REG_OST_COUNT);

	count += evt;
	writel(count, jz47xx_ost_base + JZ_REG_OST_COMPARE);

	jz4740_timer_irq_full_enable(JZ47XX_TIMER_OST);

	count = readl(jz47xx_ost_base + JZ_REG_OST_COUNT);

	return 0;
}

static struct clock_event_device jz47xx_ost_clockevent = {
	.name = "jz47xx-ost",
	.features = CLOCK_EVT_FEAT_ONESHOT,
	.set_next_event = jz47xx_ost_clockevent_set_next,
	.set_mode = jz47xx_ost_clockevent_set_mode,
	.rating = 300,
};

static struct irqaction timer_irqaction = {
	.handler	= jz47xx_ost_clockevent_irq,
	.flags		= IRQF_PERCPU | IRQF_TIMER,
	.name		= "jz47xx-ost",
	.dev_id		= &jz47xx_ost_clockevent,
};

static cycle_t jz47xx_ost_clocksource_read(struct clocksource *cs)
{
	return readl(jz47xx_ost_base + JZ_REG_OST_COUNT);
}

static struct clocksource jz47xx_ost_clocksource = {
	.name = "jz47xx-ost",
	.rating = 300,
	.read = jz47xx_ost_clocksource_read,
	.mask = CLOCKSOURCE_MASK(32),
	.flags = CLOCK_SOURCE_IS_CONTINUOUS,
};

int __init jz47xx_ost_init(unsigned int irq)
{
	unsigned int clk_rate;
	int ret;

	jz47xx_ost_base = ioremap(JZ47XX_OST_BASE_ADDR, 0x10);
	if (!jz47xx_ost_base) {
		printk(KERN_ERR "Failed to ioremap ost mem region\n");
		return -EBUSY;
	}

	jz4740_timer_start(JZ47XX_TIMER_OST);

	jz47xx_ost_clockevent.irq = irq;

	clk_rate = jz47xx_clock_bdata.ext_rate >> 4;

	clocksource_set_clock(&jz47xx_ost_clocksource, clk_rate);
	ret = clocksource_register(&jz47xx_ost_clocksource);

	if (ret) {
		printk(KERN_ERR "Failed to register clocksource: %d\n", ret);
		return ret;
	}

	clockevent_set_clock(&jz47xx_ost_clockevent, clk_rate);
	jz47xx_ost_clockevent.min_delta_ns = clockevent_delta2ns(100, &jz47xx_ost_clockevent);
	jz47xx_ost_clockevent.max_delta_ns = clockevent_delta2ns(0xffffffff, &jz47xx_ost_clockevent);
	jz47xx_ost_clockevent.cpumask = cpumask_of(0);

	clockevents_register_device(&jz47xx_ost_clockevent);

	writew(JZ_TIMER_CTRL_PRESCALE_16 | JZ_TIMER_CTRL_SRC_EXT |
		JZ_OST_CTRL_COUNT_MODE, jz47xx_ost_base + JZ_REG_OST_CTRL);

	setup_irq(irq, &timer_irqaction);

	jz4740_timer_enable(JZ47XX_TIMER_OST);

	return 0;
}

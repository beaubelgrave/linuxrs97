/*
 *  Copyright (C) 2009-2010, Lars-Peter Clausen <lars@metafoo.de>
 *  Driver for Ilitek ili8960 LCD
 *
 *  This program is free software; you can redistribute	 it and/or modify it
 *  under  the terms of	 the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the	License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/lcd.h>
#include <linux/backlight.h>
#include <linux/delay.h>

struct ili8960 {
	struct spi_device *spi;
	struct lcd_device *lcd;
	struct backlight_device *bl;
	bool enabled;
	unsigned int brightness;
};

#define ILI8960_REG_BRIGHTNESS	0x03
#define ILI8960_REG_POWER	0x05
#define ILI8960_REG_CONTRAST	0x0d

static int ili8960_write_reg(struct spi_device *spi, uint8_t reg,
				uint8_t data)
{
	uint8_t buf[2];
	buf[0] = ((reg & 0x40) << 1) | (reg & 0x3f);
	buf[1] = data;

	return spi_write(spi, buf, sizeof(buf));
}

static int ili8960_programm_power(struct spi_device *spi, bool enabled)
{
	int ret;

	if (enabled)
		mdelay(20);

	ret = ili8960_write_reg(spi, ILI8960_REG_POWER, enabled ? 0xc7 : 0xc6);

	if (!enabled)
		mdelay(20);

	return ret;
}

static int ili8960_set_power(struct lcd_device *lcd, int power)
{
	struct ili8960 *ili8960 = lcd_get_data(lcd);

	switch (power) {
	case FB_BLANK_UNBLANK:
		ili8960->enabled = true;
		break;
	default:
		ili8960->enabled = false;
		break;
	}

	return ili8960_programm_power(ili8960->spi, ili8960->enabled);
}

static int ili8960_get_power(struct lcd_device *lcd)
{
	struct ili8960 *ili8960 = lcd_get_data(lcd);
	return ili8960->enabled ? FB_BLANK_UNBLANK : FB_BLANK_POWERDOWN;
}

static int ili8960_set_contrast(struct lcd_device *lcd, int contrast)
{
	struct ili8960 *ili8960 = lcd_get_data(lcd);

	return ili8960_write_reg(ili8960->spi, ILI8960_REG_CONTRAST, contrast);
}

static int ili8960_set_mode(struct lcd_device *lcd, struct fb_videomode *mode)
{
	if (mode->xres != 320 && mode->yres != 240)
		return -EINVAL;

	return 0;
}

static int ili8960_set_brightness(struct ili8960 *ili8960, int brightness)
{
	int ret;

	ret = ili8960_write_reg(ili8960->spi, ILI8960_REG_BRIGHTNESS, brightness);

	if (ret == 0)
		ili8960->brightness = brightness;

	return ret;
}

static ssize_t ili8960_show_brightness(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct lcd_device *ld = to_lcd_device(dev);
	struct ili8960 *ili8960 = lcd_get_data(ld);

	return sprintf(buf, "%u\n", ili8960->brightness);
}

static ssize_t ili8960_store_brightness(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	char *endp;
	struct lcd_device *ld = to_lcd_device(dev);
	struct ili8960 *ili8960 = lcd_get_data(ld);
	unsigned long brightness;

	ret = strict_strtoul(buf, 0, &brightness);

	if (brightness > 255)
		return -EINVAL;

	ili8960_set_brightness(ili8960, brightness);

	return count;
}


static DEVICE_ATTR(brightness, 0644, ili8960_show_brightness,
	ili8960_store_brightness);

static struct lcd_ops ili8960_lcd_ops = {
	.set_power = ili8960_set_power,
	.get_power = ili8960_get_power,
	.set_contrast = ili8960_set_contrast,
	.set_mode = ili8960_set_mode,
};

static int __devinit ili8960_probe(struct spi_device *spi)
{
	int ret;
	struct ili8960 *ili8960;

	ili8960 = kmalloc(sizeof(*ili8960), GFP_KERNEL);
	if (!ili8960)
		return -ENOMEM;

	spi->bits_per_word = 8;
	spi->mode = SPI_MODE_3;

	ret = spi_setup(spi);
	if (ret) {
		dev_err(&spi->dev, "Failed to setup spi\n");
		goto err_free_ili8960;
	}

	ili8960->spi = spi;

	ili8960->lcd = lcd_device_register("ili8960-lcd", &spi->dev, ili8960,
						&ili8960_lcd_ops);

	if (IS_ERR(ili8960->lcd)) {
		ret = PTR_ERR(ili8960->lcd);
		dev_err(&spi->dev, "Failed to register lcd device: %d\n", ret);
		goto err_free_ili8960;
	}

	ili8960->lcd->props.max_contrast = 255;

	ret = device_create_file(&ili8960->lcd->dev, &dev_attr_brightness);
	if (ret)
		goto err_unregister_lcd;

	ili8960_programm_power(ili8960->spi, true);
	ili8960->enabled = true;

	spi_set_drvdata(spi, ili8960);

	ili8960_write_reg(spi, 0x13, 0x01);

	return 0;
err_unregister_lcd:
	lcd_device_unregister(ili8960->lcd);
err_free_ili8960:
	kfree(ili8960);
	return ret;
}

static int __devexit ili8960_remove(struct spi_device *spi)
{
	struct ili8960 *ili8960 = spi_get_drvdata(spi);

	device_remove_file(&ili8960->lcd->dev, &dev_attr_brightness);
	lcd_device_unregister(ili8960->lcd);

	spi_set_drvdata(spi, NULL);
	kfree(ili8960);
	return 0;
}

#ifdef CONFIG_PM

static int ili8960_suspend(struct spi_device *spi, pm_message_t state)
{
	struct ili8960 *ili8960 = spi_get_drvdata(spi);

	if (ili8960->enabled)
		ili8960_programm_power(ili8960->spi, false);

	return 0;
}

static int ili8960_resume(struct spi_device *spi)
{
	struct ili8960 *ili8960 = spi_get_drvdata(spi);

	if (ili8960->enabled)
		ili8960_programm_power(ili8960->spi, true);

	return 0;
}

#else
#define ili8960_suspend NULL
#define ili8960_resume NULL
#endif

static struct spi_driver ili8960_driver = {
	.driver = {
		.name = "ili8960",
		.owner = THIS_MODULE,
	},
	.probe = ili8960_probe,
	.remove = __devexit_p(ili8960_remove),
	.suspend = ili8960_suspend,
	.resume = ili8960_resume,
};

static int __init ili8960_init(void)
{
	return spi_register_driver(&ili8960_driver);
}
module_init(ili8960_init);

static void __exit ili8960_exit(void)
{
	spi_unregister_driver(&ili8960_driver);
}
module_exit(ili8960_exit)

MODULE_AUTHOR("Lars-Peter Clausen");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("LCD driver for Ilitek ili8960");
MODULE_ALIAS("spi:ili8960");

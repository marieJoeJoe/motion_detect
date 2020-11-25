/*
 * ws2812b.h - platform data structure for WS2812B NeoPixel LEDs
 *
 * Copyright (C) 2016 hyzhang <hyzhang7@msn.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 */

#ifndef __LINUX_LED_WS2812B_H__
#define __LINUX_LED_WS2812B_H__

#include <linux/leds.h>
#include <linux/workqueue.h>



struct ws2812b_led {
	int index;
	struct led_classdev ldev;
	uint8_t brightness;
};

struct ws2812b_leds_device {
	struct ws2812b_led * led;
	struct work_struct work;
	struct workqueue_struct *wq;
	int dataPin;
	int dataPinFuncMsk;
	int numberOfLEDs;
};

#endif /* __LINUX_LED_WS2812B_H__ */


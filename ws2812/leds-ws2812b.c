/*
 * ws2812b.c - platform data structure for WS2812B NeoPixel LEDs
 *
 * This driver supports BCM2835 platform, other platforms need to implement
 * their platform-specific codes.
 *
 * Based on jazzycamel's idea that use PWM to generate accurate timing
 * jazzycamel's code can be found here: github.com/jazzycamel/ws28128-rpi
 *
 * Copyright (C) 2016 hyzhang <hyzhang7@msn.com>
 *
 * 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 */

/* Some configurations */ 
#define DEBUG
#define NO_DMA_FOR_WS2812B
#define MAX_NUM_OF_LEDS_PER_WS2812B_MODULE   8

#include <linux/leds.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
//#include <linux/leds-ws2812b.h>
#include "leds-ws2812b.h"

/* Some globle variables*/
static volatile  uint8_t brightness[3];
spinlock_t ws2812b__lock;
struct ws2812b_leds_device * ws2812b_device;

#define CONFIG_ARCH_BCM2709

/* Platform-specific code */
#ifdef CONFIG_ARCH_BCM2709   // For Raspberry Pi 2

//#include <mach/platform.h>

/* Pin settings for Rpi */
#define DAT_PIN            18
#define DAT_PIN_FNC_MSK    0b010      // Func. 5 on pin18 is pwm0

/* Platform-specific parameters*/
#define PWM_COUNT          12
#define PWM_DIV            2
/* Platform-specific registers*/
static int gpioBaseAddr,clockBaseAddr,pwmBaseAddr;

#define PWMSTA             ((void *)(pwmBaseAddr+0x04))
#define PWMFIF1            ((void *)(pwmBaseAddr+0x18))
#define CM_PWMCTL          ((void *)(clockBaseAddr+0xa0))
#define CM_PWMDIV          ((void *)(clockBaseAddr+0xa4))
#define PWMCTL 		   ((void *)(pwmBaseAddr+0x00))
#define PWMRNG1    	   ((void *)(pwmBaseAddr+0x10))
#define GPFSEL_BASE        (gpioBaseAddr+0x00)
#define GPFCLR_BASE        (gpioBaseAddr+0x28)

#define GPIO_BASE    0x1c21400
#define CM_BASE      0x1c21424
#define PWM_BASE     0x1c21434
/* Some platform-specific low-level functions */
static int inline ws2812b_led_sendBit(uint8_t b)
{
	int timeout=0;
	/* Make sure read updated register value*/
	rmb();
	while(readl(PWMSTA)&0x01)              //wait until full flag clear
	{
		timeout++;
		if (timeout>=8000)
		{	
			/*  here is how to handle timeout*/
			return -EIO;
		}
		/* Make sure each readl reads latest value*/
		rmb();
	}
	
	switch (b)		
	{
	case 0:
		writel(4, PWMFIF1 ); // Send short pulse (400 ns) = 0
		break;
	case 1:
		writel(8, PWMFIF1 ); // send wide pulse (800 ns) = 1
		break;
	default:
		writel(0, PWMFIF1 ); // send no pulse, for WS2812b reset, send ~40 this pulses. 
		break;
	}
	/* Make sure date is writen to FIFO as soon as possible*/
	wmb();

	return 0;
} 


static int ws2812_led_init_clockStop(void)
{
	int timeout;
	uint32_t  old;
	/* Stop clock generator by just changing the ENAB bit, so read-modify-write*/
		/* Make sure read updated value */	
	rmb();	
	old=readl(CM_PWMCTL)&0b11100111111;
	writel((0x5a000000|old)&(~(1<<4)), CM_PWMCTL );	//Stop clock generator	
		/* Make sure value is writen to register */	
	wmb();
	/* Mow check if clock generator is stopped*/
		/* Make sure clock generator is not busy before doing anything*/
	timeout=0;	
	/* Make sure read updated value */	
	rmb();
	while(readl(CM_PWMCTL)&0x80)              //wait until busy flag clear
	{
		rmb();
		timeout++;
		if (timeout>=8000)
		{
			return -EBUSY;
		}
	}
	

	return 0;
}
static int ws2812b_led_init_pwmClock(void)
{
	int divisor=PWM_DIV ;

	/* Set divisor*/
	writel(0x5a000000|(divisor<<12), CM_PWMDIV ); 
		
	/* To start clock generator, ENAB and other bits cannot be set simultaneously, so, two steps */
	
	/* 1. Set clock source*/
	writel(0x5a000000|(1<<0), CM_PWMCTL );	//Set clock source: oscillator	
		/* Make sure value is writen to register before going to next step */
	wmb();
	/* 2. Set ENAB bit to start clock generator*/
	writel(0x5a000000|(1<<4)|(1<<0), CM_PWMCTL );	//Start clock generator	
	    /* start clock generator as soon as possible*/
	wmb();
	
	return 0;
}
static int ws2812_led_init_gpioPin(void)
{
	uint32_t old,msk;
	int registerOffset,bitOffset;
	

	registerOffset=4*((DAT_PIN)/10);
	/* Read-Modify-Write*/
	rmb();	
	old=readl((void *)(GPFSEL_BASE+registerOffset)); 
	
	bitOffset=3*((DAT_PIN)%10);
	msk=0b111 << bitOffset;          
	writel((old& ~msk), ((void *)(GPFSEL_BASE+registerOffset)) );  // First set pin as input (not sure this is necessary)
	writel((old& ~msk) | ((DAT_PIN_FNC_MSK<<bitOffset)&msk), ((void *)(GPFSEL_BASE+registerOffset)) ); // then, set it as alt. func. 5 (PWM0)
	return 0;
}
#ifdef NO_DMA_FOR_WS2812B
static int ws2812b_pwm_init(void)
{
	int count=PWM_COUNT;
	/* Clear PWMCTL register*/
	writel(0, PWMCTL ); 

	/* Set PWM range*/
	writel(count, PWMRNG1 ); 
	
	/* Clear PWM FIFO buffer by setting CLRF1 bit*/
	writel((1<<6), PWMCTL );  //Clear FIFO
		/* Make sure FIFO is cleared before starting PWM*/
	wmb();

	/* Start PWM*/
		/*MSEN1=1; USEF1=1; POLA1=0; SBIT1=0; RPTL1=0; MODE1=0; PWEN1=1 */
	writel((1<<7)|(1<<5)|(0<<3)|(0<<4)|(0<<2)|(1<<0)|(0<<1), PWMCTL ); 
	
	
	return 0;
}

static int inline ws2812b_led_sendRst(void)
{
	int i,ret;
	for (i=0;i<50;i++)
	{
		ret=ws2812b_led_sendBit(2);
		if (ret<0)
		{
			return ret;
		}
	}
	return 0;
}
static int inline ws2812b_led_sendData(uint8_t dat)
{
	int i,ret;
	
	
	for (i=0;i<8;i++)
	{
		ret=ws2812b_led_sendBit((dat&0x80)?1:0);
		if (ret<0)
		{
			return ret;
		}		
		dat<<=1;
	}
	return 0;

}
#else
#error "Please implement DMA init. and transfer functions"
#endif
/* To port to a new platform, rewrite these two functions below. */
static inline int ws2812b_led_update(void)
{
	unsigned long flags;
	int i,ret;
	/* this spin lock prevent task scheduling thus prevent buffer underflow*/
	spin_lock_irqsave(&ws2812b__lock,flags);

	ret=ws2812b_led_sendRst();
	if (ret<0)
	{
		spin_unlock_irqrestore(&ws2812b__lock,flags);
		return ret;
	}
	for (i=0;i<(ws2812b_device->numberOfLEDs);i++)
	{
		ret=ws2812b_led_sendData(ws2812b_device->led[i].brightness);
		if (ret<0)
		{
			spin_unlock_irqrestore(&ws2812b__lock,flags);
			return ret;
		}
	}
	/* not sure why I need to send a 0 to force the PWM stop, even after disabling the RPTL1 in PWMCTL*/
	ret=ws2812b_led_sendBit(2);
	if (ret<0)
	{
		return ret;
	}

	spin_unlock_irqrestore(&ws2812b__lock,flags);
	return 0;
}
static int ws2812b_led_init_hw(struct platform_device * pdev)
{
	int retry,ret;
	
	dev_dbg(&pdev->dev,"WS2812B HW_Init \n");
	/* Try to stop clock generator */
	retry=0;
	do	
	{
		ret=ws2812_led_init_clockStop();
		if (ret<0)
		{
			dev_dbg(&pdev->dev,"WS2812B PWM_Clock_Timeout. \n");
			retry++;
		}
	}while((ret<0) && (retry<3));
	
	if (ret<0)
	{
		return ret;
	}
	
	/* Set new clock generator parameters*/
	dev_dbg(&pdev->dev,"WS2812B Clock_Init \n");
	ret=ws2812b_led_init_pwmClock();
	if (ret<0)
	{
		return ret;
	}
	
	/* Set data pin as pwm0*/
	dev_dbg(&pdev->dev,"WS2812B set pin func.\n");	
	ret=ws2812_led_init_gpioPin();
	if (ret<0)
	{
		return ret;
	}

	/*set pwm registers*/
	dev_dbg(&pdev->dev,"WS2812B set PWM registers.\n");	
	ret=ws2812b_pwm_init();
	if (ret<0)
	{
		return ret;
	}
#ifndef NO_DMA_FOR_WS2812B
	/*set DMA parameters*/
	dev_dbg(&pdev->dev,"WS2812B DMA Init.\n");	
	ret=ws2812b_dma_init();
	if (ret<0)
	{
		return ret;
	}
#endif

	return 0;
	
}

#else
#error "Need to add supporting codes for current platform"
/* Required platform-specific functions:
 *    static int ws2812b_led_init_hw(struct platform_device * pdev);
 *    static inline int ws2812b_led_update(void);
 * 
 *
 *
 */
#endif


static void ws2812b_led_work(struct work_struct *work)
{
	
	ws2812b_led_update();

}

static void ws2812b_led_set(struct led_classdev *cdev, enum led_brightness bvalue)
{
	struct ws2812b_led *led = container_of(cdev, struct ws2812b_led, ldev);
	
	
	led->brightness = bvalue;
	
	//schedule_work(&ws2812b_device->work);
	queue_work(ws2812b_device->wq,&ws2812b_device->work);
}


static enum led_brightness ws2812b_led_get(struct led_classdev *cdev)
{
	struct ws2812b_led *led = container_of(cdev, struct ws2812b_led, ldev);
	return led->brightness;
}


static int ws2812b_led_init_res(struct platform_device * pdev)
{
	struct ws2812b_led * ledArray; 
	
	char * ledNames;
	int i;
	dev_dbg(&pdev->dev,"WS2812B Resource Init.\n");

	/* Get register base address */
	//gpioBaseAddr=(int)__io_address(GPIO_BASE);
	//clockBaseAddr=(int)__io_address(CM_BASE);
	//pwmBaseAddr=(int)__io_address(PWM_BASE);

	gpioBaseAddr=(int)GPIO_BASE;
	clockBaseAddr=(int)CM_BASE;
	pwmBaseAddr=(int)PWM_BASE;

	dev_dbg(&pdev->dev,"WS2812B Register Base Address:\n gpio %x\n clk %x \n pwm %x.\n",gpioBaseAddr,clockBaseAddr,pwmBaseAddr);

	/* Initialize resources*/
	spin_lock_init(&ws2812b__lock);

	/* Allocate Memory*/
	/* 1. Allocate memory for led structures and initialize them */
	ledArray=(struct ws2812b_led *)devm_kzalloc(&pdev->dev,3*MAX_NUM_OF_LEDS_PER_WS2812B_MODULE*sizeof(struct ws2812b_led),GFP_KERNEL);
	if (ledArray==NULL)
	{
		return -ENOMEM;
	}
	for(i=0;i<3*MAX_NUM_OF_LEDS_PER_WS2812B_MODULE;i++)
	{
		ledArray[i].index=i;
	}
	/* 2. Allocate memory for LED names: the leds under/sys/class/leds/ will look like "ws2812b-red1", "ws2812b-green7", etc. */
	ledNames=(char *)devm_kzalloc(&pdev->dev,3*MAX_NUM_OF_LEDS_PER_WS2812B_MODULE*sizeof(char)*20,GFP_KERNEL);
	if (ledNames==NULL)
	{
		return -ENOMEM;
	}	
	/* 3. Allocate memory for device structure and initialize it */
	ws2812b_device=(struct ws2812b_leds_device *)devm_kzalloc(&pdev->dev,sizeof(struct ws2812b_leds_device),GFP_KERNEL);
	if (ws2812b_device==NULL)
	{
		return -ENOMEM;
	}
	ws2812b_device->led = ledArray;
	ws2812b_device->dataPin = DAT_PIN;   /* Rightnow, just hardcode those parameters*/
	ws2812b_device->dataPinFuncMsk = DAT_PIN_FNC_MSK;
	ws2812b_device->numberOfLEDs = 3*MAX_NUM_OF_LEDS_PER_WS2812B_MODULE;  
   /* 4. Create a work queue for updating the brightness values to led chips */	
	ws2812b_device->wq=create_singlethread_workqueue("ws2812b-wq");
	INIT_WORK(&ws2812b_device->work, ws2812b_led_work);
	
	/* Initialize LED names*/
	for (i=0;i<(ws2812b_device->numberOfLEDs);i++)
	{
		if((i%3)==0)
		{
			snprintf(&(ledNames[20*i]),20,"ws2812b-green-%d",i/3);
		}
		else if((i%3)==1)
		{
			snprintf(&(ledNames[20*i]),20,"ws2812b-red-%d",i/3);
		}
		else
		{
			snprintf(&(ledNames[20*i]),20,"ws2812b-blue-%d",i/3);
		}
	}
	/* Initialize led_classdev*/
	for (i=0;i<(ws2812b_device->numberOfLEDs);i++)
	{
		ws2812b_device->led[i].ldev.name = &(ledNames[20*i]);
		ws2812b_device->led[i].ldev.default_trigger = "none";
		ws2812b_device->led[i].ldev.brightness_set = ws2812b_led_set;
		ws2812b_device->led[i].ldev.brightness_get = ws2812b_led_get;
	}

	return 0;
}
static int ws2812b_led_register(struct platform_device * pdev)
{
	int i,ret;
		
	dev_dbg(&pdev->dev,"WS2812B Register.\n");
	for (i=0;i<(ws2812b_device->numberOfLEDs);i++)
	{
		ret=devm_led_classdev_register(&pdev->dev,&(ws2812b_device->led[i].ldev));
		if (ret<0)
		{
			return ret;
		}
	}
	return 0;
}
static void ws2812b_led_unregister(struct platform_device * pdev)
{
	int i;
	dev_dbg(&pdev->dev,"WS2812B Un-register.\n");

	for (i=0;i<(ws2812b_device->numberOfLEDs);i++)
	{
		devm_led_classdev_unregister(&pdev->dev,&(ws2812b_device->led[i].ldev));
	}
	cancel_work_sync(&ws2812b_device->work);

	flush_workqueue(ws2812b_device->wq);
	destroy_workqueue(ws2812b_device->wq);
}
static int __init ws2812b_led_probe(struct platform_device * pdev)
{
	int ret=0;
	
	dev_dbg(&pdev->dev,"WS2812B Probe Func.\n");
	
	if (MAX_NUM_OF_LEDS_PER_WS2812B_MODULE<=0)
	{
		return -ENODEV;
	}

	/* Initialize system resource */
	ret=ws2812b_led_init_res(pdev);
	if (ret<0)
	{
		return ret;
	}

	/* Initialize hardware*/
	ret = ws2812b_led_init_hw( pdev);
	if (ret<0)
	{
		return ret;
	}

	/* register LEDs to system */
	ret=ws2812b_led_register(pdev);
	if (ret<0)
	{
		return ret;
	}
		
	return 0;
}
static int ws2812b_led_remove(struct platform_device * pdev)
{
	ws2812b_led_unregister(pdev);
	return 0;
}
static struct platform_driver ws2812b_led_driver = 
{
	.probe  =  ws2812b_led_probe,
	.remove =  ws2812b_led_remove,
	.driver = 
	{
		.name = "ws2812b-led",
		.owner= THIS_MODULE,
	},
};

module_platform_driver(ws2812b_led_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("LED driver for WS2812B NeoPixel");
MODULE_AUTHOR("hyzhang <hyzhang7@msn.com>");
MODULE_ALIAS("platform:rb532-led");

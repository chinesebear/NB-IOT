/*
 * File      : padconfig.c
 * http://www.openloongson.org/forum.php
 * description:
 * pad multiplex for smart loong(ls1c)
 * Change Logs:
 * Date                Author       Notes
 * 2015-07-16     chinesebear  first version
 */
#include <rthw.h>
#include <rtthread.h>
#include "padconfig.h"

void rt_pad_multiplex_init(void)
{

	/***************0~31*******************/
	/*GPIO 0~1 uart3*/
	CBUS_FOURTH(PAD_MUTI_BASE0) |=(1<<0);
	CBUS_FOURTH(PAD_MUTI_BASE0) |=(1<<1);
	/*GPIO 2~3 uart1*/
	CBUS_FOURTH(PAD_MUTI_BASE0) |=(1<<2);
	CBUS_FOURTH(PAD_MUTI_BASE0) |=(1<<3);
	/*GPIO 4~5*/
	GPIO_CFG(GPIO_BASE0) |= (1<<4);
	GPIO_CFG(GPIO_BASE0) |= (1<<5);

	/**************32~63******************/
	/*GPIO 36~37 uart2*/
	CBUS_FIFTH(PAD_MUTI_BASE1) |= (1<<4);//36-32=4
	CBUS_FIFTH(PAD_MUTI_BASE1) |= (1<<5);//37-32=5
	/*GPIO 46~47 pwm2& pwm3*/
	CBUS_FOURTH(PAD_MUTI_BASE1) |= (1<<14);
	CBUS_FOURTH(PAD_MUTI_BASE1) |= (1<<15);
	/*GPIO 48~53*/
	GPIO_CFG(GPIO_BASE1) |= (1<<16);//48-32=15
	GPIO_CFG(GPIO_BASE1) |= (1<<17);
	GPIO_CFG(GPIO_BASE1) |= (1<<18);//LED1
	GPIO_CFG(GPIO_BASE1) |= (1<<19);//LED2
	GPIO_CFG(GPIO_BASE1) |= (1<<20);//LED3
	GPIO_CFG(GPIO_BASE1) |= (1<<21);//LED4
	GPIO_CFG(GPIO_BASE1) |= (1<<22);//LED5

	/*GPIO 54~55 I2C1*/
	//CBUS_FOURTH(PAD_MUTI_BASE1) |= (1<<22);//54-32=22
	//CBUS_FOURTH(PAD_MUTI_BASE1) |= (1<<23);
	/*GPIO 56~57 UART7*/
	//CBUS_FIFTH(PAD_MUTI_BASE1) |= (1<<24);//56-32=24
	//CBUS_FIFTH(PAD_MUTI_BASE1) |= (1<<25);


	/**************64~95*****************/
	/*GPIO 85~86*/
	GPIO_CFG(GPIO_BASE2) |= (1<<21);//85-64=23 KEY2
	GPIO_CFG(GPIO_BASE2) |= (1<<22);//KEY3
	/*GPIO 87~90*/
	GPIO_CFG(GPIO_BASE2) |= (1<<23);//87-64=23
	GPIO_CFG(GPIO_BASE2) |= (1<<24);
	GPIO_CFG(GPIO_BASE2) |= (1<<25);
	GPIO_CFG(GPIO_BASE2) |= (1<<26);
	/*GPIO 91*/
	GPIO_CFG(GPIO_BASE2) |= (1<<27);
	/*GPIO 92*/
	GPIO_CFG(GPIO_BASE2) |= (1<<28);//KEY1
}

void rt_pad_show(void)
{
	rt_kprintf("Pad Multplex Show:\n");
	rt_kprintf("**Pad00~31**\n");
	rt_kprintf("1: 0x%08X\n",CBUS_FIRST(PAD_MUTI_BASE0));
	rt_kprintf("2: 0x%08X\n",CBUS_SECOND(PAD_MUTI_BASE0));
	rt_kprintf("3: 0x%08X\n",CBUS_THIRD(PAD_MUTI_BASE0));
	rt_kprintf("4: 0x%08X\n",CBUS_FOURTH(PAD_MUTI_BASE0));
	rt_kprintf("5: 0x%08X\n",CBUS_FIFTH(PAD_MUTI_BASE0));
	rt_kprintf("G: 0x%08X\n",GPIO_CFG(GPIO_BASE0));
	rt_kprintf("**Pad32~63**\n");
	rt_kprintf("1: 0x%08X\n",CBUS_FIRST(PAD_MUTI_BASE1));
	rt_kprintf("2: 0x%08X\n",CBUS_SECOND(PAD_MUTI_BASE1));
	rt_kprintf("3: 0x%08X\n",CBUS_THIRD(PAD_MUTI_BASE1));
	rt_kprintf("4: 0x%08X\n",CBUS_FOURTH(PAD_MUTI_BASE1));
	rt_kprintf("5: 0x%08X\n",CBUS_FIFTH(PAD_MUTI_BASE1));
	rt_kprintf("G: 0x%08X\n",GPIO_CFG(GPIO_BASE1));
	rt_kprintf("**Pad64~95**\n");
	rt_kprintf("1: 0x%08X\n",CBUS_FIRST(PAD_MUTI_BASE2));
	rt_kprintf("2: 0x%08X\n",CBUS_SECOND(PAD_MUTI_BASE2));
	rt_kprintf("3: 0x%08X\n",CBUS_THIRD(PAD_MUTI_BASE2));
	rt_kprintf("4: 0x%08X\n",CBUS_FOURTH(PAD_MUTI_BASE2));
	rt_kprintf("5: 0x%08X\n",CBUS_FIFTH(PAD_MUTI_BASE2));
	rt_kprintf("G: 0x%08X\n",GPIO_CFG(GPIO_BASE2));
	rt_kprintf("**Pad96~127**\n");
	rt_kprintf("1: 0x%08X\n",CBUS_FIRST(PAD_MUTI_BASE3));
	rt_kprintf("2: 0x%08X\n",CBUS_SECOND(PAD_MUTI_BASE3));
	rt_kprintf("3: 0x%08X\n",CBUS_THIRD(PAD_MUTI_BASE3));
	rt_kprintf("4: 0x%08X\n",CBUS_FOURTH(PAD_MUTI_BASE3));
	rt_kprintf("5: 0x%08X\n",CBUS_FIFTH(PAD_MUTI_BASE3));
	rt_kprintf("G: 0x%08X\n",GPIO_CFG(GPIO_BASE3));

		
}






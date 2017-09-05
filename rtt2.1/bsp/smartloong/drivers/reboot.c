/*
 * File      : reboot.c
 * http://www.openloongson.org/forum.php
 * description:
 * reboot board through setting watchdog for smart loong(ls1c)
 * Change Logs:
 * Date                 Author       Notes
 * 2015-07-14     chinesebear  first version
 */
#include <rthw.h>
#include <rtthread.h>
#include "ls1c.h"
#include "reboot.h"


void rt_wdt_reboot(rt_uint32_t time_val)
{
	rt_kprintf("Watchdog rebooting in %d seconds. \n", time_val);
	RB_WDT_EN(RB_WDT_EN_ADDR)  = RB_WDT_EN_TRUE;
	RB_WDT_TIMER(RB_WDT_TIMER_ADDR) = time_val*100; //RT_TICK_PER_SECOND
	RB_WDT_SET(RB_WDT_SET_ADDR)=RB_WDT_SET_TRUE;
	
}
void rt_reboot_control(rt_uint32_t  time_val, rt_uint8_t reboot_type)
{
	switch(reboot_type)
	{
		case 's':
			rt_wdt_reboot(time_val);
			break;
		case 'h':
			rt_kprintf("Hard reboot is not surpported. \n");
			break;
		default:
			rt_kprintf("Reboot type is wrong.\n");
			break;
	}
}









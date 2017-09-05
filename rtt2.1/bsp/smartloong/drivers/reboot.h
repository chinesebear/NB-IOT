/*
 * File      : pwm.h
 * http://www.openloongson.org/forum.php
 * description:
 * reboot board through setting watchdog for smart loong(ls1c)
 * Change Logs:
 * Date                Author             Notes
 * 2015-07-14     chinesebear    first version
 */


#ifndef __PWM_H__
#define __PWM_H__
 
#include "ls1c.h"

#define RB_WDT_EN_ADDR           0xBFE5C060
#define RB_WDT_TIMER_ADDR     0xBFE5C064
#define RB_WDT_SET_ADDR         0xBFE5C068

#define RB_WDT_EN(base)            __REG32(base)
#define RB_WDT_TIMER(base)      __REG32(base)
#define RB_WDT_SET(base)          __REG32(base)

#define RB_WDT_EN_TRUE       (1<<0)
#define RB_WDT_EN_FALSE      (0<<0)
#define RB_WDT_RESET_TIME  500 //reset in 5s 100 ticks one second
#define RB_WDT_SET_TRUE     (1<<0)
#define RB_WDT_SET_FALSE    (0<<0)

#define RB_SOFT_REBOOT 's'
#define RB_HARD_REBOOT 'h'

void rt_reboot_control(rt_uint32_t  time_val, rt_uint8_t reboot_type);

#endif 



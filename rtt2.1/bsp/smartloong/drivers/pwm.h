/*
 * File      : pwm.h
 * http://www.openloongson.org/forum.php
 * description:
 * for smart loong(ls1c)
 * Change Logs:
 * Date                Author             Notes
 * 2015-07-05     chinesebear    first version
 */


#ifndef __PWM_H__
#define __PWM_H__
 
#include "ls1c.h"
 
#define PWM0_BASE			0xBFE5C000
#define PWM1_BASE			0xBFE5C010
#define PWM2_BASE			0xBFE5C020
#define PWM3_BASE			0xBFE5C030 

#define Buffer_HRC  (0)
#define Buffer_LRC  (1)

 /* pwm registers */
#define PWM_CNTR(base)		__REG32(base + 0x00) //& 0x00ffffff
#define PWM_HRC(base)		__REG32(base + 0x04) //& 0x00ffffff
#define PWM_LRC(base)		__REG32(base + 0x08) //& 0x00ffffff
#define PWM_CTRL(base)		__REG32(base + 0x0C) //& 0x00000fff



/*pwm cmd*/
#define PWM_SET_CNTR 0
#define PWM_SET_HRC 1
#define PWM_SET_LRC 2
#define PWM_SINGLE_EN 3
#define PWM_SINGLE_DISEN 4
#define PWM_OUTPUT_EN 5
#define PWM_OUTPUT_DISEN 6
#define PWM_CNT_EN 7
#define PWM_CNT_DISEN 8


/*config : multplex for pwm2 & pwm3*/

 
 void rt_hw_pwm_init(void);
 
#endif

 

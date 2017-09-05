/*
 * File      : gpio.h
 * http://www.openloongson.org/forum.php
 * description:
 * for smart loong(ls1c)
 * Change Logs:
 * Date                Author             Notes
 * 2015-07-20     chinesebear    first version
 */


#ifndef __GPIO_H__
#define __GPIO_H__
 
#include "ls1c.h"


#define  GPIO_READ_MODE     (0)
#define  GPIO_WRITE_MODE   (1)
#define  GPIO_INT_MODE   	(2)

#define GPIO_TRIG_EDGE		1
#define GPIO_TRIG_LEVEL		0

#define GPIO_POSEDGE		1
#define GPIO_NEGEDGE		0

#define GPIO_HIGH_LEVEL    1
#define GPIO_LOW_LEVEL     0
typedef struct Gpio_Int_Config_t
{
	int trigmethod;
	int edge;
	int level;
	char *isrName;
	rt_isr_handler_t handler;
}gpio_int_config_t;
void rt_hw_gpio_init(void);



#endif




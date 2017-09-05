/*
 * File      : gpio.c
 * http://www.openloongson.org/forum.php
 * description:
 * for smart loong(ls1c)
 * Change Logs:
 * Date                 Author       Notes
 * 2015-07-20     chinesebear  first version
 */
#include <rthw.h>
#include <rtthread.h>
#include "ls1c.h"
#include "gpio.h"
#define GPIO_MAX 17
/*
smartloong v2.0
LED1=GPIO50
LED2=GPIO51
LED3=GPIO52
LED4=GPIO53
LED5=GPIO54

KEY1=GPIO92
KEY2=GPIO85
KEY3=GPIO86

*/
struct rt_device gpio[GPIO_MAX];

rt_uint32_t gpio_num[8] = {50,51,52,53,54,92,85,86};
char* gpio_string[8] = {"LED1","LED2","LED3","LED4","LED5",\
	"KEY1","KEY2","KEY3"};

void key1_irqhandler(int irqno, void *param)
{

}

void key2_irqhandler(int irqno, void *param)
{

}

void key3_irqhandler(int irqno, void *param)
{

}
rt_err_t gpio_ctrl_num(rt_device_t dev,  rt_uint32_t * base_addr, rt_uint32_t * ctlbit_pos)
{
	RT_ASSERT(dev != RT_NULL);
	rt_uint32_t * p_tmp = (rt_uint32_t *)dev->user_data;
	rt_uint32_t p_gpio_num = *p_tmp;
	#if 0
	if (!((p_gpio_num >=0 && p_gpio_num <= 5) \
		||(p_gpio_num >=48 && p_gpio_num <= 53)\
		||(p_gpio_num >=87 && p_gpio_num <= 91)))
	return RT_EIO;
	#endif
	if (p_gpio_num >=0 && p_gpio_num <= 31)
	{
		*base_addr = GPIO_BASE0;
		*ctlbit_pos  = p_gpio_num -0;
	}
	else if(p_gpio_num >=32 && p_gpio_num <= 63)
	{
		*base_addr = GPIO_BASE1;
		*ctlbit_pos  = p_gpio_num -32;
	}
	else if(p_gpio_num >=64 && p_gpio_num <= 95)
	{
		*base_addr = GPIO_BASE2;
		*ctlbit_pos  = p_gpio_num -64;
	}
	else if(p_gpio_num >=96 && p_gpio_num <= 127)
	{
		*base_addr = GPIO_BASE3;
		*ctlbit_pos  = p_gpio_num -96;
	}
	else
		return RT_EIO;
	return RT_EOK;
}

static rt_err_t gpio_int_set(rt_device_t dev, int trigmethod, int edge, int level,char *isrName,rt_isr_handler_t handler)
{
	RT_ASSERT(dev != RT_NULL);
	rt_uint32_t int_base;
	int setbit;
	rt_uint32_t * p_tmp = (rt_uint32_t *)dev->user_data;
	rt_uint32_t p_gpio_num = *p_tmp;
	if (p_gpio_num >=96 && p_gpio_num <= 105)
	{
		int_base = LS1C_INT1_BASE;
		setbit = p_gpio_num - 74;
	}
	else if(p_gpio_num >=0 && p_gpio_num <= 31)
	{
		int_base = LS1C_INT2_BASE;
		setbit = p_gpio_num;
	}
	else if(p_gpio_num >=32 && p_gpio_num <= 63)
	{
		int_base = LS1C_INT3_BASE;
		setbit = p_gpio_num - 32;
	}
	else if(p_gpio_num >=64 && p_gpio_num <= 95)
	{
		int_base = LS1C_INT4_BASE;
		setbit = p_gpio_num - 64;
	}
	else
		return RT_EIO;
	
	//int trigmethod
	switch(trigmethod)
	{
		case GPIO_TRIG_EDGE:
			LS1C_INT_EDGE(int_base) |= (1<<setbit);
			if(edge == GPIO_POSEDGE)
			{
				LS1C_INT_POL(int_base) |= (1<<setbit);
			}
			else if(edge == GPIO_NEGEDGE)
			{
				LS1C_INT_POL(int_base) &= ~(1<<setbit);
			}
			else
				rt_kprintf("GPIO int edge err\r\n");
			break;
		case GPIO_TRIG_LEVEL:
			LS1C_INT_EDGE(int_base) &= ~(1<<setbit);
			if(edge == GPIO_HIGH_LEVEL)
			{
				LS1C_INT_POL(int_base) |= (1<<setbit);
			}
			else if(edge == GPIO_LOW_LEVEL)
			{
				LS1C_INT_POL(int_base) &= ~(1<<setbit);
			}
			else
				rt_kprintf("GPIO int level err\r\n");
			break;
		default:
			rt_kprintf("GPIO trig method err\r\n");
			break;
	}
	rt_hw_interrupt_install(LS1C_GPIO_IRQ(p_gpio_num),handler,RT_NULL,isrName);
	LS1C_INT_EN(int_base) |= (1<<setbit);
	return RT_EOK;
}


static rt_err_t rt_gpio_init(rt_device_t dev)
{
	RT_ASSERT(dev != RT_NULL);
	
	
	return RT_EOK;
}
static rt_err_t rt_gpio_open(rt_device_t dev, rt_uint16_t oflag)
{
	RT_ASSERT(dev != RT_NULL);
	return RT_EOK;
}
static rt_err_t rt_gpio_close(rt_device_t dev)
{
	RT_ASSERT(dev != RT_NULL);

	return RT_EOK;
}
static rt_size_t rt_gpio_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
	RT_ASSERT(dev != RT_NULL);
	rt_uint32_t p_gpio_base = 0;
	rt_uint32_t p_ctlbit_pos =0;
	rt_uint32_t  *p_buf =(rt_uint32_t*) buffer ;
	gpio_ctrl_num(dev, &p_gpio_base,&p_ctlbit_pos);
	if (!(GPIO_EN(p_gpio_base)&(1<<p_ctlbit_pos)))
		rt_kprintf("GPIO is not read mode \n");
	*p_buf=(GPIO_IN(p_gpio_base)&(1<<p_ctlbit_pos))==0?0:1;
	
	return 1;
}
static rt_size_t rt_gpio_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
	RT_ASSERT(dev != RT_NULL);
	rt_uint32_t p_gpio_base = 0;
	rt_uint32_t p_ctlbit_pos =0;
	rt_uint32_t  *p_buf =(rt_uint32_t*) buffer ;
	gpio_ctrl_num(dev, &p_gpio_base,&p_ctlbit_pos);
	if ((GPIO_EN(p_gpio_base)&(1<<p_ctlbit_pos)))
		rt_kprintf("GPIO is not write mode \n");
	if( *p_buf  == 1)
		GPIO_OUT(p_gpio_base) |= (1<<p_ctlbit_pos);
	else if(*p_buf  == 0)
		GPIO_OUT(p_gpio_base) &= ~(1<<p_ctlbit_pos);
	else
		rt_kprintf("GPIO output value(%d) is wrong \n",*p_buf);
	return 1;
}



rt_err_t  rt_gpio_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
	RT_ASSERT(dev != RT_NULL);
	rt_uint32_t p_gpio_base = 0;
	rt_uint32_t p_ctlbit_pos =0;
	gpio_int_config_t *config;
	if(gpio_ctrl_num(dev, &p_gpio_base,&p_ctlbit_pos) == RT_EIO)
		return RT_EIO;
	
	switch(cmd)
	{
		case GPIO_READ_MODE :
			GPIO_EN(p_gpio_base) |= (1<<p_ctlbit_pos);//1= in
			break;
		case GPIO_WRITE_MODE :
			GPIO_EN(p_gpio_base) &= ~(1<<p_ctlbit_pos);//0=out
			break;
		case GPIO_INT_MODE :
			config =(gpio_int_config_t *) args;
			gpio_int_set(dev,config->trigmethod,config->edge,config->level,config->isrName,config->handler);
			break;
		default: break;
	}
	return RT_EOK;
}
void rt_hw_gpio_init(void)
{
	rt_uint32_t i;
	rt_device_t dev;
	for(i=0; i < 8; i++)
	{
		dev = &gpio[i];
		dev->type       = RT_Device_Class_Miscellaneous;
		dev->init         = rt_gpio_init;
		dev->open       = rt_gpio_open;
		dev->close       = rt_gpio_close;
		dev->read        = rt_gpio_read;
		dev->write       = rt_gpio_write;
		dev->control     = rt_gpio_control;
		dev->user_data  = &gpio_num[i];
		rt_device_register(&gpio[i], gpio_string[i],RT_DEVICE_FLAG_RDWR );
	}

}

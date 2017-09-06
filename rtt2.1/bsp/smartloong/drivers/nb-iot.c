/*
 * File      : nb-iot.c
 * http://www.openloongson.org/forum.php
 * description:
 * NB-IOT is the hotest iot proctocol. This driver is for huawei BC95 module.
 * for smart loong(ls1c)
 * Change Logs:
 * Date                 Author       Notes
 * 2017-09-06     chinesebear  first version
 */
#include <rthw.h>
#include <rtthread.h>
#include "ls1c.h"
#include "nb-iot.h"
static rt_device_t NB_IOT_UART = NULL;
static rt_err_t found_uart3(rt_device_t* NBdev)
{
	rt_device_t newdev;
	rt_err_t iRet = RT_EOK;
	newdev = rt_device_find("uart3");
	if(newdev == NULL)
	{
		rt_kprintf("find no uart3\r\n");
		return RT_EEMPTY;
	}
	iRet = rt_device_open(newdev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM);
	if(iRet)
	{
		rt_kprintf("open uart3 failure\r\n");
		return iRet;
	}
	return RT_EOK;
}
static rt_err_t AT_NBR(void)//reboot
{
	
}
static rt_err_t AT_NBAND(void)
{
	
}
static rt_err_t AT_NCONFIG(void)
{
	
}
static rt_err_t AT_CFUN(void)
{
	
}
static rt_err_t AT_CIMI(void)
{
	
}
static rt_err_t AT_CSQ(void)
{
	
}
static rt_err_t AT_NUESTATS(void)
{
	
}
static rt_err_t AT_CGATT(void)
{
	
}
static rt_err_t AT_CEREG(void)
{
	
}
static rt_err_t AT_CSCON(void)
{
	
}
static rt_err_t AT_NSOCR(void)//create socket
{
	
}
static rt_err_t AT_NSOST(void)//send msg
{
	
}
static rt_err_t AT_NSONMI(void)// recv msg interrupt
{
	
}
static rt_err_t AT_NSORF(void)//recv msg
{
	
}
static rt_err_t AT_NSCL(void)
{
	
}

static rt_err_t rt_nb_iot_init(rt_device_t dev)
{
	rt_err_t iRet = RT_EOK;
	RT_ASSERT(dev != RT_NULL);
	iRet = found_uart3(&NB_IOT_UART);
	if(iRet) return iRet;
	return RT_EOK;
}
static rt_err_t rt_nb_iot_open(rt_device_t dev, rt_uint16_t oflag)
{
	RT_ASSERT(dev != RT_NULL);
	return RT_EOK;
}
static rt_err_t rt_nb_iot_close(rt_device_t dev)
{
	RT_ASSERT(dev != RT_NULL);

	return RT_EOK;
}
static rt_size_t rt_nb_iot_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
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
static rt_size_t rt_nb_iot_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
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



rt_err_t  rt_nb_iot_control(rt_device_t dev, rt_uint8_t cmd, void *args)
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
void rt_hw_nb_iot_init(void)
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

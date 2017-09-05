/*
 * File      : pwm.c
 * http://www.openloongson.org/forum.php
 * description:
 * for smart loong(ls1c)
 * Change Logs:
 * Date                 Author       Notes
 * 2015-07-05     chinesebear  first version
 */
#include <rthw.h>
#include <rtthread.h>
#include "ls1c.h"
#include "pwm.h"

struct rt_pwm_ls1c
{
	struct rt_device parent;
	rt_uint32_t hw_base;
	rt_uint32_t irq;
	/*buffer*/
	rt_uint32_t userdata_buffer;
	
}pwm0_device,pwm1_device,pwm2_device,pwm3_device;

static void rt_pwm_irqhandler(int irqno, void *param)
{

}

static rt_err_t rt_pwm_init(rt_device_t dev)
{
	RT_ASSERT(dev != RT_NULL);
	rt_uint32_t * p_tmp = (rt_uint32_t *) dev->user_data;
	rt_uint32_t pwm_base = *p_tmp;
	PWM_CTRL(pwm_base) |= (1<<0);
	
	return RT_EOK;
}
static rt_err_t rt_pwm_open(rt_device_t dev, rt_uint16_t oflag)
{
	RT_ASSERT(dev != RT_NULL);
	rt_pwm_init(dev);
	return RT_EOK;
}
static rt_err_t rt_pwm_close(rt_device_t dev)
{
	RT_ASSERT(dev != RT_NULL);
	rt_uint32_t * p_tmp = (rt_uint32_t *) dev->user_data;
	rt_uint32_t pwm_base = *p_tmp;
	PWM_CTRL(pwm_base) = 0x1;
	return RT_EOK;
}
static rt_size_t rt_pwm_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{

}
static rt_size_t rt_pwm_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{

}
rt_err_t  rt_pwm_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
	RT_ASSERT(dev != RT_NULL);
	rt_uint32_t * p_tmp = (rt_uint32_t *)dev->user_data;
	rt_uint32_t pwm_base = *p_tmp;
	p_tmp = (rt_uint32_t *)args;
	rt_uint32_t pwm_value =  *p_tmp;
	
	switch(cmd)
	{
		case PWM_SET_CNTR :
			PWM_CNTR(pwm_base) = (pwm_value & 0xFFFFFF);
			break;
		case PWM_SET_HRC :
			PWM_HRC(pwm_base) = (pwm_value & 0xFFFFFF);
			break;
		case PWM_SET_LRC :
			PWM_LRC(pwm_base) = (pwm_value & 0xFFFFFF);
			break;
		case PWM_SINGLE_EN ://one time output
			PWM_CTRL(pwm_base) |= (1<<4);
			break;
		case PWM_SINGLE_DISEN ://continuous output
			PWM_CTRL(pwm_base) &= 0xFFFFEF;
			break;
		case PWM_OUTPUT_EN ://pulse output
			PWM_CTRL(pwm_base) &= 0xFFFFF7;
			break;
		case PWM_OUTPUT_DISEN ://no pluse 
			PWM_CTRL(pwm_base) |= (1<<3);
		case PWM_CNT_EN :
			PWM_CTRL(pwm_base) |= (1<<0);
			break;
		case PWM_CNT_DISEN :
			PWM_CTRL(pwm_base) &= 0xFFFFFE;
			break;
		default: break;
	}
	return RT_EOK;
}
void rt_hw_pwm_init(void)
{

	struct rt_pwm_ls1c  * pwm;
	
	pwm = &pwm0_device;
	pwm->parent.type = RT_Device_Class_Char;
	pwm->hw_base = PWM0_BASE;
	pwm->userdata_buffer = PWM0_BASE;
	pwm->irq=LS1C_PWM0_IRQ;
		/* device interface */
	pwm->parent.init       =   rt_pwm_init;
	pwm->parent.open       = rt_pwm_open;
	pwm->parent.close      = rt_pwm_close;
	pwm->parent.read       = RT_NULL;
	pwm->parent.write      = RT_NULL;
	pwm->parent.control    = rt_pwm_control;
	pwm->parent.user_data  = &pwm->userdata_buffer;
	rt_device_register(&pwm->parent, "pwm0",RT_DEVICE_FLAG_RDWR );

	pwm = &pwm1_device;
	pwm->parent.type = RT_Device_Class_Char;
	pwm->hw_base = PWM1_BASE;
	pwm->userdata_buffer = PWM1_BASE;
	pwm->irq=LS1C_PWM1_IRQ;
		/* device interface */
	pwm->parent.init       = rt_pwm_init;
	pwm->parent.open       = rt_pwm_open;
	pwm->parent.close      = rt_pwm_close;
	pwm->parent.read       = RT_NULL;
	pwm->parent.write      = RT_NULL;
	pwm->parent.control    = rt_pwm_control;
	pwm->parent.user_data  = &pwm->userdata_buffer;
	rt_device_register(&pwm->parent, "pwm1",RT_DEVICE_FLAG_RDWR );

	pwm = &pwm2_device;
	pwm->parent.type = RT_Device_Class_Char;
	pwm->hw_base = PWM2_BASE;
	pwm->userdata_buffer = PWM2_BASE;
	pwm->irq=LS1C_PWM2_IRQ;
		/* device interface */
	pwm->parent.init       = rt_pwm_init;
	pwm->parent.open       = rt_pwm_open;
	pwm->parent.close      = rt_pwm_close;
	pwm->parent.read       = RT_NULL;
	pwm->parent.write      = RT_NULL;
	pwm->parent.control    = rt_pwm_control;
	pwm->parent.user_data  = &pwm->userdata_buffer;
	rt_device_register(&pwm->parent, "pwm2",RT_DEVICE_FLAG_RDWR );

	pwm = &pwm3_device;
	pwm->parent.type = RT_Device_Class_Char;
	pwm->hw_base = PWM3_BASE;
	pwm->userdata_buffer = PWM3_BASE;
	pwm->irq=LS1C_PWM3_IRQ;
		/* device interface */
	pwm->parent.init       = rt_pwm_init;
	pwm->parent.open       = rt_pwm_open;
	pwm->parent.close      = rt_pwm_close;
	pwm->parent.read       = RT_NULL;
	pwm->parent.write      = RT_NULL;
	pwm->parent.control    = rt_pwm_control;
	pwm->parent.user_data  = &pwm->userdata_buffer;
	rt_device_register(&pwm->parent, "pwm3",RT_DEVICE_FLAG_RDWR );
	
	
}


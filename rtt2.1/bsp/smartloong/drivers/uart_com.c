/*
 * File      : uart_com.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006-2012, RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date               Author               Notes
 * 2011-08-08     lgnq                  first version
 * 2015-07-24     chinesebear       uart common driver
 */

#include <rthw.h>
#include <rtthread.h>
#include "ls1c.h"
#include "uart.h"
#include "finsh.h"

/**
 * @addtogroup Loongson LS1B
 */

/*@{*/

#if defined(RT_USING_UART) && defined(RT_USING_DEVICE)

#define MAX_UART_COM 12//12 uarts in loongson 1c

rt_uint32_t uart_base[12]={UART0_BASE, UART1_BASE, UART2_BASE,UART3_BASE,\
UART4_BASE,UART5_BASE, UART6_BASE, UART7_BASE, UART8_BASE, UART9_BASE,\
UART10_BASE, UART11_BASE};
rt_uint32_t uart_irq[12] = {LS1C_UART0_IRQ, LS1C_UART1_IRQ, LS1C_UART2_IRQ, LS1C_UART3_IRQ,\
	LS1C_UART4_IRQ, LS1C_UART5_IRQ, LS1C_UART6_IRQ, LS1C_UART7_IRQ, LS1C_UART8_IRQ,\
	LS1C_UART9_IRQ, LS1C_UART10_IRQ, LS1C_UART11_IRQ};
rt_uint32_t uart_no[12] ={0,1,2,3,4,5,6,7,8,9,10,11};
char * uart_name[12]={"uart0","uart1","uart2","uart3","uart4","uart5",\
	"uart6","uart7","uart8","uart9","uart10","uart11"};

struct rt_uart_com_ls1c
{
	struct rt_device parent;

	rt_uint32_t hw_base;
	rt_uint32_t irq;

	/* buffer for reception */
	rt_uint8_t read_index, save_index;
	rt_uint8_t rx_buffer[RT_UART_RX_BUFFER_SIZE];
}uart_com_device[12];

static void rt_uart_com_irqhandler(int irqno, void *param)
{
	rt_ubase_t level;
	rt_uint8_t isr;
	struct rt_uart_com_ls1c *uart = RT_NULL;
	
	if(irqno == 2)
		uart  = &uart_com_device[0];
	else if(irqno== 4|| irqno == 5)
		uart  = &uart_com_device[irqno-3];
	else if(irqno== 29)
		uart  = &uart_com_device[3];
	else if(irqno >= 37 && irqno <= 41)
		uart  = &uart_com_device[irqno-33];
	else if(irqno >=45 && irqno <= 47)
		uart  = &uart_com_device[irqno-36];
	else
		return;
	/* read interrupt status and clear it */
	isr = UART_IIR(uart->hw_base);
	isr = (isr >> 1) & 0x3;

	/* receive data available */
	if (isr & 0x02)
	{
		/* Receive Data Available */
		while (UART_LSR(uart->hw_base) & UARTLSR_DR)
		{
			uart->rx_buffer[uart->save_index] = UART_DAT(uart->hw_base);

			level = rt_hw_interrupt_disable();
			uart->save_index ++;
			if (uart->save_index >= RT_UART_RX_BUFFER_SIZE)
				uart->save_index = 0;
			rt_hw_interrupt_enable(level);
		}

		/* invoke callback */
		if (uart->parent.rx_indicate != RT_NULL)
		{
			rt_size_t length;
			if (uart->read_index > uart->save_index)
				length = RT_UART_RX_BUFFER_SIZE - uart->read_index + uart->save_index;
			else
				length = uart->save_index - uart->read_index;

			uart->parent.rx_indicate(&uart->parent, length);
		}
	}

	return;
}

static rt_err_t rt_uart_com_init(rt_device_t dev)
{
	rt_uint32_t baud_div;
	struct rt_uart_com_ls1c *uart = (struct rt_uart_com_ls1c *)dev;

	RT_ASSERT(uart != RT_NULL);

//#if 0
	/* init UART Hardware */
	UART_IER(uart->hw_base) = 0; /* clear interrupt */
	UART_FCR(uart->hw_base) = 0xc3; /* reset UART Rx/Tx */

	/* enable UART clock */
	/* set databits, stopbits and parity. (8-bit data, 1 stopbit, no parity) */
	UART_LCR(uart->hw_base) = 0x80;

	/* set baudrate */
	baud_div = DEV_CLK / 16 / UART_BAUDRATE /2;

	UART_MSB(uart->hw_base) = (baud_div >> 8) & 0xff;
	UART_LSB(uart->hw_base) = baud_div & 0xff;

	UART_LCR(uart->hw_base) = 0x03;

	/* Enable UART unit, enable and clear FIFO */
	//UART_FCR(uart->hw_base) = UARTFCR_UUE | UARTFCR_FE | UARTFCR_TFLS | UARTFCR_RFLS;
	UART_FCR(uart->hw_base) |= 0x6;//clear fifo
	/* enable RX interrupt  */	
	UART_IER(uart->hw_base) = UARTIER_IRXE|UARTIER_ILE; 
	
//#endif

	return RT_EOK;
}

static rt_err_t rt_uart_com_open(rt_device_t dev, rt_uint16_t oflag)
{
	struct rt_uart_com_ls1c *uart = (struct rt_uart_com_ls1c *)dev;

	RT_ASSERT(uart != RT_NULL);
	if (dev->flag & RT_DEVICE_FLAG_INT_RX)
	{
		/* Enable the UART Interrupt */
		UART_IER(uart->hw_base) |= UARTIER_IRXE;

		/* install interrupt */
		rt_hw_interrupt_install(uart->irq, rt_uart_com_irqhandler, RT_NULL, "UART");
		rt_hw_interrupt_umask(uart->irq);
	}
	return RT_EOK;
}

static rt_err_t rt_uart_com_close(rt_device_t dev)
{
	struct rt_uart_com_ls1c *uart = (struct rt_uart_com_ls1c *)dev;

	RT_ASSERT(uart != RT_NULL);
	if (dev->flag & RT_DEVICE_FLAG_INT_RX)
	{
		/* Disable the UART Interrupt */
		UART_IER(uart->hw_base) &= ~(UARTIER_IRXE);
	}

	return RT_EOK;
}

static rt_size_t rt_uart_com_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
	rt_uint8_t *ptr;
	struct rt_uart_com_ls1c *uart = (struct rt_uart_com_ls1c *)dev;

	RT_ASSERT(uart != RT_NULL);

	/* point to buffer */
	ptr = (rt_uint8_t *)buffer;
	if (dev->flag & RT_DEVICE_FLAG_INT_RX)
	{
		while (size)
		{
			/* interrupt receive */
			rt_base_t level;

			/* disable interrupt */
			level = rt_hw_interrupt_disable();
			if (uart->read_index != uart->save_index)
			{
				*ptr = uart->rx_buffer[uart->read_index];

				uart->read_index ++;
				if (uart->read_index >= RT_UART_RX_BUFFER_SIZE)
					uart->read_index = 0;
			}
			else
			{
				/* no data in rx buffer */

				/* enable interrupt */
				rt_hw_interrupt_enable(level);
				break;
			}
			/* enable interrupt */
			rt_hw_interrupt_enable(level);
			
			ptr ++;
			size --;
		}

		return (rt_uint32_t)ptr - (rt_uint32_t)buffer;
	}

	return 0;
}

static rt_size_t rt_uart_com_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
	char *ptr;
	struct rt_uart_com_ls1c *uart = (struct rt_uart_com_ls1c *)dev;

	RT_ASSERT(uart != RT_NULL);

	ptr = (char *)buffer;

	if (dev->flag & RT_DEVICE_FLAG_STREAM)
	{
		/* stream mode */
		while (size)
		{
			#if 0
			if (*ptr == '\n')
			{
				/* FIFO status, contain valid data */
				while (!(UART_LSR(uart->hw_base) & (UARTLSR_TE | UARTLSR_TFE)));
				/* write data */
				UART_DAT(uart->hw_base) = '\r';
			}
			#endif
			/* FIFO status, contain valid data */
			while (!(UART_LSR(uart->hw_base) & (UARTLSR_TE | UARTLSR_TFE)));
			/* write data */
			UART_DAT(uart->hw_base) = *ptr;

			ptr ++;
			size --;
		}
	}
	else
	{
		while (size != 0)
		{
			/* FIFO status, contain valid data */
			while (!(UART_LSR(uart->hw_base) & (UARTLSR_TE | UARTLSR_TFE)));

			/* write data */
			UART_DAT(uart->hw_base) = *ptr;

			ptr++;
			size--;
		}
	}

	return (rt_size_t)ptr - (rt_size_t)buffer;
}
rt_err_t  rt_uart_com_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
	rt_uint32_t* p_arg = (rt_uint32_t*) args;
	
	switch(cmd)
	{
		case UART_SET_BAUD:
			break;
		default:
			break;
	}
}
rt_err_t rt_uart_show(rt_uint32_t num)
{
	rt_uint32_t base = uart_base[num];
	rt_kprintf("%s registers show:\n",uart_name[num]);
	rt_kprintf("DAT:0x%02X,        FIFO data reg\n",UART_DAT(base));
	rt_kprintf("IIR:0x%02X,        interrupt indication reg\n",UART_IIR(base));
	rt_kprintf("LCR:0x%02X,        line control reg\n",UART_LCR(base));
	rt_kprintf("LSR:0x%02X,        line status reg\n",UART_LSR(base));
	rt_kprintf("MSR:0x%02X,        modem status reg\n",UART_MSR(base));
	rt_kprintf("LSB:0x%02X,        freq div reg 7~0\n",UART_LSB(base));
	rt_kprintf("MSB:0x%02X,        freq div reg 15~8\n",UART_MSB(base));
}
void rt_hw_uart_com_init(void)
{
	struct rt_uart_com_ls1c *uart;
	rt_uint32_t i;
	for(i=0 ; i < MAX_UART_COM; i++)
	{
		if(i==0 ||i ==2)  continue;

		if( i >3) break;

		/* get uart device */
		uart = &uart_com_device[i];

		/* device initialization */
		uart->parent.type = RT_Device_Class_Char;
		rt_memset(uart->rx_buffer, 0, sizeof(uart->rx_buffer));
		uart->read_index = uart->save_index = 0;
		uart->hw_base = uart_base[i];
		uart->irq = uart_irq[i];

		/* device interface */
		uart->parent.init       = rt_uart_com_init;
		uart->parent.open       = rt_uart_com_open;
		uart->parent.close      = rt_uart_com_close;
		uart->parent.read       = rt_uart_com_read;
		uart->parent.write      = rt_uart_com_write;
		uart->parent.control    = rt_uart_com_control;
		uart->parent.user_data  = &uart_no[i];

		rt_device_register(&uart->parent, uart_name[i],
							RT_DEVICE_FLAG_RDWR |
							RT_DEVICE_FLAG_STREAM |
							RT_DEVICE_FLAG_INT_RX);
	}
}
#endif /* end of UART */



int show_uart(int num)
{	
	rt_uart_show(num);	
	return RT_EOK;
}

//FINSH_FUNCTION_EXPORT(show_uart,uart registers show(num)0to4);




/*@}*/




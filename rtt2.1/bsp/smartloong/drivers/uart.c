/*
 * File      : board.c
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
 * 2015-07-09     chinesebear       modified for loongson 1c
 */

#include <rthw.h>
#include <rtthread.h>
#include "ls1c.h"
#include "uart.h"

/**
 * @addtogroup Loongson LS1B
 */

/*@{*/

#if defined(RT_USING_UART) && defined(RT_USING_DEVICE)

struct rt_uart_ls1c
{
	struct rt_device parent;

	rt_uint32_t hw_base;
	rt_uint32_t irq;

	/* buffer for reception */
	rt_uint8_t read_index, save_index;
	rt_uint8_t rx_buffer[RT_UART_RX_BUFFER_SIZE];
}uart1_device,uart2_device;

rt_device_t uart_dev[12];

static void uart_rx_process(struct rt_uart_ls1c *uart)
{
	rt_ubase_t level;
	rt_uint8_t isr;
	
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
static rt_int32_t uart_get_dev_numnber(const char * name)
{
	int num[2];
	int uartnum;
	if(rt_memcmp(name,"uart",4))return 0xFF;
	num[0] = *(name+4) - '0';
	
	if(*(name+5) == '\0')
	{
		uartnum = num[0];
		return uartnum;
	}
	num[1] = *(name+5)- '0';
	uartnum = num[0]*10 + num[1];
	if(uartnum >UART_NUM_MAX) return 0xFF;
	return uartnum;
}
static void rt_uart1_irqhandler(int irqno, void *param)
{
	//rt_kprintf("rx int\r\n");
	uart_rx_process(&uart1_device);
	return;
}
static void rt_uart2_irqhandler(int irqno, void *param)
{
	uart_rx_process(&uart2_device);
	return;
}

static rt_err_t rt_uart_init(rt_device_t dev)
{
	rt_uint32_t baud_div;
	struct rt_uart_ls1c *uart = (struct rt_uart_ls1c *)dev;

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

static rt_err_t rt_uart_open(rt_device_t dev, rt_uint16_t oflag)
{
	struct rt_uart_ls1c *uart = (struct rt_uart_ls1c *)dev;

	RT_ASSERT(uart != RT_NULL);
	if (dev->flag & RT_DEVICE_FLAG_INT_RX)
	{
		/* Enable the UART Interrupt */
		UART_IER(uart->hw_base) |= UARTIER_IRXE;

		/* install interrupt */
		if(uart->irq == LS1C_UART1_IRQ)
			rt_hw_interrupt_install(uart->irq, rt_uart1_irqhandler, RT_NULL, "UART1");
		else if(uart->irq == LS1C_UART2_IRQ)
			rt_hw_interrupt_install(uart->irq, rt_uart2_irqhandler, RT_NULL, "UART2");
		
		rt_hw_interrupt_umask(uart->irq);
	}
	return RT_EOK;
}

static rt_err_t rt_uart_close(rt_device_t dev)
{
	struct rt_uart_ls1c *uart = (struct rt_uart_ls1c *)dev;

	RT_ASSERT(uart != RT_NULL);
	if (dev->flag & RT_DEVICE_FLAG_INT_RX)
	{
		/* Disable the UART Interrupt */
		UART_IER(uart->hw_base) &= ~(UARTIER_IRXE);
	}

	return RT_EOK;
}

static rt_size_t rt_uart_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
	rt_uint8_t *ptr;
	struct rt_uart_ls1c *uart = (struct rt_uart_ls1c *)dev;

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

static rt_size_t rt_uart_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
	char *ptr;
	struct rt_uart_ls1c *uart = (struct rt_uart_ls1c *)dev;

	RT_ASSERT(uart != RT_NULL);

	ptr = (char *)buffer;

	if (dev->flag & RT_DEVICE_FLAG_STREAM)
	{
		/* stream mode */
		while (size)
		{
			if (*ptr == '\n')
			{
				/* FIFO status, contain valid data */
				while (!(UART_LSR(uart->hw_base) & (UARTLSR_TE | UARTLSR_TFE)));
				/* write data */
				UART_DAT(uart->hw_base) = '\r';
			}

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

void rt_hw_uart_init(void)
{
	struct rt_uart_ls1c *uart;
	#if 0
	/* get uart device */
	uart = &uart1_device;

	/* device initialization */
	uart->parent.type = RT_Device_Class_Char;
	rt_memset(uart->rx_buffer, 0, sizeof(uart->rx_buffer));
	uart->read_index = uart->save_index = 0;
	uart->hw_base = UART1_BASE;
	uart->irq = LS1C_UART1_IRQ;

	/* device interface */
	uart->parent.init       = rt_uart_init;
	uart->parent.open       = rt_uart_open;
	uart->parent.close      = rt_uart_close;
	uart->parent.read       = rt_uart_read;
	uart->parent.write      = rt_uart_write;
	uart->parent.control    = RT_NULL;
	uart->parent.user_data  = RT_NULL;

	rt_device_register(&uart->parent, "uart1",
						RT_DEVICE_FLAG_RDWR |
						RT_DEVICE_FLAG_STREAM |
						RT_DEVICE_FLAG_INT_RX);
	#endif

	/* get uart device */
	uart = &uart2_device;

	/* device initialization */
	uart->parent.type = RT_Device_Class_Char;
	rt_memset(uart->rx_buffer, 0, sizeof(uart->rx_buffer));
	uart->read_index = uart->save_index = 0;
	uart->hw_base = UART2_BASE;
	uart->irq = LS1C_UART2_IRQ;

	/* device interface */
	uart->parent.init       = rt_uart_init;
	uart->parent.open       = rt_uart_open;
	uart->parent.close      = rt_uart_close;
	uart->parent.read       = rt_uart_read;
	uart->parent.write      = rt_uart_write;
	uart->parent.control    = RT_NULL;
	uart->parent.user_data  = RT_NULL;

	rt_device_register(&uart->parent, "uart2",
						RT_DEVICE_FLAG_RDWR |
						RT_DEVICE_FLAG_STREAM |
						RT_DEVICE_FLAG_INT_RX);
}

void rt_hw_uart_set_device(const char* name)
{
	rt_device_t newdev;
	rt_int32_t devnum;
	devnum = uart_get_dev_numnber(name);
	if(devnum > UART_NUM_MAX) return ;
    /* find new console device */
    newdev = rt_device_find(name);
    if (newdev != RT_NULL)
    {
	     /* set new device */
        rt_device_open(newdev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM);
        uart_dev[devnum] = newdev;
    }
    return;
}


#endif /* end of UART */



/*@}*/

/*
 * File      : ls1c.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006-2011, RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date               Author             Notes
 * 2011-08-08     lgnq                first version
 * 2015-07-06     chinesebear      modified for loongson 1c
 */

#ifndef __LS1C_H__
#define __LS1C_H__

#include "../common/mipsregs.h"

/*
*There are 5 groups of interrupt registers in loonson 1C300
* and 6 registers every group (state, enable, setting,clear, electronic level(pole), edge) 
*/
#define LS1C_INT0_BASE    0xBFD01040
#define LS1C_INT1_BASE    0xBFD01058
#define LS1C_INT2_BASE    0xBFD01070
#define LS1C_INT3_BASE    0xBFD01088
#define LS1C_INT4_BASE    0xBFD010A0

#define LS1C_INT_SR(base)     __REG32(base) //specail register 0
#define LS1C_INT_EN(base)     __REG32(base+0x4)
#define LS1C_INT_SET(base)   __REG32(base+0x8)
#define LS1C_INT_CLR(base)   __REG32(base+0xC)
#define LS1C_INT_POL(base)     __REG32(base+0x10)
#define LS1C_INT_EDGE(base)   __REG32(base+0x14)

#define LS1C_ACPI_IRQ	0
#define LS1C_HPET_IRQ	1
#define LS1C_UART0_IRQ	2
//Null 3
#define LS1C_UART1_IRQ	4
#define LS1C_UART2_IRQ	5
#define LS1C_CAN0_IRQ	6
#define LS1C_CAN1_IRQ	7
#define LS1C_SPI0_IRQ	8
#define LS1C_SPI1_IRQ	9
#define LS1C_AC97_IRQ	10
//Reserved 11~12
#define LS1C_DMA0_IRQ	13
#define LS1C_DMA1_IRQ	14
#define LS1C_DMA2_IRQ	15
#define LS1C_NAND_IRQ	16
#define LS1C_PWM0_IRQ	17
#define LS1C_PWM1_IRQ	18
#define LS1C_PWM2_IRQ	19
#define LS1C_PWM3_IRQ	20
#define LS1C_RTC0_IRQ	21
#define LS1C_RTC1_IRQ	22
#define LS1C_RTC2_IRQ   23
//Reserved 24~28
#define LS1C_UART3_IRQ  29
#define LS1C_ADC_IRQ     30
#define LS1C_SDIO_IRQ    31
#define LS1C_EHCI_IRQ	32
#define LS1C_OHCI_IRQ	33
#define LS1C_OTG_IRQ	34
#define LS1C_MAC_IRQ	35
#define LS1C_CAM_IRQ	36
#define LS1C_UART4_IRQ	37
#define LS1C_UART5_IRQ 38
#define LS1C_UART6_IRQ 39
#define LS1C_UART7_IRQ 40
#define LS1C_UART8_IRQ 41
//Null 42~44
#define LS1C_UART9_IRQ 45
#define LS1C_UART10_IRQ 46
#define LS1C_UART11_IRQ 47
//Reserved 48
#define LS1C_I2C2_IRQ 49
#define LS1C_I2C1_IRQ 50
#define LS1C_I2C0_IRQ 51
//Reserved 52~53
//GPIO irq: 54~159
//GPIO95~0,irq= gpio + 64 ,GPIO105~96, irq=gpio-42
#define LS1C_GPIO_IRQ(gpio) (gpio>=96 && gpio <= 105)?(gpio-42):(gpio+64)


#define LS1C_GPIO_FIRST_IRQ 64
#define LS1C_GPIO_IRQ_COUNT 96
#define LS1C_GPIO_LAST_IRQ  (LS1C_GPIO_FIRST_IRQ + LS1C_GPIO_IRQ_COUNT-1)

#define INT_PCI_INTA	(1<<6)
#define INT_PCI_INTB	(1<<7)
#define INT_PCI_INTC	(1<<8)
#define INT_PCI_INTD	(1<<9)

#define LS1C_LAST_IRQ 159
#define MIPS_CPU_TIMER_IRQ	167
//#define LS1C_INTREG_BASE 0xbfd01040

#define LS1C_DMA_IRQ_BASE 168
#define LS1C_DMA_IRQ_COUNT 16

struct ls1c_intc_regs
{
	volatile unsigned int int_isr;
	volatile unsigned int int_en;
	volatile unsigned int int_set;
	volatile unsigned int int_clr;		/* offset 0x10*/
	volatile unsigned int int_pol;
   	volatile unsigned int int_edge;		/* offset 0 */
}; 

struct ls1c_cop_global_regs
{
	volatile unsigned int control;
	volatile unsigned int rd_inten;
	volatile unsigned int wr_inten;
	volatile unsigned int rd_intisr;		/* offset 0x10*/
	volatile unsigned int wr_intisr;
	unsigned int unused[11];
} ; 

struct ls1c_cop_channel_regs
{
	volatile unsigned int rd_control;
	volatile unsigned int rd_src;
	volatile unsigned int rd_cnt;
	volatile unsigned int rd_status;		/* offset 0x10*/
	volatile unsigned int wr_control;
	volatile unsigned int wr_src;
	volatile unsigned int wr_cnt;
	volatile unsigned int wr_status;		/* offset 0x10*/
} ; 

struct ls1c_cop_regs
{
	struct ls1c_cop_global_regs global;
	struct ls1c_cop_channel_regs chan[8][2];
} ;

#define __REG8(addr)		*((volatile unsigned char *)(addr))
#define __REG16(addr)		*((volatile unsigned short *)(addr))
#define __REG32(addr)		*((volatile unsigned int *)(addr))

#define GMAC0_BASE			0xBFE10000
#define GMAC0_DMA_BASE		0xBFE11000
#define GMAC1_BASE			0xBFE20000
#define GMAC1_DMA_BASE		0xBFE21000
#define I2C0_BASE			0xBFE58000
#define WDT_BASE			0xBFE5C060
#define RTC_BASE			0xBFE64000
#define I2C1_BASE			0xBFE68000
#define I2C2_BASE			0xBFE70000
#define AC97_BASE			0xBFE74000
#define NAND_BASE			0xBFE78000
#define SPI_BASE			0xBFE80000
#define CAN1_BASE			0xBF004300
#define CAN0_BASE			0xBF004400

/* Watch Dog registers */
#define WDT_EN				__REG32(WDT_BASE + 0x00)
#define WDT_SET				__REG32(WDT_BASE + 0x04)
#define WDT_TIMER			__REG32(WDT_BASE + 0x08)

#define PLL_FREQ 				__REG32(0xbfe78030)
#define PLL_DIV_PARAM 			__REG32(0xbfe78034)


/* GPIO mutiplex*/
#define GPIO_BASE0                  0xBFD010C0  //31~0
#define GPIO_BASE1                  0xBFD010C4  //63~32
#define GPIO_BASE2                  0xBFD010C8  //95~64
#define GPIO_BASE3                  0xBFD010CC  //127~96

#define GPIO_CFG(base)            __REG32(base)
#define GPIO_EN(base)              __REG32(base + 0x10)
#define GPIO_IN(base)               __REG32(base + 0x20)
#define GPIO_OUT(base)            __REG32(base + 0x30)

/*pads mutiplex*/
#define PAD_MUTI_BASE0           0XBFD011C0 //31~0
#define PAD_MUTI_BASE1           0XBFD011C4 //63~32
#define PAD_MUTI_BASE2           0XBFD011C8 //95~64
#define PAD_MUTI_BASE3           0XBFD011CC //127~96

#define CBUS_FIRST(base)            __REG32(base)
#define CBUS_SECOND(base)         __REG32(base+0x10)
#define CBUS_THIRD(base)           __REG32(base+0x20)
#define CBUS_FOURTH(base)         __REG32(base+0x30)
#define CBUS_FIFTH(base)             __REG32(base+0x40)







#endif


/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006-2012, RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date                Author         Notes
 * 2010-06-25          Bernard        first version
 * 2011-08-08          lgnq           modified for Loongson LS1B
 * 2015-07-06          chinesebear    modified for Loongson LS1C
 * 2017-07-26	chinesebear	NB-IOT communication
 */

#include <rtthread.h>
#include <components.h>
#include "uart.h"
#include "gpio.h"


#define AT_DEBUG					0
#define SEND_TO_NBIOT				0
#define RECV_FROM_NBIOT				1

#define  atCmdGapTime  		100 //tick
#define  atUart3RxTimeOut   400 //tick

#define atNbiotCreateSocket_type				"DGRAM"
#define atNbiotCreateSocket_protocol			"17"
#define atNbiotCreateSocket_listen_port			"5683"
#define atNbiotCreateSocket_receive_control		"1"

#define atNbiotSendMessage_ip		"118.190.93.84"
#define atNbiotSendMessage_port		"2317"

#define atNbiotTxBufferSize				256
#define atNbiotRxBufferSize				512

#define AT_OK					0
#define AT_FAIL					(-1)

#define AT_RESPNOSE_OK			AT_OK
#define AT_RESPNOSE_ERROR		AT_FAIL


enum AT_NBIOT_UDP_STATE{
	AT_AUTO_CONN_CFG,
	AT_CREATE_SOCKET,
	AT_SEND_MSG,
	AT_RECV_MSG,
	AT_READ_MSG,
	AT_CLOSE_SOCKET,
	AT_IDLE
};
enum AT_NBIOT_CoAP_STATE{
	AT_QUERY_IEMI,
	AT_CFG_CDP_SERVER,
	AT_QUERY_CDP_SERVER,
	AT_ENABLE_SEND_INDICATE,
	AT_ENABLE_NEW_INDICATE,
	AT_SEND_CoAP_MSG,
	AT_QUERY_RECV,
	AT_GET_MSG,
	AT_QUERY_RECV2
};
typedef struct socket_message{
	int socketid;
	int msglen;
}socket_msg;
int atstate = AT_AUTO_CONN_CFG;
int link_NBIOT = SEND_TO_NBIOT;
int socketID;
rt_uint8_t pt_rx_buffer[atNbiotRxBufferSize];
rt_uint8_t pt_tx_buffer[atNbiotTxBufferSize];
int pt_tx_size = 0;
int pt_rx_size = 0;
socket_msg smsg;

static const rt_uint16_t crctab16[] =
{
0X0000, 0X1189, 0X2312, 0X329B, 0X4624, 0X57AD, 0X6536, 0X74BF,
0X8C48, 0X9DC1, 0XAF5A, 0XBED3, 0XCA6C, 0XDBE5, 0XE97E, 0XF8F7,
0X1081, 0X0108, 0X3393, 0X221A, 0X56A5, 0X472C, 0X75B7, 0X643E,
0X9CC9, 0X8D40, 0XBFDB, 0XAE52, 0XDAED, 0XCB64, 0XF9FF, 0XE876,
0X2102, 0X308B, 0X0210, 0X1399, 0X6726, 0X76AF, 0X4434, 0X55BD,
0XAD4A, 0XBCC3, 0X8E58, 0X9FD1, 0XEB6E, 0XFAE7, 0XC87C, 0XD9F5,
0X3183, 0X200A, 0X1291, 0X0318, 0X77A7, 0X662E, 0X54B5, 0X453C,
0XBDCB, 0XAC42, 0X9ED9, 0X8F50, 0XFBEF, 0XEA66, 0XD8FD, 0XC974,
0X4204, 0X538D, 0X6116, 0X709F, 0X0420, 0X15A9, 0X2732, 0X36BB,
0XCE4C, 0XDFC5, 0XED5E, 0XFCD7, 0X8868, 0X99E1, 0XAB7A, 0XBAF3,
0X5285, 0X430C, 0X7197, 0X601E, 0X14A1, 0X0528, 0X37B3, 0X263A,
0XDECD, 0XCF44, 0XFDDF, 0XEC56, 0X98E9, 0X8960, 0XBBFB, 0XAA72,
0X6306, 0X728F, 0X4014, 0X519D, 0X2522, 0X34AB, 0X0630, 0X17B9,
0XEF4E, 0XFEC7, 0XCC5C, 0XDDD5, 0XA96A, 0XB8E3, 0X8A78, 0X9BF1,
0X7387, 0X620E, 0X5095, 0X411C, 0X35A3, 0X242A, 0X16B1, 0X0738,
0XFFCF, 0XEE46, 0XDCDD, 0XCD54, 0XB9EB, 0XA862, 0X9AF9, 0X8B70,
0X8408, 0X9581, 0XA71A, 0XB693, 0XC22C, 0XD3A5, 0XE13E, 0XF0B7,
0X0840, 0X19C9, 0X2B52, 0X3ADB, 0X4E64, 0X5FED, 0X6D76, 0X7CFF,
0X9489, 0X8500, 0XB79B, 0XA612, 0XD2AD, 0XC324, 0XF1BF, 0XE036,
0X18C1, 0X0948, 0X3BD3, 0X2A5A, 0X5EE5, 0X4F6C, 0X7DF7, 0X6C7E,
0XA50A, 0XB483, 0X8618, 0X9791, 0XE32E, 0XF2A7, 0XC03C, 0XD1B5,
0X2942, 0X38CB, 0X0A50, 0X1BD9, 0X6F66, 0X7EEF, 0X4C74, 0X5DFD,
0XB58B, 0XA402, 0X9699, 0X8710, 0XF3AF, 0XE226, 0XD0BD, 0XC134,
0X39C3, 0X284A, 0X1AD1, 0X0B58, 0X7FE7, 0X6E6E, 0X5CF5, 0X4D7C,
0XC60C, 0XD785, 0XE51E, 0XF497, 0X8028, 0X91A1, 0XA33A, 0XB2B3,
0X4A44, 0X5BCD, 0X6956, 0X78DF, 0X0C60, 0X1DE9, 0X2F72, 0X3EFB,
0XD68D, 0XC704, 0XF59F, 0XE416, 0X90A9, 0X8120, 0XB3BB, 0XA232,
0X5AC5, 0X4B4C, 0X79D7, 0X685E, 0X1CE1, 0X0D68, 0X3FF3, 0X2E7A,
0XE70E, 0XF687, 0XC41C, 0XD595, 0XA12A, 0XB0A3, 0X8238, 0X93B1,
0X6B46, 0X7ACF, 0X4854, 0X59DD, 0X2D62, 0X3CEB, 0X0E70, 0X1FF9,
0XF78F, 0XE606, 0XD49D, 0XC514, 0XB1AB, 0XA022, 0X92B9, 0X8330,
0X7BC7, 0X6A4E, 0X58D5, 0X495C, 0X3DE3, 0X2C6A, 0X1EF1, 0X0F78,
};
extern rt_device_t uart_dev[];




static void DumpData(const rt_uint8_t *pcStr,rt_uint8_t *pucBuf,rt_uint32_t usLen)
{

    rt_uint32_t i;
    rt_uint8_t acTmp[17];
    rt_uint8_t *p;
    rt_uint8_t *pucAddr = pucBuf;
    if(usLen == 0)
    {
        return;
    }
	    if(pcStr)
    {
        rt_kprintf("%s: length = %d [0x%X]\r\n", pcStr, usLen, usLen);
    }
    p = acTmp;
    rt_kprintf("%p  ", pucAddr);
    for(i=0;i<usLen;i++)
    {

        rt_kprintf("%02X ",pucBuf[i]);
        if((pucBuf[i]>=0x20) && (pucBuf[i]<0x7F))
        {
            *p++ = pucBuf[i];
        }
        else
        {
            *p++ = '.';
        }
        if((i+1)%16==0)
        {
            *p++ = 0;//string end
            rt_kprintf("        | %s", acTmp);
            p = acTmp;

            rt_kprintf("\r\n");

            if((i+1) < usLen)
            {
                pucAddr += 16;
                rt_kprintf("%p  ", pucAddr);
            }
        }
        else if((i+1)%8==0)
        {
            rt_kprintf("- ");
        }
    }
    if(usLen%16!=0)
    {
        for(i=usLen%16;i<16;i++)
        {
            rt_kprintf("   ");
            if(((i+1)%8==0) && ((i+1)%16!=0))
            {
                rt_kprintf("- ");
            }
        }
        *p++ = 0;//string end
        rt_kprintf("        | %s", acTmp);
        rt_kprintf("\r\n");
    }
}
static int AtStr2Hex(const char* src, unsigned char* dest)
{
	int srcLen,destLen;
	unsigned char val;
	unsigned char tmp;
	int i =0;
	//rt_kprintf("Entering AtStr2Hex()\r\n");
	if(src == NULL || dest == NULL)
	{
		return -1;
	}
	srcLen = strlen(src);
	//rt_kprintf("srcLen = %d\r\n",srcLen);
	if(srcLen < 1)
	{
		return -2;
	}

	if(srcLen%2)
		destLen = srcLen/2 + 1;
	else
		destLen = srcLen/2;
	rt_memset(dest,0x00,destLen);	//clr dest buf
	//rt_kprintf("AtStr2Hex()--1\r\n");
	for(i=0;i < srcLen;i++)
	{
		tmp = *(src+ i);
		if(tmp >= '0' && tmp <= '9')
		{
			val = tmp -'0';
		}
		else if(tmp >= 'A' && tmp <= 'F')
		{
			val = tmp -'A' + 10;
		}
		else 
			return -3;
		if(i%2 == 0)//high half byte
		{
			*(dest+ i/2) |= (val<<4);
		}
		else // low half byte
		{
			*(dest+ i/2) |= val;
		}
		//rt_kprintf("dest(%d) = 0x%02X\r\n",i/2,*(dest+ i/2));
	}
	return 0;
}
static rt_uint16_t AtGetCrc16(const rt_uint8_t* pData, int nLength)
{
	rt_uint16_t fcs = 0xffff; 
	while(nLength>0){
	fcs = (fcs >> 8) ^ crctab16[(fcs ^ *pData) & 0xff];
	nLength--;
	pData++;
}
return ~fcs;
}
static int AtStrCompare(const void * recvBuf,const int recvSize,const void* cmpStr)
{
	int i,iRet =0;
	int cmpStrSize;
	cmpStrSize = strlen(cmpStr);
	#if AT_DEBUG
	{
		DumpData("StrCompare",(rt_uint8_t *)recvBuf,recvSize);
		DumpData("StrCmpwith",(rt_uint8_t *)cmpStr,cmpStrSize);
	}
	#endif
	for(i = 0; i< recvSize;i++)
	{
		if(rt_memcmp(recvBuf+i,cmpStr,cmpStrSize)==0)
		{
			iRet= 1;
			break;
		}
	}
	return iRet;
}
static int AtCheckRecvData(const void * recvBuf,const int recvSize)
{
	if( AtStrCompare(recvBuf,recvSize,"OK"))
		return AT_RESPNOSE_OK;
	else
		return AT_RESPNOSE_ERROR;

}
static int AtGetSocketId(const void * recvBuf,const int recvSize)
{
	int i;
	rt_uint8_t ch;
	int socketid = AT_FAIL;
	if(AtCheckRecvData(recvBuf,recvSize) == AT_RESPNOSE_OK)
	{
		for(i=0;i<recvSize;i++)
		{
			ch = *(rt_uint8_t*)(recvBuf+i);
			if('0' <= ch && ch <= '9')
			{
				socketid = socketid*10 + (ch-'0');
			}
		}
		rt_kprintf("socket ID = %d!\r\n",socketid);
		return socketid;
	}
	else
	{
		rt_kprintf("reponse msg is NOK!\r\n");
		return AT_FAIL;
	}
}
 rt_size_t AtUart3Read(rt_device_t dev,void *buffer,rt_size_t   size)
{
	int rx_size = 0,rx_size1 =0;
	int i=0,j=0,flag = 0;
	unsigned char * tmp;
	tmp = (unsigned char*)buffer;
	while(i < atUart3RxTimeOut)
	{
		// check string with key words "ERROR" & "OK"
		if(AtStrCompare(buffer,rx_size,"ERROR")|| AtStrCompare(buffer,rx_size,"OK"))
		{
			break;
		}
		rt_thread_delay(2);
		rx_size1 = rt_device_read(dev,0,buffer+rx_size,size-rx_size);
		rx_size += rx_size1;//update rx_size value
		if(rx_size)break;
		i++;
	}
	DumpData("Uart3Rx",buffer,rx_size);
	return rx_size;
}

static rt_size_t AtUart3ReadTimeOut(rt_device_t dev,void *buffer,rt_size_t   size,int TimeOutMs)
{
	int rx_size = 0,rx_size1 =0;
	int i=0,j=0,flag = 0;
	int flagLoop=1;
	int ticktimeout=0;
	unsigned char * tmp;
	tmp = (unsigned char*)buffer;
	if(TimeOutMs == -1)
	{
		while(1)
		{
			// check string with key words "ERROR" & "OK"
			if(AtStrCompare(buffer,rx_size,"ERROR")|| AtStrCompare(buffer,rx_size,"OK"))
			{
				break;
			}
			rt_thread_delay(2);
			rx_size1 = rt_device_read(dev,0,buffer+rx_size,size-rx_size);
			rx_size += rx_size1;//update rx_size value
		}
	}
	else
	{
		ticktimeout = TimeOutMs / 20;
		while(i < ticktimeout)
		{
			// check string with key words "ERROR" & "OK"
			if(AtStrCompare(buffer,rx_size,"ERROR")|| AtStrCompare(buffer,rx_size,"OK"))
			{
				break;
			}
			rt_thread_delay(2);
			rx_size1 = rt_device_read(dev,0,buffer+rx_size,size-rx_size);
			rx_size += rx_size1;//update rx_size value
			i++;
		}
	}
	DumpData("AtUart3Rx",buffer,rx_size);
	return rx_size;
}
static void nbiot_atcmd_NCONFIG(rt_device_t newdev)
{
	char* ATcmd;
	int rx_size,iRet,infopos;
	rt_uint8_t rx_buffer[atNbiotRxBufferSize];
	rt_uint8_t tx_buffer[atNbiotTxBufferSize];
	rt_memset(rx_buffer,0x00,atNbiotRxBufferSize);
	rt_memset(tx_buffer,0x00,atNbiotTxBufferSize);
	switch(link_NBIOT)
	{
		case SEND_TO_NBIOT:
			rt_kprintf("[AT_AUTO_CONN_CFG] enter\r\n");
			ATcmd = "AT+NCONFIG=AUTOCONNECT,TRUE\n";
			DumpData("ATcmd",ATcmd,strlen(ATcmd));
			rt_device_write(newdev,0,ATcmd,strlen(ATcmd));
			link_NBIOT = RECV_FROM_NBIOT;
			break;
		case RECV_FROM_NBIOT:
			rx_size = AtUart3ReadTimeOut(newdev,rx_buffer,256,-1);
			iRet = AtCheckRecvData(rx_buffer,rx_size);
			if(iRet == AT_RESPNOSE_OK)
			{
				atstate = AT_CREATE_SOCKET;
				link_NBIOT = SEND_TO_NBIOT;
				rt_kprintf("[AT_AUTO_CONN_CFG] OK\r\n");
				rt_thread_delay(atCmdGapTime);
				
			}
			else
			{
				rt_kprintf("[AT_AUTO_CONN_CFG](error)\r\n");
			}
			rt_kprintf("[AT_AUTO_CONN_CFG] exit\r\n");
			break;
		default:
			rt_kprintf("[AT_AUTO_CONN_CFG] link_NBIOT err state\r\n");
			break;
}
}
static void nbiot_atcmd_NSOCR(rt_device_t newdev)
{
	/*
		Create a socket
		AT+NSOCR= DGRAM,17,5683,1
	*/
	char* ATcmd;
	int rx_size,iRet,infopos;
	rt_uint8_t rx_buffer[atNbiotRxBufferSize];
	rt_uint8_t tx_buffer[atNbiotTxBufferSize];
	rt_memset(rx_buffer,0x00,atNbiotRxBufferSize);
	rt_memset(tx_buffer,0x00,atNbiotTxBufferSize);
	switch(link_NBIOT)
	{
		case SEND_TO_NBIOT:
			rt_kprintf("[AT_CREATE_SOCKET] enter\r\n");
			rt_sprintf(tx_buffer,"AT+NSOCR=%s,%s,%s,%s\n",
				atNbiotCreateSocket_type,atNbiotCreateSocket_protocol,
				atNbiotCreateSocket_listen_port,atNbiotCreateSocket_receive_control);
			DumpData("ATcmd",tx_buffer,strlen(tx_buffer));
			rt_device_write(newdev,0,tx_buffer,strlen(tx_buffer));
			link_NBIOT = RECV_FROM_NBIOT;
			break;
		case RECV_FROM_NBIOT:
			rx_size = AtUart3ReadTimeOut(newdev,rx_buffer,256,-1);
			iRet = AtCheckRecvData(rx_buffer,rx_size);
			if(iRet == AT_RESPNOSE_OK)
			{
				atstate = SEND_TO_NBIOT;
				link_NBIOT = SEND_TO_NBIOT;
				socketID = AtGetSocketId(rx_buffer,rx_size);
				rt_kprintf("[AT_CREATE_SOCKET] OK\r\n");
				rt_thread_delay(atCmdGapTime);
			}
			else
			{
				rt_kprintf("[AT_CREATE_SOCKET](error)\r\n");
			}
			rt_kprintf("[AT_CREATE_SOCKET] exit\r\n");
			break;
		default:
			rt_kprintf("[AT_CREATE_SOCKET] link_NBIOT err state\r\n");
			break;
	}
}
static void nbiot_atcmd_NSOST(rt_device_t newdev)
{
	/*
		Send a message
		AT+NSOST=0,192.53.100.53,5683,25,400241C7B17401724D0265703D323031363038323331363438
		<socket> Socket number returned by AT+NSOCR
		<remote_addr> IPv4 A dot notation IP address
		<remote_port> A number in the range 0-65535. This is the remote port on which messages will be received
		<length> Decimal length of data to be sent
		<data> Data received in hex string format, or quoted string format
	*/
	char* ATcmd;
	int rx_size,iRet,infopos;
	rt_uint8_t rx_buffer[atNbiotRxBufferSize];
	rt_uint8_t tx_buffer[atNbiotTxBufferSize];
	rt_memset(rx_buffer,0x00,atNbiotRxBufferSize);
	rt_memset(tx_buffer,0x00,atNbiotTxBufferSize);
	switch(link_NBIOT)
	{
		case SEND_TO_NBIOT:
			rt_kprintf("[AT_SEND_MSG] enter\r\n");
			rt_sprintf(tx_buffer,"AT+NSOST=%d,%s,%s,%d,%s\n",
				socketID,atNbiotSendMessage_ip,
				atNbiotSendMessage_port,10,"hello NB!!!!!!");
			DumpData("ATcmd",tx_buffer,strlen(tx_buffer));
			rt_device_write(newdev,0,tx_buffer,strlen(tx_buffer));
			link_NBIOT = RECV_FROM_NBIOT;
			break;
		case RECV_FROM_NBIOT:
			rx_size = AtUart3ReadTimeOut(newdev,rx_buffer,256,-1);
			iRet = AtCheckRecvData(rx_buffer,rx_size);
			if(iRet == AT_RESPNOSE_OK)
			{
				atstate = AT_RECV_MSG;
				link_NBIOT = SEND_TO_NBIOT;
				rt_kprintf("[AT_SEND_MSG] OK\r\n");
				rt_thread_delay(atCmdGapTime);
				
			}
			else
			{
				rt_kprintf("[AT_SEND_MSG](error)\r\n");
			}
			rt_kprintf("[AT_SEND_MSG] exit\r\n");
			break;
		default:
			rt_kprintf("[AT_SEND_MSG] link_NBIOT err state\r\n");
			break;
	}
}
static void nbiot_atcmd_NSONMI(rt_device_t newdev)
{
	/*
		Receive the message
		+NSONMI:0,4
	*/
	int rx_size,i,startpos =0,socketid=0,slen=0;
	rt_uint8_t rx_buffer[atNbiotRxBufferSize];
	rt_uint8_t tx_buffer[atNbiotTxBufferSize];
	rt_uint8_t ch;
	rt_memset(rx_buffer,0x00,atNbiotRxBufferSize);
	rt_memset(tx_buffer,0x00,atNbiotTxBufferSize);
	rt_kprintf("[AT_RECV_MSG] enter\r\n");
	rx_size = AtUart3ReadTimeOut(newdev,rx_buffer,256,1000);
	if(rx_size)
	{
		for(i=0;i<rx_size;i++)
		{

			if(rt_memcmp((rx_buffer+i),"+NSONMI:",8)==0)
			{
				startpos = i+8;
				break;
			}

		}
		if(startpos == 0 )return ;
		for(i=startpos;i<rx_size;i++)
		{
			ch= *(rx_buffer + i);
			if('0' <= ch && ch <= '9')
			{
				socketid = socketid*10+(ch -'0');
			}
			if(ch == ',')
			{
				startpos = i+1;
				break;
			}
		}
		for(i=startpos;i<rx_size;i++)
		{
			ch= *(rx_buffer + i);
			if('0' <= ch && ch <= '9')
			{
				slen = slen*10+(ch -'0');
			}
		}
		rt_kprintf("socket ID = %d,message len = !\r\n",socketid,slen);
		smsg.socketid = socketid;
		smsg.msglen = slen;
		atstate = AT_READ_MSG;
		link_NBIOT = SEND_TO_NBIOT;
	}
	rt_kprintf("[AT_RECV_MSG] exit\r\n");
}

static void nbiot_atcmd_NSORF(rt_device_t newdev)
{
	/*
		Read the messages
		AT+NSORF=0,4 
	*/
	char* ATcmd;
	int rx_size,iRet,infopos;
	rt_uint8_t rx_buffer[atNbiotRxBufferSize];
	rt_uint8_t tx_buffer[atNbiotTxBufferSize];
	rt_memset(rx_buffer,0x00,atNbiotRxBufferSize);
	rt_memset(tx_buffer,0x00,atNbiotTxBufferSize);
	switch(link_NBIOT)
	{
		case SEND_TO_NBIOT:
			rt_kprintf("[AT_READ_MSG] enter\r\n");
			rt_sprintf(tx_buffer,"AT+NSORF=%d,%d\n",smsg.socketid,smsg.msglen);
			DumpData("ATcmd",tx_buffer,strlen(tx_buffer));
			rt_device_write(newdev,0,tx_buffer,strlen(tx_buffer));
			link_NBIOT = RECV_FROM_NBIOT;
			break;
		case RECV_FROM_NBIOT:
			rx_size = AtUart3ReadTimeOut(newdev,rx_buffer,256,-1);
			iRet = AtCheckRecvData(rx_buffer,rx_size);
			if(iRet == AT_RESPNOSE_OK)
			{
				atstate = AT_RECV_MSG;
				link_NBIOT = RECV_FROM_NBIOT;
				/*
					<socket>,<ip_addr>,<port>,<length>,<data>,<remaining_length>
				*/
				rt_kprintf("[AT_READ_MSG] OK\r\n");
				rt_thread_delay(atCmdGapTime);
				
			}
			else
			{
				rt_kprintf("[AT_READ_MSG](error)\r\n");
			}
			rt_kprintf("[AT_READ_MSG] exit\r\n");
			break;
		default:
			rt_kprintf("[AT_READ_MSG] link_NBIOT err state\r\n");
			break;
	}
}

static void nbiot_atcmd_NSOCL(rt_device_t newdev)
{
	/*
		Close the socket
		AT+NSOCL=0 
	*/
	char* ATcmd;
	int rx_size,iRet,infopos;
	rt_uint8_t rx_buffer[atNbiotRxBufferSize];
	rt_uint8_t tx_buffer[atNbiotTxBufferSize];
	rt_memset(rx_buffer,0x00,atNbiotRxBufferSize);
	rt_memset(tx_buffer,0x00,atNbiotTxBufferSize);
	switch(link_NBIOT)
	{
		case SEND_TO_NBIOT:
			rt_kprintf("[AT_CLOSE_SOCKET] enter\r\n");
			rt_sprintf(tx_buffer,"AT+NSOCL=%d\n",socketID);
			DumpData("ATcmd",tx_buffer,strlen(tx_buffer));
			rt_device_write(newdev,0,tx_buffer,strlen(tx_buffer));
			link_NBIOT = RECV_FROM_NBIOT;
			break;
		case RECV_FROM_NBIOT:
			rx_size = AtUart3ReadTimeOut(newdev,rx_buffer,256,-1);
			iRet = AtCheckRecvData(rx_buffer,rx_size);
			if(iRet == AT_RESPNOSE_OK)
			{
				atstate = AT_IDLE;
				link_NBIOT = RECV_FROM_NBIOT;
				rt_kprintf("[AT_CLOSE_SOCKET] OK\r\n");
				rt_thread_delay(atCmdGapTime);
			}
			else
			{
				rt_kprintf("[AT_CLOSE_SOCKET](error)\r\n");
			}
			rt_kprintf("[AT_CLOSE_SOCKET] exit\r\n");
			break;
		default:
			rt_kprintf("[AT_CLOSE_SOCKET] link_NBIOT err state\r\n");
			break;
	}
}

void rt_init_thread_entry(void *parameter)
{
	/* initialization RT-Thread Components */
	rt_components_init();
}

void rt_run_example_thread_entry(void *parameter)
{
	rt_uint8_t rx_buffer[256];
	rt_uint8_t tx_buffer[256];
	rt_device_t newdev;
	rt_err_t iRet;
	rt_uint32_t rx_size,tx_size;
	rt_kprintf("uart1 thread start...\r\n");
	 newdev = rt_device_find("uart1");
	if(newdev == NULL)
	{
		rt_kprintf("find no uart1\r\n");
		return ;
	}
	iRet = rt_device_open(newdev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM);
	if(iRet)
	{
		rt_kprintf("open uart1 failure\r\n");
		return ;
	}
	//rt_pad_show();
	for(;;)
	{
		rt_thread_delay(100);
	}
	rt_kprintf("never get here...\r\n");
}

void rt_run_example2_thread_entry(void *parameter)
{

	rt_device_t newdev;
	rt_uint32_t rx_size,tx_size;
	char* ATcmd,PtData;
	rt_uint8_t rx_buffer[256];
	rt_uint8_t tx_buffer[256];
	int pos,iRet;
	rt_kprintf("uart3 thread start...\r\n");
	 newdev = rt_device_find("uart3");
	if(newdev == NULL)
	{
		rt_kprintf("find no uart3\r\n");
		return ;
	}
	iRet = rt_device_open(newdev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM);
	if(iRet)
	{
		rt_kprintf("open uart3 failure\r\n");
		return ;
	}
	//rt_pad_show();
	for(;;)
	{
		rt_memset(rx_buffer,0,256);
		rt_memset(tx_buffer,0,256);
		switch(atstate)
		{
			case AT_AUTO_CONN_CFG:
				nbiot_atcmd_NCONFIG(newdev);
				break;
			case AT_CREATE_SOCKET:
				nbiot_atcmd_NSOCR(newdev);
				break;
			case AT_SEND_MSG:
				nbiot_atcmd_NSOST(newdev);
				break;
			case AT_RECV_MSG:
				nbiot_atcmd_NSONMI(newdev);
				break;
			case AT_READ_MSG:
				nbiot_atcmd_NSORF(newdev);
				break;
			case AT_CLOSE_SOCKET:
				nbiot_atcmd_NSOCL(newdev);
				break;
			case AT_IDLE:
				rt_thread_delay(100);
				break;
			default:
				rt_thread_delay(100);
				break;
		}
		rt_thread_delay(1);		
	}
	rt_kprintf("never get here...\r\n");
}

int rt_application_init(void)
{
	rt_thread_t tid,tid1,tid2,tid3;
	rt_thread_t sid;
	/* create initialization thread */
	tid = rt_thread_create("init",
							rt_init_thread_entry, RT_NULL,
							4096, RT_THREAD_PRIORITY_MAX/3, 20);
	if (tid != RT_NULL)
		rt_thread_startup(tid);
	else
		return -1;

	/*create example thread*/
	tid1 = rt_thread_create("example",
							rt_run_example_thread_entry, RT_NULL,
							4096, 16, 20);
	if (tid1 != RT_NULL)
		rt_thread_startup(tid1);
	else
		return -1;


	/*create example2 thread*/
	tid2 = rt_thread_create("example2",
							rt_run_example2_thread_entry, RT_NULL,
							4096, 17, 20);
	if (tid2 != RT_NULL)
		rt_thread_startup(tid2);
	else
		return -1;


	return 0;
}

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
static int socketId;
static rt_err_t found_uart3(rt_device_t* NBdev){
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
static rt_err_t found_str(char* str,const char* substr,int*subendpos)
{
	int i;
	rt_err_t ret=RT_ERROR;
	for(i=0;i<strlen(str);i++)
	if(rt_memcmp(str+i,substr,strlen(substr))==0)
	{
		*subendpos = i+ strlen(substr);
		return RT_EOK;
	}
	return ret;
}
static int str2int(const char* str)
{
	int len = strlen(str);
	int value;
	int i;
	for(i=0;i<len;i++)
	{
		if(*(str+i) <'0' || *(str+i) > '9')return 0;
		value += (*(str+i) - '0')*(10^(len-1));
	}
	if(value >512) return 0;// NB-IOT message max len=512
	return value;
}
static rt_err_t SendData(rt_uint8_t* data,int len)
{
	rt_err_t ret = rt_device_write(NB_IOT_UART,0,data,len);
	rt_kprintf("%s\r\n",data);
	return ret;
}
static rt_err_t RecvData(rt_uint8_t* data,int* len,int timeoutms)
{
	int i=0,ticks=0;
	int ilen=0;
	rt_memset(data,0x00,*len);
	ticks = timeoutms/10;
	while(i<ticks)
	{
		rt_thread_delay(10);
		i+=10;
		ilen=rt_device_read(NB_IOT_UART,0,data,*len);
		if(ilen)break;
	}
	if(i>ticks)return RT_ETIMEOUT;
	rt_kprintf("%s\r\n",data);
	return RT_EOK;
}
static rt_err_t AT_NBR(void)//reboot
{
	rt_err_t ret;
	char* cmd = "AT+NBR\r\n";
	char resp[256],rlen=256,int pos;

	SendData(cmd,strlen(cmd));
	ret = RecvData(resp,&rlen,2000);
	if(ret)return ret;
	return found_str(resp,"REBOOT",&pos);
}
static rt_err_t AT_NBAND(int* band)
{
	rt_err_t ret;
	char* cmd0 = "AT+NBAND?\r\n";
	char resp[256],rlen=256,pos;
	SendData(cmd0,strlen(cmd0));
	ret = RecvData(resp,&rlen,2000);
	if(ret)return ret;
	found_str(resp,"+NBAND:",&pos)
	*band = resp[pos]-'0';
	return RT_EOK;

}
static rt_err_t AT_NBAND2(int band)
{
	rt_err_t ret;
	char* cmd0 = "AT+NBAND";
	char cmd1[20];//band5 850mhz china telecom
	char resp[256],rlen=256,pos;
	rt_sprintf(cmd1,"%s=%d\r\n",cmd0,band);
	ret = SendData(cmd1,strlen(cmd1));
	if(ret)return ret;
	ret = RecvData(resp,&rlen,2000);
	if(ret)return ret;
	ret = found_str(resp,"OK",&pos);
	if(ret)return ret;
	return RT_EOK;
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
static rt_err_t AT_NCONFIG(void)
{
	rt_err_t ret;
	char* cmd = "AT+NCONFIG=AUTOCONNECT,TRUE\r\n";
	char resp[256],rlen=256,int pos;

	SendData(cmd,strlen(cmd));
	ret = RecvData(resp,&rlen,2000);
	if(ret)return ret;
	return found_str(resp,"OK",&pos);
}
static rt_err_t AT_NSOCR(int localport)//create socket
{
	rt_err_t ret;
	char* cmd0 = "AT+NSOCR";
	char cmd1[100];//band5 850mhz china telecom
	char resp[256],rlen=256,pos;
	rt_sprintf(cmd1,"%s=DGRAM,17,%d,1\r\n",cmd0,band);
	ret = SendData(cmd1,strlen(cmd1));
	if(ret)return ret;
	ret = RecvData(resp,&rlen,2000);
	if(ret)return ret;
	ret = found_str(resp,"OK",&pos);
	if(ret)return ret;
	scoketId = resp[0]-'0';
	return RT_EOK;
}
static rt_err_t AT_NSOST(const char* ip,const int remoteport,char* msg,int len)//send msg
{
	rt_err_t ret;
	char* cmd0 = "AT+NSOST";
	char cmd1[1024];
	char resp[256],rlen=256,pos;
	rt_sprintf(cmd1,"%s=%d,%s,%d,%d,%s\r\n",cmd0,socketId,ip,remoteport,len,msg);
	ret = SendData(cmd1,strlen(cmd1));
	if(ret)return ret;
	ret = RecvData(resp,&rlen,2000);
	if(ret)return ret;
	return found_str(resp,"OK",&pos);
}
static rt_err_t AT_NSONMI(char* msgflag, int* len)// recv msg interrupt
{
	rt_err_t ret;
	char resp[1024],rlen=1024,pos,pos1;
	ret = RecvData(resp,&rlen,10000);
	if(ret)return ret;
	ret= found_str(resp,"+NSONMI:",&pos);
	if(ret != RT_EOK)return ret;
	ret = found(resp+pos,"\r\n",&pos1);
	if(ret != RT_EOK)
	{
		*len=0;
		return ret;
	}
	rt_memcpy(msgflag,resp+pos,pos1-pos-2);
	*len=pos1-pos-2;
	return RT_EOK;
}
static rt_err_t AT_NSORF(char* msgflag,char* msg,int* len)//recv msg
{
	rt_err_t ret;
	char* cmd0 = "AT+NSORF";
	char cmd1[1024];
	char resp[1024],rlen=1024,pos;
	char* socket,ip,port,length,message;
	char* substr;
	int ilen;
	rt_sprintf(cmd1,"%s=%s\r\n",cmd0,msgflag);
	ret = SendData(cmd1,strlen(cmd1));
	if(ret)return ret;
	ret = RecvData(resp,&rlen,2000);
	if(ret)return ret;
	ret = found_str(resp,"OK",&pos);
	if(ret)
	{
		*len =0;
		return ret;
	}
	else
	{
		substr=resp;

		socket=substr;
		found_str(substr,',',&pos);
		substr[pos-1] = '\0';
		substr += pos;

		ip=substr;
		found_str(substr,',',&pos);
		substr[pos-1] = '\0';
		substr += pos;

		port = substr+pos;
		found_str(substr,',',&pos);
		substr[pos-1] = '\0';
		substr += pos;

		length = substr+pos;
		found_str(substr,',',&pos);
		substr[pos-1] = '\0';
		substr += pos;

		message = substr+pos;
		found_str(substr,',',&pos);
		substr[pos-1] = '\0';
		substr += pos;

		ilen  =str2int(length);
		rt_memcpy(msg,message,ilen);
	}
	return RT_EOK;
}
static rt_err_t AT_NSCL(void)
{
	rt_err_t ret;
	char* cmd0 = "AT+NSCL";
	char cmd1[1024];
	char resp[256],rlen=256,pos;
	rt_sprintf(cmd1,"%s=%d\r\n",cmd0,socketId);
	ret = SendData(cmd1,strlen(cmd1));
	if(ret)return ret;
	ret = RecvData(resp,&rlen,2000);
	if(ret)return ret;
	return found_str(resp,"OK",&pos);
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
	int band;
	AT_NBAND(&band);
	if(band != 5)
	{
		AT_NBAND2(5);
		AT_NBR();
		rt_thread_delay(50);
		AT_NBAND(&band);
		if(band != 5)return RT_EOK;
	}
	AT_NCONFIG();
	AT_NSOCR(5683);

	return RT_EOK;
}
static rt_err_t rt_nb_iot_close(rt_device_t dev)
{
	RT_ASSERT(dev != RT_NULL);

	return RT_EOK;
}
static rt_size_t rt_nb_iot_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{charchar
	RT_ASSERT(dev != RT_NULL);char
	rt_err_t ret;
	char* msgflag[30];
	int len;
	AT_NSONMI();
	ret=AT_NSOST("114.115.144.122",4567,buffer,size);
	if(ret)
	{
		return  0;
	}
	return size;

	return 1;
}
static rt_size_t rt_nb_iot_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
	RT_ASSERT(dev != RT_NULL);
	rt_err_t ret;
	ret=AT_NSOST("114.115.144.122",4567,buffer,size);
	if(ret)
	{
		return  0;
	}
	return size;
}



rt_err_t  rt_nb_iot_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
	RT_ASSERT(dev != RT_NULL);
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

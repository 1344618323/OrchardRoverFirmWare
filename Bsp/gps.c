#include "gps.h"
#include "cmsis_os.h"
#include "string.h"
#include "math.h"
#include "usart.h"

/********************直接在正点原子源码的基础上修改************************/
uint8_t gps_uart_tx_buff[GPS_UART_TX_MAX_BUFLEN];
uint8_t gps_uart_rx_buff[GPS_UART_RX_MAX_BUFLEN];
uint8_t gps_init = 0;
uint8_t ack_flag = 0;
uint16_t ack_len = 0;

nmea_msg gpsx;

void GPS_Uart_Callback_Handle(uint32_t len, uint8_t *send_flag)
{
	if (gps_init == 0)
	{
		ack_len = len;
		ack_flag = 1;
	}
	else
	{
		memcpy(gps_uart_tx_buff, gps_uart_rx_buff, len);
		GPS_Analysis(&gpsx, (uint8_t *)gps_uart_tx_buff, send_flag); //分析字符串
	}
}

void GPS_INIT(void)
{
	if (SkyTra_Cfg_Rate(5) != 0) //设置定位信息更新速度为5Hz,顺便判断GPS模块是否在位.
	{
		uint8_t key = 0xFF;
		do
		{
			LEONARD_USART3_UART_Init(9600);			   //初始化串口3波特率为9600
			SkyTra_Cfg_Prt(3);						   //重新设置模块的波特率为38400
			LEONARD_USART3_UART_Init(38400);		   //初始化串口3波特率为38400
			key = SkyTra_Cfg_Tp(100000);			   //脉冲宽度为100ms
		} while (SkyTra_Cfg_Rate(5) != 0 && key != 0); //配置SkyTraF8-BD的更新速率为5Hz
		gps_init = 1;
		HAL_Delay(500);
	}
	gps_init = 1;
}

const uint32_t BAUD_id[9] = {4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600}; //模块支持波特率数组
//从buf里面得到第cx个逗号所在的位置
//返回值:0~0XFE,代表逗号所在位置的偏移.
//       0XFF,代表不存在第cx个逗号
uint8_t NMEA_Comma_Pos(uint8_t *buf, uint8_t cx)
{
	uint8_t *p = buf;
	while (cx)
	{
		if (*buf == '*' || *buf < ' ' || *buf > 'z')
			return 0XFF; //遇到'*'或者非法字符,则不存在第cx个逗号
		if (*buf == ',')
			cx--;
		buf++;
	}
	return buf - p;
}
//m^n函数
//返回值:m^n次方.
uint32_t NMEA_Pow(uint8_t m, uint8_t n)
{
	uint32_t result = 1;
	while (n--)
		result *= m;
	return result;
}
//str转换为数字,以','或者'*'结束
//buf:数字存储区
//dx:小数点位数,返回给调用函数
//返回值:转换后的数值
int NMEA_Str2num(uint8_t *buf, uint8_t *dx)
{
	uint8_t *p = buf;
	uint32_t ires = 0, fres = 0;
	uint8_t ilen = 0, flen = 0, i;
	uint8_t mask = 0;
	int res;
	while (1) //得到整数和小数的长度
	{
		if (*p == '-')
		{
			mask |= 0X02;
			p++;
		} //是负数
		if (*p == ',' || (*p == '*'))
			break; //遇到结束了
		if (*p == '.')
		{
			mask |= 0X01;
			p++;
		}								 //遇到小数点了
		else if (*p > '9' || (*p < '0')) //有非法字符
		{
			ilen = 0;
			flen = 0;
			break;
		}
		if (mask & 0X01)
			flen++;
		else
			ilen++;
		p++;
	}
	if (mask & 0X02)
		buf++;				   //去掉负号
	for (i = 0; i < ilen; i++) //得到整数部分数据
	{
		ires += NMEA_Pow(10, ilen - 1 - i) * (buf[i] - '0');
	}
	if (flen > 5)
		flen = 5;			   //最多取5位小数
	*dx = flen;				   //小数点位数
	for (i = 0; i < flen; i++) //得到小数部分数据
	{
		fres += NMEA_Pow(10, flen - 1 - i) * (buf[ilen + 1 + i] - '0');
	}
	res = ires * NMEA_Pow(10, flen) + fres;
	if (mask & 0X02)
		res = -res;
	return res;
}
//分析GPGSV信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GPGSV_Analysis(nmea_msg *gpsx, uint8_t *buf)
{
	uint8_t *p, *p1, dx;
	uint8_t len, i, j, slx = 0;
	uint8_t posx;
	p = buf;
	p1 = (uint8_t *)strstr((const char *)p, "$GPGSV");
	len = p1[7] - '0';			  //得到GPGSV的条数
	posx = NMEA_Comma_Pos(p1, 3); //得到可见卫星总数
	if (posx != 0XFF)
		gpsx->svnum = NMEA_Str2num(p1 + posx, &dx);
	for (i = 0; i < len; i++)
	{
		p1 = (uint8_t *)strstr((const char *)p, "$GPGSV");
		for (j = 0; j < 4; j++)
		{
			posx = NMEA_Comma_Pos(p1, 4 + j * 4);
			if (posx != 0XFF)
				gpsx->slmsg[slx].num = NMEA_Str2num(p1 + posx, &dx); //得到卫星编号
			else
				break;
			posx = NMEA_Comma_Pos(p1, 5 + j * 4);
			if (posx != 0XFF)
				gpsx->slmsg[slx].eledeg = NMEA_Str2num(p1 + posx, &dx); //得到卫星仰角
			else
				break;
			posx = NMEA_Comma_Pos(p1, 6 + j * 4);
			if (posx != 0XFF)
				gpsx->slmsg[slx].azideg = NMEA_Str2num(p1 + posx, &dx); //得到卫星方位角
			else
				break;
			posx = NMEA_Comma_Pos(p1, 7 + j * 4);
			if (posx != 0XFF)
				gpsx->slmsg[slx].sn = NMEA_Str2num(p1 + posx, &dx); //得到卫星信噪比
			else
				break;
			slx++;
		}
		p = p1 + 1; //切换到下一个GPGSV信息
	}
}
//分析BDGSV信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_BDGSV_Analysis(nmea_msg *gpsx, uint8_t *buf)
{
	uint8_t *p, *p1, dx;
	uint8_t len, i, j, slx = 0;
	uint8_t posx;
	p = buf;
	p1 = (uint8_t *)strstr((const char *)p, "$BDGSV");
	len = p1[7] - '0';			  //得到BDGSV的条数
	posx = NMEA_Comma_Pos(p1, 3); //得到可见北斗卫星总数
	if (posx != 0XFF)
		gpsx->beidou_svnum = NMEA_Str2num(p1 + posx, &dx);
	for (i = 0; i < len; i++)
	{
		p1 = (uint8_t *)strstr((const char *)p, "$BDGSV");
		for (j = 0; j < 4; j++)
		{
			posx = NMEA_Comma_Pos(p1, 4 + j * 4);
			if (posx != 0XFF)
				gpsx->beidou_slmsg[slx].beidou_num = NMEA_Str2num(p1 + posx, &dx); //得到卫星编号
			else
				break;
			posx = NMEA_Comma_Pos(p1, 5 + j * 4);
			if (posx != 0XFF)
				gpsx->beidou_slmsg[slx].beidou_eledeg = NMEA_Str2num(p1 + posx, &dx); //得到卫星仰角
			else
				break;
			posx = NMEA_Comma_Pos(p1, 6 + j * 4);
			if (posx != 0XFF)
				gpsx->beidou_slmsg[slx].beidou_azideg = NMEA_Str2num(p1 + posx, &dx); //得到卫星方位角
			else
				break;
			posx = NMEA_Comma_Pos(p1, 7 + j * 4);
			if (posx != 0XFF)
				gpsx->beidou_slmsg[slx].beidou_sn = NMEA_Str2num(p1 + posx, &dx); //得到卫星信噪比
			else
				break;
			slx++;
		}
		p = p1 + 1; //切换到下一个BDGSV信息
	}
}
//分析GNGGA信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GNGGA_Analysis(nmea_msg *gpsx, uint8_t *buf)
{
	uint8_t *p1, dx;
	uint8_t posx;
	p1 = (uint8_t *)strstr((const char *)buf, "$GNGGA");
	posx = NMEA_Comma_Pos(p1, 6); //得到GPS状态
	if (posx != 0XFF)
		gpsx->gpssta = NMEA_Str2num(p1 + posx, &dx);
	posx = NMEA_Comma_Pos(p1, 7); //得到用于定位的卫星数
	if (posx != 0XFF)
		gpsx->posslnum = NMEA_Str2num(p1 + posx, &dx);
	posx = NMEA_Comma_Pos(p1, 9); //得到海拔高度
	if (posx != 0XFF)
		gpsx->altitude = NMEA_Str2num(p1 + posx, &dx);
}
//分析GNGSA信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GNGSA_Analysis(nmea_msg *gpsx, uint8_t *buf)
{
	uint8_t *p1, dx;
	uint8_t posx;
	uint8_t i;
	p1 = (uint8_t *)strstr((const char *)buf, "$GNGSA");
	posx = NMEA_Comma_Pos(p1, 2); //得到定位类型
	if (posx != 0XFF)
		gpsx->fixmode = NMEA_Str2num(p1 + posx, &dx);
	for (i = 0; i < 12; i++) //得到定位卫星编号
	{
		posx = NMEA_Comma_Pos(p1, 3 + i);
		if (posx != 0XFF)
			gpsx->possl[i] = NMEA_Str2num(p1 + posx, &dx);
		else
			break;
	}
	posx = NMEA_Comma_Pos(p1, 15); //得到PDOP位置精度因子
	if (posx != 0XFF)
		gpsx->pdop = NMEA_Str2num(p1 + posx, &dx);
	posx = NMEA_Comma_Pos(p1, 16); //得到HDOP位置精度因子
	if (posx != 0XFF)
		gpsx->hdop = NMEA_Str2num(p1 + posx, &dx);
	posx = NMEA_Comma_Pos(p1, 17); //得到VDOP位置精度因子
	if (posx != 0XFF)
		gpsx->vdop = NMEA_Str2num(p1 + posx, &dx);
}
//分析GNRMC信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GNRMC_Analysis(nmea_msg *gpsx, uint8_t *buf, uint8_t *send_flag)
{
	uint8_t *p1, dx;
	uint8_t posx;
	uint32_t temp;
	float rs;
	p1 = (uint8_t *)strstr((const char *)buf, "$GNRMC"); //"$GNRMC",经常有&和GNRMC分开的情况,故只判断GPRMC.
	posx = NMEA_Comma_Pos(p1, 1);						 //得到UTC时间
	if (posx != 0XFF)
	{
		temp = NMEA_Str2num(p1 + posx, &dx) / NMEA_Pow(10, dx); //得到UTC时间,去掉ms
		gpsx->utc.hour = temp / 10000;
		gpsx->utc.min = (temp / 100) % 100;
		gpsx->utc.sec = temp % 100;
	}
	posx = NMEA_Comma_Pos(p1, 3); //得到纬度
	if (posx != 0XFF)
	{
		temp = NMEA_Str2num(p1 + posx, &dx);
		gpsx->latitude = temp / NMEA_Pow(10, dx + 2);										  //得到°
		rs = temp % NMEA_Pow(10, dx + 2);													  //得到'
		gpsx->latitude = gpsx->latitude * NMEA_Pow(10, 6) + (rs * NMEA_Pow(10, 6 - dx)) / 60; //转换为°
	}
	posx = NMEA_Comma_Pos(p1, 4); //南纬还是北纬
	if (posx != 0XFF)
		gpsx->nshemi = *(p1 + posx);
	posx = NMEA_Comma_Pos(p1, 5); //得到经度
	if (posx != 0XFF)
	{
		temp = NMEA_Str2num(p1 + posx, &dx);
		gpsx->longitude = temp / NMEA_Pow(10, dx + 2);											//得到°
		rs = temp % NMEA_Pow(10, dx + 2);														//得到'
		gpsx->longitude = gpsx->longitude * NMEA_Pow(10, 6) + (rs * NMEA_Pow(10, 6 - dx)) / 60; //转换为°

		*send_flag = 1;
	}
	posx = NMEA_Comma_Pos(p1, 6); //东经还是西经
	if (posx != 0XFF)
		gpsx->ewhemi = *(p1 + posx);
	posx = NMEA_Comma_Pos(p1, 9); //得到UTC日期
	if (posx != 0XFF)
	{
		temp = NMEA_Str2num(p1 + posx, &dx); //得到UTC日期
		gpsx->utc.date = temp / 10000;
		gpsx->utc.month = (temp / 100) % 100;
		gpsx->utc.year = 2000 + temp % 100;
	}
}
//分析GNVTG信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GNVTG_Analysis(nmea_msg *gpsx, uint8_t *buf)
{
	uint8_t *p1, dx;
	uint8_t posx;
	p1 = (uint8_t *)strstr((const char *)buf, "$GNVTG");
	posx = NMEA_Comma_Pos(p1, 7); //得到地面速率
	if (posx != 0XFF)
	{
		gpsx->speed = NMEA_Str2num(p1 + posx, &dx);
		if (dx < 3)
			gpsx->speed *= NMEA_Pow(10, 3 - dx); //确保扩大1000倍
	}
}
//提取NMEA-0183信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void GPS_Analysis(nmea_msg *gpsx, uint8_t *buf, uint8_t *send_flag)
{
	//NMEA_GPGSV_Analysis(gpsx, buf); //GPGSV解析
	//NMEA_BDGSV_Analysis(gpsx, buf); //BDGSV解析
	NMEA_GNGGA_Analysis(gpsx, buf);			   //GNGGA解析
	NMEA_GNGSA_Analysis(gpsx, buf);			   //GPNSA解析
	NMEA_GNRMC_Analysis(gpsx, buf, send_flag); //GPNMC解析
											   //NMEA_GNVTG_Analysis(gpsx, buf); //GPNTG解析
}
///////////////////////////////////////////UBLOX 配置代码/////////////////////////////////////
////检查CFG配置执行情况
////返回值:0,ACK成功
////       1,接收超时错误
////       2,没有找到同步字符
////       3,接收到NACK应答
uint8_t SkyTra_Cfg_Ack_Check(void)
{
	uint16_t len = 0, i;
	uint8_t rval = 0;
	while (ack_flag == 0 && len < 100) //等待接收到应答
	{
		len++;
		HAL_Delay(5);
	}
	if (len < 100) //超时错误.
	{
		len = ack_len; //此次接收到的数据长度
		for (i = 0; i < len; i++)
		{
			if (gps_uart_rx_buff[i] == 0X83)
				break;
			else if (gps_uart_rx_buff[i] == 0X84)
			{
				rval = 3;
				break;
			}
		}
		if (i == len)
			rval = 2; //没有找到同步字符
	}
	else
		rval = 1; //接收超时错误
	ack_flag = 0; //清除接收
	return rval;
}

//配置SkyTra_GPS/北斗模块波特率
//baud_id:0~8，对应波特率,4800/9600/19200/38400/57600/115200/230400/460800/921600
//返回值:0,执行成功;其他,执行失败(这里不会返回0了)
uint8_t SkyTra_Cfg_Prt(uint32_t baud_id)
{
	SkyTra_baudrate *cfg_prt = (SkyTra_baudrate *)gps_uart_tx_buff;
	cfg_prt->sos = 0XA1A0;		//引导序列(小端模式)
	cfg_prt->PL = 0X0400;		//有效数据长度(小端模式)
	cfg_prt->id = 0X05;			//配置波特率的ID
	cfg_prt->com_port = 0X00;	//操作串口1
	cfg_prt->Baud_id = baud_id; ////波特率对应编号
	cfg_prt->Attributes = 1;	//保存到SRAM&FLASH
	cfg_prt->CS = cfg_prt->id ^ cfg_prt->com_port ^ cfg_prt->Baud_id ^ cfg_prt->Attributes;
	cfg_prt->end = 0X0A0D;										   //发送结束符(小端模式)
	SkyTra_Send_Date((uint8_t *)cfg_prt, sizeof(SkyTra_baudrate)); //发送数据给SkyTra
	HAL_Delay(200);												   //等待发送完成
	LEONARD_USART3_UART_Init(BAUD_id[baud_id]);					   //重新初始化串口3
	return SkyTra_Cfg_Ack_Check();								   //这里不会反回0,因为UBLOX发回来的应答在串口重新初始化的时候已经被丢弃了.
}
//配置SkyTra_GPS/北斗模块的时钟脉冲宽度
//width:脉冲宽度1~100000(us)
//返回值:0,发送成功;其他,发送失败.
uint8_t SkyTra_Cfg_Tp(uint32_t width)
{
	uint32_t temp = width;
	SkyTra_pps_width *cfg_tp = (SkyTra_pps_width *)gps_uart_tx_buff;
	temp = (width >> 24) | ((width >> 8) & 0X0000FF00) | ((width << 8) & 0X00FF0000) | ((width << 24) & 0XFF000000);														   //小端模式
	cfg_tp->sos = 0XA1A0;																																					   //cfg header(小端模式)
	cfg_tp->PL = 0X0700;																																					   //有效数据长度(小端模式)
	cfg_tp->id = 0X65;																																						   //cfg tp id
	cfg_tp->Sub_ID = 0X01;																																					   //数据区长度为20个字节.
	cfg_tp->width = temp;																																					   //脉冲宽度,us
	cfg_tp->Attributes = 0X01;																																				   //保存到SRAM&FLASH
	cfg_tp->CS = cfg_tp->id ^ cfg_tp->Sub_ID ^ (cfg_tp->width >> 24) ^ (cfg_tp->width >> 16) & 0XFF ^ (cfg_tp->width >> 8) & 0XFF ^ cfg_tp->width & 0XFF ^ cfg_tp->Attributes; //用户延时为0ns
	cfg_tp->end = 0X0A0D;																																					   //发送结束符(小端模式)
	SkyTra_Send_Date((uint8_t *)cfg_tp, sizeof(SkyTra_pps_width));																											   //发送数据给NEO-6M
	return SkyTra_Cfg_Ack_Check();
}
//配置SkyTraF8-BD的更新速率
//Frep:（取值范围:1,2,4,5,8,10,20,25,40,50）测量时间间隔，单位为Hz，最大不能大于50Hz
//返回值:0,发送成功;其他,发送失败.
uint8_t SkyTra_Cfg_Rate(uint8_t Frep)
{
	SkyTra_PosRate *cfg_rate = (SkyTra_PosRate *)gps_uart_tx_buff;
	cfg_rate->sos = 0XA1A0;												 //cfg header(小端模式)
	cfg_rate->PL = 0X0300;												 //有效数据长度(小端模式)
	cfg_rate->id = 0X0E;												 //cfg rate id
	cfg_rate->rate = Frep;												 //更新速率
	cfg_rate->Attributes = 0X01;										 //保存到SRAM&FLASH	.
	cfg_rate->CS = cfg_rate->id ^ cfg_rate->rate ^ cfg_rate->Attributes; //脉冲间隔,us
	cfg_rate->end = 0X0A0D;												 //发送结束符(小端模式)
	SkyTra_Send_Date((uint8_t *)cfg_rate, sizeof(SkyTra_PosRate));		 //发送数据给NEO-6M
	return SkyTra_Cfg_Ack_Check();
}
//发送一批数据给Ublox NEO-6M，这里通过串口3发送
//dbuf：数据缓存首地址
//len：要发送的字节数
void SkyTra_Send_Date(uint8_t *dbuf, uint16_t len)
{
	HAL_UART_Transmit_DMA(&GPS_HUART, gps_uart_tx_buff, len);
}

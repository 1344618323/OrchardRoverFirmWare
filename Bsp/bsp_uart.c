#include "bsp_uart.h"

uint8_t upper_sys_uart_rx_buff[UPPER_SYS_UART_RX_MAX_BUFLEN];
uint8_t upper_sys_uart_tx_buff[UPPER_SYS_UART_TX_MAX_BUFLEN];
uint8_t JY61_uart_buff[JY61_UART_RX_MAX_BUFLEN];

extern uint8_t gps_uart_tx_buff[GPS_UART_TX_MAX_BUFLEN];
extern uint8_t gps_uart_rx_buff[GPS_UART_RX_MAX_BUFLEN];
extern nmea_msg gpsx;

JY61_Data JY61_data;

uint8_t set_yaw_zero_cmd[3] = {0xFF, 0xAA, 0x52};

#define g_const 9.8

void JY61_Set_Yaw_Zero(void)
{
	HAL_UART_Transmit_DMA(&JY61_HUART, set_yaw_zero_cmd, 3);
}

void Sys_Uart_Init(void)
{

	HAL_UART_Receive_DMA(&UPPER_SYS_HUART, upper_sys_uart_rx_buff, UPPER_SYS_UART_RX_MAX_BUFLEN);
	__HAL_UART_ENABLE_IT(&UPPER_SYS_HUART, UART_IT_IDLE);

	HAL_UART_Receive_DMA(&JY61_HUART, JY61_uart_buff, JY61_UART_RX_MAX_BUFLEN);
	__HAL_UART_ENABLE_IT(&JY61_HUART, UART_IT_IDLE);

	HAL_UART_Receive_DMA(&GPS_HUART, gps_uart_rx_buff, GPS_UART_RX_MAX_BUFLEN);
	__HAL_UART_ENABLE_IT(&GPS_HUART, UART_IT_IDLE);

	JY61_Set_Yaw_Zero();

	if (gps_used)
		GPS_INIT();
}

void Uart_Rx_Idle_Callback(UART_HandleTypeDef *huart)
{
	if ((__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) != RESET))
	{
		__HAL_UART_CLEAR_IDLEFLAG(huart);
		HAL_UART_DMAStop(huart);
		__HAL_UART_DISABLE_IT(huart, UART_IT_IDLE);

		uint32_t temp;
		uint32_t rx_len;
		temp = huart->hdmarx->Instance->CNDTR;

		if (huart == &UPPER_SYS_HUART)
		{
			rx_len = UPPER_SYS_UART_RX_MAX_BUFLEN - temp;
			Upper_Sys_Uart_Callback_Handle(upper_sys_uart_rx_buff, rx_len);
			HAL_UART_Receive_DMA(huart, upper_sys_uart_rx_buff, UPPER_SYS_UART_RX_MAX_BUFLEN);
		}
		else if (huart == &JY61_HUART)
		{
			rx_len = JY61_UART_RX_MAX_BUFLEN - temp;
			JY61_Uart_Callback_Handle(JY61_uart_buff, rx_len);
			HAL_UART_Receive_DMA(huart, JY61_uart_buff, JY61_UART_RX_MAX_BUFLEN);
		}
		else if (huart == &GPS_HUART)
		{
			rx_len = GPS_UART_RX_MAX_BUFLEN - temp;

			//对gps数据进行分析，若send_flag在分析过程中被置为1，则将gps数据发送给上位机
			uint8_t send_flag = 0;
			GPS_Uart_Callback_Handle(rx_len, &send_flag);
			if (send_flag == 1 && gpsx.fixmode == 3)
				Transmit_Gps_Msg();
			HAL_UART_Receive_DMA(huart, gps_uart_rx_buff, GPS_UART_RX_MAX_BUFLEN);
		}

		__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
	}
}

void Upper_Sys_Uart_Callback_Handle(uint8_t *buff, uint32_t len)
{
	ReceiveHandle(buff, len);
}

void JY61_Uart_Callback_Handle(uint8_t *buff, uint32_t len)
{
	if (len == 33 && buff[0] == 0x55 && buff[11] == 0x55 && buff[22] == 0x55)
	{
		uint8_t sum = 0;
		for (uint8_t i = 0; i < 10; i++)
		{
			sum += buff[i];
		}
		if (buff[10] == sum && buff[1] == 0x51)
		{
			JY61_data.ax = ((short)(buff[3] << 8 | buff[2])) / 32768.0 * 16 * g_const;
			JY61_data.ay = ((short)(buff[5] << 8 | buff[4])) / 32768.0 * 16 * g_const;
			JY61_data.az = ((short)(buff[7] << 8 | buff[6])) / 32768.0 * 16 * g_const;
		}

		sum = 0;
		for (uint8_t i = 11; i < 21; i++)
		{
			sum += buff[i];
		}
		if (buff[21] == sum && buff[12] == 0x52)
		{
			JY61_data.wx = ((short)(buff[14] << 8 | buff[13])) / 32768.0 * 2000.0;
			JY61_data.wy = ((short)(buff[16] << 8 | buff[15])) / 32768.0 * 2000.0;
			JY61_data.wz = ((short)(buff[18] << 8 | buff[17])) / 32768.0 * 2000.0;
		}

		sum = 0;
		for (uint8_t i = 22; i < 32; i++)
		{
			sum += buff[i];
		}
		if (buff[32] == sum && buff[23] == 0x53)
		{
			JY61_data.roll = ((short)(buff[25] << 8 | buff[24])) / 32768.0 * 180.0;
			JY61_data.pitch = ((short)(buff[27] << 8 | buff[26])) / 32768.0 * 180.0;
			JY61_data.yaw = ((short)(buff[29] << 8 | buff[28])) / 32768.0 * 180.0;
		}
	}
}

double Get_Yaw_Angle(void)
{
	return JY61_data.yaw;
}

void Transmit_Chassis_Msg(double *msg)
{
	cmd_chassis_info info;
	info.position_x_mm = msg[0];
	info.position_y_mm = msg[1];
	info.gyro_angle = msg[2] * 10;
	uint16_t len = ProtocolPack((uint8_t *)(&info), sizeof(cmd_chassis_info), CMD_PUSH_CHASSIS_INFO, upper_sys_uart_tx_buff);
	if (len > 0)
		HAL_UART_Transmit_DMA(&UPPER_SYS_HUART, upper_sys_uart_tx_buff, len);
}

void Transmit_Gps_Msg(void)
{
	cmd_gps_info info;

	if (gpsx.fixmode == 3)
		info.fixmode = 1;
	else
		info.fixmode = 0;

	if (gpsx.nshemi == 'N')
		info.latitude = gpsx.latitude;
	else
		info.latitude = -gpsx.latitude;

	if (gpsx.ewhemi == 'E')
		info.longitude = gpsx.longitude;
	else
		info.longitude = -gpsx.longitude;

	info.altitude = gpsx.altitude;
	info.pdop = gpsx.pdop;

	uint16_t len = ProtocolPack((uint8_t *)(&info), sizeof(cmd_gps_info), CMD_PUSH_GPS_INFO, upper_sys_uart_tx_buff);
	if (len > 0)
		HAL_UART_Transmit_DMA(&UPPER_SYS_HUART, upper_sys_uart_tx_buff, len);
}

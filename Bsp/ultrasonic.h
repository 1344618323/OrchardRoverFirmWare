#ifndef __ULTRASONIC_H__
#define __ULTRASONIC_H__

#include "usart.h"
#include "leonard_delay.h"
#include "bsp_uart.h"
#include "orchard_rover_sys.h"

#define L_KS109_HUART  huart2
#define R_KS109_HUART  huart3

#define L_SENSOR_ID 0
#define R_SENSOR_ID 1

#define KS109_UART_RX_MAX_BUFLEN  5
#define KS109_UART_TX_MAX_BUFLEN  5

typedef struct
{
    uint8_t id;
    int16_t range;
    uint8_t rec_flag;
} UltrasonicSensor;

void KS109_Uart_Callback_Handle(uint8_t *rx_buf, uint32_t len, uint8_t id);
void KS109_Init(void);
void Transmit_Cmd_All_KS109(uint8_t cmd);
void Read_KS109_Cmd(uint8_t id);
int16_t Read_KS109_Range(uint8_t id);
#endif

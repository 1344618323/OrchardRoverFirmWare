#ifndef __BSP_UART_H__
#define __BSP_UART_H__

#include"orchard_rover_sys.h"
#include "usart.h"
#include "protocol.h"
#include "ultrasonic.h"

#define UPPER_SYS_HUART  huart1
#define UPPER_SYS_UART_RX_MAX_BUFLEN  RX_MAX_SIZE_PC
#define UPPER_SYS_UART_TX_MAX_BUFLEN  TX_MAX_SIZE_PC

#define OBSERVE_SYS_HUART  huart3
#define OBSERVE_SYS_UART_RX_MAX_BUFLEN  RX_MAX_SIZE_PC
#define OBSERVE_SYS_UART_TX_MAX_BUFLEN  TX_MAX_SIZE_PC

#define JY61_HUART  huart2
#define JY61_UART_RX_MAX_BUFLEN  50

 typedef struct
 {
     double ax;
     double ay;
     double az;
     double wx;
     double wy;
     double wz;
     double roll;
     double pitch;
     double yaw;
 } JY61_Data;

void Sys_Uart_Init(void);
void Uart_Rx_Idle_Callback(UART_HandleTypeDef *huart);
void Upper_Sys_Uart_Callback_Handle(uint8_t*buff, uint32_t len);
void JY61_Uart_Callback_Handle(uint8_t*buff, uint32_t len);
void JY61_Set_Yaw_Zero(void);
double Get_Yaw_Angle(void);

void Transmit_Chassis_Msg(double *msg);
void Transmit_KS109_Cmd(uint8_t cmd, uint8_t id);
void Transmit_Observe_Msg(uint8_t *msg);
void Forward_Upper_Msg_To_Observe(uint8_t *msg, uint16_t len);

#endif

#ifndef __MOTOR_CON_H
#define __MOTOR_CON_H

#include "stm32f1xx_hal.h"
#include "pid.h"
#include "dac.h"
#include "bsp_uart.h"
#include <math.h>
#include <string.h>

#define L_MOT_ID 0
#define R_MOT_ID 1

typedef struct
{
    int8_t dir;
    int32_t cur_cnt;
    int32_t last_cnt;
    double cnt_per_mm;
    double delta_dis_mm; //单位时间内，距离变化 单位mm

    GPIO_TypeDef *enable_gpio;
    uint16_t enable_pin;
    GPIO_TypeDef *dir_gpio;
    uint16_t dir_pin;
    uint32_t dac_channel;

    int32_t last_val;
} MotorCon;

void MotorInit(void);
void EncoderDateCalc(void);
void MotorSetOut(MotorCon *motor, int32_t val);
void MotorTIMHandler(TIM_HandleTypeDef *htim);
void MotorStart(MotorCon *motor);
void MotorStop(MotorCon *motor);
void MotorGoAhead(MotorCon *motor);
void MotorGoBack(MotorCon *motor);
double pi_2_pi(double angle);

extern MotorCon motorCon[2];

#endif

#ifndef __MOTOR_CON_H
#define __MOTOR_CON_H

#include <math.h>
#include <string.h>
#include "bsp_uart.h"
#include "dac.h"
#include "pid.h"
#include "stm32f1xx_hal.h"

#define L_MOT_ID 0
#define R_MOT_ID 1

typedef struct {
    int8_t dir;
    int32_t cur_cnt;
    int32_t last_cnt;
    double cnt_per_mm;
    double delta_dis_mm;  //单位时间内，距离变化 单位mm

    GPIO_TypeDef* enable_gpio;
    uint16_t enable_pin;
    GPIO_TypeDef* dir_gpio;
    uint16_t dir_pin;
    uint32_t dac_channel;

    int32_t last_val;
} MotorCon;

void MotorInit(void);
void EncoderDateCalc(void);
void MotorSetOut(MotorCon* motor, int32_t val);
void MotorTIMHandler(TIM_HandleTypeDef* htim);
inline void MotorStart(MotorCon* motor);
inline void MotorStop(MotorCon* motor);
inline void MotorGoAhead(MotorCon* motor);
inline void MotorGoBack(MotorCon* motor);
double pi_2_pi(double angle);
void CmdExecute(double vl, double vr);
void CmdDecomposition(double v, double w); 

extern MotorCon motorCon[2];

extern double upper_vel[2];
#endif

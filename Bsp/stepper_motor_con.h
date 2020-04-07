#ifndef __STEPPER_MOTOR_CON_H
#define __STEPPER_MOTOR_CON_H

#include "tim.h"
#include "string.h"
#include "orchard_rover_sys.h"
#include "cmsis_os.h"

enum
{
  eNoRotating = 0,
  eRotating = 1,
};

enum{
  eCantRotate = 0,
  eCantReach = 1,
  eSuceess =2
};

typedef struct
{

  double angle_per_cnt; // 每个脉冲对应的角度

  TIM_HandleTypeDef *pwm_tim;
  uint32_t pwm_channel;
  uint32_t pwm_it_channel;

  TIM_HandleTypeDef *cap_tim;
  uint32_t cap_channel;
  uint32_t cap_it_channel;

  GPIO_TypeDef *dir_gpio;
  uint16_t dir_pin;

  int32_t cur_cnt;
  int32_t target_cnt;
  uint8_t zero_flag;
  uint8_t rotating;
  int8_t dir;
  double angle;
  int32_t bias_cnt;
} StepperMotorCon;

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim);
void StepperMotorICTIMHandler(TIM_HandleTypeDef *htim);
void StepperMotorInit(void);
void StepperMotorRotateHandle(StepperMotorCon *motroCon);
void StepperMotorRotate(int16_t angle, uint8_t id);
void StepperMotorZero(void);
int8_t StepperMotorXRotate(int16_t angle, uint8_t id);
extern StepperMotorCon stepMotCon[2];
#endif

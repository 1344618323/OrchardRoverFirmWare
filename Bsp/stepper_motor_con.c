#include "stepper_motor_con.h"

StepperMotorCon stepMotCon[2];

#define L_ZERO_BIAS -3.0
#define R_ZERO_BIAS -19.0

enum
{
	eNoZero = 0,
	eZeroing = 1,
	eZeroDone = 2,
	eZeroBias = 3
};


void StepperMotorInit(void)
{
	memset(&stepMotCon[L_SENSOR_ID], 0, sizeof(stepMotCon));
	memset(&stepMotCon[R_SENSOR_ID], 0, sizeof(stepMotCon));

	stepMotCon[L_SENSOR_ID].dir = 1;
	stepMotCon[L_SENSOR_ID].dir_gpio = GPIOC;
	stepMotCon[L_SENSOR_ID].dir_pin = GPIO_PIN_4;
	stepMotCon[L_SENSOR_ID].pwm_tim = &htim3;
	stepMotCon[L_SENSOR_ID].pwm_channel = TIM_CHANNEL_1;
	stepMotCon[L_SENSOR_ID].pwm_it_channel = HAL_TIM_ACTIVE_CHANNEL_1;
	stepMotCon[L_SENSOR_ID].cap_tim = &htim2;
	stepMotCon[L_SENSOR_ID].cap_channel = TIM_CHANNEL_4;
	stepMotCon[L_SENSOR_ID].cap_it_channel = HAL_TIM_ACTIVE_CHANNEL_4;
	stepMotCon[L_SENSOR_ID].angle_per_cnt = 0.1125;
	stepMotCon[L_SENSOR_ID].bias_cnt = (int32_t)(L_ZERO_BIAS / stepMotCon[L_SENSOR_ID].angle_per_cnt);

	stepMotCon[R_SENSOR_ID].dir = 1;
	stepMotCon[R_SENSOR_ID].dir_gpio = GPIOC;
	stepMotCon[R_SENSOR_ID].dir_pin = GPIO_PIN_2;
	stepMotCon[R_SENSOR_ID].pwm_tim = &htim5;
	stepMotCon[R_SENSOR_ID].pwm_channel = TIM_CHANNEL_1;
	stepMotCon[R_SENSOR_ID].pwm_it_channel = HAL_TIM_ACTIVE_CHANNEL_1;
	stepMotCon[R_SENSOR_ID].cap_tim = &htim2;
	stepMotCon[R_SENSOR_ID].cap_channel = TIM_CHANNEL_3;
	stepMotCon[R_SENSOR_ID].cap_it_channel = HAL_TIM_ACTIVE_CHANNEL_3;
	stepMotCon[R_SENSOR_ID].angle_per_cnt = 0.1125;
	stepMotCon[R_SENSOR_ID].bias_cnt = (int32_t)(R_ZERO_BIAS / stepMotCon[R_SENSOR_ID].angle_per_cnt);

	HAL_TIM_IC_Start_IT(stepMotCon[L_SENSOR_ID].cap_tim, stepMotCon[L_SENSOR_ID].cap_channel);
	HAL_TIM_IC_Start_IT(stepMotCon[R_SENSOR_ID].cap_tim, stepMotCon[R_SENSOR_ID].cap_channel);
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
	if (htim == stepMotCon[L_SENSOR_ID].pwm_tim &&
		htim->Channel == stepMotCon[L_SENSOR_ID].pwm_it_channel)
	{
		StepperMotorRotateHandle(&(stepMotCon[L_SENSOR_ID]));
	}
	else if (htim == stepMotCon[R_SENSOR_ID].pwm_tim &&
			 htim->Channel == stepMotCon[R_SENSOR_ID].pwm_it_channel)
	{
		StepperMotorRotateHandle(&(stepMotCon[R_SENSOR_ID]));
	}
}

void StepperMotorICTIMHandler(TIM_HandleTypeDef *htim)
{
	if (htim == stepMotCon[L_SENSOR_ID].cap_tim &&
		htim->Channel == stepMotCon[L_SENSOR_ID].cap_it_channel)
	{
		if (stepMotCon[L_SENSOR_ID].zero_flag == eZeroing)
		{
			HAL_TIM_PWM_Stop_IT(stepMotCon[L_SENSOR_ID].pwm_tim, stepMotCon[L_SENSOR_ID].pwm_channel);
			stepMotCon[L_SENSOR_ID].zero_flag = eZeroDone;
		}
	}
	else if (htim == stepMotCon[R_SENSOR_ID].cap_tim &&
			 htim->Channel == stepMotCon[R_SENSOR_ID].cap_it_channel)
	{
		if (stepMotCon[R_SENSOR_ID].zero_flag == eZeroing)
		{
			HAL_TIM_PWM_Stop_IT(stepMotCon[R_SENSOR_ID].pwm_tim, stepMotCon[R_SENSOR_ID].pwm_channel);
			stepMotCon[R_SENSOR_ID].zero_flag = eZeroDone;
		}
	}
}

void StepperMotorRotateHandle(StepperMotorCon *motroCon)
{
	if (motroCon->zero_flag >= eZeroDone)
	{
		motroCon->cur_cnt += motroCon->dir;
		motroCon->angle = motroCon->angle_per_cnt * (motroCon->cur_cnt + motroCon->bias_cnt);
		if (motroCon->target_cnt == motroCon->cur_cnt && motroCon->rotating)
		{
			motroCon->rotating = eNoRotating;
			HAL_TIM_PWM_Stop_IT(motroCon->pwm_tim, motroCon->pwm_channel);

			if (motroCon->zero_flag == eZeroDone)
				motroCon->zero_flag = eZeroBias;
		}
	}
}

void StepperMotorRotate(int16_t angle, uint8_t id)
{
	if (angle == 0 || stepMotCon[id].zero_flag < eZeroDone)
		return;
	//stepMotCon[id].target_cnt = angle/ stepMotCon[id].angle_per_cnt + stepMotCon[id].cur_cnt;
	stepMotCon[id].target_cnt = (stepMotCon[id].angle + angle) / stepMotCon[id].angle_per_cnt - stepMotCon[id].bias_cnt;
	if (stepMotCon[id].target_cnt >= stepMotCon[id].cur_cnt)
	{
		stepMotCon[id].dir = 1;
		HAL_GPIO_WritePin(stepMotCon[id].dir_gpio, stepMotCon[id].dir_pin, GPIO_PIN_SET);
	}
	else
	{
		stepMotCon[id].dir = -1;
		HAL_GPIO_WritePin(stepMotCon[id].dir_gpio, stepMotCon[id].dir_pin, GPIO_PIN_RESET);
	}
	if (!stepMotCon[id].rotating)
	{
		HAL_TIM_PWM_Start_IT(stepMotCon[id].pwm_tim, stepMotCon[id].pwm_channel);
		stepMotCon[id].rotating = eRotating;
	}
}

int8_t StepperMotorXRotate(int16_t angle, uint8_t id)
{
	if (stepMotCon[id].zero_flag < eZeroBias)
		return eCantRotate;

	if (angle == 0)
		return eSuceess;

	int16_t val = angle + stepMotCon[id].angle;
	if (val < -80)
	{
		val = (int16_t)(-80 - stepMotCon[id].angle);
		StepperMotorRotate(val, id);
		return eCantReach;
	}
	else if (val > 80)
	{
		StepperMotorRotate((int16_t)(80 - stepMotCon[id].angle), id);
		return eCantReach;
	}
	StepperMotorRotate(angle, id);
	return eSuceess;
}

void StepperMotorZero(void)
{
	stepMotCon[L_SENSOR_ID].zero_flag=eZeroBias;// 作弊了
	if (stepMotCon[L_SENSOR_ID].zero_flag == eNoZero)
	{
		stepMotCon[L_SENSOR_ID].dir = 1;
		HAL_GPIO_WritePin(stepMotCon[L_SENSOR_ID].dir_gpio, stepMotCon[L_SENSOR_ID].dir_pin, GPIO_PIN_SET);
		HAL_TIM_PWM_Start_IT(stepMotCon[L_SENSOR_ID].pwm_tim, stepMotCon[L_SENSOR_ID].pwm_channel);
		stepMotCon[L_SENSOR_ID].zero_flag = eZeroing;
	}

	if (stepMotCon[R_SENSOR_ID].zero_flag == eNoZero)
	{
		stepMotCon[R_SENSOR_ID].dir = -1;
		HAL_GPIO_WritePin(stepMotCon[R_SENSOR_ID].dir_gpio, stepMotCon[R_SENSOR_ID].dir_pin, GPIO_PIN_RESET);
		HAL_TIM_PWM_Start_IT(stepMotCon[R_SENSOR_ID].pwm_tim, stepMotCon[R_SENSOR_ID].pwm_channel);
		stepMotCon[R_SENSOR_ID].zero_flag = eZeroing;
	}

	// 等待两个电机都触碰到行程开关
	for (int i = 1; i < 2; i++)// 作弊了
	{
		while (stepMotCon[i].zero_flag != eZeroDone)
		{
			osDelay(10);
		}
	}
	osDelay(50);

	stepMotCon[L_SENSOR_ID].angle = stepMotCon[L_SENSOR_ID].angle_per_cnt * (stepMotCon[L_SENSOR_ID].cur_cnt + stepMotCon[L_SENSOR_ID].bias_cnt);
	stepMotCon[R_SENSOR_ID].angle = stepMotCon[R_SENSOR_ID].angle_per_cnt * (stepMotCon[R_SENSOR_ID].cur_cnt + stepMotCon[R_SENSOR_ID].bias_cnt);
	StepperMotorRotate(-stepMotCon[L_SENSOR_ID].bias_cnt * stepMotCon[L_SENSOR_ID].angle_per_cnt, L_SENSOR_ID);
	StepperMotorRotate(-stepMotCon[R_SENSOR_ID].bias_cnt * stepMotCon[R_SENSOR_ID].angle_per_cnt, R_SENSOR_ID);

	// 等待两个电机都bias结束
	for (int i = 1; i < 2; i++)// 作弊了
	{
		while (stepMotCon[i].zero_flag != eZeroBias)
		{
			osDelay(10);
		}
	}
}

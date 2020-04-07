#include "motor_con.h"

MotorCon motorCon[2];

int32_t x = 0;
int32_t y = 0;
int32_t theta = 0;
double pose[3] = {0.0, 0.0, 0.0}; //mm,mm,deg
#define D 0.301
// 两轮之间距离为 2D
#define pi 3.141592653589793

void MotorInit(void)
{
    memset(&motorCon[L_MOT_ID], 0, sizeof(MotorCon));
    memset(&motorCon[R_MOT_ID], 0, sizeof(MotorCon));

    motorCon[L_MOT_ID].enable_gpio = GPIOA;
    motorCon[L_MOT_ID].enable_pin = GPIO_PIN_11;
    motorCon[L_MOT_ID].dir_gpio = GPIOC;
    motorCon[L_MOT_ID].dir_pin = GPIO_PIN_7;
    motorCon[L_MOT_ID].dir = 1;
    motorCon[L_MOT_ID].dac_channel = DAC_CHANNEL_1;
    motorCon[L_MOT_ID].cnt_per_mm = 728.0/ 2040; //每mm几个脉冲

    motorCon[R_MOT_ID].enable_gpio = GPIOA;
    motorCon[R_MOT_ID].enable_pin = GPIO_PIN_12;
    motorCon[R_MOT_ID].dir_gpio = GPIOC;
    motorCon[R_MOT_ID].dir_pin = GPIO_PIN_8;
    motorCon[R_MOT_ID].dir = 1;
    motorCon[R_MOT_ID].dac_channel = DAC_CHANNEL_2;
    motorCon[R_MOT_ID].cnt_per_mm = 728.0 / 2040;

    MotorStop(&motorCon[L_MOT_ID]);
    MotorStop(&motorCon[R_MOT_ID]);
    MotorGoAhead(&motorCon[L_MOT_ID]);
    MotorGoAhead(&motorCon[R_MOT_ID]);
    MotorSetOut(&motorCon[L_MOT_ID], 0);
    MotorSetOut(&motorCon[R_MOT_ID], 0);
}

void MotorStart(MotorCon *motor)
{
    HAL_GPIO_WritePin(motor->enable_gpio, motor->enable_pin, GPIO_PIN_SET);
}

void MotorStop(MotorCon *motor)
{
    HAL_GPIO_WritePin(motor->enable_gpio, motor->enable_pin, GPIO_PIN_RESET);
}

void MotorGoAhead(MotorCon *motor)
{
    motor->dir = 1;
    HAL_GPIO_WritePin(motor->dir_gpio, motor->dir_pin, GPIO_PIN_RESET);
}

void MotorGoBack(MotorCon *motor)
{
    motor->dir = -1;
    HAL_GPIO_WritePin(motor->dir_gpio, motor->dir_pin, GPIO_PIN_SET);
}

void MotorTIMHandler(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1)
    {
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
        {
            if (motorCon[L_MOT_ID].dir == 1)
                motorCon[L_MOT_ID].cur_cnt++;
            else if (motorCon[L_MOT_ID].dir == -1)
                motorCon[L_MOT_ID].cur_cnt--;
        }
    }
    else if (htim->Instance == TIM8)
    {
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
        {
            if (motorCon[R_MOT_ID].dir == 1)
                motorCon[R_MOT_ID].cur_cnt++;
            else if (motorCon[R_MOT_ID].dir == -1)
                motorCon[R_MOT_ID].cur_cnt--;
        }
    }
}

uint32_t last_calc_vec_time;
void EncoderDateCalc(void)
{
    // delta脉冲/每mm对应的脉冲数 = delta_mm
    motorCon[L_MOT_ID].delta_dis_mm = (motorCon[L_MOT_ID].cur_cnt - motorCon[L_MOT_ID].last_cnt) / motorCon[L_MOT_ID].cnt_per_mm;
    motorCon[R_MOT_ID].delta_dis_mm = (motorCon[R_MOT_ID].cur_cnt - motorCon[R_MOT_ID].last_cnt) / motorCon[R_MOT_ID].cnt_per_mm;

    motorCon[L_MOT_ID].last_cnt = motorCon[L_MOT_ID].cur_cnt;
    motorCon[R_MOT_ID].last_cnt = motorCon[R_MOT_ID].cur_cnt;

    double delta_d = (motorCon[L_MOT_ID].delta_dis_mm + motorCon[R_MOT_ID].delta_dis_mm) / 2.0;
    pose[2] = Get_Yaw_Angle();
    double yaw_angle_rad = pose[2] * pi / 180.0;
    pose[0] += cos(yaw_angle_rad) * delta_d;
    pose[1] += sin(yaw_angle_rad) * delta_d;

    Transmit_Chassis_Msg(pose);

    // uint32_t cur_time = HAL_GetTick();
    // if (last_calc_vec_time == 0)
    // {
    //     last_calc_vec_time = cur_time;
    //     return;
    // }
    // uint32_t delta_time = cur_time - last_calc_vec_time;
    // last_calc_vec_time = cur_time;

    // delta_mm/delta_time(ms)=速度(m/s)
    // OdomModelByVec(motorCon[L_MOT_ID].delta_dis_mm/delta_time, motorCon[L_MOT_ID].delta_dis_mm/delta_time, delta_time);
}

void MotorSetOut(MotorCon *motor, int32_t val)
{
    //val=[-4095,4095]
    VAL_LIMIT(val, -4095, 4095);
    if (motor->last_val != 0 && val == 0)
    {
        MotorStop(motor);
    }
    else if (motor->last_val == 0 && val != 0)
    {
        MotorStart(motor);
    }

    if (motor->last_val >= 0 && val < 0)
    {
        MotorGoBack(motor);
    }
    else if (motor->last_val <= 0 && val > 0)
    {
        MotorGoAhead(motor);
    }

    HAL_DAC_SetValue(&hdac, motor->dac_channel, DAC_ALIGN_12B_R, MyAbs(val));
}

double pi_2_pi(double angle)
{
    return fmod(angle + 5 * pi, 2 * pi) - pi;
}

/************* 速度推算轨迹 *************/
void OdomModelByVec(double vl, double vr, uint32_t delta_time)
{
    double v, w;
    v = (vr + vl) / 2.0;
    w = (vr - vl) / 2.0 / D;
    pose[0] = pose[0] + cos(pose[2]) * v * 0.05;
    pose[1] = pose[1] + sin(pose[2]) * v * 0.05;
    pose[2] = pose[2] + w * 0.05;
    pose[2] = pi_2_pi(pose[2]);

    x = pose[0] * 1000;
    y = pose[1] * 1000;
    theta = pose[2] * 1000;

    // //    pose[0] = pose[0] - v / w * sin(pose[2]) + v / w * sin(pose[2] + w * delta_time);
    // //    pose[1] = pose[1] + v / w * cos(pose[2]) - v / w * cos(pose[2] + w * delta_time);
    // //    pose[2] = pose[2] + w * delta_time;
}

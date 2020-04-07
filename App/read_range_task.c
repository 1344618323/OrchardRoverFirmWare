#include "read_range_task.h"

enum
{
  eFree = 0,
  eFakeWorking = 1,
  eWorking = 2,
  eDone = 3
};

typedef struct
{
  uint8_t working;
  uint8_t valid;
  int16_t bearing;
  int16_t range;
  uint32_t cmd_time;
} MotorTask;

MotorTask motorTask[2];

int16_t test_angle1, test_angle2;

cmd_observe_info observe_info;

void Read_Range_Task(void const *argument)
{
  KS109_Init();
  StepperMotorInit();
  StepperMotorZero();

  while (1)
  {
    osDelay(10);

    /***********测试电机***********/
    // if (test_angle1 != 0)
    // {
    //   StepperMotorXRotate(test_angle1, 0);
    //   test_angle1 = 0;
    // }

    // if (test_angle2 != 0)
    // {
    //   StepperMotorXRotate(test_angle2, 1);
    //   test_angle2 = 0;
    // }

    /***********测试超声波***********/
    // Read_KS109_Cmd(1);
    // while(Read_KS109_Range(1)==-1){

    // }
    // motorTask[1].range=Read_KS109_Range(1);

    // 有转角命令，判定是否在电机活动范围内范围内
    // cmd_cam_angle中valid: 1表示转电机; 2 表示转电机并测量
    if (cam_angle.exec_flag)
    {
      cam_angle.exec_flag = 0;
      for (int i = 0; i < 2; i++)
      {
        if (cam_angle.valid[i])
        {
          if (motorTask[i].working == eDone && MyAbs(cam_angle.angle[i]) <= 7.5)
            continue;

          uint8_t user_val = StepperMotorXRotate(cam_angle.angle[i], i);
          if (user_val == eSuceess && cam_angle.valid[i] == 2)
          {
            motorTask[i].working = eWorking;
          }
          else if (user_val == eCantReach || (user_val == eSuceess && cam_angle.valid[i] == 1))
          {
            motorTask[i].working = eFakeWorking;
          }
        }
      }
    }

    // 电机运动到目标位置
    for (int i = 0; i < 2; i++)
    {
      if (motorTask[i].working == eWorking && stepMotCon[i].rotating == eNoRotating)
      {
        motorTask[i].working = eDone;
        Read_KS109_Cmd(i);
        motorTask[i].cmd_time = HAL_GetTick();
      }
      else if (motorTask[i].working == eFakeWorking && stepMotCon[i].rotating == eNoRotating)
      {
        motorTask[i].working = eFree;
      }
    }

    // 等待超声波测量完成
    uint8_t transmit_sign = 0;
    for (int i = 0; i < 2; i++)
    {
      if (motorTask[i].working == eDone)
      {
        int16_t range_val = Read_KS109_Range(i);
        if (range_val != -1)
        {
          motorTask[i].valid = 1;
          motorTask[i].bearing = (int16_t)(stepMotCon[i].angle);
          motorTask[i].range = range_val;
          motorTask[i].working = eFree;
          transmit_sign = 1;
          continue;
        }
        if (HAL_GetTick() > (motorTask[i].cmd_time + 200))
        {
          motorTask[i].valid = 1;
          motorTask[i].bearing = (int16_t)(stepMotCon[i].angle);
          motorTask[i].range = -1;
          motorTask[i].working = eFree;
          transmit_sign = 1;
        }
      }
    }

    // 完成测量，发送测量数据
    if (transmit_sign)
    {
      for (int i = 0; i < 2; i++)
      {
        if (motorTask[i].valid == 1)
        {
          observe_info.valid[i] = 2;
          observe_info.bearing[i] = motorTask[i].bearing;
          observe_info.range[i] = motorTask[i].range;
          motorTask[i].valid = 0;
        }
        else
        {
          observe_info.valid[i] = 1;
          observe_info.bearing[i] = (int16_t)(stepMotCon[i].angle);
        }
      }
      Transmit_Observe_Msg((uint8_t *)(&observe_info));
    }

    // 完成旋转，发送数据
    static uint8_t last_work_sta[2] = {eFree, eFree};
    uint8_t transmit_sign2 = 0;
    for (int i = 0; i < 2; i++)
    {
      if ((last_work_sta[i] == eWorking || last_work_sta[i] == eFakeWorking) &&
          motorTask[i].working != last_work_sta[i])
      {
        transmit_sign2 = 1;
        observe_info.valid[i] = 1;
        observe_info.bearing[i] = (int16_t)(stepMotCon[i].angle);
      }
      last_work_sta[i] = motorTask[i].working;
    }
    if (transmit_sign2)
    {
      Transmit_Observe_Msg((uint8_t *)(&observe_info));
    }
  }
}

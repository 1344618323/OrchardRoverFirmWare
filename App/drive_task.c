#include "drive_task.h"

float vl = 0, vr = 0;
float scale_factor = 12.0;
// 输出模拟电压2400mv对应1m/s的速度, 只要保证 250(255) * scale_factor ~= 2400 即可

void Drive_Task(void const *argument)
{
    osDelay(100);
    uint8_t start_flag = 0;
    while (1)
    {
        uint8_t cmd = GetRemoteCmd();
        if (cmd != 0)
        {
            switch (cmd)
            {
            case 1:
                start_flag = 1; // 启动
                break;
            case 2:
                start_flag = 0; // 暂停
                break;
            case 3:
                start_flag = 1;
                vl = -40; // 左转
                vr = 40;
                break;
            case 4:
                start_flag = 1;
                vl = 40; // 右转
                vr = -40;
                break;
            case 5:
                start_flag = 1;
                vl = vr = -80; // 倒车
                break;
            case 6:
                start_flag = 1;
                vl = vr = 80; // 前进
                break;
            case 7:

                break;
            case 8:

                break;
            default:
                break;
            }

            MotorSetOut(&motorCon[L_MOT_ID], (int32_t)(vl * scale_factor * start_flag));
            MotorSetOut(&motorCon[R_MOT_ID], (int32_t)(vr * scale_factor * start_flag));
        }
        osDelay(50);
    }
}

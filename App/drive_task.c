#include "drive_task.h"

int velocity;
int dir[2];
float scale_factor = 12.0;

int max_velocity = 200;

// 输出模拟电压2400mv对应1m/s的速度, 只要保证 250(255) * scale_factor ~= 2400
// 即可
/*
 * 1 启动
 * 2 暂停
 * 3 左转
 * 4 右转
 * 5 倒车
 * 6 前进
 */

void Drive_Task(void const* argument) {
    osDelay(100);
    while (1) {
        uint8_t cmd = GetRemoteCmd();
        if (cmd != 0) {
            switch (cmd) {
                case 1:
                    // 启动
                    dir[0] = dir[1] = 1;
                    break;
                case 2:
                    // 暂停
                    dir[0] = dir[1] = 0;
                    velocity = 0;
                    break;
                case 3:
                    // 左转
                    dir[0] = -1;
                    dir[1] = 1;
                    velocity = 40;
                    break;
                case 4:
                    // 右转
                    dir[0] = 1;
                    dir[1] = -1;
                    velocity = 40;
                    break;
                case 5:
                    dir[0] = -1;
                    dir[1] = -1;
                    velocity = 80;
                    break;
                case 6:
                    // 前进
                    dir[0] = 1;
                    dir[1] = 1;
                    velocity = 80;
                    break;
                case 7:
                    // 加速
                    velocity *= 2;
                    if (velocity > max_velocity)
                        velocity = max_velocity;
                    break;
                case 8:
                    // 减速
                    velocity /= 2;
                    break;
                default:
                    break;
            }

            MotorSetOut(&motorCon[L_MOT_ID],
                        (int32_t)(dir[0] * velocity * scale_factor));
            MotorSetOut(&motorCon[R_MOT_ID],
                        (int32_t)(dir[1] * velocity * scale_factor));
        }
        osDelay(50);
    }
}

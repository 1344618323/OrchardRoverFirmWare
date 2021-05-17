#include "drive_task.h"
/*
 * 1 切模式
 * 2 暂停 0,0
 * 3 左转 重置-0.2 0.2
 * 4 右转 重置0.2 -0.2
 * 5 倒车 重置-0.4 -0.4
 * 6 前进 重置0.4 0.4
 * 7 加速 *2 <1.5
 * 8 减速 /2 >0.1
 */

float velocity[2] = {0, 0};
uint8_t upper_flag = 0;

void Drive_Task(void const* argument) {
    osDelay(100);
    while (1) {
        uint8_t cmd = GetRemoteCmd();
        if (cmd != 0) {
            switch (cmd) {
                case 1:
                    // 启动
                    upper_flag = !upper_flag;
                    velocity[0] = 0.0;
                    velocity[1] = 0.0;
                    break;
                case 2:
                    // 暂停
                    velocity[0] = 0.0;
                    velocity[1] = 0.0;
                    break;
                case 3:
                    // 左转
                    velocity[0] = -0.2;
                    velocity[1] = 0.2;
                    break;
                case 4:
                    // 右转
                    velocity[0] = 0.2;
                    velocity[1] = -0.2;
                    break;
                case 5:
                    velocity[0] = -0.4;
                    velocity[1] = -0.4;
                    break;
                case 6:
                    // 前进
                    velocity[0] = 0.4;
                    velocity[1] = 0.4;
                    break;
                case 8:
                    // 加速
                    for (int i = 0; i < 2; i++) {
                        velocity[i] *= 2;
                        if (velocity[i] > MAX_VEL)
                            velocity[i] = MAX_VEL;
                        else if (velocity[i] < -MAX_VEL)
                            velocity[i] = -MAX_VEL;
                    }
                    break;
                case 7:
                    // 减速
                    for (int i = 0; i < 2; i++) {
                        velocity[i] /= 2;
                        if (velocity[i] < MIN_VEL && velocity[i] > 0)
                            velocity[i] = MIN_VEL;
                        else if (velocity[i] > -MIN_VEL && velocity[i] < 0)
                            velocity[i] = -MIN_VEL;
                    }
                    break;
                default:
                    break;
            }
            if (!upper_flag)
                CmdExecute(velocity[0], velocity[1]);
        }

        if (upper_flag) {
            CmdExecute(upper_vel[0], upper_vel[1]);
        }

        osDelay(50);
    }
}

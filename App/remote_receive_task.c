#include "remote_receive_task.h"

uint8_t remote_cmd = 0;

GPIO_PinState last_p[8][2];

void Remote_Receive_Task(void const *argument)
{
    while (1)
    {
        uint8_t i = 0;

        osDelay(10);

        GPIO_PinState p[8];
        p[0] = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_7);
        p[1] = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_8);
        p[2] = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_9);
        p[3] = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_10);
        p[4] = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_11);
        p[5] = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_12);
        p[6] = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_13);
        p[7] = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14);

        for (i = 0; i < 8; i++)
        {
            if (last_p[i][0] != GPIO_PIN_SET && last_p[i][1] == GPIO_PIN_SET && p[i] == GPIO_PIN_SET)
            {
                remote_cmd = i + 1;
            }
        }
        for (i = 0; i < 8; i++)
        {
            last_p[i][0] = last_p[i][1];
            last_p[i][1] = p[i];
        }
    }
}

uint8_t GetRemoteCmd(void)
{
    uint8_t return_cmd = remote_cmd;
    remote_cmd = 0;
    return return_cmd;
}

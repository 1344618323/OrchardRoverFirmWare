#ifndef __REMOTE_RECEIVE_TASK_H
#define __REMOTE_RECEIVE_TASK_H

#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

void Remote_Receive_Task(void const * argument);

uint8_t GetRemoteCmd(void);

#endif

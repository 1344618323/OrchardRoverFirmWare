#ifndef __LEONARD_DELAY_H__
#define __LEONARD_DELAY_H__

#include "stm32f1xx.h"
#include "freertos.h"
#include "task.h"

void LeonardDelayInit(uint8_t SYSCLK);
void LeonardDelayUs(uint32_t nus);
void LeonardDelayXMs(uint32_t nms);
void LeonardDelayMs(uint32_t nms);

#endif

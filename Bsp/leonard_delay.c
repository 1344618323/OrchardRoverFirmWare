#include "leonard_delay.h"

static uint32_t fac_us = 0;
static uint16_t fac_ms = 0;

void LeonardDelayInit(uint8_t SYSCLK)
{
	fac_us = SYSCLK;
	fac_ms = 1000 / configTICK_RATE_HZ; //代表OS可以延时的最少单位
}

//延时nus
//nus:要延时的us数.	
//nus:0~190887435(最大值即2^32/fac_us@fac_us=22.5)	  
void LeonardDelayUs(uint32_t nus)
{
	taskENTER_CRITICAL();
	uint32_t ticks;
	uint32_t told, tnow, tcnt = 0;
	uint32_t reload = SysTick->LOAD; //LOAD的值
	ticks = nus * fac_us;			 //需要的节拍数
	told = SysTick->VAL;			 //刚进入时的计数器值
	while (1)
	{
		tnow = SysTick->VAL;
		if (tnow != told)
		{
			if (tnow < told)
				tcnt += told - tnow; //这里注意一下SYSTICK是一个递减的计数器就可以了.
			else
				tcnt += reload - tnow + told;
			told = tnow;
			if (tcnt >= ticks)
				break; //时间超过/等于要延迟的时间,则退出.
		}
	};
	taskEXIT_CRITICAL();
}

//延时nms,不会引起任务调度
//nms:要延时的ms数
void LeonardDelayXMs(uint32_t nms)
{
	uint32_t i;
	for (i = 0; i < nms; i++)
		LeonardDelayUs(1000);
}

//延时nms,会引起任务调度
//nms:要延时的ms数
//nms:0~65535
void LeonardDelayMs(uint32_t nms)
{
	if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) //系统已经运行
	{
		if (nms >= fac_ms) //延时的时间大于OS的最少时间周期
		{
			vTaskDelay(nms / fac_ms); //FreeRTOS延时
		}
		nms %= fac_ms; //OS已经无法提供这么小的延时了,采用普通方式延时
	}
	LeonardDelayUs((uint32_t)(nms * 1000)); //普通方式延时
}

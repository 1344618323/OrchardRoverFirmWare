#include "encoder_task.h"
void Encoder_Task(void const *argument)
{
	while (1)
	{
		EncoderDateCalc();
		osDelay(50);
	}
}

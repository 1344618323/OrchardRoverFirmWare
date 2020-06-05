#ifndef __ORCHARD_ROVER_SYS_H
#define __ORCHARD_ROVER_SYS_H

#include "stm32f1xx.h"

#define VAL_LIMIT(val, min, max) \
    if (val <= min)              \
    {                            \
        val = min;               \
    }                            \
    else if (val >= max)         \
    {                            \
        val = max;               \
    }

#define MyAbs(x) ((x > 0) ? (x) : (-x))

extern uint8_t chassis_mode; 

#endif

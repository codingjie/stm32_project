#ifndef __TIM_H
#define __TIM_H

#include "stm32f10x.h"
#include "stm32f10x_tim.h"

extern uint16_t duty;  // PWM占空比 0-100

void TIM2_Init(void);

#endif

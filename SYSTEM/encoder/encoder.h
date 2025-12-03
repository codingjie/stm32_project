#ifndef __ENCODER_H
#define __ENCODER_H

#include "stm32f10x.h"
#include "stm32f10x_tim.h"

#define ENCODER_PPR 24  // 每圈脉冲数

extern volatile float motor_rpm;
extern volatile uint8_t speed_update_flag;

void Encoder_Init(void);

#endif

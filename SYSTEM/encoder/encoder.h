#ifndef __ENCODER_H
#define __ENCODER_H

#include "stm32f10x.h"
#include "usart.h"
#include "stm32f10x_tim.h"

#define MOTOR_PULSES_PER_REV 24  // 编码器每圈发出的脉冲数

void Encoder_Init(void);

#endif

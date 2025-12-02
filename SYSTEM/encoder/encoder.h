#ifndef __ENCODER_H
#define __ENCODER_H

#include "stm32f10x.h"

// 编码器每圈产生的脉冲数（根据实际电机修改）
#define ENCODER_PPR 20

void Encoder_Init(void);
float Get_Motor_RPM(void);
float Get_Motor_Speed_Hz(void);

#endif

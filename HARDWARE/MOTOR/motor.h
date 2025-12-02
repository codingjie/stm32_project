#ifndef __MOTOR_H
#define __MOTOR_H

#include "stm32f10x.h"
#include "stm32f10x_tim.h"

#define MOTOR_DC_IN1 GPIO_Pin_4
#define MOTOR_DC_IN2 GPIO_Pin_5
#define MOTOR_GPIO_PORT GPIOA
#define MOTOR_GPIO_CLK RCC_APB2Periph_GPIOA
#define MOTOR_AFIO_CLK RCC_APB2Periph_AFIO

#define MOTOR_GPIO_PWM GPIO_Pin_6
#define MOTOR_PWM_CLK RCC_APB1Periph_TIM3

void MOTOR_Init(void);

#endif

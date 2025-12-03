#include "motor.h"

void MOTOR_Init(void) {
    /*-------- 配置电机GPIO --------*/
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(MOTOR_GPIO_CLK | MOTOR_AFIO_CLK, ENABLE);

    GPIO_InitStructure.GPIO_Pin = MOTOR_DC_IN1 | MOTOR_DC_IN2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MOTOR_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = MOTOR_GPIO_PWM;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MOTOR_GPIO_PORT, &GPIO_InitStructure);

    GPIO_SetBits(MOTOR_GPIO_PORT, MOTOR_DC_IN1);
    GPIO_ResetBits(MOTOR_GPIO_PORT, MOTOR_DC_IN2);

    /*-------- 配置PWM --------*/
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    // 使能时钟
    RCC_APB1PeriphClockCmd(MOTOR_PWM_CLK, ENABLE);

	// 定时器基本配置（系统时钟 8MHz）
	TIM_TimeBaseStructure.TIM_Prescaler = 799;  // 预分频值：8MHz / (799+1) = 10kHz 计数频率
	TIM_TimeBaseStructure.TIM_Period = 99;      // 自动重载值：10kHz / (99+1) = 100Hz PWM 频率
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; // 无输入滤波分频
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	
	// PWM 输出配置（通道2，对应 TIM3_CH2）
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;  // PWM 模式1：CNT < CCR 时输出高电平
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // 使能通道输出
	TIM_OCInitStructure.TIM_Pulse = 0; // 初始比较值CCR
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; // 输出高电平有效
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);

    // 使能预装载寄存器
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable); // 启用CCR预装载
    TIM_ARRPreloadConfig(TIM3, ENABLE); // 启用ARR预装载
	
    // 启动定时器
    TIM_Cmd(TIM3, ENABLE);
}

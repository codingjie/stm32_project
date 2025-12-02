#include "tim.h"

uint16_t duty = 0;        // PWM占空比值，范围0-100，对应0%-100%
int8_t direction = 1;     // 占空比变化方向，1=增加，-1=减少

void TIM2_Init(void) {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // 使能TIM2时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
    // TIM2时基配置
    TIM_TimeBaseStructure.TIM_Prescaler = 7199;         // 72MHz/7200 = 10kHz
    TIM_TimeBaseStructure.TIM_Period = 199;            // 10kHz/200 = 50Hz (20ms)
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    
    // 使能TIM2更新中断
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    
    // NVIC配置
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // 启动定时器
    TIM_Cmd(TIM2, ENABLE);
}

// 定时器中断
void TIM2_IRQHandler(void) {
    if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        
        // 更新PWM占空比
        TIM_SetCompare1(TIM3, duty);
        
        // 调整占空比
        duty += direction;
        
        if(duty >= 100) direction = -1;  // 到最大，开始减小
        if(duty <= 0) direction = 1;      // 到最小，开始增加
    }
}

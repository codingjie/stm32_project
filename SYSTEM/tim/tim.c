#include "tim.h"

uint16_t duty = 0;
int8_t direction = 1;

void TIM1_Init(void) {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // 使能TIM1时钟（注意：TIM1在APB2上）
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    
    // TIM1时基配置
    TIM_TimeBaseStructure.TIM_Prescaler = 799;  // 预分频值：8MHz / 800 = 10kHz 定时器时钟
    TIM_TimeBaseStructure.TIM_Period = 1999;    // 自动重装值：10kHz / 2000 = 5Hz 中断频率（200ms周期）
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;  // TIM1特有，必须设置
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
    
    // 使能TIM1更新中断
    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
    
    // NVIC配置（注意：TIM1更新中断是TIM1_UP_IRQn）
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // 启动定时器
    TIM_Cmd(TIM1, ENABLE);
}

// TIM1更新中断服务函数
void TIM1_UP_IRQHandler(void) {
    if(TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
        
        // 更新PWM占空比
        TIM_SetCompare1(TIM3, duty);
        
        // 调整占空比
        duty += direction;
        
        if(duty >= 99) direction = -1;
        if(duty <= 0) direction = 1;
    }
}

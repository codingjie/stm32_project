#include "stm32f10x.h"
#include "oled.h"
#include "motor.h"
#include "delay.h"
#include "usart.h"
#include "tim.h"
#include "encoder.h"

int main(void) {
	RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI); // 使用内部高速时钟
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 设置中断优先级分组
    OLED_Init();
    OLED_ShowString(0, 0, "123");
    MOTOR_Init();
    USART1_Init(9600);
    delay_init();
    TIM2_Init(); // PWM循环周期20S
    Encoder_Init(); // 编码器读取速度
    
    while(1) {
        delay_ms(1000);
    }
}

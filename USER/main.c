#include "stm32f10x.h"
#include "oled.h"
#include "motor.h"
#include "delay.h"
#include "usart.h"
#include "tim.h"
#include "encoder.h"
#include <stdio.h>

int main(void) {
    RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);     // 使用内部高速时钟
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 设置中断优先级分组

    delay_init();           // 延时初始化
    USART1_Init(9600);      // 串口初始化
    OLED_Init();            // OLED初始化
    MOTOR_Init();           // 电机初始化
    TIM1_Init();            // PWM循环定时器（20ms * 200）
    Encoder_Init();         // 编码器测速初始值
    OLED_ShowString(0, 0, "RPM:");
    OLED_CurveInit();
    
    while (1) {
        if (speed_update_flag) {
            speed_update_flag = 0;
            
            // 显示数值
            OLED_ShowNum(40, 0, (int32_t)motor_rpm, 3);
            // 绘制曲线
            OLED_DrawCurve(motor_rpm);
            // 串口输出
            printf("Speed: %d\r\n", (int)motor_rpm);
        }
    }
}

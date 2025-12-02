#include "stm32f10x.h"
#include "oled.h"
#include "motor.h"
#include "delay.h"
#include "usart.h"
#include "tim.h"
#include "encoder.h"
#include <stdio.h>

int main(void) {
    char oled_buffer[32];
    float rpm, hz;

    // 系统初始化
    RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);     // 使用内部高速时钟
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 设置中断优先级分组

    delay_init();           // 延时初始化
    USART1_Init(9600);      // 串口初始化
    OLED_Init();            // OLED初始化
    MOTOR_Init();           // 电机初始化
    TIM2_Init();            // PWM循环定时器（20ms）
    Encoder_Init();         // 编码器测速初始化（PB7相位检测）

    // 显示标题
    OLED_Clear();
    OLED_ShowString(0, 0, "Speed Monitor", 12);
    OLED_ShowString(0, 2, "RPM:", 12);
    OLED_ShowString(0, 4, "Hz:", 12);

    printf("STM32 Motor Speed Monitor Started\r\n");
    printf("Using PB7 Phase Detection Mode\r\n");

    while(1) {
        // 读取转速数据
        rpm = Get_Motor_RPM();
        hz = Get_Motor_Speed_Hz();

        // 在OLED上显示
        sprintf(oled_buffer, "%.1f  ", rpm);
        OLED_ShowString(40, 2, oled_buffer, 12);

        sprintf(oled_buffer, "%.2f  ", hz);
        OLED_ShowString(30, 4, oled_buffer, 12);

        // 通过串口输出
        printf("RPM: %.1f, Hz: %.2f\r\n", rpm, hz);

        delay_ms(500);  // 每500ms刷新一次显示
    }
}

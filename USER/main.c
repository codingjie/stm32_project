#include "stm32f10x.h"
#include "oled.h"
#include "motor.h"
#include "delay.h"
#include "usart.h"
#include "tim.h"
#include "encoder.h"
#include <stdio.h>

int main(void) {
	RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI); // 使用内部高速时钟
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 设置中断优先级分组

    // 初始化外设
    OLED_Init();
    MOTOR_Init();
    USART1_Init(9600);
    delay_init();
    TIM2_Init();        // PWM自动调制，周期20ms
    Encoder_Init();     // 编码器读取速度

    // OLED 显示标题
    OLED_Clear();
    OLED_ShowString(0, 0, (uint8_t*)"Motor Control");
    OLED_ShowString(0, 2, (uint8_t*)"Speed:");
    OLED_ShowString(72, 2, (uint8_t*)"RPM");
    OLED_ShowString(0, 4, (uint8_t*)"PWM  :");

    printf("\r\n===== Motor Control System =====\r\n");
    printf("System initialized successfully\r\n\r\n");

    float rpm;
    uint8_t duty_percent;
    char buffer[16];

    while(1) {
        // 读取电机转速
        rpm = Get_Motor_RPM();

        // 获取当前PWM占空比
        duty_percent = (uint8_t)duty;

        // OLED显示转速和占空比
        sprintf(buffer, "%4d", (int)rpm);
        OLED_ShowString(40, 2, (uint8_t*)buffer);

        sprintf(buffer, "%3d%%", duty_percent);
        OLED_ShowString(40, 4, (uint8_t*)buffer);

        // 串口输出数据
        printf("RPM: %4d | PWM: %3d%% | Time: %lu ms\r\n",
               (int)rpm, duty_percent, (unsigned long)(0)); // 时间戳可根据需要添加

        delay_ms(200);  // 200ms 刷新一次显示
    }
}

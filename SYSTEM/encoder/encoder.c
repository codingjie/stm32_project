#include "encoder.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

// 静态变量保存测速数据
static volatile uint32_t last_capture = 0;      // 上次捕获值
static volatile uint32_t capture_period = 0;    // 捕获周期
static volatile uint8_t capture_flag = 0;       // 捕获标志
static volatile float motor_rpm = 0;            // 电机转速(RPM)
static volatile float motor_hz = 0;             // 脉冲频率(Hz)
static volatile uint32_t overflow_count = 0;    // 溢出计数

/**
 * @brief  初始化编码器测速（PB7输入捕获模式）
 * @note   使用TIM4_CH2(PB7)输入捕获功能测量脉冲周期
 */
void Encoder_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_ICInitTypeDef TIM_ICInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 1. 使能时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    // 2. 配置PB7为浮空输入（TIM4_CH2）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 3. 配置TIM4时基
    // 72MHz / 72 = 1MHz，计数周期为1us
    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;           // 最大计数值65535
    TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;        // 预分频为72，计数频率1MHz
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    // 4. 配置输入捕获通道2
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;              // 选择通道2
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;   // 上升沿捕获
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; // 映射到TI2
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;         // 不分频
    TIM_ICInitStructure.TIM_ICFilter = 0x0;                       // 不滤波
    TIM_ICInit(TIM4, &TIM_ICInitStructure);

    // 5. 使能更新中断和捕获中断
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);   // 溢出中断
    TIM_ITConfig(TIM4, TIM_IT_CC2, ENABLE);      // 捕获中断

    // 6. 配置NVIC
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 7. 启动定时器
    TIM_Cmd(TIM4, ENABLE);
}

/**
 * @brief  获取电机转速(RPM)
 * @return 转速值(转/分钟)
 */
float Get_Motor_RPM(void)
{
    return motor_rpm;
}

/**
 * @brief  获取脉冲频率(Hz)
 * @return 频率值(Hz)
 */
float Get_Motor_Speed_Hz(void)
{
    return motor_hz;
}

/**
 * @brief  TIM4中断服务函数
 * @note   处理输入捕获和定时器溢出
 */
void TIM4_IRQHandler(void)
{
    // 捕获中断
    if (TIM_GetITStatus(TIM4, TIM_IT_CC2) != RESET)
    {
        uint32_t current_capture = TIM_GetCapture2(TIM4);

        if (capture_flag == 0)
        {
            // 第一次捕获
            last_capture = current_capture;
            overflow_count = 0;
            capture_flag = 1;
        }
        else
        {
            // 第二次捕获，计算周期
            if (current_capture >= last_capture)
            {
                // 没有溢出
                capture_period = current_capture - last_capture + overflow_count * 65536;
            }
            else
            {
                // 发生了溢出
                capture_period = (65536 - last_capture) + current_capture + overflow_count * 65536;
            }

            // 计算频率和转速
            if (capture_period > 0)
            {
                // 频率 = 1,000,000 / 周期(us)
                motor_hz = 1000000.0f / capture_period;

                // 转速(RPM) = (频率 / 每圈脉冲数) * 60
                motor_rpm = (motor_hz / ENCODER_PPR) * 60.0f;
            }

            // 重置状态
            last_capture = current_capture;
            overflow_count = 0;
            capture_flag = 1;
        }

        TIM_ClearITPendingBit(TIM4, TIM_IT_CC2);
    }

    // 溢出中断
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
    {
        if (capture_flag == 1)
        {
            overflow_count++;

            // 如果溢出次数过多，说明转速很慢或停止
            if (overflow_count > 100)
            {
                motor_rpm = 0;
                motor_hz = 0;
                capture_flag = 0;
                overflow_count = 0;
            }
        }

        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
    }
}

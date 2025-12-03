#include "encoder.h"

volatile float motor_rpm = 0;           // 电机转速（RPM）
volatile uint8_t speed_update_flag = 0; // 转速更新标志

static uint16_t capture_last = 0;       // 上次捕获值
static uint8_t first_capture = 1;       // 首次捕获标志

// 滤波参数
#define FILTER_SIZE 8                   // 滑动平均滤波深度
static float rpm_buffer[FILTER_SIZE] = {0};
static uint8_t filter_index = 0;
static uint8_t filter_count = 0;

/**
 * @brief  编码器初始化（输入捕获测周期法）
 * @note   PA1 -> TIM2_CH2，上升沿捕获
 *         定时器时钟：8MHz / 800 = 10kHz，每计数 = 100us
 */
void Encoder_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_ICInitTypeDef TIM_ICInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    /* 开启时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    /* PA1 浮空输入 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* 定时器基础配置 */
    TIM_TimeBaseStructure.TIM_Prescaler = 799;      // 预分频值 8MHz / 800 = 10kHz
    TIM_TimeBaseStructure.TIM_Period = 65535;       // 最大计数值
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    
    /* 输入捕获配置 */
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;      // 上升沿捕获
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;  // 直接映射
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;            // 不分频
    TIM_ICInitStructure.TIM_ICFilter = 0x0F;                         // 最大硬件滤波
    TIM_ICInit(TIM2, &TIM_ICInitStructure);
    
    /* 使能捕获中断 */
    TIM_ITConfig(TIM2, TIM_IT_CC2, ENABLE);
    
    /* NVIC 配置 */
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    /* 启动定时器 */
    TIM_Cmd(TIM2, ENABLE);
}

/**
 * @brief  TIM2 中断服务函数
 * @note   计算公式：RPM = 60 / (周期 × 每圈脉冲数)
 *         周期单位 100us，所以 RPM = 600000 / (period × 24)
 */
void TIM2_IRQHandler(void) {
    if (TIM_GetITStatus(TIM2, TIM_IT_CC2) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_CC2);
        
        /* 读取当前捕获值 */
        uint16_t capture_now = TIM_GetCapture2(TIM2);
        
        if (first_capture) {
            /* 首次捕获只记录值，不计算 */
            first_capture = 0;
        } else {
            /* 计算两次捕获的时间差（无符号减法自动处理溢出） */
            uint16_t period = capture_now - capture_last;
            
            if (period > 0) {
                /* 计算原始转速 */
                float raw_rpm = 600000.0f / (period * ENCODER_PPR);
                
                /* 范围限制：只接受 0-100 RPM，过滤干扰产生的异常值 */
                if (raw_rpm <= 100.0f) {
                    /* 滑动平均滤波：存入缓冲区 */
                    rpm_buffer[filter_index] = raw_rpm;
                    filter_index = (filter_index + 1) % FILTER_SIZE;
                    if (filter_count < FILTER_SIZE) filter_count++;
                    
                    /* 计算平均值 */
                    float sum = 0;
                    for (uint8_t i = 0; i < filter_count; i++) {
                        sum += rpm_buffer[i];
                    }
                    motor_rpm = sum / filter_count;
                    
                    speed_update_flag = 1;
                }
            }
        }
        
        /* 保存本次捕获值，供下次计算 */
        capture_last = capture_now;
    }
}

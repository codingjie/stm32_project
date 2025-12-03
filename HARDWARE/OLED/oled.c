#include "oled.h"
#include "oledfont.h"

/* 曲线参数 */
#define CURVE_WIDTH     128   // 曲线显示区域宽度（像素），对应OLED屏幕水平分辨率
#define CURVE_Y_TOP     16    // 曲线显示区域顶部Y坐标（像素），上方留出空间显示数值等信息
#define CURVE_Y_BOTTOM  63    // 曲线显示区域底部Y坐标（像素），对应OLED屏幕最底行
#define MAX_RPM         100   // 转速最大量程（RPM），用于将实际转速映射到曲线Y轴范围

/* 曲线数据缓冲区 */
static uint8_t curve_data[CURVE_WIDTH] = {0};

/* 显存缓冲区（第2-7页） */
static uint8_t oled_buffer[6][128] = {0};

/*引脚初始化*/
void OLED_I2C_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_I2C_PORT, ENABLE);

    GPIO_InitStructure.GPIO_Pin = I2C_SCL_PIN | I2C_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIO_PORT_I2C, &GPIO_InitStructure);

    OLED_W_SCL(1);
    OLED_W_SDA(1);
}

// IIC开始信号 
void OLED_I2C_Start(void) {
    OLED_W_SDA(1);
    OLED_W_SCL(1);
    OLED_W_SDA(0);
    OLED_W_SCL(0);
}

// IIC停止信号
void OLED_I2C_Stop(void) {
    OLED_W_SDA(0);
    OLED_W_SCL(1);
    OLED_W_SDA(1);
}

// IIC发送一个字节
void OLED_I2C_SendByte(uint8_t Byte) {
    uint8_t i;
    for (i = 0; i < 8; i++) {
        OLED_W_SDA(Byte & (0x80 >> i));
        OLED_W_SCL(1);
        OLED_W_SCL(0);
    }
    OLED_W_SCL(1); // 额外的一个时钟，不处理应答信号
    OLED_W_SCL(0);
}

// IIC写命令
void OLED_Write_Cmd(uint8_t Command) {
    OLED_I2C_Start();
    OLED_I2C_SendByte(0x78); // 从机地址
    OLED_I2C_SendByte(0x00); // 写命令
    OLED_I2C_SendByte(Command);
    OLED_I2C_Stop();
}

// IIC写数据
void OLED_Write_Data(uint8_t Data) {
    OLED_I2C_Start();
    OLED_I2C_SendByte(0x78); // 从机地址
    OLED_I2C_SendByte(0x40); // 写数据
    OLED_I2C_SendByte(Data);
    OLED_I2C_Stop();
}


// OLED设置光标位置 x:0~127 y:0~7
void OLED_Set_Pos(uint8_t x, uint8_t y) {
    OLED_Write_Cmd(0xb0 + y); // 设置页地址
    OLED_Write_Cmd(x & 0x0f); // 设置列地址低四位
    OLED_Write_Cmd(((x & 0xf0) >> 4) | 0x10); // 设置列地址高四位
}

// 清屏
void OLED_Clear(void) {
    uint8_t i, j;
    for (j = 0; j < 8; j++) {
        OLED_Set_Pos(j, 0);
        for (i = 0; i < 128; i++) {
            OLED_Write_Data(0x00);
        }
    }
}

/**
* @brief 在指定位置显示一个字符,包括部分字符
* @param x: x轴起点坐标 0 ~ 127
* @param y: y轴起点坐标 0 ~ 7页面坐标
* @param chr: 字符
* @param size: 选择字体大小 16
* @retval None
*/
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr) {
    uint8_t c = chr - ' '; // 得到偏移后的值
    if (x > 127) {
        x = 0;
        y += 2;
    }

    OLED_Set_Pos(x, y);
    for (uint8_t i = 0; i < 8; i++) { // 显示字符上半部分
        OLED_Write_Data(ascii_1608[c][i]);
    }
    OLED_Set_Pos(x, y + 1);
    for (uint8_t i = 0; i < 8; i++) { // 显示字符下半部分
        OLED_Write_Data(ascii_1608[c][i + 8]);
    }
}

void OLED_ShowString(uint8_t x, uint8_t y, uint8_t* str) {
    for (uint8_t i = 0; str[i] != '\0'; i++) {
        OLED_ShowChar(x, y, str[i]);
        x += 8;
        if (x > 120) {
            x = 0; 
            y += 2;
        }
    }
}

// 用于显示数字时计算需要的幂函数，表示 m 的 n 次方
uint32_t oled_pow(uint8_t m, uint8_t n) {
    uint32_t result = 1;
    while(n--) result *= m;
    return result;
}

void OLED_ShowNum(uint8_t x, uint8_t y, int32_t num, uint8_t len)
{
    uint8_t temp = 0; // 记录当前处理的单个数字
    uint8_t enshow = 0; // 标志位，用于去掉前导零

    // 处理负号
    if (num < 0)
    {
        num = -num;
        OLED_ShowChar(x, y, '-'); // 在起始位置显示负号
        x += 8; // 偏移 x 坐标，为数字留空间
        len--;
    }

    for (uint8_t i = 0; i < len; i++)
    {
        temp = (num / oled_pow(10, len - i - 1)) % 10; // 从高位取数字
        if (enshow == 0 && i < (len - 1)) // 对除最后一位之外的位进行前导零检查
        {
            if (temp == 0)
            {
                OLED_ShowChar(x + 8 * i, y, ' ');
                continue;
            }
            else
            {
                enshow = 1;
            }
        }
        OLED_ShowChar(x + 8 * i, y, temp + '0'); // 显示数字
    }
}

void OLED_Init(void) {
    OLED_I2C_Init(); // 端口初始化

    OLED_Write_Cmd(0xAE); // 关闭显示
    OLED_Write_Cmd(0xD5); // 设置显示时钟分频比/振荡器频率
    OLED_Write_Cmd(0x80);
    OLED_Write_Cmd(0xA8); // 设置多路复用率
    OLED_Write_Cmd(0x3F);
    OLED_Write_Cmd(0xD3); // 设置显示偏移
    OLED_Write_Cmd(0x00);
    OLED_Write_Cmd(0x40); // 设置显示开始行
    OLED_Write_Cmd(0xA1); // 设置左右方向，0xA1正常 0xA0左右反置
    OLED_Write_Cmd(0xC8); // 设置上下方向，0xC8正常 0xC0上下反置
    OLED_Write_Cmd(0xDA); // 设置COM引脚硬件配置
    OLED_Write_Cmd(0x12);
    OLED_Write_Cmd(0x81); // 设置对比度控制
    OLED_Write_Cmd(0xCF);
    OLED_Write_Cmd(0xD9); // 设置预充电周期
    OLED_Write_Cmd(0xF1);
    OLED_Write_Cmd(0xDB); // 设置VCOMH取消选择级别
    OLED_Write_Cmd(0x30);
    OLED_Write_Cmd(0xA4); // 设置整个显示打开/关闭
    OLED_Write_Cmd(0xA6); // 设置正常/倒转显示
    OLED_Write_Cmd(0x8D); // 设置充电泵
    OLED_Write_Cmd(0x14);
    OLED_Write_Cmd(0xAF); // 开启显示

    OLED_Clear(); // OLED清屏
    OLED_Set_Pos(0, 0);
}

/**
 * @brief  初始化曲线数据
 */
void OLED_CurveInit(void) {
    for (uint8_t i = 0; i < CURVE_WIDTH; i++) {
        curve_data[i] = CURVE_Y_BOTTOM;
    }
}

/**
 * @brief  更新曲线区域到OLED
 */
static void OLED_UpdateCurve(void) {
    for (uint8_t page = 0; page < 6; page++) {
        OLED_Set_Pos(0, page + 2);
        for (uint8_t x = 0; x < 128; x++) {
            OLED_Write_Data(oled_buffer[page][x]);
        }
    }
}

/**
 * @brief  在缓冲区画点
 */
static void Buffer_DrawPoint(uint8_t x, uint8_t y) {
    if (y < CURVE_Y_TOP || y > CURVE_Y_BOTTOM) return;
    
    uint8_t page = (y - 16) / 8; // 确定在哪一页
    uint8_t bit = (y - 16) % 8; // 确定在该页的哪一位
    
    oled_buffer[page][x] |= (1 << bit);
}

/**
 * @brief  绘制滚动曲线
 * @param  rpm: 当前转速 0~100
 */
void OLED_DrawCurve(float rpm) {
    /* 限制范围 */
    if (rpm > MAX_RPM) rpm = MAX_RPM;
    if (rpm < 0) rpm = 0;
    
    /* 转换为Y坐标 */
    uint8_t y = CURVE_Y_BOTTOM - (uint8_t)(rpm * (CURVE_Y_BOTTOM - CURVE_Y_TOP) / MAX_RPM);
    
    /* 数据左移 */
    for (uint8_t i = 0; i < CURVE_WIDTH - 1; i++) {
        curve_data[i] = curve_data[i + 1];
    }
    curve_data[CURVE_WIDTH - 1] = y;
    
    /* 清空缓冲区 */
    for (uint8_t page = 0; page < 6; page++) {
        for (uint8_t x = 0; x < 128; x++) {
            oled_buffer[page][x] = 0;
        }
    }
    
    /* 绘制所有点并连线 */
    for (uint8_t x = 0; x < CURVE_WIDTH; x++) {
        if (curve_data[x] > 0) {
            Buffer_DrawPoint(x, curve_data[x]);
            
            /* 连线，在相邻两点之间进行垂直填充 */
            if (x > 0 && curve_data[x - 1] > 0) {
                int8_t y1 = curve_data[x - 1];
                int8_t y2 = curve_data[x];
                int8_t step = (y2 > y1) ? 1 : -1;
                for (int8_t yy = y1; yy != y2; yy += step) {
                    Buffer_DrawPoint(x, yy);
                }
            }
        }
    }
    
    /* 更新到OLED */
    OLED_UpdateCurve();
}

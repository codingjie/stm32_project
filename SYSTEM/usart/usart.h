#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"
#include <stdio.h>

#define USART_REC_LEN 200  	//定义最大接收字节数 200

extern uint8_t  USART_RX_BUF[USART_REC_LEN];

void USART1_Init(uint32_t bound);
void USART_SendString(char* str);

#endif

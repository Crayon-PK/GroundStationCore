#ifndef __TELEMETERING_USART_H
#define __TELEMETERING_USART_H

#include "stm32f4xx.h"                  // Device header

#define MAX_LONG 256
typedef void (*USART1_RxCallback_t)(uint8_t* pData, uint16_t len);

void USART1_Init(void);
void USART1_DMA_Init(uint8_t* buffer, uint16_t size);
void USART1_SendBuffer_DMA(uint8_t* arr,uint16_t len);
void USART1_RegisterCallback(USART1_RxCallback_t callback);

#endif

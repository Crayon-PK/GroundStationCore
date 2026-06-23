#ifndef __TELEMETRY_UART_H
#define __TELEMETRY_UART_H

#include "stm32f4xx.h"

#define TELEMETRY_BUFF_SIZE         1024

#define TELEMETRY_UART              USART3
#define TELEMETRY_UART_CLK_CMD      RCC_APB1PeriphClockCmd
#define TELEMETRY_UART_CLK          RCC_APB1Periph_USART3
#define TELEMETRY_UART_IRQn         USART3_IRQn
#define TELEMETRY_UART_IRQHandler   USART3_IRQHandler

#define TELEMETRY_GPIO_CLK_CMD      RCC_AHB1PeriphClockCmd
#define TELEMETRY_GPIO_CLK          RCC_AHB1Periph_GPIOB
#define TELEMETRY_GPIO_PORT         GPIOB
#define TELEMETRY_TX_PIN            GPIO_Pin_10
#define TELEMETRY_TX_SOURCE         GPIO_PinSource10
#define TELEMETRY_RX_PIN            GPIO_Pin_11
#define TELEMETRY_RX_SOURCE         GPIO_PinSource11
#define TELEMETRY_UART_AF           GPIO_AF_USART3

#define TELEMETRY_DMA_CLK_CMD       RCC_AHB1PeriphClockCmd
#define TELEMETRY_DMA_CLK           RCC_AHB1Periph_DMA1

#define TELEMETRY_RX_DMA_STREAM     DMA1_Stream1
#define TELEMETRY_RX_DMA_CHANNEL    DMA_Channel_4
#define TELEMETRY_RX_DMA_FLAG_TC    DMA_FLAG_TCIF1

#define TELEMETRY_TX_DMA_STREAM     DMA1_Stream3
#define TELEMETRY_TX_DMA_CHANNEL    DMA_Channel_4
#define TELEMETRY_TX_DMA_FLAG_TC    DMA_FLAG_TCIF3

#define TELEMETRY_TX_DMA_IRQn       DMA1_Stream3_IRQn
#define TELEMETRY_TX_DMA_IRQHandler DMA1_Stream3_IRQHandler
#define TELEMETRY_TX_DMA_IT_TC      DMA_IT_TCIF3

/* 外部可调用函数声明 */
typedef void (*Telemetry_RxCallback_t)(uint8_t* pData, uint16_t len);
typedef void (*Telemetry_TxCallback_t)(void);
int Telemetry_UART_Init(void);
void Telemetry_UART_SendBuffer_DMA(uint8_t* arr, uint16_t len);
void Telemetry_UART_RegisterRxCallback(Telemetry_RxCallback_t callback);
void Telemetry_UART_RegisterTxCallback(Telemetry_TxCallback_t callback);

#endif /* __TELEMETRY_UART_H */
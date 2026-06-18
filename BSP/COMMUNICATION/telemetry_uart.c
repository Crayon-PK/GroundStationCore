#include "telemetry_uart.h"
#include <stddef.h>

static uint8_t g_ring_buffer[TELEMETRY_BUFF_SIZE];
static uint16_t g_last_read_ptr = 0; 

static Telemetry_RxCallback_t g_rx_callback = NULL;
static Telemetry_TxCallback_t g_tx_callback = NULL;

/**
  * @brief  注册串口接收完成（IDLE）回调函数
  * @param  callback: 应用层接收回调函数指针
  */
void Telemetry_UART_RegisterRxCallback(Telemetry_RxCallback_t callback) 
{ 
    g_rx_callback = callback; 
}

/**
  * @brief  注册串口发送完成（DMA-TC）回调函数
  * @param  callback: 应用层发送完成回调函数指针
  */
void Telemetry_UART_RegisterTxCallback(Telemetry_TxCallback_t callback) 
{ 
    g_tx_callback = callback; 
}

/**
  * @brief  初始化USART3的DMA传输配置（RX循环模式，TX单次模式）
  * @param  buffer: 接收缓冲区指针
  * @param  size: 缓冲区大小
  * @retval 0: 成功; -1: 硬件使能超时失败
  */
static int Telemetry_UART_DMA_Init(uint8_t* buffer, uint16_t size)
{
    uint32_t timeout = 0xFFFF;
    if (buffer == NULL || size == 0) return -1;

    TELEMETRY_DMA_CLK_CMD(TELEMETRY_DMA_CLK, ENABLE);
    
    // RX 配置
    DMA_DeInit(TELEMETRY_RX_DMA_STREAM);
    DMA_Init(TELEMETRY_RX_DMA_STREAM, &(DMA_InitTypeDef){
        .DMA_Channel            = TELEMETRY_RX_DMA_CHANNEL,
        .DMA_PeripheralBaseAddr = (uint32_t)&TELEMETRY_UART->DR,
        .DMA_Memory0BaseAddr    = (uint32_t)buffer,
        .DMA_DIR                = DMA_DIR_PeripheralToMemory,
        .DMA_BufferSize         = size,
        .DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
        .DMA_MemoryInc          = DMA_MemoryInc_Enable,
        .DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
        .DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
        .DMA_Mode               = DMA_Mode_Circular, 
        .DMA_Priority           = DMA_Priority_High,
        .DMA_FIFOMode           = DMA_FIFOMode_Disable,
        .DMA_FIFOThreshold      = DMA_FIFOThreshold_HalfFull,
        .DMA_MemoryBurst        = DMA_MemoryBurst_Single,
        .DMA_PeripheralBurst    = DMA_PeripheralBurst_Single
    });
    
    // TX 配置
    DMA_DeInit(TELEMETRY_TX_DMA_STREAM);
    DMA_Init(TELEMETRY_TX_DMA_STREAM, &(DMA_InitTypeDef){
        .DMA_Channel            = TELEMETRY_TX_DMA_CHANNEL,
        .DMA_PeripheralBaseAddr = (uint32_t)&TELEMETRY_UART->DR,
        .DMA_Memory0BaseAddr    = 0,
        .DMA_DIR                = DMA_DIR_MemoryToPeripheral,
        .DMA_BufferSize         = 0,
        .DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
        .DMA_MemoryInc          = DMA_MemoryInc_Enable,
        .DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
        .DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
        .DMA_Mode               = DMA_Mode_Normal,
        .DMA_Priority           = DMA_Priority_High,
        .DMA_FIFOMode           = DMA_FIFOMode_Disable,
        .DMA_FIFOThreshold      = DMA_FIFOThreshold_HalfFull,
        .DMA_MemoryBurst        = DMA_MemoryBurst_Single,
        .DMA_PeripheralBurst    = DMA_PeripheralBurst_Single
    });
    
    USART_DMACmd(TELEMETRY_UART, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);
    DMA_ITConfig(TELEMETRY_TX_DMA_STREAM, DMA_IT_TC, ENABLE);

    DMA_Cmd(TELEMETRY_RX_DMA_STREAM, ENABLE);
    
    while(DMA_GetCmdStatus(TELEMETRY_RX_DMA_STREAM) == DISABLE)
    {
        if(timeout-- == 0) return -1;
    }
    return 0;
}

/**
  * @brief  初始化遥测串口硬件外设（GPIO、USART、NVIC、DMA）
  * @param  无
  * @retval 0: 初始化成功; -1: 硬件配置或使能失败
  */
int Telemetry_UART_Init(void)
{
    TELEMETRY_GPIO_CLK_CMD(TELEMETRY_GPIO_CLK, ENABLE);
    TELEMETRY_UART_CLK_CMD(TELEMETRY_UART_CLK, ENABLE);
    
    GPIO_Init(TELEMETRY_GPIO_PORT, &(GPIO_InitTypeDef){
        .GPIO_Pin   = TELEMETRY_TX_PIN,
        .GPIO_Mode  = GPIO_Mode_AF,
        .GPIO_OType = GPIO_OType_PP,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_PuPd  = GPIO_PuPd_UP
    });
    GPIO_Init(TELEMETRY_GPIO_PORT, &(GPIO_InitTypeDef){
        .GPIO_Pin   = TELEMETRY_RX_PIN,
        .GPIO_Mode  = GPIO_Mode_AF,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_PuPd  = GPIO_PuPd_UP
    });
    
    GPIO_PinAFConfig(TELEMETRY_GPIO_PORT, TELEMETRY_TX_SOURCE, TELEMETRY_UART_AF);
    GPIO_PinAFConfig(TELEMETRY_GPIO_PORT, TELEMETRY_RX_SOURCE, TELEMETRY_UART_AF);
    
    USART_Init(TELEMETRY_UART, &(USART_InitTypeDef){
        .USART_BaudRate   = 115200,
        .USART_Mode       = USART_Mode_Tx | USART_Mode_Rx,
        .USART_Parity     = USART_Parity_No,
        .USART_StopBits   = USART_StopBits_1,
        .USART_WordLength = USART_WordLength_8b,
    });
    
    if (Telemetry_UART_DMA_Init(g_ring_buffer, TELEMETRY_BUFF_SIZE) != 0) return -1; 
    
    USART_ITConfig(TELEMETRY_UART, USART_IT_IDLE, ENABLE);
    
    NVIC_Init(&(NVIC_InitTypeDef){
        .NVIC_IRQChannel                   = TELEMETRY_UART_IRQn,
        .NVIC_IRQChannelPreemptionPriority = 5,
        .NVIC_IRQChannelSubPriority        = 0,
        .NVIC_IRQChannelCmd                = ENABLE
    });
    NVIC_Init(&(NVIC_InitTypeDef){
        .NVIC_IRQChannel                   = TELEMETRY_TX_DMA_IRQn,
        .NVIC_IRQChannelPreemptionPriority = 6,
        .NVIC_IRQChannelSubPriority        = 0,
        .NVIC_IRQChannelCmd                = ENABLE
    });
    
    USART_Cmd(TELEMETRY_UART, ENABLE);
    return 0;
}

/**
  * @brief  发起异步 DMA 发送（非阻塞寄存器配置）
  * @param  arr: 待发送数据缓冲区指针
  * @param  len: 待发送数据长度
  */
void Telemetry_UART_SendBuffer_DMA(uint8_t* arr, uint16_t len)
{
    if (arr == NULL || len == 0) return;
    TELEMETRY_TX_DMA_STREAM->M0AR = (uint32_t)arr;
    DMA_SetCurrDataCounter(TELEMETRY_TX_DMA_STREAM, len);
    DMA_ClearFlag(TELEMETRY_TX_DMA_STREAM, TELEMETRY_TX_DMA_FLAG_TC);
    DMA_Cmd(TELEMETRY_TX_DMA_STREAM, ENABLE);
}

/**
  * @brief  DMA1_Stream3全局中断服务函数（数传TX发送完成中断）
  */
void TELEMETRY_TX_DMA_IRQHandler(void)
{
    if (DMA_GetITStatus(TELEMETRY_TX_DMA_STREAM, TELEMETRY_TX_DMA_IT_TC) != RESET)
    {
        DMA_ClearITPendingBit(TELEMETRY_TX_DMA_STREAM, TELEMETRY_TX_DMA_IT_TC);
        DMA_Cmd(TELEMETRY_TX_DMA_STREAM, DISABLE);
        if (g_tx_callback != NULL) g_tx_callback();
    }
}

/**
  * @brief  USART3全局中断服务函数
  * @param  无
  * @retval 无
  */
void TELEMETRY_UART_IRQHandler(void)
{
    if(USART_GetITStatus(TELEMETRY_UART, USART_IT_IDLE) != RESET)
    {
        volatile uint32_t clear = TELEMETRY_UART->SR;
        clear = TELEMETRY_UART->DR;
        (void)clear; 

        if (g_rx_callback != NULL)
        {
            uint16_t current_ptr = TELEMETRY_BUFF_SIZE - DMA_GetCurrDataCounter(TELEMETRY_RX_DMA_STREAM);
            uint16_t rx_len = 0;

            if (current_ptr != g_last_read_ptr)
            {
                if (current_ptr > g_last_read_ptr)
                {
                    rx_len = current_ptr - g_last_read_ptr;
                    g_rx_callback(&g_ring_buffer[g_last_read_ptr], rx_len);
                }
                else 
                {
                    rx_len = TELEMETRY_BUFF_SIZE - g_last_read_ptr;
                    g_rx_callback(&g_ring_buffer[g_last_read_ptr], rx_len);
                    
                    if (current_ptr > 0)
                    {
                        g_rx_callback(&g_ring_buffer[0], current_ptr);
                    }
                }
                g_last_read_ptr = current_ptr; 
            }
        }
    }
}
#include "telemetry_uart.h"
#include <stddef.h>

static uint8_t g_ring_buffer[TELEMETRY_BUFF_SIZE];
static Telemetry_RxCallback_t g_rx_callback = NULL;

/**
  * @brief  注册串口数据接收回调函数
  * @param  callback: 指向回调函数的指针，格式为 void(*)(uint8_t*, uint16_t)
  */
void Telemetry_UART_RegisterCallback(Telemetry_RxCallback_t callback)
{
    g_rx_callback = callback;
}

/**
  * @brief  初始化USART3的DMA传输配置
  * @param  buffer: 指向DMA接收缓冲区的指针
  * @param  size: DMA接收缓冲区的大小(字节)
  */
static void Telemetry_UART_DMA_Init(uint8_t* buffer, uint16_t size)
{
    if (buffer == NULL || size == 0) return;

    TELEMETRY_DMA_CLK_CMD(TELEMETRY_DMA_CLK, ENABLE);
    
    // RX 配置 (DMA1_Stream1_Channel4)
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
        .DMA_Mode               = DMA_Mode_Normal, // 变长包使用Normal
        .DMA_Priority           = DMA_Priority_High,
        .DMA_FIFOMode           = DMA_FIFOMode_Disable,
        .DMA_FIFOThreshold      = DMA_FIFOThreshold_HalfFull,
        .DMA_MemoryBurst        = DMA_MemoryBurst_Single,
        .DMA_PeripheralBurst    = DMA_PeripheralBurst_Single
    });
    
    // TX 配置 (DMA1_Stream3_Channel4)
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
    DMA_Cmd(TELEMETRY_RX_DMA_STREAM, ENABLE);
}

/**
  * @brief  初始化遥测串口硬件外设
  * @note   配置包括GPIO引脚、USART参数、DMA控制器和NVIC中断
  */
void Telemetry_UART_Init(void)
{
    TELEMETRY_GPIO_CLK_CMD(TELEMETRY_GPIO_CLK, ENABLE);
    TELEMETRY_UART_CLK_CMD(TELEMETRY_UART_CLK, ENABLE);
    
    // TX (PB10) - 复用推挽
    GPIO_Init(TELEMETRY_GPIO_PORT, &(GPIO_InitTypeDef){
        .GPIO_Pin   = TELEMETRY_TX_PIN,
        .GPIO_Mode  = GPIO_Mode_AF,
        .GPIO_OType = GPIO_OType_PP,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_PuPd  = GPIO_PuPd_UP
    });
    
    // RX (PB11) - 复用浮空/上拉
    GPIO_Init(TELEMETRY_GPIO_PORT, &(GPIO_InitTypeDef){
        .GPIO_Pin   = TELEMETRY_RX_PIN,
        .GPIO_Mode  = GPIO_Mode_AF,
        .GPIO_OType = GPIO_OType_PP,
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
    
    Telemetry_UART_DMA_Init(g_ring_buffer, TELEMETRY_BUFF_SIZE);
    
    USART_ITConfig(TELEMETRY_UART, USART_IT_IDLE, ENABLE);
    NVIC_Init(&(NVIC_InitTypeDef){
        .NVIC_IRQChannel                   = TELEMETRY_UART_IRQn,
        .NVIC_IRQChannelPreemptionPriority = 1,
        .NVIC_IRQChannelSubPriority        = 0,
        .NVIC_IRQChannelCmd                = ENABLE
    });
    
    USART_Cmd(TELEMETRY_UART, ENABLE);
}

/**
  * @brief  通过DMA发送数据缓冲区
  * @param  arr: 指向待发送数据缓冲区的指针
  * @param  len: 待发送数据的长度(字节)
  * @note   函数会等待前一次DMA传输完成后再启动新传输
  */
void Telemetry_UART_SendBuffer_DMA(uint8_t* arr, uint16_t len)
{
    if (arr == NULL || len == 0) return;

    // 等待上一次发送完成
    while(DMA_GetCmdStatus(TELEMETRY_TX_DMA_STREAM) != DISABLE);
    
    TELEMETRY_TX_DMA_STREAM->M0AR = (uint32_t)arr;
    DMA_SetCurrDataCounter(TELEMETRY_TX_DMA_STREAM, len);
    DMA_ClearFlag(TELEMETRY_TX_DMA_STREAM, TELEMETRY_TX_DMA_FLAG_TC);
    
    DMA_Cmd(TELEMETRY_TX_DMA_STREAM, ENABLE);
}

/**
  * @brief  USART3全局中断服务函数
  * @note   主要处理IDLE中断事件，在检测到总线空闲时读取DMA接收到的数据
  *         并通过回调函数通知上层应用
  */
void TELEMETRY_UART_IRQHandler(void)
{
    if(USART_GetITStatus(TELEMETRY_UART, USART_IT_IDLE) != RESET)
    {
        // 清除硬件 IDLE 标志位
        volatile uint32_t clear = TELEMETRY_UART->SR;
        clear = TELEMETRY_UART->DR;
        (void)clear; 
        
        // 获取数据长度
        uint16_t rx_len = TELEMETRY_BUFF_SIZE - DMA_GetCurrDataCounter(TELEMETRY_RX_DMA_STREAM);
        
        if(rx_len > 0)
        {
            // 暂时关闭 DMA 接收以重置计数器
            DMA_Cmd(TELEMETRY_RX_DMA_STREAM, DISABLE);
            while(DMA_GetCmdStatus(TELEMETRY_RX_DMA_STREAM) != DISABLE);
            
            if(g_rx_callback != NULL)
            {
                g_rx_callback(g_ring_buffer, rx_len);
            }
            
            DMA_SetCurrDataCounter(TELEMETRY_RX_DMA_STREAM, TELEMETRY_BUFF_SIZE);
            DMA_ClearFlag(TELEMETRY_RX_DMA_STREAM, TELEMETRY_RX_DMA_FLAG_TC);
            DMA_Cmd(TELEMETRY_RX_DMA_STREAM, ENABLE);
        }
    }
}
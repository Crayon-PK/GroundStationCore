#include "bsp_telemetering_usart.h"
#include "stddef.h"

// 定义全局接收缓冲区
static uint8_t ring_buffer[MAX_LONG];

static USART1_RxCallback_t g_rx_callback = NULL;

void USART1_RegisterCallback(USART1_RxCallback_t callback)
{
	g_rx_callback = callback;
}

/**
  * @brief  初始化USART1的DMA配置
  * @param  buffer: 接收缓冲区首地址
  * @param  size: 接收缓冲区大小
  */
void USART1_DMA_Init(uint8_t* buffer, uint16_t size)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
    
    // RX 配置 (DMA2_Stream5_Channel4)
    DMA_DeInit(DMA2_Stream5);
    DMA_Init(DMA2_Stream5, &(DMA_InitTypeDef){
        .DMA_Channel            = DMA_Channel_4,
        .DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR,
        .DMA_Memory0BaseAddr    = (uint32_t)buffer,
        .DMA_DIR                = DMA_DIR_PeripheralToMemory, // 外设到内存
        .DMA_BufferSize         = size,
        .DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
        .DMA_MemoryInc          = DMA_MemoryInc_Enable,
        .DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
        .DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
        .DMA_Mode               = DMA_Mode_Normal,            // 变长接收用Normal模式
        .DMA_Priority           = DMA_Priority_High,
        .DMA_FIFOMode           = DMA_FIFOMode_Disable,
        .DMA_FIFOThreshold      = DMA_FIFOThreshold_HalfFull,
        .DMA_MemoryBurst        = DMA_MemoryBurst_Single,
        .DMA_PeripheralBurst    = DMA_PeripheralBurst_Single
    });
    
    // TX 配置 (DMA2_Stream7_Channel4)
    DMA_DeInit(DMA2_Stream7);
    DMA_Init(DMA2_Stream7, &(DMA_InitTypeDef){
        .DMA_Channel            = DMA_Channel_4,
        .DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR,
        .DMA_Memory0BaseAddr    = 0,                          // 发送时动态指定
        .DMA_DIR                = DMA_DIR_MemoryToPeripheral, // 内存到外设
        .DMA_BufferSize         = 0,                          // 发送时动态指定
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
    
    // 同时使能串口的 DMA 发送和接收请求
    USART_DMACmd(USART1, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);
    
    // 开启 DMA 接收流，进入等待接收状态
    DMA_Cmd(DMA2_Stream5, ENABLE);
}

void USART1_Init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    
    GPIO_Init(GPIOA, &(GPIO_InitTypeDef){
        .GPIO_Pin   = GPIO_Pin_9,
        .GPIO_Mode  = GPIO_Mode_AF,
        .GPIO_OType = GPIO_OType_PP,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_PuPd  = GPIO_PuPd_UP
    });
    
    GPIO_Init(GPIOA, &(GPIO_InitTypeDef){
        .GPIO_Pin   = GPIO_Pin_10,
        .GPIO_Mode  = GPIO_Mode_AF,
        .GPIO_PuPd  = GPIO_PuPd_NOPULL
    });
    
    GPIO_PinAFConfig(GPIOA, GPIO_Pin_9,  GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_Pin_10, GPIO_AF_USART1);
    
    USART_Init(USART1, &(USART_InitTypeDef){
        .USART_BaudRate   = 115200,
        .USART_Mode       = USART_Mode_Tx | USART_Mode_Rx,
        .USART_Parity     = USART_Parity_No,
        .USART_StopBits   = USART_StopBits_1,
        .USART_WordLength = USART_WordLength_8b,
    });
    
	// 初始化 DMA，传入全局缓冲区
	USART1_DMA_Init(ring_buffer, MAX_LONG);
	
    // 配置串口中断（IDLE 中断）
    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
    NVIC_Init(&(NVIC_InitTypeDef){
        .NVIC_IRQChannel                   = USART1_IRQn,
        .NVIC_IRQChannelPreemptionPriority = 1,
        .NVIC_IRQChannelSubPriority        = 0,
        .NVIC_IRQChannelCmd                = ENABLE
    });
    
    USART_Cmd(USART1, ENABLE);
}

/**
  * @brief  使用 DMA 异步发送数组
  * @param  arr: 发送数据缓冲区首地址
  * @param  len: 发送数据长度
  */
void USART1_SendBuffer_DMA(uint8_t* arr,uint16_t len)
{
	// 等待上一次 DMA 发送完成
	while(DMA_GetCmdStatus(DMA2_Stream7) != DISABLE);
	
	// 更新 DMA 发送的目标地址和长度
	DMA2_Stream7->M0AR = (uint32_t)arr;
	DMA_SetCurrDataCounter(DMA2_Stream7,len);
	
	// 清除上一次发送的标志位
	DMA_ClearFlag(DMA2_Stream7,DMA_FLAG_TCIF7);
	
	// 开启 DMA 发送
	DMA_Cmd(DMA2_Stream7,ENABLE);
}

/**
  * @brief  串口1中断服务函数，处理空闲中断实现变长包接收
  */
void USART1_IRQHandler(void)
{
    // 检测是否发生空闲中断
    if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
    {
        // 清除空闲中断标志位：硬件规定需先读 SR 再读 DR
        volatile uint8_t clear = USART1->SR;
        clear = USART1->DR;
        (void)clear; 
        
        // 暂时关闭 DMA 接收
        DMA_Cmd(DMA2_Stream5, DISABLE);
        while(DMA_GetCmdStatus(DMA2_Stream5) != DISABLE);
        
        uint16_t rx_len = MAX_LONG - DMA_GetCurrDataCounter(DMA2_Stream5);
        
        if(rx_len > 0 && g_rx_callback != NULL)
        {
            g_rx_callback(ring_buffer, rx_len);
        }
        
        // 重新设置计数器并开启 DMA，等待下一帧数据
        DMA_SetCurrDataCounter(DMA2_Stream5, MAX_LONG);
        DMA_Cmd(DMA2_Stream5, ENABLE);
    }
}

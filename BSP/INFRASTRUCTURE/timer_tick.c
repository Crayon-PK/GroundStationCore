#include "timer_tick.h"

static __IO uint32_t g_system_tick = 0;

/**
  * @brief  初始化基本定时器TIM6作为系统时钟源
  */
void System_Timebase_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

    // TIM6 挂载在 APB1(84MHz)，分频比 84 后进入定时器的时钟为 1MHz (即 1us 计数一次)
    TIM_TimeBaseInit(TIM6, &(TIM_TimeBaseInitTypeDef){
        .TIM_Prescaler         = 84 - 1,
        .TIM_Period            = 1000 - 1, // 1000个中断 = 1ms
        .TIM_CounterMode       = TIM_CounterMode_Up,
        .TIM_ClockDivision     = TIM_CKD_DIV1,
    });

    TIM_ClearFlag(TIM6, TIM_FLAG_Update);
    TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);

    NVIC_Init(&(NVIC_InitTypeDef){
        .NVIC_IRQChannel                   = TIM6_DAC_IRQn,
        .NVIC_IRQChannelPreemptionPriority = 0,
        .NVIC_IRQChannelSubPriority        = 0,
        .NVIC_IRQChannelCmd                = ENABLE,
    });

    TIM_Cmd(TIM6, ENABLE);
}

/**
  * @brief  获取系统运行毫秒数
  * @retval uint32_t 类型的系统 Tick 值
  */
uint32_t Get_SystemTick(void)
{
    return g_system_tick;
}

/**
  * @brief  微秒级延时
  */
void Delay_us(uint16_t us)
{
    __IO uint32_t delay = us * 12; 
    while (delay--);
}

/**
  * @brief  毫秒级延时函数
  * @param  ms: 需要延时的毫秒数
  */
void Delay_ms(uint32_t ms)
{
    uint32_t tickstart = Get_SystemTick();
    while ((Get_SystemTick() - tickstart) < ms);
}

/**
  * @brief  TIM6定时器中断处理函数
  */
void TIM6_DAC_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
    {
        g_system_tick++;
        TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
    }
}
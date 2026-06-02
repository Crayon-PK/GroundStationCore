#include "timer_tick.h"

// 外部声明 LVGL 的心跳递增函数
extern void lv_tick_inc(uint32_t tick_period);

static __IO uint32_t g_system_tick = 0;

/**
  * @brief  初始化系统时钟源定时器
  * @retval 0: 成功; -1: 定时器使能超时失败
  */
int System_Timebase_Init(void)
{
    uint32_t timeout = 0xFFFF;
    
    // 总线时钟使能
    MS_TICK_TIM_CLK_CMD(MS_TICK_TIM_CLK, ENABLE);

    TIM_TimeBaseInit(MS_TICK_TIM, &(TIM_TimeBaseInitTypeDef){
        .TIM_Prescaler         = MS_TICK_TIM_PRESCALER,
        .TIM_Period            = MS_TICK_TIM_PERIOD, 
        .TIM_CounterMode       = TIM_CounterMode_Up,
        .TIM_ClockDivision     = TIM_CKD_DIV1,
    });

    TIM_ClearFlag(MS_TICK_TIM, TIM_FLAG_Update);
    TIM_ITConfig(MS_TICK_TIM, TIM_IT_Update, ENABLE);

    // 中断通道配置
    NVIC_Init(&(NVIC_InitTypeDef){
        .NVIC_IRQChannel                   = MS_TICK_TIM_IRQn,
        .NVIC_IRQChannelPreemptionPriority = 0,
        .NVIC_IRQChannelSubPriority        = 0,
        .NVIC_IRQChannelCmd                = ENABLE,
    });

    TIM_Cmd(MS_TICK_TIM, ENABLE);
    
    // 寄存器状态校验
    while ((MS_TICK_TIM->CR1 & TIM_CR1_CEN) == 0)
    {
        if (timeout-- == 0) return -1;
    }
    return 0;
}

uint32_t Get_SystemTick(void)
{
    return g_system_tick;
}

void Delay_us(uint16_t us)
{
    __IO uint32_t delay = us * 12; 
    while (delay--);
}

void Delay_ms(uint32_t ms)
{
    uint32_t tickstart = Get_SystemTick();
    while ((Get_SystemTick() - tickstart) < ms);
}

/**
  * @brief  动态映射的系统时基中断服务函数
  */
void MS_TICK_TIM_IRQHandler(void)
{
    if (TIM_GetITStatus(MS_TICK_TIM, TIM_IT_Update) != RESET)
    {
        g_system_tick++;
        lv_tick_inc(1); // Call LVGL's tick increment function
        TIM_ClearITPendingBit(MS_TICK_TIM, TIM_IT_Update);
    }
}
#include "timer_tick.h"

static __IO uint32_t uwTick = 0;

void System_Timebase_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

    TIM_TimeBaseInit(TIM6, &(TIM_TimeBaseInitTypeDef){
        .TIM_Prescaler         = 84 - 1,
        .TIM_Period            = 1000 -1,
        .TIM_CounterMode       = TIM_CounterMode_Up,
        .TIM_ClockDivision     = TIM_CKD_DIV1,
    });

    TIM_ClearFlag(TIM6, TIM_FLAG_Update);
    TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);

    NVIC_Init(&(NVIC_InitTypeDef){
        .NVIC_IRQChannel = TIM6_DAC_IRQn,
        .NVIC_IRQChannelPreemptionPriority = 0,
        .NVIC_IRQChannelSubPriority = 0,
        .NVIC_IRQChannelCmd = ENABLE,
    });

    TIM_Cmd(TIM6, ENABLE);
}

uint32_t Get_SystemTick(void)
{
    return uwTick;
}

void Delay_us(uint16_t us)
{
    if (us > 1000) us = 1000;
    uint16_t start = TIM6->CNT;
    while ((uint16_t)(TIM6->CNT - start) < us);
}

void Delay_ms(uint32_t ms)
{
    uint32_t tickstart = Get_SystemTick();
    while ((Get_SystemTick() - tickstart) < ms);
}

void TIM6_DAC_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
    {
        uwTick++;
        TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
    }
}
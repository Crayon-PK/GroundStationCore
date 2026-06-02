#ifndef __TIMER_TICK_H
#define __TIMER_TICK_H

#include "stm32f4xx.h"

#define MS_TICK_TIM                 TIM6
#define MS_TICK_TIM_CLK_CMD         RCC_APB1PeriphClockCmd
#define MS_TICK_TIM_CLK             RCC_APB1Periph_TIM6
#define MS_TICK_TIM_PRESCALER       (84 - 1)
#define MS_TICK_TIM_PERIOD          (1000 - 1)

#define MS_TICK_TIM_IRQn            TIM6_DAC_IRQn
#define MS_TICK_TIM_IRQHandler      TIM6_DAC_IRQHandler

int System_Timebase_Init(void);
uint32_t Get_SystemTick(void);
void Delay_us(uint16_t us);
void Delay_ms(uint32_t ms);

#endif /* __TIMER_TICK_H */
#ifndef __TIMER_TICK_H
#define __TIMER_TICK_H

#include "stm32f4xx.h"

void     System_Timebase_Init(void);
uint32_t Get_SystemTick(void);
void     Delay_us(uint16_t us);
void     Delay_ms(uint32_t ms);

#endif
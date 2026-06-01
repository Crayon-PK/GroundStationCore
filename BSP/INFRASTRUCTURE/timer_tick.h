#ifndef __TIMER_TICK_H
#define __TIMER_TICK_H

#include "stm32f4xx.h"

/* 外部可调用函数声明 */
void System_Timebase_Init(void);
void Delay_us(uint16_t us);
void Delay_ms(uint32_t ms);

#endif /* __TIMER_TICK_H */
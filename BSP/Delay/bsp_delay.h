#ifndef __BSP_DELAY_H
#define __BSP_DELAY_H

#include "stm32f4xx.h"                  // Device header

void delay_us(uint32_t us);
void delay_ms(uint32_t ms);
void delay_s(uint32_t s);

#endif

#include "stm32f4xx.h"
#include "drv_ssd1963.h"
#include "delay.h"

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    LCD_Init();

    while(1)
    {
        
    }
}


#include "stm32f4xx.h"
#include "bsp_drv_ssd1963.h"
#include "timer_tick.h"

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    System_Timebase_Init();

    LCD_Init();

    LCD_Clear(0xBBBB);
    while(1)
    {

    }
}


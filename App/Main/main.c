#include "stm32f4xx.h"
#include "bsp_drv_ssd1963.h"

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    LCD_Init();

    LCD_Clear(0xAAAA); // 黑色
    while(1)
    {

    }
}


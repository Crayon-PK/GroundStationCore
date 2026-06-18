#include "lcd_ssd1963.h"
#include "timer_tick.h"
#include "touch_gt911.h"
#include "telemetry_uart.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "tasks_manager.h"
#include "page_home.h"

extern void Telemetry_Rx_Callback(uint8_t* pData, uint16_t len);

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    
    // 硬件初始化
    System_Timebase_Init();
    LCD_Init();
    GT911_Init();
    Telemetry_UART_Init();

    // 注册UART接收回调
    Telemetry_UART_RegisterRxCallback(Telemetry_Rx_Callback);

    // 初始化LVGL图形库
    lv_init();
    lv_port_disp_init(); 
    lv_port_indev_init();

    // 任务初始化
    Tasks_Manager_Init();
    while (1);
}

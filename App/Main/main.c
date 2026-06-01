#include "lcd_ssd1963.h"
#include "timer_tick.h"
#include "touch_gt911.h"

CT_Point_t touch_points[5]; 

int main(void)
{
    // 基础初始化
    System_Timebase_Init();
    LCD_Init();
    
    // 检查 GT911 初始化
    if (GT911_Init() == 0)
    {
        LCD_Clear(0xFFFF); // 初始化成功，刷白屏作为画板
    }
    else
    {
        LCD_Clear(0xF800); // 初始化失败，刷红屏报警
        while(1); 
    }

    while (1)
    {
        // 轮询扫描触摸屏，最大读取1个点即可（单指测试）
        uint8_t tp_cnt = GT911_Scan(touch_points, 1);
        
        if (tp_cnt > 0)
        {
            // 获取当前触摸点的坐标
            uint16_t x = touch_points[0].x;
            uint16_t y = touch_points[0].y;
            
            x = 800 - touch_points[0].x;

            // 防止画方块时边缘溢出 LCD 屏幕边界
            // 假设你的屏幕宽高是定义的，这里做个安全保护
            if (x < 790 && y < 470) 
            {
                // 在触摸的中心位置画一个 10x10 的黑色(0x0000)小方块
                LCD_Fill_Solid(x, y, x + 10, y + 10, 0x0000);
            }
        }
        Delay_ms(10); 
    }
}
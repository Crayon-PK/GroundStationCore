#include "lcd_ssd1963.h"
#include "timer_tick.h"
#include "touch_gt911.h"

/* 必须引入 FreeRTOS 的核心头文件 */
#include "FreeRTOS.h"
#include "task.h"

/* 触摸数据缓存 */
CT_Point_t touch_points[5]; 

/* 声明任务句柄（用于管理任务，可选） */
TaskHandle_t TouchTask_Handler = NULL;
TaskHandle_t BlinkTask_Handler = NULL;

/* 声明任务函数原型 */
void vTask_TouchPad(void *pvParameters);
void vTask_Blink(void *pvParameters);

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    // 1. 硬件基础初始化（此时依然是裸机环境）
    System_Timebase_Init();
    LCD_Init();
    
    // 2. 检查 GT911 初始化
    if (GT911_Init() == 0)
    {
        LCD_Clear(0xFFFF); // 初始化成功，刷白屏作为画板
    }
    else
    {
        LCD_Clear(0xF800); // 初始化失败，刷红屏报警
        while(1); 
    }

    // 3. 【核心动作】：动态创建并行任务
    // 创建触摸画板任务
    xTaskCreate((TaskFunction_t )vTask_TouchPad,     // 任务函数指针
                (const char* )"Task_Touch",       // 任务调试别名
                (uint16_t       )512,                // 分配的堆栈大小（512字 = 2048字节，因为有触摸和画图算法，给大点）
                (void* )NULL,               // 传递给任务的参数
                (UBaseType_t    )3,                  // 任务优先级（触摸画板需要实时响应，给高优先级 3）
                (TaskHandle_t* )&TouchTask_Handler);// 任务句柄

    // 创建后台闪烁/状态指示任务
    xTaskCreate((TaskFunction_t )vTask_Blink, 
                (const char* )"Task_Blink", 
                (uint16_t       )128,                // 简单跑马灯，128字堆栈足够
                (void* )NULL, 
                (UBaseType_t    )2,                  // 优先级给 2，比触摸任务低
                (TaskHandle_t* )&BlinkTask_Handler);

    // 4. 【大功告成】：启动 FreeRTOS 任务调度器
    // 这一步一敲下，CPU 就会开始触发 SVC 中断，彻底告别 main 的 while(1)，完全由操作系统接管
    vTaskStartScheduler();          

    // 如果操作系统正常运行，代码绝对不会执行到这里！
    while (1)
    {
        LCD_Clear(0x001F); // 如果跑到了这里，说明堆内存不够，系统崩溃了，刷蓝屏报警
    }
}

/**
  * @brief  触摸画板主任务
  */
void vTask_TouchPad(void *pvParameters)
{
    uint8_t tp_cnt = 0;
    uint16_t x = 0, y = 0;

    while (1)
    {
        // 轮询扫描触摸屏
        tp_cnt = GT911_Scan(touch_points, 1);
        
        if (tp_cnt > 0)
        {
            x = 800 - touch_points[0].x;
            y = touch_points[0].y;
            
            if (x < 790 && y < 470) 
            {
                // 在触摸位置画黑色小方块
                LCD_Fill_Solid(x, y, x + 10, y + 10, 0x0000);
            }
        }
        
        // 【避坑铁律】：绝对不能再用 Delay_ms(10)！
        // 必须换用系统的非阻塞延时。在这 10ms 挂起期间，CPU 会自动去跑下面的 Blink 任务
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}

/**
  * @brief  后台状态指示任务（验证多任务是否并发成功）
  */
void vTask_Blink(void *pvParameters)
{
    while (1)
    {
        // 在屏幕的左上角极小的区域（0,0 到 10,10）交替闪烁红色和绿色
        // 如果你一边用手指在屏幕上流畅地画方块，左上角的色块还在以 500ms 的频率独自闪烁，说明任务切换完美！
        LCD_Fill_Solid(0, 0, 10, 10, 0xF800); // 红色
        vTaskDelay(pdMS_TO_TICKS(500));        // 挂起 500ms
        
        LCD_Fill_Solid(0, 0, 10, 10, 0x07E0); // 绿色
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
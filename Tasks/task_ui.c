#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "lvgl.h"
#include "page_home.h"

extern SemaphoreHandle_t g_lvgl_mutex;

// ================================================================
// vTask_LVGL_System
// 职责：驱动 LVGL 渲染引擎，每 5ms 调用一次 lv_timer_handler()
// 注意：锁等待最多 4ms，避免在等锁上消耗整个周期
// ================================================================
void vTask_LVGL_System(void *parameters)
{
    while(1)
    {
        if(xSemaphoreTake(g_lvgl_mutex, pdMS_TO_TICKS(4)) == pdTRUE)
        {
            lv_timer_handler();
            xSemaphoreGive(g_lvgl_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(5));   // 5ms 周期 = 200Hz 渲染上限
    }
}

// ================================================================
// vTask_Data_Refresh
// 职责：从 DataPool 读取最新飞行数据，驱动 LVGL 标签刷新
// 频率：20ms（50Hz）—— 比原来 50ms(20Hz) 提升 2.5 倍
// 锁等待：最多 15ms，给 LVGL_System 足够的喘息时间
// ================================================================
void vTask_Data_Refresh(void *parameters)
{
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(20));      // 20ms 刷新周期

        if (g_lvgl_mutex == NULL) continue;

        if (xSemaphoreTake(g_lvgl_mutex, pdMS_TO_TICKS(15)) == pdTRUE)
        {
            Page_Home_Update();
            xSemaphoreGive(g_lvgl_mutex);
        }
    }
}
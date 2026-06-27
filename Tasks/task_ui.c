#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "lvgl.h"
#include "ui_manager.h"

extern SemaphoreHandle_t g_lvgl_mutex;

void vTask_LVGL_System(void *parameters)
{
    while(1)
    {
        if(xSemaphoreTake(g_lvgl_mutex, pdMS_TO_TICKS(4)) == pdTRUE)
        {
            lv_timer_handler();
            xSemaphoreGive(g_lvgl_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void vTask_Data_Refresh(void *parameters)
{
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(20));

        if (g_lvgl_mutex == NULL) continue;

        if (xSemaphoreTake(g_lvgl_mutex, pdMS_TO_TICKS(15)) == pdTRUE)
        {
            UI_Manager_Update();
            xSemaphoreGive(g_lvgl_mutex);
        }
    }
}
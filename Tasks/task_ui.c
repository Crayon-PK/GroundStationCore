#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "lvgl.h"
#include "page_home.h"

SemaphoreHandle_t g_lvgl_mutex = NULL;

void vTask_LVGL_System(void *parameters)
{
    g_lvgl_mutex = xSemaphoreCreateMutex();

    Page_Home_Create();

    while(1)
    {
        if(xSemaphoreTake(g_lvgl_mutex, pdMS_TO_TICKS(20)) == pdTRUE)
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
        vTaskDelay(pdMS_TO_TICKS(50));

        if (g_lvgl_mutex == NULL) continue;

        if (xSemaphoreTake(g_lvgl_mutex, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            Page_Home_Update();
            xSemaphoreGive(g_lvgl_mutex);
        }
    }
}
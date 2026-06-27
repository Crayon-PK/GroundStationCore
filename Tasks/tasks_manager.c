#include "tasks_manager.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "data_pool.h"
#include "ui_manager.h"

QueueHandle_t     g_mavlink_queue = NULL;
SemaphoreHandle_t g_lvgl_mutex    = NULL;

void vTask_MAVLink_Parser(void *pvParameters);
void vTask_LVGL_System(void *pvParameters);
void vTask_Data_Refresh(void *pvParameters);

void vApplicationMallocFailedHook(void)
{
    // 内存分配失败时触发，可在此设置断点定位问题
    while(1);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    // 栈溢出时触发，pcTaskName 指示出问题的任务名
    (void)xTask;
    (void)pcTaskName;
    while(1);
}

void Tasks_Manager_Init(void)
{
    g_mavlink_queue = xQueueCreate(256, sizeof(uint8_t));
    g_lvgl_mutex    = xSemaphoreCreateMutex();

    DataPool_Init();
    UI_Manager_Init();

    xTaskCreate(vTask_MAVLink_Parser, "MAVLink_Parser", 512,  NULL, 4, NULL);
    xTaskCreate(vTask_LVGL_System,    "LVGL_System",    2048, NULL, 3, NULL);
    xTaskCreate(vTask_Data_Refresh,   "DataRefresh",    512,  NULL, 2, NULL);

    vTaskStartScheduler();
}
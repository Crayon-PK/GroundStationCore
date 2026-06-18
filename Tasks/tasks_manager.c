#include "tasks_manager.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "data_pool.h"

QueueHandle_t g_mavlink_queue = NULL;

void vTask_MAVLink_Parser(void *pvParameters);
void vTask_LVGL_System(void *pvParameters);
void vTask_Data_Refresh(void *pvParameters);

void vApplicationMallocFailedHook(void)
{
    // 打断点在这里，一旦内存分配失败就会进来
    while(1);
}

void Tasks_Manager_Init(void)
{
    g_mavlink_queue = xQueueCreate(256, sizeof(uint8_t));

    xTaskCreate(vTask_MAVLink_Parser, "MAVLink_Parser", 512,  NULL, 5, NULL);
    xTaskCreate(vTask_LVGL_System,    "LVGL_System",    1024, NULL, 4, NULL);
    xTaskCreate(vTask_Data_Refresh,   "DataRefresh",    256,  NULL, 3, NULL);

    vTaskStartScheduler();
}
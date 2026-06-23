#include "tasks_manager.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "data_pool.h"
#include "page_home.h"

QueueHandle_t g_mavlink_queue = NULL;
SemaphoreHandle_t g_lvgl_mutex = NULL;

void vTask_MAVLink_Parser(void *pvParameters);
void vTask_LVGL_System(void *pvParameters);
void vTask_Data_Refresh(void *pvParameters);

void vApplicationMallocFailedHook(void)
{
    // 打断点在这里，一旦内存分配失败就会进来
    while(1);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    // 如果是栈溢出，程序会死在这里，pcTaskName 会显示是哪个任务漏了
    while(1);
}

void Tasks_Manager_Init(void)
{
    g_mavlink_queue = xQueueCreate(256, sizeof(uint8_t));
    g_lvgl_mutex = xSemaphoreCreateMutex();

    DataPool_Init();
    Page_Home_Create();

    xTaskCreate(vTask_MAVLink_Parser, "MAVLink_Parser", 512,  NULL, 4, NULL);
    xTaskCreate(vTask_LVGL_System,    "LVGL_System",    2048, NULL, 3, NULL);
    xTaskCreate(vTask_Data_Refresh,   "DataRefresh",    512,  NULL, 2, NULL);

    vTaskStartScheduler();
}
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "lcd_ssd1963.h"
#include "timer_tick.h"
#include "touch_gt911.h"
#include "telemetry_uart.h"

/* 引入 LVGL 核心组件 */
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"

/* 引入 MAVLink 核心头文件 */
#include "mavlink.h"

/* ================== 全局变量与句柄 ================== */
TaskHandle_t LVGLTask_Handler = NULL;
TaskHandle_t MAVLinkTask_Handler = NULL;

QueueHandle_t UartRxQueue;          // 串口数据接收队列
SemaphoreHandle_t AttitudeMutex;    // 姿态数据保护锁

// 存放飞控姿态的结构体 (加入诊断统计变量)
typedef struct {
    float roll;
    float pitch;
    float yaw;
    uint32_t total_packets;     // 记录成功解析的任何包总数
    uint32_t heartbeat_count;   // 记录心跳包总数
    uint32_t attitude_count;    // 记录姿态包总数
} AttitudeData_t;

AttitudeData_t g_attitude = {0};

/* ================== 局部函数声明 ================== */
void vTask_LVGL_Driver(void *pvParameters);
void vTask_MAVLink_Parser(void *pvParameters);
static void btn_click_event_cb(lv_event_t * e);
void Telemetry_Rx_Callback(uint8_t* pData, uint16_t len);

/* ================== 主函数 ================== */
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    
    // 1. 硬件初始化
    System_Timebase_Init();
    LCD_Init();
    GT911_Init();
    
    // 初始化遥测串口
    Telemetry_UART_Init();
    Telemetry_UART_RegisterRxCallback(Telemetry_Rx_Callback);

    // 2. 初始化 LVGL
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();

    // 3. 创建 FreeRTOS IPC 机制
    UartRxQueue = xQueueCreate(1024, sizeof(uint8_t));
    AttitudeMutex = xSemaphoreCreateMutex();

    // 4. 创建任务
    xTaskCreate((TaskFunction_t )vTask_LVGL_Driver, 
                (const char* )"Task_LVGL", 
                (uint16_t       )1024, 
                (void* )NULL, 
                (UBaseType_t    )3,    
                (TaskHandle_t* )&LVGLTask_Handler);

    xTaskCreate((TaskFunction_t )vTask_MAVLink_Parser, 
                (const char* )"Task_MAVLink", 
                (uint16_t       )1024, 
                (void* )NULL, 
                (UBaseType_t    )4,    
                (TaskHandle_t* )&MAVLinkTask_Handler);

    // 5. 启动调度器
    vTaskStartScheduler();          

    while (1);
}

/* ================== 串口接收回调 (中断环境) ================== */
void Telemetry_Rx_Callback(uint8_t* pData, uint16_t len)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    for(uint16_t i = 0; i < len; i++) {
        xQueueSendFromISR(UartRxQueue, &pData[i], &xHigherPriorityTaskWoken);
    }
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* ================== MAVLink 解析任务 ================== */
void vTask_MAVLink_Parser(void *pvParameters)
{
    uint8_t c;
    mavlink_message_t msg;
    mavlink_status_t status;

    while (1)
    {
        if (xQueueReceive(UartRxQueue, &c, portMAX_DELAY) == pdTRUE) 
        {
            if (mavlink_parse_char(MAVLINK_COMM_0, c, &msg, &status)) 
            {
                // 成功解析出数据包，进行分类统计
                if (xSemaphoreTake(AttitudeMutex, portMAX_DELAY) == pdTRUE)
                {
                    g_attitude.total_packets++; 

                    if (msg.msgid == MAVLINK_MSG_ID_HEARTBEAT) {
                        g_attitude.heartbeat_count++;
                    }
                    else if (msg.msgid == MAVLINK_MSG_ID_ATTITUDE) {
                        g_attitude.attitude_count++;
                        mavlink_attitude_t att;
                        mavlink_msg_attitude_decode(&msg, &att);
                        g_attitude.roll  = att.roll;
                        g_attitude.pitch = att.pitch;
                        g_attitude.yaw   = att.yaw;
                    }
                    xSemaphoreGive(AttitudeMutex);
                }
            }
        }
    }
}

/* ================== LVGL UI 刷新任务 ================== */
void vTask_LVGL_Driver(void *pvParameters)
{
    // --- 测试按钮 ---
    lv_obj_t * btn = lv_btn_create(lv_scr_act());           
    lv_obj_set_size(btn, 200, 80);                          
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, -80);             
    lv_obj_add_event_cb(btn, btn_click_event_cb, LV_EVENT_CLICKED, NULL); 

    lv_obj_t * btn_label = lv_label_create(btn);                
    lv_label_set_text(btn_label, "Click Count: 0");             
    lv_obj_center(btn_label);                                   

    // --- 姿态与诊断标签 ---
    lv_obj_t * att_label = lv_label_create(lv_scr_act());
    lv_obj_align(att_label, LV_ALIGN_CENTER, 0, 80); 
    lv_label_set_text(att_label, "Waiting for MAVLink...\n...");

    AttitudeData_t local_att = {0}; 

    while (1)
    {
        if (xSemaphoreTake(AttitudeMutex, pdMS_TO_TICKS(5)) == pdTRUE) 
        {
            local_att = g_attitude;
            xSemaphoreGive(AttitudeMutex);
            
            // 使用 %lu 打印无符号整数统计，用 %d 打印整数角度（规避浮点数打印问题）
            lv_label_set_text_fmt(att_label, 
                "Total Pkg: %lu\nHeartbeats: %lu\nAttitudes: %lu\nRoll: %d deg\nPitch: %d deg\nYaw: %d deg", 
                local_att.total_packets, 
                local_att.heartbeat_count, 
                local_att.attitude_count,
                (int)(local_att.roll * 57.3f), 
                (int)(local_att.pitch * 57.3f), 
                (int)(local_att.yaw * 57.3f));
        }

        lv_timer_handler(); 
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}

/* ================== 按钮回调函数 ================== */
static void btn_click_event_cb(lv_event_t * e)
{
    static uint32_t cnt = 0;
    lv_obj_t * btn = lv_event_get_target(e);                
    lv_obj_t * label = lv_obj_get_child(btn, 0);            
    
    cnt++;
    lv_label_set_text_fmt(label, "Click Count: %ld", cnt);
}

/* ================== 系统心跳 ================== */
void vApplicationTickHook(void)
{
    lv_tick_inc(1);
}
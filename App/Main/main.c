#include "FreeRTOS.h"
#include "task.h"
#include "lcd_ssd1963.h"
#include "timer_tick.h"
#include "touch_gt911.h"
/* 引入升级后的 LVGL 核心组件 */
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"

/* 任务管理句柄 */
TaskHandle_t LVGLTask_Handler = NULL;

void vTask_LVGL_Driver(void *pvParameters);
static void btn_click_event_cb(lv_event_t * e);

int main(void)
{
    // 按你的配置，FreeRTOS 下必须中断分组全为 4
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    
    // 1. 硬件时基源初始化
    System_Timebase_Init();
    LCD_Init();
    GT911_Init();
    // 2. 初始化 LVGL 核心图形库
    lv_init();
    
    // 3. 注册你的 SSD1963 屏幕驱动与 GT911 触摸驱动
    lv_port_disp_init();
    lv_port_indev_init();

    // 4. 创建 LVGL 核心高优先级执行任务
    xTaskCreate((TaskFunction_t )vTask_LVGL_Driver, 
                (const char* )"Task_LVGL", 
                (uint16_t       )1024, // 留足 4096 字节
                (void* )NULL, 
                (UBaseType_t    )3,    // 优先级给 3 保证流畅度
                (TaskHandle_t* )&LVGLTask_Handler);

    // 5. 启动系统任务调度器
    vTaskStartScheduler();          

    while (1);
}

/**
  * @brief  LVGL 核心轮询驱动任务
  */
void vTask_LVGL_Driver(void *pvParameters)
{
    /* ==========================================
     * 创建一个可触控的按钮组件进行系统联调测试
     * ========================================== */
    lv_obj_t * btn = lv_btn_create(lv_scr_act());           // 创建按钮
    lv_obj_set_size(btn, 200, 80);                          // 设置按钮大小
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);              // 按钮在 800x480 正中央居中
    lv_obj_add_event_cb(btn, btn_click_event_cb, LV_EVENT_CLICKED, NULL); // 绑定点击回调

    lv_obj_t * label = lv_label_create(btn);                // 在按钮内创建文字标签
    lv_label_set_text(label, "Click Count: 0");             // 初始文本
    lv_obj_center(label);                                   // 文字在按钮内居中

    while (1)
    {
        // 核心周期处理函数：会自动调用你的 disp_flush 和 touchpad_read
        lv_timer_handler(); 
        
        // 配合时钟切出执行权 10ms
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}

/**
  * @brief  按钮点击事件回调函数
  */
static void btn_click_event_cb(lv_event_t * e)
{
    static uint32_t cnt = 0;
    lv_obj_t * btn = lv_event_get_target(e);                // 获取触发事件的按钮对象
    lv_obj_t * label = lv_obj_get_child(btn, 0);            // 获取按钮内的第一个子对象（即文本标签）
    
    cnt++;
    // 动态刷新按钮内部的点击计数值，用作屏幕与触摸同步的终极验证
    lv_label_set_text_fmt(label, "Click Count: %ld", cnt);
}

/**
  * @brief  FreeRTOS 系统时基钩子（由操作系统时钟每 1ms 自动调用）
  */
void vApplicationTickHook(void)
{
    // 为 LVGL 提供精准的生命心跳
    lv_tick_inc(1);
}
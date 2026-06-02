#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* 1. 基础全局配置 */
#define configUSE_PREEMPTION                    1       // 1:抢占式调度
#define configUSE_TIME_SLICING                  1       // 1:同优先级时间片轮转
#define configCPU_CLOCK_HZ                      168000000 // STM32F407主频
#define configTICK_RATE_HZ                      1000    // 1ms 一次系统心跳
#define configMAX_PRIORITIES                    32      // 最大优先级数量
#define configMINIMAL_STACK_SIZE                ((unsigned short)130)
#define configTOTAL_HEAP_SIZE                   ((size_t)(15 * 1024)) // 15KB 堆空间

/* 核心修复：新版内核严格校验，只允许定义位数，禁止同时定义16位使能宏 */
#define configTICK_TYPE_WIDTH_IN_BITS           TICK_TYPE_WIDTH_32_BITS

/* 核心钩子函数显式关闭（新版内核要求必须显式定义为0或1） */
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configUSE_MALLOC_FAILED_HOOK            0

/* 内存分配与软件定时器 */
#define configSUPPORT_DYNAMIC_ALLOCATION        1       
#define configSUPPORT_STATIC_ALLOCATION         0       
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH                5
#define configTIMER_TASK_STACK_DEPTH            (configMINIMAL_STACK_SIZE * 2)

/* 可选的 API 函数控制 */
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet                1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_vTaskDelay                      1       

/* Cortex-M 内核的中断优先级配置 */
#define configPRIO_BITS                         4
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY   15    // 最低中断优先级
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5  // 可调用 RTOS API 的最高优先级

/* 硬件所需的寄存器位移转换 */
#define configKERNEL_INTERRUPT_PRIORITY         ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/* 核心中断向量映射 */
#define vPortSVCHandler                         SVC_Handler
#define xPortPendSVHandler                      PendSV_Handler
#define xPortSysTickHandler                     SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
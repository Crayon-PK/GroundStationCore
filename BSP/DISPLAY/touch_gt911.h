#ifndef __TOUCH_GT911_H
#define __TOUCH_GT911_H

#include "stm32f4xx.h"

/* GT911 I2C 从机设备地址 */
#define GT911_CMD_WR                0xBA    // 写操作地址
#define GT911_CMD_RD                0xBB    // 读操作地址

/* 核心控制与状态寄存器 */
#define GT911_REG_CTRL              0x8040  // 控制寄存器 (写0x02软件复位)
#define GT911_REG_PID               0x8140  // 产品ID寄存器 (4字节ASCII)
#define GT911_REG_TPINFO            0x814E  // 触摸状态寄存器 (含Buffer状态与点数)
#define GT911_REG_TP1               0x8150  // 触摸点1数据寄存器起始地址

/* 触摸状态寄存器掩码 */
#define GT911_TPINFO_MASK_STA       0x80    // Buffer status 掩码 (是否有新数据)
#define GT911_TPINFO_MASK_CNT       0x0F    // Number of touch points 掩码 (点数)

/* 硬件引脚映射定义 */
#define CT_SCL_PORT                 GPIOB
#define CT_SCL_PIN                  GPIO_Pin_0
#define CT_SDA_PORT                 GPIOF
#define CT_SDA_PIN                  GPIO_Pin_11
#define CT_RST_PORT                 GPIOC
#define CT_RST_PIN                  GPIO_Pin_13
#define CT_INT_PORT                 GPIOB
#define CT_INT_PIN                  GPIO_Pin_1

/* 触摸点坐标数据结构 */
typedef struct
{
    uint16_t x;     // X 坐标
    uint16_t y;     // Y 坐标
    uint16_t size;  // 触摸点大小
} CT_Point_t;

/* 外部可调用函数声明 */
void touch_GPIO_Init(void);
int GT911_Init(void);
uint8_t GT911_Scan(CT_Point_t *points, uint8_t max_points_to_read);

#endif /* __TOUCH_GT911_H */

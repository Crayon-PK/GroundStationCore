#ifndef __BSP_DRV_SSD1963_H
#define __BSP_DRV_SSD1963_H

#include "stm32f4xx.h"                  // Device header

typedef struct{
	__IO uint16_t LCD_REG;
	__IO uint16_t LCD_RAM;
	
}LCD_TypeDef;

#define LCD_BASE        ((uint32_t)(0x6C000000 | 0x0000007E))
#define LCD             ((LCD_TypeDef *) LCD_BASE)

#define CMD_NOP                      0x00
#define CMD_SOFT_RESET               0x01
#define CMD_GET_POWER_MODE           0x0A
#define CMD_GET_ADDRESS_MODE         0x0B
#define CMD_DISPLAY_MODE             0x0D
#define CMD_TEAR_EFFECT_STATUS       0x0E
#define CMD_ENTER_SLEEP_MODE         0x10
#define CMD_EXIT_SLEEP_MODE          0x11
#define CMD_ENTER_PARTIAL_MODE       0x12
#define CMD_ENTER_NORMAL_MODE        0x13 
#define CMD_EXIT_INVERT_MODE         0x20 
#define CMD_ENTER_INVERT_MODE        0x21
#define CMD_SET_GAMMA_CURVE          0x26 
#define CMD_SET_DISPLAY_OFF          0x28
#define CMD_SET_DISPLAY_ON           0x29
#define CMD_SET_COLUMN_ADDRESS       0x2A
#define CMD_SET_PAGE_ADDRESS         0x2B
#define CMD_WRITE_MEMORY_START       0x2C
#define CMD_READ_MEMORY_START        0x2E
#define CMD_SET_PARTIAL_AREA         0x30
#define CMD_SET_SCROLL_AREA          0x33
#define CMD_SET_TEAR_OFF             0x34
#define CMD_SET_TEAR_ON              0x35
#define CMD_SET_ADDRESS_MODE         0x36
#define CMD_SET_SCROLL_START         0x37
#define CMD_EXIT_IDLE_MODE           0x38
#define CMD_ENTER_IDLE_MODE          0x39
#define CMD_WRITE_MEMORY_CONTINUE    0x3C
#define CMD_READ_MEMORY_CONTINUE     0x3E
#define CMD_SET_TEAR_SCANLINE        0x44
#define CMD_GET_SCANLINE             0x45
#define CMD_READ_DDB                 0xA1  
#define CMD_SET_LCD_MODE_            0xB0 
#define CMD_GET_LCD_MODE             0xB1
#define CMD_SET_HORI_PERIOD          0xB4
#define CMD_GET_HORI_PERIOD          0xB5
#define CMD_SET_VERT_PERIOD          0xB6
#define CMD_GET_VERT_PERIOD          0xB7
#define CMD_SET_GPIO_CONF            0xB8 
#define CMD_GET_GPIO_CONF            0xB9  
#define CMD_SET_GPIO_VALUE           0xBA 
#define CMD_GET_GPIO_STATUS          0xBB
#define CMD_SET_POST_PROC            0xBC
#define CMD_GET_POST_PROC            0xBD
#define CMD_SET_PWM_CONF             0xBE
#define CMD_GET_PWM_CONF             0xBF
#define CMD_SET_LCD_GEN0             0xC0
#define CMD_GET_LCD_GEN0             0xC1
#define CMD_SET_LCD_GEN1             0xC2
#define CMD_GET_LCD_GEN1             0xC3
#define CMD_SET_LCD_GEN2             0xC4
#define CMD_GET_LCD_GEN2             0xC5
#define CMD_SET_LCD_GEN3             0xC6
#define CMD_GET_LCD_GEN3             0xC7
#define CMD_SET_GPIO0_ROP            0xC8
#define CMD_GET_GPIO0_ROP            0xC9
#define CMD_SET_GPIO1_ROP            0xCA
#define CMD_GET_GPIO1_ROP            0xCB
#define CMD_SET_GPIO2_ROP            0xCC
#define CMD_GET_GPIO2_ROP            0xCD
#define CMD_SET_GPIO3_ROP            0xCE
#define CMD_GET_GPIO3_ROP            0xCF
#define CMD_SET_DBC_CONF             0xD0
#define CMD_GET_DBC_CONF             0xD1
#define CMD_SET_DBC_TH               0xD4
#define CMD_GET_DBC_TH               0xD5
#define CMD_SET_PLL                  0xE0
#define CMD_SET_PLL_MN               0xE2
#define CMD_GET_PLL_MN               0xE3
#define CMD_GET_PLL_STATUS           0xE4
#define CMD_SET_DEEP_SLEEP           0xE5
#define CMD_SET_LSHIFT_FREQ          0xE6
#define CMD_GET_LSHIFT_FREQ          0xE7
#define CMD_SET_PIXEL_DATA_INTERFACE 0xF0
#define CMD_GET_PIXEL_DATA_INTERFACE 0xF1

// 片选引脚 CS -> 对应 FSMC_NE4 (PG12)
#define LCD_CS_PORT          GPIOG
#define LCD_CS_PIN           GPIO_Pin_12
#define LCD_CS_PIN_SOURCE    GPIO_PinSource12

// 命令/数据选择引脚 RS -> 对应 FSMC_A6 (PF12)
#define LCD_RS_PORT          GPIOF
#define LCD_RS_PIN           GPIO_Pin_12
#define LCD_RS_PIN_SOURCE    GPIO_PinSource12

// 背光控制引脚 BL (PB15)
#define LCD_BL_PORT          GPIOB
#define LCD_BL_PIN           GPIO_Pin_15

void LCD_Init(void);
uint16_t LCD_Read_ID(void);
void LCD_SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_Color_Fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t *color_p);
void LCD_Fill_Solid(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void LCD_Clear(uint16_t color);
	
#endif

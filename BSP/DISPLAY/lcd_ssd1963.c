#include "lcd_ssd1963.h"
#include "timer_tick.h"
#include <stddef.h>

/**
  * @brief  初始化FSMC接口用于LCD控制器
  */
void LCD_FSMC_Init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD | 
                           RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOF | 
                           RCC_AHB1Periph_GPIOG, ENABLE);
    RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);
    
    // 初始化 GPIOD (D0,D1,D8,D9,D10,D14,D15, RD,WR)
    GPIO_Init(GPIOD, &(GPIO_InitTypeDef){
        .GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | 
                      GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15,
        .GPIO_Mode  = GPIO_Mode_AF,
        .GPIO_OType = GPIO_OType_PP,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_PuPd  = GPIO_PuPd_UP
    });
    
    // 初始化 GPIOE (D4 ~ D12)
    GPIO_Init(GPIOE, &(GPIO_InitTypeDef){
        .GPIO_Pin   = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | 
                      GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15,
        .GPIO_Mode  = GPIO_Mode_AF,
        .GPIO_OType = GPIO_OType_PP,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_PuPd  = GPIO_PuPd_UP
    });
    
    // 命令/数据选择线 RS
    GPIO_Init(LCD_RS_PORT, &(GPIO_InitTypeDef){
        .GPIO_Pin   = LCD_RS_PIN,
        .GPIO_Mode  = GPIO_Mode_AF,
        .GPIO_OType = GPIO_OType_PP,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_PuPd  = GPIO_PuPd_UP
    });
    
    // 片选线 CS
    GPIO_Init(LCD_CS_PORT, &(GPIO_InitTypeDef){
        .GPIO_Pin   = LCD_CS_PIN,
        .GPIO_Mode  = GPIO_Mode_AF,
        .GPIO_OType = GPIO_OType_PP,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_PuPd  = GPIO_PuPd_UP
    });
    
    // 背光控制线 BL
    GPIO_Init(LCD_BL_PORT, &(GPIO_InitTypeDef){
        .GPIO_Pin   = LCD_BL_PIN,
        .GPIO_Mode  = GPIO_Mode_OUT,
        .GPIO_OType = GPIO_OType_PP,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_PuPd  = GPIO_PuPd_NOPULL
    });
    GPIO_SetBits(LCD_BL_PORT, LCD_BL_PIN);
    
    // 复用映射配置
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource0,  GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource1,  GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource4,  GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource5,  GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource8,  GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource9,  GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FSMC);
    
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource7,  GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource8,  GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource9,  GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource10, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource12, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource15, GPIO_AF_FSMC);
    
    GPIO_PinAFConfig(LCD_RS_PORT, LCD_RS_PIN_SOURCE, GPIO_AF_FSMC);
    GPIO_PinAFConfig(LCD_CS_PORT, LCD_CS_PIN_SOURCE, GPIO_AF_FSMC);
    
    FSMC_NORSRAMTimingInitTypeDef timing = {
        .FSMC_AddressSetupTime      = 0x02,
        .FSMC_AddressHoldTime       = 0x00,
        .FSMC_DataSetupTime         = 0x3C,
        .FSMC_BusTurnAroundDuration = 0x00,
        .FSMC_CLKDivision           = 0x00,
        .FSMC_DataLatency           = 0x00,
        .FSMC_AccessMode            = FSMC_AccessMode_A,
    };
    
    FSMC_NORSRAMInit(&(FSMC_NORSRAMInitTypeDef){
        .FSMC_Bank                  = FSMC_Bank1_NORSRAM4,
        .FSMC_DataAddressMux        = FSMC_DataAddressMux_Disable,
        .FSMC_MemoryType            = FSMC_MemoryType_SRAM,
        .FSMC_MemoryDataWidth       = FSMC_MemoryDataWidth_16b,
        .FSMC_BurstAccessMode       = FSMC_BurstAccessMode_Disable,
        .FSMC_AsynchronousWait      = FSMC_AsynchronousWait_Disable,
        .FSMC_WaitSignalPolarity    = FSMC_WaitSignalPolarity_Low,
        .FSMC_WrapMode              = FSMC_WrapMode_Disable,
        .FSMC_WaitSignalActive      = FSMC_WaitSignalActive_BeforeWaitState,
        .FSMC_WriteOperation        = FSMC_WriteOperation_Enable,
        .FSMC_WaitSignal            = FSMC_WaitSignal_Disable,
        .FSMC_ExtendedMode          = FSMC_ExtendedMode_Disable,
        .FSMC_WriteBurst            = FSMC_WriteBurst_Disable,
        .FSMC_ReadWriteTimingStruct = &timing,
        .FSMC_WriteTimingStruct     = &timing,
    });
    
    FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE);
}

/**
  * @brief       向LCD写入命令
  * @param       cmd: 要写入的命令值
  */
void LCD_Write_Cmd(uint16_t cmd)
{
    LCD->LCD_REG = cmd;
}

/**
  * @brief       向LCD写入数据
  * @param       data: 要写入的数据值
  */
void LCD_Write_Data(uint16_t data)
{
    LCD->LCD_RAM = data;
}

/**
  * @brief       从LCD读取数据
  * @retval      读取到的16位数据
  */
uint16_t LCD_Read_Data(void)
{
    return LCD->LCD_RAM;
}

/**
  * @brief  切换LCD到快速模式
  * @param  fast_data_setup: 快速模式下的数据建立时间配置值
  */
void LCD_Switch_FastMode(uint32_t fast_data_setup)
{
    FSMC_Bank1->BTCR[7] &= ~(0xFF << 8); 
    FSMC_Bank1->BTCR[7] |= ((fast_data_setup & 0xFF) << 8); 
}

/**
  * @brief  初始化LCD控制器并校验芯片ID
  * @retval 0:成功; -1:FSMC使能超时; -2:未检测到屏幕; -3:未知或兼容芯片
  */
int LCD_Init(void)
{
    uint16_t lcd_id = 0;
    uint32_t timeout = 0xFFFF; // 引入超时计数器
    
    LCD_FSMC_Init();
    
    LCD_Write_Cmd(CMD_SOFT_RESET);
    Delay_ms(20);
    
    LCD_Write_Cmd(CMD_SET_PLL_MN);
    LCD_Write_Data(0x1D);    // M = 29
    LCD_Write_Data(0x02);    // N = 2
    LCD_Write_Data(0x54);    // C[2] = 1
    
    LCD_Write_Cmd(CMD_SET_PLL);
    LCD_Write_Data(0x01);
    Delay_us(100);

    LCD_Write_Cmd(CMD_SET_PLL);
    LCD_Write_Data(0x03);
    
    LCD_Write_Cmd(CMD_SOFT_RESET);
    Delay_ms(10);
    
    // 基础配置：分辨率等
    LCD_Write_Cmd(CMD_SET_LCD_MODE_);
    LCD_Write_Data(0x20);              
    LCD_Write_Data(0x00);              
    LCD_Write_Data((uint16_t)(LCD_WIDTH - 1) >> 8);              
    LCD_Write_Data((uint16_t)(LCD_WIDTH - 1) & 0xFF);              
    LCD_Write_Data((uint16_t)(LCD_HEIGHT - 1) >> 8);              
    LCD_Write_Data((uint16_t)(LCD_HEIGHT - 1) & 0xFF);              
    LCD_Write_Data(0x00);              
    
    // 水平时序
    LCD_Write_Cmd(CMD_SET_HORI_PERIOD); 
    LCD_Write_Data(0x03);               
    LCD_Write_Data(0xAF);               
    LCD_Write_Data(0x00);               
    LCD_Write_Data(0x58);               
    LCD_Write_Data(0x0F);               
    LCD_Write_Data(0x00);               
    LCD_Write_Data(0x00);               
    LCD_Write_Data(0x00);               
    
    // 垂直时序
    LCD_Write_Cmd(CMD_SET_VERT_PERIOD); 
    LCD_Write_Data(0x02);               
    LCD_Write_Data(0x0F);               
    LCD_Write_Data(0x00);               
    LCD_Write_Data(0x10);               
    LCD_Write_Data(0x07);               
    LCD_Write_Data(0x00);               
    LCD_Write_Data(0x00);               
    
    LCD_Write_Cmd(CMD_SET_PIXEL_DATA_INTERFACE);
    LCD_Write_Data(0x03);  // 16位 RGB565 格式
    
    LCD_Write_Cmd(CMD_SET_ADDRESS_MODE);   // 发送 Set Address Mode 命令
    LCD_Write_Data(0x01);

    LCD_Switch_FastMode(0x0F); 
    
    // 检查 FSMC 底层硬件使能状态，防止死锁
    while ((FSMC_Bank1->BTCR[6] & 0x01) == 0)
    {
        if (timeout-- == 0) return -1;
    }

    // 读取芯片 ID 并进行合法性校验
    lcd_id = LCD_Read_ID();
    
    if (lcd_id == 0x0000 || lcd_id == 0xFFFF) 
    {
        return -2;
    }
    
    LCD_Clear(0xFFFF);

    LCD_Write_Cmd(CMD_SET_PWM_CONF); 
    LCD_Write_Data(0x01);
    LCD_Write_Data(0xFF);
    LCD_Write_Data(0x01);
    LCD_Write_Data(0x00);
    LCD_Write_Data(0x00);
    LCD_Write_Data(0x00);
    
    LCD_Write_Cmd(CMD_EXIT_SLEEP_MODE);
    Delay_ms(20);
    
    LCD_Write_Cmd(CMD_SET_DISPLAY_ON);
    
    if (lcd_id != 0x1963 && lcd_id != 0x570B) 
    {
        return -3;
    }

    return 0; // 初始化成功
}

/**
  * @brief       读取LCD控制器ID
  * @retval      16位的LCD控制器ID值
  */
uint16_t LCD_Read_ID(void)
{
    uint16_t id = 0;
    
    LCD_Write_Cmd(CMD_READ_DDB);
    LCD_Read_Data(); // 哑读
    id = LCD_Read_Data();
    id = (id << 8) | LCD_Read_Data();
    
    return id; 
}

/**
  * @brief       设置LCD显示窗口区域
  * @param       x1: 窗口左上角X坐标
  * @param       y1: 窗口左上角Y坐标
  * @param       x2: 窗口右下角X坐标
  * @param       y2: 窗口右下角Y坐标
  */
void LCD_SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    LCD_Write_Cmd(CMD_SET_COLUMN_ADDRESS); 
    LCD_Write_Data(x1 >> 8);
    LCD_Write_Data(x1 & 0xFF);             
    LCD_Write_Data(x2 >> 8);
    LCD_Write_Data(x2 & 0xFF);

    LCD_Write_Cmd(CMD_SET_PAGE_ADDRESS);
    LCD_Write_Data(y1 >> 8);
    LCD_Write_Data(y1 & 0xFF); 
    LCD_Write_Data(y2 >> 8);
    LCD_Write_Data(y2 & 0xFF);

    LCD_Write_Cmd(CMD_WRITE_MEMORY_START);
}

/**
  * @brief       用颜色数组填充指定区域
  * @param       x1: 区域左上角X坐标
  * @param       y1: 区域左上角Y坐标  
  * @param       x2: 区域右下角X坐标
  * @param       y2: 区域右下角Y坐标
  * @param       color_p: 颜色数组指针(需包含足够像素数据)
  */
void LCD_Color_Fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t *color_p)
{
    if (color_p == NULL) return;
    uint32_t total_pixels = (x2 - x1 + 1) * (y2 - y1 + 1);
    LCD_SetWindow(x1, y1, x2, y2);
    for(uint32_t i = 0; i < total_pixels; i++)
    {
        LCD_Write_Data(color_p[i]);
    }
}

/**
  * @brief       用单一颜色填充指定区域
  * @param       x1: 区域左上角X坐标
  * @param       y1: 区域左上角Y坐标
  * @param       x2: 区域右下角X坐标
  * @param       y2: 区域右下角Y坐标
  * @param       color: 16位RGB565颜色值
  */
void LCD_Fill_Solid(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    uint32_t total_pixels = (x2 - x1 + 1) * (y2 - y1 + 1);
    
    LCD_SetWindow(x1, y1, x2, y2);
    for(uint32_t i = 0; i < total_pixels; i++)
    {
        LCD_Write_Data(color);
    }
}

/**
  * @brief       清屏(全屏填充指定颜色)
  * @param       color: 16位RGB565颜色值
  */
void LCD_Clear(uint16_t color)
{
    LCD_Fill_Solid(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, color);
}
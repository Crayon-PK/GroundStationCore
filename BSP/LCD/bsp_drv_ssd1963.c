#include "bsp_drv_ssd1963.h"
#include "timer_tick.h"

void LCD_FSMC_Init()
{
	// 开启所有相关的 GPIO 时钟以及 FSMC 外设时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOG, ENABLE);
	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC,ENABLE);
	
	// 初始化 GPIOD（包含数据线 D0,D1,D8,D9,D10,D14,D15 以及读写控制线 RD,WR）
	GPIO_Init(GPIOD, &(GPIO_InitTypeDef){
        .GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15,
        .GPIO_Mode  = GPIO_Mode_AF,
        .GPIO_OType = GPIO_OType_PP,
        .GPIO_Speed = GPIO_Speed_50MHz,
		.GPIO_PuPd  = GPIO_PuPd_UP
    });
	
	// 初始化 GPIOE（包含数据线 D4 ~ D12）
	GPIO_Init(GPIOE, &(GPIO_InitTypeDef){
        .GPIO_Pin   = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15,
        .GPIO_Mode  = GPIO_Mode_AF,
        .GPIO_OType = GPIO_OType_PP,
        .GPIO_Speed = GPIO_Speed_50MHz,
		.GPIO_PuPd  = GPIO_PuPd_UP
    });
	
	// 初始化 RS 命令/数据选择线
	GPIO_Init(LCD_RS_PORT, &(GPIO_InitTypeDef){
        .GPIO_Pin   = LCD_RS_PIN,
        .GPIO_Mode  = GPIO_Mode_AF,
        .GPIO_OType = GPIO_OType_PP,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_PuPd  = GPIO_PuPd_UP
    });
    
	// 初始化 CS 片选线
    GPIO_Init(LCD_CS_PORT, &(GPIO_InitTypeDef){
        .GPIO_Pin   = LCD_CS_PIN,
        .GPIO_Mode  = GPIO_Mode_AF,
        .GPIO_OType = GPIO_OType_PP,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_PuPd  = GPIO_PuPd_UP
    });
    
	// 初始化 BL 背光控制线
    GPIO_Init(LCD_BL_PORT, &(GPIO_InitTypeDef){
        .GPIO_Pin   = LCD_BL_PIN,
        .GPIO_Mode  = GPIO_Mode_OUT,
        .GPIO_OType = GPIO_OType_PP,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_PuPd  = GPIO_PuPd_NOPULL
    });
    GPIO_SetBits(LCD_BL_PORT, LCD_BL_PIN);  // 初始化完毕立刻点亮背光
	
	// 集中配置引脚复用映射 (将这 20 个引脚的控制权彻底移交给 FSMC 外设)
	// GPIOD 映射
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource0,  GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource1,  GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource4,  GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource5,  GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource8,  GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource9,  GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FSMC);
    
    // GPIOE 映射
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource7,  GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource8,  GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource9,  GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource10, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource12, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource15, GPIO_AF_FSMC);
    
    // RS 和 CS 映射
    GPIO_PinAFConfig(LCD_RS_PORT, LCD_RS_PIN_SOURCE, GPIO_AF_FSMC);
    GPIO_PinAFConfig(LCD_CS_PORT, LCD_CS_PIN_SOURCE, GPIO_AF_FSMC);
	
	// 配置 FSMC 读写时序结构
	FSMC_NORSRAMTimingInitTypeDef timing = {
		.FSMC_AddressSetupTime      = 0x02,
		.FSMC_AddressHoldTime       = 0x00,
		.FSMC_DataSetupTime         = 0x3C,
		.FSMC_BusTurnAroundDuration = 0x00,
		.FSMC_CLKDivision           = 0x00,
		.FSMC_DataLatency           = 0x00,
		.FSMC_AccessMode            = FSMC_AccessMode_A,
	};
	
	// 配置 FSMC 控制器核心参数
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
	
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4,ENABLE);
}

void LCD_Write_Cmd(uint16_t cmd)
{
	LCD->LCD_REG = cmd;
}

void LCD_Write_Data(uint16_t data)
{
	LCD->LCD_RAM = data;
}

uint16_t LCD_Read_Data(void)
{
    return LCD->LCD_RAM;
}

/**
  * @brief  动态切换 FSMC 到高速模式
  * @param  fast_data_setup: 极限数据建立时间
  */
void LCD_Switch_FastMode(uint32_t fast_data_setup)
{
    FSMC_Bank1->BTCR[7] &= ~(0xFF << 8); 
    FSMC_Bank1->BTCR[7] |= (fast_data_setup << 8); 
}

void LCD_Init()
{
	// 初始化硬件通道
	LCD_FSMC_Init();
	
	// 软件复位
	LCD_Write_Cmd(CMD_SOFT_RESET);
	Delay_ms(20);
	
	// 配置 PLL 的 M 和 N
	LCD_Write_Cmd(CMD_SET_PLL_MN);
	LCD_Write_Data(0x1D);	// M = 29
	LCD_Write_Data(0x02);	// N = 2
	LCD_Write_Data(0x54);	// C[2] = 1，使能 MN 配置
	
	// 开启 PLL 
	LCD_Write_Cmd(CMD_SET_PLL);
	LCD_Write_Data(0x01);
	Delay_us(100);

	// 切换屏幕系统时钟到 PLL 输出
	LCD_Write_Cmd(CMD_SET_PLL);
	LCD_Write_Data(0x03);
	
	// 再次软件复位，让屏幕在新的 100MHz 高速时钟下重新投产
	LCD_Write_Cmd(CMD_SOFT_RESET);
	Delay_ms(10);
	
	// 配置 LCD 面板模式与分辨率
	LCD_Write_Cmd(CMD_SET_LCD_MODE_);
	LCD_Write_Data(0x20);              // 参数 1: 24位面板, 极性正常 (根据实际屏幕可调为 0x24)
    LCD_Write_Data(0x00);              // 参数 2: 标准 TFT 模式
    LCD_Write_Data(0x03);              // 参数 3: 水平大小高字节 (799 的高位)
    LCD_Write_Data(0x1F);              // 参数 4: 水平大小低字节 (799 的低位)
    LCD_Write_Data(0x01);              // 参数 5: 垂直大小高字节 (479 的高位)
    LCD_Write_Data(0xDF);              // 参数 6: 垂直大小低字节 (479 的低位)
    LCD_Write_Data(0x00);			   // 参数 7: 默认 RGB 序列
	
	// 配置 LCD 水平时序
	LCD_Write_Cmd(CMD_SET_HORI_PERIOD); 
    LCD_Write_Data(0x03);               // 参数 1: HT 高字节 (943 的高位)
    LCD_Write_Data(0xAF);               // 参数 2: HT 低字节 (943 的低位)
    LCD_Write_Data(0x00);               // 参数 3: HPS 高字节 (88 的高位)
    LCD_Write_Data(0x58);               // 参数 4: HPS 低字节 (88 的低位)
    LCD_Write_Data(0x0F);               // 参数 5: HPW 脉宽 (15)
    LCD_Write_Data(0x00);               // 参数 6: LPS 高字节 (0)
    LCD_Write_Data(0x00);               // 参数 7: LPS 低字节 (0)
    LCD_Write_Data(0x00);               // 参数 8: LPSPP 串行副像素偏置 (0)
	
	// 配置 LCD 垂直时序
	LCD_Write_Cmd(CMD_SET_VERT_PERIOD); // 0xB6
    LCD_Write_Data(0x02);               // 参数 1: VT 高字节 (527 的高位)
    LCD_Write_Data(0x0F);               // 参数 2: VT 低字节 (527 的低位)
    LCD_Write_Data(0x00);               // 参数 3: VPS 高字节 (16 的高位)
    LCD_Write_Data(0x10);               // 参数 4: VPS 低字节 (16 的低位)
    LCD_Write_Data(0x07);               // 参数 5: VPW 脉宽 (7)
    LCD_Write_Data(0x00);               // 参数 6: FPS 高字节 (0)
    LCD_Write_Data(0x00);               // 参数 7: FPS 低字节 (0)
	
	// 设置单片机与大屏的数据接口格式
	LCD_Write_Cmd(CMD_SET_PIXEL_DATA_INTERFACE);
	LCD_Write_Data(0x03);  // 0x03 代表标准的 16位 RGB565 接口格式
	
    // 将 LCD 读写速度加快
	LCD_Switch_FastMode(0x0F); 
	
	// 清屏
	LCD_Clear(0xFFFF);
	
	// 配置内部硬件 PWM 控制器
	LCD_Write_Cmd(CMD_SET_PWM_CONF); 
    LCD_Write_Data(0x01);
    LCD_Write_Data(0xFF);
    LCD_Write_Data(0x01);
    LCD_Write_Data(0x00);
    LCD_Write_Data(0x00);
    LCD_Write_Data(0x00);
	
	// 退出睡眠模式
	LCD_Write_Cmd(CMD_EXIT_SLEEP_MODE);
	Delay_ms(20);
	
	// 开启显示
	LCD_Write_Cmd(CMD_SET_DISPLAY_ON);
}

uint16_t LCD_Read_ID(void)
{
    uint16_t id = 0;
    
    LCD_Write_Cmd(0xA1);
    LCD_Read_Data();
    id = LCD_Read_Data();
    id = (id << 8) | LCD_Read_Data();
    
    return id; 
}

void LCD_SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    // 设置列地址 (X轴)
    LCD_Write_Cmd(CMD_SET_COLUMN_ADDRESS); 
    LCD_Write_Data(x1 >> 8);
    LCD_Write_Data(x1 & 0xFF);             
    LCD_Write_Data(x2 >> 8);
    LCD_Write_Data(x2 & 0xFF);

    // 设置行地址 (Y轴)
    LCD_Write_Cmd(CMD_SET_PAGE_ADDRESS);
    LCD_Write_Data(y1 >> 8);
    LCD_Write_Data(y1 & 0xFF); 
    LCD_Write_Data(y2 >> 8);
    LCD_Write_Data(y2 & 0xFF);
}

void LCD_Color_Fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t *color_p)
{
    uint32_t total_pixels = (x2 - x1 + 1) * (y2 - y1 + 1);
    LCD_SetWindow(x1, y1, x2, y2);
    LCD_Write_Cmd(CMD_WRITE_MEMORY_START);
    for(uint32_t i = 0; i < total_pixels; i++)
    {
        LCD_Write_Data(color_p[i]);
    }
}

void LCD_Fill_Solid(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    uint32_t total_pixels = (x2 - x1 + 1) * (y2 - y1 + 1);
    LCD_SetWindow(x1, y1, x2, y2);
    LCD_Write_Cmd(CMD_WRITE_MEMORY_START);
    
    for(uint32_t i = 0; i < total_pixels; i++)
    {
        LCD_Write_Data(color);
    }
}

void LCD_Clear(uint16_t color)
{
    LCD_Fill_Solid(0, 0, 799, 479, color);
}
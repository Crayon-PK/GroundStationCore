#include "touch_gt911.h"
#include "timer_tick.h"
#include <string.h>

/* 直接操作BSRR/IDR寄存器实现引脚快捷电平控制 */
#define SCL_H()         GPIO_SetBits(CT_SCL_PORT, CT_SCL_PIN)
#define SCL_L()         GPIO_ResetBits(CT_SCL_PORT, CT_SCL_PIN)
#define SDA_H()         GPIO_SetBits(CT_SDA_PORT, CT_SDA_PIN)
#define SDA_L()         GPIO_ResetBits(CT_SDA_PORT, CT_SDA_PIN)
#define RST_H()         GPIO_SetBits(CT_RST_PORT, CT_RST_PIN)
#define RST_L()         GPIO_ResetBits(CT_RST_PORT, CT_RST_PIN)
#define INT_H()         GPIO_SetBits(CT_INT_PORT, CT_INT_PIN)
#define INT_L()         GPIO_ResetBits(CT_INT_PORT, CT_INT_PIN)
#define READ_SDA()      GPIO_ReadInputDataBit(CT_SDA_PORT, CT_SDA_PIN)

/**
 * @brief       产生IIC起始信号
 */
void GT911_I2C_Start(void)
{
    SDA_H();
    SCL_H();
    Delay_us(2);
    SDA_L();        // SCL高电平期间，SDA拉低，触发开始信号
    Delay_us(2);
    SCL_L();        // 钳住总线，准备发送或接收数据
}

/**
 * @brief       产生IIC停止信号
 */
void GT911_I2C_Stop(void)
{
    SDA_L();
    Delay_us(2);
    SCL_H();
    Delay_us(2);
    SDA_H();        // SCL高电平期间，SDA拉高，触发停止信号
}

/**
 * @brief       IIC发送一个字节
 */
void GT911_I2C_Send_Byte(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if ((byte & 0x80) >> 7)
        {
            SDA_H();
        }
        else
        {
            SDA_L();
        }
        
        byte <<= 1;
        Delay_us(2);
        SCL_H();        // 高电平期间数据有效，GT911读取SDA
        Delay_us(2);
        SCL_L();        // 低电平期间允许SDA改变状态
        Delay_us(2);
    }
    SDA_H();            // 发送完毕后释放SDA线
}

/**
 * @brief       IIC接收一个字节
 */
uint8_t GT911_I2C_Recv_Byte(uint8_t ack)
{
    uint8_t dat = 0;

    SDA_H();            // 主机释放SDA数据线
    Delay_us(2);

    for (uint8_t i = 0; i < 8; i++)
    {
        dat <<= 1;
        SCL_H();        // 拉高SCL，此时总线电平稳定，单片机开始读取SDA的数据
        Delay_us(2);

        if (READ_SDA())
        {
            dat++;
        }

        SCL_L();        // 拉低SCL，允许GT911改变SDA状态准备下一位
        Delay_us(2);
    }

    // 根据主机的要求回复应答信号
    if (ack == 0)
    {
        SDA_H();        // 回复 NACK (SDA维持高电平)
        Delay_us(2);
        SCL_H();
        Delay_us(2);
        SCL_L();
    }
    else
    {
        SDA_L();        // 回复 ACK (SDA拉低)
        Delay_us(2);
        SCL_H();
        Delay_us(2);
        SCL_L();
        SDA_H();        // 释放SDA
    }

    return dat;
}

/**
  * @brief       等待IIC应答信号
  * @retval      0: 收到ACK应答; 1: 超时未收到应答
  */
uint8_t GT911_I2C_WaitAck(void)
{
    uint8_t retry = 0;

    SDA_H();            // 释放SDA数据线
    Delay_us(2);
    SCL_H();            // 驱动第9个时钟脉冲
    Delay_us(2);
    
    while (READ_SDA())
    {
        retry++;
        if (retry > 250)
        {
            GT911_I2C_Stop();
            return 1;
        }
    }
    
    SCL_L();
    Delay_us(2);
    return 0;
}

/**
 * @brief       根据数据手册时序，配置设备I2C从机地址为0xBA
 */
void GT911_Reset(void)
{
    // 临时把 INT 引脚配置为推挽输出模式，以便在复位过程中控制它的电平状态
    GPIO_Init(CT_INT_PORT, &(GPIO_InitTypeDef){
        .GPIO_Pin   = CT_INT_PIN,
        .GPIO_Mode  = GPIO_Mode_OUT,
        .GPIO_OType = GPIO_OType_PP,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_PuPd  = GPIO_PuPd_NOPULL
    });

    // 开始复位时序 (按照0xBA地址选择时序)
    RST_L();
    INT_L();
    Delay_us(200);      // 保持拉低 >100us [cite: 214, 226]
    RST_H();
    Delay_ms(10);       // 拉高Reset后，INT必须继续维持至少5ms低电平 [cite: 219, 227]

    // 释放INT引脚，重新将其配置为输入模式（触摸中断信号输入）
    GPIO_Init(CT_INT_PORT, &(GPIO_InitTypeDef){
        .GPIO_Pin   = CT_INT_PIN,
        .GPIO_Mode  = GPIO_Mode_IN,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_PuPd  = GPIO_PuPd_NOPULL
    });

    Delay_ms(50);       // 等待GT911内部运行稳定 [cite: 383]
}

/**
  * @brief       向 GT911 寄存器连续写入数据
  * @param       reg: 要写入的寄存器地址
  * @param       buf: 要写入的数据缓冲区
  * @param       len: 要写入的数据长度
  * @retval      0: 写入成功; 1: 写入失败
  */
uint8_t GT911_Write_Reg(uint16_t reg, uint8_t *buf, uint8_t len)
{
    GT911_I2C_Start();
    
    // 发送从机写地址
    GT911_I2C_Send_Byte(GT911_CMD_WR);
    if (GT911_I2C_WaitAck()) return 1;
    
    // 发送16位寄存器高8位
    GT911_I2C_Send_Byte((uint8_t)(reg >> 8));
    if (GT911_I2C_WaitAck()) return 1;
    
    // 发送16位寄存器低8位
    GT911_I2C_Send_Byte((uint8_t)(reg & 0xFF));
    if (GT911_I2C_WaitAck()) return 1;
    
    // 连续写入数据
    for (uint8_t i = 0; i < len; i++)
    {
        GT911_I2C_Send_Byte(buf[i]);
        if (GT911_I2C_WaitAck()) return 1;
    }
    
    GT911_I2C_Stop();
    return 0;
}

/**
  * @brief       从 GT911 寄存器连续读取数据
  * @param       reg: 要读取的寄存器地址
  * @param       buf: 存储读取数据的缓冲区
  * @param       len: 要读取的数据长度
  */
void GT911_Read_Reg(uint16_t reg, uint8_t *buf, uint8_t len)
{
    GT911_I2C_Start();
    
    GT911_I2C_Send_Byte(GT911_CMD_WR);
    GT911_I2C_WaitAck();
    
    GT911_I2C_Send_Byte((uint8_t)(reg >> 8));
    GT911_I2C_WaitAck();
    
    GT911_I2C_Send_Byte((uint8_t)(reg & 0xFF));
    GT911_I2C_WaitAck();
    
    // 重启总线换方向
    GT911_I2C_Start(); 
    
    GT911_I2C_Send_Byte(GT911_CMD_RD);
    GT911_I2C_WaitAck();
    
    for (uint8_t i = 0; i < len; i++)
    {
        // 最后一个字节读完给 NACK(0)，其余给 ACK(1)
        buf[i] = GT911_I2C_Recv_Byte((i == (len - 1)) ? 0 : 1);
    }
    
    GT911_I2C_Stop();
}

/**
 * @brief       GT911 初始化与通信状态多级校验
 * @retval      0:成功; -2:I2C断线无响应; -3:未知的兼容触摸芯片
 */
int GT911_Init(void)
{
    char product_id[5];
    
    touch_GPIO_Init();
    GT911_Reset();
    
    // 读取 4 字节 Product ID（ASCII）
    GT911_Read_Reg(GT911_REG_PID, (uint8_t *)product_id, 4);
    product_id[4] = '\0';
    
    if (product_id[0] == 0x00 || (uint8_t)product_id[0] == 0xFF)
    {
        return -2;
    }
    
    if (strcmp(product_id, "911") == 0 || strcmp(product_id, "9147") == 0 || 
        strcmp(product_id, "9271") == 0 || strcmp(product_id, "1158") == 0)
    {
        return 0;
    }
    
    return -3;
}

/**
  * @brief       触摸扫描函数（建议主轮询间隔 > 10ms）
  * @param       points: 存储触摸点数据的数组
  * @param       max_points_to_read: 最多读取的触摸点数(最大5)
  * @retval      实际检测到的触摸点数
  */
uint8_t GT911_Scan(CT_Point_t *points, uint8_t max_points_to_read)
{
    uint8_t status;
    uint8_t touch_num = 0;
    uint8_t point_buf[6]; 
    
    if (max_points_to_read > 5) max_points_to_read = 5;

    // 读取状态寄存器
    GT911_Read_Reg(GT911_REG_TPINFO, &status, 1);
    
    // 判断最高位 buffer status 是否为 1
    if ((status & GT911_TPINFO_MASK_STA) != 0) 
    {
        // 提取当前的实际触摸点数
        touch_num = status & GT911_TPINFO_MASK_CNT; 
        
        if (touch_num > 0 && touch_num <= max_points_to_read)
        {
            // 循环读取每个有效触摸点的数据
            for (uint8_t i = 0; i < touch_num; i++)
            {
                // 计算当前点的数据寄存器地址（使用宏代替硬编码数字）
                uint16_t point_reg_addr = GT911_REG_TP1 + (i * 8); 
                
                // 读出 6 字节的核心坐标和大小数据
                GT911_Read_Reg(point_reg_addr, point_buf, 6);
                
                // 解析原始坐标值 (低字节在前)
                points[i].x = ((uint16_t)point_buf[1] << 8) | point_buf[0];
                points[i].y = ((uint16_t)point_buf[3] << 8) | point_buf[2];
                points[i].size = ((uint16_t)point_buf[5] << 8) | point_buf[4];
            }
        }
        
        // 必须清零状态寄存器，准备下一次接收
        status = 0;
        GT911_Write_Reg(GT911_REG_TPINFO, &status, 1);
    }
    
    return touch_num; 
}

/**
  * @brief       底层 GPIO 引脚时钟与基本工作模式配置
  * @details     初始化I2C、RST和INT引脚的工作模式
  */
void touch_GPIO_Init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOF, ENABLE);

    // SCL 配置
    GPIO_Init(CT_SCL_PORT, &(GPIO_InitTypeDef){
        .GPIO_Pin   = CT_SCL_PIN,
        .GPIO_Mode  = GPIO_Mode_OUT,
        .GPIO_OType = GPIO_OType_PP,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_PuPd  = GPIO_PuPd_NOPULL
    });

    // SDA 配置
    GPIO_Init(CT_SDA_PORT, &(GPIO_InitTypeDef){
        .GPIO_Pin   = CT_SDA_PIN,
        .GPIO_Mode  = GPIO_Mode_OUT,
        .GPIO_OType = GPIO_OType_OD,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_PuPd  = GPIO_PuPd_UP
    });

    // RST 配置
    GPIO_Init(CT_RST_PORT, &(GPIO_InitTypeDef){
        .GPIO_Pin   = CT_RST_PIN,
        .GPIO_Mode  = GPIO_Mode_OUT,
        .GPIO_OType = GPIO_OType_PP,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_PuPd  = GPIO_PuPd_UP
    });

    // INT 配置
    GPIO_Init(CT_INT_PORT, &(GPIO_InitTypeDef){
        .GPIO_Pin   = CT_INT_PIN,
        .GPIO_Mode  = GPIO_Mode_IN,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_PuPd  = GPIO_PuPd_NOPULL
    });
}
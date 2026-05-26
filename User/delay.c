#include "delay.h"

/**
  * @brief  微秒级延时
  * @param  xus 延时时长，范围：0~99864 (由于24位寄存器限制，F4下最大值变小)
  * @retval 无
  */
void delay_us(uint32_t xus)
{
    SysTick->LOAD = 168 * xus;              // F407主频168MHz，1us对应168个时钟周期
    SysTick->VAL = 0x00;                    // 清空当前计数值
    SysTick->CTRL = 0x00000005;             // 设置时钟源为HCLK，启动定时器
    while(!(SysTick->CTRL & 0x00010000));   // 等待计数到0
    SysTick->CTRL = 0x00000004;             // 关闭定时器
}

/**
  * @brief  毫秒级延时
  * @param  xms 延时时长，范围：0~4294967295
  * @retval 无
  */
void delay_ms(uint32_t xms)
{
    while(xms--)
    {
        delay_us(1000);                     // 每次循环延时1ms (1000us在99864范围内，安全)
    }
}
 
/**
  * @brief  秒级延时
  * @param  xs 延时时长，范围：0~4294967295
  * @retval 无
  */
void delay_s(uint32_t xs)
{
    while(xs--)
    {
        delay_ms(1000);
    }
}

_estack = ORIGIN(RAM) + LENGTH(RAM);
/* 内存区域定义 */
MEMORY
{
  CCMRAM (xrw)      : ORIGIN = 0x10000000, LENGTH = 64K
  RAM (xrw)         : ORIGIN = 0x20000000, LENGTH = 128K
  FLASH (rx)        : ORIGIN = 0x08000000, LENGTH = 1024K
}

/* 入口点定义 */
ENTRY(Reset_Handler)

SECTIONS
{
  /* 中断向量表和代码段放进 FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH

  .text :
  {
    . = ALIGN(4);
    *(.text)           
    *(.text*)          
    *(.rodata)         
    *(.rodata*)        
    . = ALIGN(4);
  } >FLASH

  /* 初始化数据段，运行内存是 RAM，加载内存是 FLASH */
  _sidata = LOADADDR(.data);
  .data : 
  {
    . = ALIGN(4);
    _sdata = .;        
    *(.data)           
    *(.data*)          
    . = ALIGN(4);
    _edata = .;        
  } >RAM AT > FLASH

  /* 未初始化数据段（BSS）放进 RAM */
  .bss :
  {
    _sbss = .;         
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;         
  } >RAM
}
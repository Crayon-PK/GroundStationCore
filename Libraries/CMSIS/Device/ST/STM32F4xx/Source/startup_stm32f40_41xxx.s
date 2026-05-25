/**
  ******************************************************************************
  * @file      startup_stm32f40_41xxx.s
  * @brief     STM32F407xx/STM32F417xx Devices vector table for GCC toolchain.
  ******************************************************************************
  */

  .syntax unified
  .cpu cortex-m4
  .fpu softvfp
  .thumb

.global g_pfnVectors
.global Default_Handler

/* start address for the initialization values of the .data section.
defined in linker script */
.word _sidata
/* start address for the .data section. defined in linker script */
.word _sdata
/* end address for the .data section. defined in linker script */
.word _edata
/* start address for the .bss section. defined in linker script */
.word _sbss
/* end address for the .bss section. defined in linker script */
.word _ebss

.section .text.Reset_Handler
  .weak Reset_Handler
  .type Reset_Handler, %function
Reset_Handler:
  ldr   sp, =_estack    /* Set stack pointer */

/* Copy the data segment initializers from flash to SRAM */
  ldr   r0, =_sdata
  ldr   r1, =_edata
  ldr   r2, =_sidata
  movs  r3, #0
  b     LoopCopyDataInit

CopyDataInit:
  ldr   r4, [r2, r3]
  str   r4, [r0, r3]
  adds  r3, r3, #4

LoopCopyDataInit:
  adds  r4, r0, r3
  cmp   r4, r1
  bcc   CopyDataInit

/* Fill the bss segment with 0x00 */
  ldr   r2, =_sbss
  ldr   r4, =_ebss
  movs  r3, #0
  b     LoopFillZerobss

FillZerobss:
  str   r3, [r2]
  adds  r2, r2, #4

LoopFillZerobss:
  cmp   r2, r4
  bcc   FillZerobss

/* Call the clock system initialization function.*/
  bl  SystemInit
/* Call static constructors */
  bl __libc_init_array
/* Call the application's entry point.*/
  bl  main

LoopForever:
    b LoopForever

.size Reset_Handler, .-Reset_Handler

/**
 * @brief  This is the code that gets called when the processor receives an
 * unexpected interrupt.  This simply enters an infinite loop, preserving
 * the system state for examination by a debugger.
 */
    .section .text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
    b Infinite_Loop
    .size Default_Handler, .-Default_Handler

/******************************************************************************
*
* The minimal vector table for a Cortex M4.  Note that the proper constructs
* must be placed on this to ensure that it ends up at physical address
* 0x0000.0000.
*
******************************************************************************/
   .section .isr_vector,"a",%progbits
  .type g_pfnVectors, %object
  .size g_pfnVectors, .-g_pfnVectors

g_pfnVectors:
  .word _estack
  .word Reset_Handler
  .word NMI_Handler
  .word HardFault_Handler
  .word MemManage_Handler
  .word BusFault_Handler
  .word UsageFault_Handler
  .word 0
  .word 0
  .word 0
  .word 0
  .word SVC_Handler
  .word DebugMon_Handler
  .word 0
  .word PendSV_Handler
  .word SysTick_Handler

  /* External Interrupts */
  .word WWDG_IRQHandler                   /* Window WatchDog              */
  .word PVD_IRQHandler                    /* PVD through EXTI Line detection */
  .word TAMP_STAMP_IRQHandler             /* Tamper and TimeStamps through the EXTI line */
  .word RTC_WKUP_IRQHandler               /* RTC Wakeup through the EXTI line */
  .word FLASH_IRQHandler                  /* FLASH                        */
  .word RCC_IRQHandler                    /* RCC                          */
  .word EXTI0_IRQHandler                  /* EXTI Line0                   */
  .word EXTI1_IRQHandler                  /* EXTI Line1                   */
  .word EXTI2_IRQHandler                  /* EXTI Line2                   */
  .word EXTI3_IRQHandler                  /* EXTI Line3                   */
  .word EXTI4_IRQHandler                  /* EXTI Line4                   */
  .word DMA1_Stream0_IRQHandler           /* DMA1 Stream 0                */
  .word DMA1_Stream1_IRQHandler           /* DMA1 Stream 1                */
  .word DMA1_Stream2_IRQHandler           /* DMA1 Stream 2                */
  .word DMA1_Stream3_IRQHandler           /* DMA1 Stream 3                */
  .word DMA1_Stream4_IRQHandler           /* DMA1 Stream 4                */
  .word DMA1_Stream5_IRQHandler           /* DMA1 Stream 5                */
  .word DMA1_Stream6_IRQHandler           /* DMA1 Stream 6                */
  .word ADC_IRQHandler                    /* ADC1, ADC2 and ADC3s         */
  .word CAN1_TX_IRQHandler                /* CAN1 TX                      */
  .word CAN1_RX0_IRQHandler               /* CAN1 RX0                     */
  .word CAN1_RX1_IRQHandler               /* CAN1 RX1                     */
  .word CAN1_SCE_IRQHandler               /* CAN1 SCE                     */
  .word EXTI9_5_IRQHandler                /* External Line[9:5]s          */
  .word TIM1_BRK_TIM9_IRQHandler          /* TIM1 Break and TIM9          */
  .word TIM1_UP_TIM10_IRQHandler          /* TIM1 Update and TIM10        */
  .word TIM1_TRG_COM_TIM11_IRQHandler     /* TIM1 Trigger and Commutation and TIM11 */
  .word TIM1_CC_IRQHandler                /* TIM1 Capture Compare         */
  .word TIM2_IRQHandler                   /* TIM2                         */
  .word TIM3_IRQHandler                   /* TIM3                         */
  .word TIM4_IRQHandler                   /* TIM4                         */
  .word I2C1_EV_IRQHandler                /* I2C1 Event                   */
  .word I2C1_ER_IRQHandler                /* I2C1 Error                   */
  .word I2C2_EV_IRQHandler                /* I2C2 Event                   */
  .word I2C2_ER_IRQHandler                /* I2C2 Error                   */
  .word SPI1_IRQHandler                   /* SPI1                         */
  .word SPI2_IRQHandler                   /* SPI2                         */
  .word USART1_IRQHandler                 /* USART1                       */
  .word USART2_IRQHandler                 /* USART2                       */
  .word USART3_IRQHandler                 /* USART3                       */
  .word EXTI15_10_IRQHandler              /* External Line[15:10]s        */
  .word RTC_Alarm_IRQHandler              /* RTC Alarm (A and B) through EXTI Line */
  .word OTG_FS_WKUP_IRQHandler            /* USB OTG FS Wakeup through EXTI line */
  .word TIM8_BRK_TIM12_IRQHandler         /* TIM8 Break and TIM12         */
  .word TIM8_UP_TIM13_IRQHandler          /* TIM8 Update and TIM13        */
  .word TIM8_TRG_COM_TIM14_IRQHandler     /* TIM8 Trigger and Commutation and TIM14 */
  .word TIM8_CC_IRQHandler                /* TIM8 Capture Compare         */
  .word DMA1_Stream7_IRQHandler           /* DMA1 Stream7                 */
  .word FSMC_IRQHandler                   /* FSMC                         */
  .word SDIO_IRQHandler                   /* SDIO                         */
  .word TIM5_IRQHandler                   /* TIM5                         */
  .word SPI3_IRQHandler                   /* SPI3                         */
  .word UART4_IRQHandler                  /* UART4                        */
  .word UART5_IRQHandler                  /* UART5                        */
  .word TIM6_DAC_IRQHandler               /* TIM6 and DAC1&2 underrun errors */
  .word TIM7_IRQHandler                   /* TIM7                         */
  .word DMA2_Stream0_IRQHandler           /* DMA2 Stream 0                */
  .word DMA2_Stream1_IRQHandler           /* DMA2 Stream 1                */
  .word DMA2_Stream2_IRQHandler           /* DMA2 Stream 2                */
  .word DMA2_Stream3_IRQHandler           /* DMA2 Stream 3                */
  .word DMA2_Stream4_IRQHandler           /* DMA2 Stream 4                */
  .word ETH_IRQHandler                    /* Ethernet                     */
  .word ETH_WKUP_IRQHandler               /* Ethernet Wakeup through EXTI line */
  .word CAN2_TX_IRQHandler                /* CAN2 TX                      */
  .word CAN2_RX0_IRQHandler               /* CAN2 RX0                     */
  .word CAN2_RX1_IRQHandler               /* CAN2 RX1                     */
  .word CAN2_SCE_IRQHandler               /* CAN2 SCE                     */
  .word OTG_FS_IRQHandler                 /* USB OTG FS                   */
  .word DMA2_Stream5_IRQHandler           /* DMA2 Stream 5                */
  .word DMA2_Stream6_IRQHandler           /* DMA2 Stream 6                */
  .word DMA2_Stream7_IRQHandler           /* DMA2 Stream 7                */
  .word USART6_IRQHandler                 /* USART6                       */
  .word I2C3_EV_IRQHandler                /* I2C3 event                   */
  .word I2C3_ER_IRQHandler                /* I2C3 error                   */
  .word OTG_HS_EP1_OUT_IRQHandler         /* USB OTG HS End Point 1 Out   */
  .word OTG_HS_EP1_IN_IRQHandler          /* USB OTG HS End Point 1 In    */
  .word OTG_HS_WKUP_IRQHandler            /* USB OTG HS Wakeup through EXTI */
  .word OTG_HS_IRQHandler                 /* USB OTG HS                   */
  .word DCMI_IRQHandler                   /* DCMI                         */
  .word 0                                 /* Reserved                     */
  .word HASH_RNG_IRQHandler               /* Hash and Rng                 */
  .word FPU_IRQHandler                    /* FPU                          */

/*******************************************************************************
*
* Provide weak aliases for each Exception handler to the Default_Handler.
* As they are weak aliases, any function with the same name will override
* this definition.
*
*******************************************************************************/
   .macro ProvidesWeakAlias name
   .thumb_set \name,Default_Handler
   .weak \name
   .endm

  ProvidesWeakAlias NMI_Handler
  ProvidesWeakAlias HardFault_Handler
  ProvidesWeakAlias MemManage_Handler
  ProvidesWeakAlias BusFault_Handler
  ProvidesWeakAlias UsageFault_Handler
  ProvidesWeakAlias SVC_Handler
  ProvidesWeakAlias DebugMon_Handler
  ProvidesWeakAlias PendSV_Handler
  ProvidesWeakAlias SysTick_Handler
  ProvidesWeakAlias WWDG_IRQHandler
  ProvidesWeakAlias PVD_IRQHandler
  ProvidesWeakAlias TAMP_STAMP_IRQHandler
  ProvidesWeakAlias RTC_WKUP_IRQHandler
  ProvidesWeakAlias FLASH_IRQHandler
  ProvidesWeakAlias RCC_IRQHandler
  ProvidesWeakAlias EXTI0_IRQHandler
  ProvidesWeakAlias EXTI1_IRQHandler
  ProvidesWeakAlias EXTI2_IRQHandler
  ProvidesWeakAlias EXTI3_IRQHandler
  ProvidesWeakAlias EXTI4_IRQHandler
  ProvidesWeakAlias DMA1_Stream0_IRQHandler
  ProvidesWeakAlias DMA1_Stream1_IRQHandler
  ProvidesWeakAlias DMA1_Stream2_IRQHandler
  ProvidesWeakAlias DMA1_Stream3_IRQHandler
  ProvidesWeakAlias DMA1_Stream4_IRQHandler
  ProvidesWeakAlias DMA1_Stream5_IRQHandler
  ProvidesWeakAlias DMA1_Stream6_IRQHandler
  ProvidesWeakAlias ADC_IRQHandler
  ProvidesWeakAlias CAN1_TX_IRQHandler
  ProvidesWeakAlias CAN1_RX0_IRQHandler
  ProvidesWeakAlias CAN1_RX1_IRQHandler
  ProvidesWeakAlias CAN1_SCE_IRQHandler
  ProvidesWeakAlias EXTI9_5_IRQHandler
  ProvidesWeakAlias TIM1_BRK_TIM9_IRQHandler
  ProvidesWeakAlias TIM1_UP_TIM10_IRQHandler
  ProvidesWeakAlias TIM1_TRG_COM_TIM11_IRQHandler
  ProvidesWeakAlias TIM1_CC_IRQHandler
  ProvidesWeakAlias TIM2_IRQHandler
  ProvidesWeakAlias TIM3_IRQHandler
  ProvidesWeakAlias TIM4_IRQHandler
  ProvidesWeakAlias I2C1_EV_IRQHandler
  ProvidesWeakAlias I2C1_ER_IRQHandler
  ProvidesWeakAlias I2C2_EV_IRQHandler
  ProvidesWeakAlias I2C2_ER_IRQHandler
  ProvidesWeakAlias SPI1_IRQHandler
  ProvidesWeakAlias SPI2_IRQHandler
  ProvidesWeakAlias USART1_IRQHandler
  ProvidesWeakAlias USART2_IRQHandler
  ProvidesWeakAlias USART3_IRQHandler
  ProvidesWeakAlias EXTI15_10_IRQHandler
  ProvidesWeakAlias RTC_Alarm_IRQHandler
  ProvidesWeakAlias OTG_FS_WKUP_IRQHandler
  ProvidesWeakAlias TIM8_BRK_TIM12_IRQHandler
  ProvidesWeakAlias TIM8_UP_TIM13_IRQHandler
  ProvidesWeakAlias TIM8_TRG_COM_TIM14_IRQHandler
  ProvidesWeakAlias TIM8_CC_IRQHandler
  ProvidesWeakAlias DMA1_Stream7_IRQHandler
  ProvidesWeakAlias FSMC_IRQHandler
  ProvidesWeakAlias SDIO_IRQHandler
  ProvidesWeakAlias TIM5_IRQHandler
  ProvidesWeakAlias SPI3_IRQHandler
  ProvidesWeakAlias UART4_IRQHandler
  ProvidesWeakAlias UART5_IRQHandler
  ProvidesWeakAlias TIM6_DAC_IRQHandler
  ProvidesWeakAlias TIM7_IRQHandler
  ProvidesWeakAlias DMA2_Stream0_IRQHandler
  ProvidesWeakAlias DMA2_Stream1_IRQHandler
  ProvidesWeakAlias DMA2_Stream2_IRQHandler
  ProvidesWeakAlias DMA2_Stream3_IRQHandler
  ProvidesWeakAlias DMA2_Stream4_IRQHandler
  ProvidesWeakAlias ETH_IRQHandler
  ProvidesWeakAlias ETH_WKUP_IRQHandler
  ProvidesWeakAlias CAN2_TX_IRQHandler
  ProvidesWeakAlias CAN2_RX0_IRQHandler
  ProvidesWeakAlias CAN2_RX1_IRQHandler
  ProvidesWeakAlias CAN2_SCE_IRQHandler
  ProvidesWeakAlias OTG_FS_IRQHandler
  ProvidesWeakAlias DMA2_Stream5_IRQHandler
  ProvidesWeakAlias DMA2_Stream6_IRQHandler
  ProvidesWeakAlias DMA2_Stream7_IRQHandler
  ProvidesWeakAlias USART6_IRQHandler
  ProvidesWeakAlias I2C3_EV_IRQHandler
  ProvidesWeakAlias I2C3_ER_IRQHandler
  ProvidesWeakAlias OTG_HS_EP1_OUT_IRQHandler
  ProvidesWeakAlias OTG_HS_EP1_IN_IRQHandler
  ProvidesWeakAlias OTG_HS_WKUP_IRQHandler
  ProvidesWeakAlias OTG_HS_IRQHandler
  ProvidesWeakAlias DCMI_IRQHandler
  ProvidesWeakAlias HASH_RNG_IRQHandler
  ProvidesWeakAlias FPU_IRQHandler
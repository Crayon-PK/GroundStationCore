PLATFORM_DIR := ./Platform/STM32F4

C_INCLUDES += \
    -I$(PLATFORM_DIR)/CMSIS/Include \
    -I$(PLATFORM_DIR)/CMSIS/Device/Include \
    -I$(PLATFORM_DIR)/StdPeriph/inc \
    -I$(PLATFORM_DIR)

C_SOURCES += \
    $(PLATFORM_DIR)/CMSIS/Device/Source/system_stm32f4xx.c \
    $(wildcard $(PLATFORM_DIR)/StdPeriph/src/*.c)

ASM_SOURCES += \
    $(PLATFORM_DIR)/startup_stm32f407xx.s
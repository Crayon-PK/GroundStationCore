# ==========================================
# 通用 STM32F4 GCC Makefile (标准外设库版)
# ==========================================

# 1. 工具链配置
CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

# 2. 目标与构建目录
TARGET    = build/firmware

# 3. 全局宏定义
DEFINES = \
-DUSE_STDPERIPH_DRIVER \
-DSTM32F40_41xxx \
-DHSE_VALUE=8000000

# 4. 头文件包含路径
INCLUDES = \
-I./App \
-I./App/Main \
-I./BSP \
-I./BSP/Delay \
-I./BSP/LCD \
-I./BSP/USART \
-I./Platform/STM32F4 \
-I./Platform/STM32F4/CMSIS/Include \
-I./Platform/STM32F4/CMSIS/Device/Include \
-I./Platform/STM32F4/StdPeriph/inc

# 5. 链接脚本路径
LDSCRIPT = ./Platform/STM32F4/stm32f407vg_flash.ld

# 6. 源文件自动查找 (不生成中间文件，直接编译源码)
SRCS  := $(shell find ./App -name "*.c")
SRCS  += $(shell find ./BSP -name "*.c")
SRCS  += $(shell find ./Platform -name "*.c")
SRCS  += $(shell find ./Platform -name "*.s")

# 7. 编译参数 (整合 -O2 优化、硬件浮点与垃圾回收)
CFLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -O2 $(DEFINES) $(INCLUDES) -g -gdwarf-2
CFLAGS += -Wall -ffunction-sections -fdata-sections -std=gnu99

# 8. 链接参数
LDFLAGS = -T $(LDSCRIPT) -Wl,--gc-sections -specs=nano.specs -lc -lm -lnosys -lgcc

# ==========================================
# 编译规则
# ==========================================
all: clean $(TARGET).hex

# 生成 HEX
$(TARGET).hex: $(TARGET).elf
	@$(OBJCOPY) -O ihex $< $@
	@$(SIZE) $<
	@echo "==============================> Build Success!"

# 直接通过源码编译并链接成 ELF (无中间 .o 文件)
$(TARGET).elf:
	@mkdir -p build
	$(CC) $(CFLAGS) $(SRCS) $(LDFLAGS) -o $@

# 清理
clean:
	@rm -rf build

# ==========================================
# 烧录规则 (OpenOCD + ST-Link)
# ==========================================
flash: all
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program $(TARGET).hex verify reset exit"

.PHONY: all clean flash
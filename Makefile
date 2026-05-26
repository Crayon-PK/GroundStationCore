# ==========================================
# 通用 STM32F4 GCC Makefile (标准外设库版)
# ==========================================

# 1. 工具链配置
CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

# 2. 目标与全局宏定义
TARGET  = build/firmware
DEFINES = -DSTM32F40_41xxx -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000

# 3. 头文件包含路径 (根据项目结构增删)
INCLUDES = -IUser \
           -IUser/bsp \
           -IUser/app \
           -ILibraries/CMSIS/Include \
           -ILibraries/CMSIS/Device/ST/STM32F4xx/Include \
           -ILibraries/STM32F4xx_StdPeriph_Driver/inc

# 4. 源文件与核心底层文件
STARTUP = Libraries/CMSIS/Device/ST/STM32F4xx/Source/startup_stm32f407xx.s
LDSCRIPT = Libraries/CMSIS/Device/ST/STM32F4xx/Source/stm32f407vg_flash.ld

SRCS = $(wildcard User/*.c) \
       $(wildcard User/bsp/*.c) \
       $(wildcard User/app/*.c) \
       Libraries/CMSIS/Device/ST/STM32F4xx/Source/system_stm32f4xx.c \
       $(wildcard Libraries/STM32F4xx_StdPeriph_Driver/src/*.c)

# 5. 编译参数 (开启 -O2 优化, 开启 FPU 硬件浮点, 开启无用函数回收)
CFLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
         -O2 $(DEFINES) $(INCLUDES) -g -gdwarf-2 \
         -ffunction-sections -fdata-sections -std=gnu99

# 6. 链接参数 (启用垃圾回收机制)
LDFLAGS = -T $(LDSCRIPT) -Wl,--gc-sections -lc -lm -lgcc

# ==========================================
# 编译规则
# ==========================================
all: clean $(TARGET).hex

$(TARGET).hex: $(TARGET).elf
	@$(OBJCOPY) -O ihex $< $@
	@$(SIZE) $<
	@echo "==============================> Build Success!"

$(TARGET).elf:
	@mkdir -p build
	@$(CC) $(CFLAGS) $(SRCS) $(STARTUP) $(LDFLAGS) -o $@

clean:
	@rm -rf build

# ==========================================
# 烧录规则 (OpenOCD + ST-Link)
# ==========================================
flash: all
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program $(TARGET).hex verify reset exit"
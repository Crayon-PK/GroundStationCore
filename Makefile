CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

TARGET = build/firmware

DEFINES = -DSTM32F40_41xxx -DUSE_STDPERIPH_DRIVER

# 精准适配你当前的深层嵌套头文件路径
INCLUDES = -IUser \
           -IUser/bsp \
           -IUser/app \
           -ILibraries/CMSIS/Include \
           -ILibraries/CMSIS/Device/ST/STM32F4xx/Include \
           -ILibraries/STM32F4xx_StdPeriph_Driver/inc

STARTUP = Libraries/CMSIS/Device/ST/STM32F4xx/Source/startup_stm32f40_41xxx.s

# 全自动横扫你当前层级下的所有 .c 源码
SRCS = $(wildcard User/*.c) \
       $(wildcard User/bsp/*.c) \
       $(wildcard User/app/*.c) \
	   Libraries/CMSIS/Device/ST/STM32F4xx/Source/system_stm32f4xx.c \
       $(wildcard Libraries/CMSIS/Device/ST/STM32F4xx/Source/Templates/*.c) \
       $(wildcard Libraries/STM32F4xx_StdPeriph_Driver/src/*.c)

CFLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O2 $(DEFINES) $(INCLUDES) -g -gdwarf-2
# 修正后：
LDFLAGS = -T Libraries/CMSIS/Device/ST/STM32F4xx/Source/stm32f407vg_flash.ld -Wl,--gc-sections

all: clean $(TARGET).hex

$(TARGET).hex: $(TARGET).elf
	@$(OBJCOPY) -O ihex $< $@
	@$(SIZE) $<
	@echo "==============================> Build Success!

$(TARGET).elf:
	@mkdir -p build
	@$(CC) $(CFLAGS) $(SRCS) $(STARTUP) $(LDFLAGS) -o $@

clean:
	@rm -rf build

# 一键烧录规则：直接调用 OpenOCD 驱动 ST-Link 写入 hex
flash: all
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program build/firmware.hex verify reset exit"
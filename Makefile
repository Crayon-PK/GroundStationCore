# ====================================================================
# 通用 STM32F4 GCC Makefile (多级影子目录绝对闭环版 - 彻底根治VPATH冒号冲突)
# ====================================================================

CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

TARGET    = build/firmware
OBJ_DIR   = build/obj

DEFINES = \
-DUSE_STDPERIPH_DRIVER \
-DSTM32F40_41xxx \
-DHSE_VALUE=8000000 \
-DLV_CONF_INCLUDE_SIMPLE

# 头文件路径死绑定
ST_INCLUDES := -I./Platform/STM32F4 \
               -I./Platform/STM32F4/CMSIS/Include \
               -I./Platform/STM32F4/CMSIS/Device/Include \
               -I./Platform/STM32F4/StdPeriph/inc

RTOS_INCLUDES := -I./Middlewares/FreeRTOS/include \
                 -I./Middlewares/FreeRTOS/port

LVGL_DIR_NAME ?= Middlewares/LVGL

LVGL_INCLUDES := -I./$(LVGL_DIR_NAME) \
                 -I./$(LVGL_DIR_NAME)/src \
                 -I./$(LVGL_DIR_NAME)/porting \
                 -I./$(LVGL_DIR_NAME)/src/core \
                 -I./$(LVGL_DIR_NAME)/src/draw \
                 -I./$(LVGL_DIR_NAME)/src/draw/sw \
                 -I./$(LVGL_DIR_NAME)/src/draw/stm32_dma2d \
                 -I./$(LVGL_DIR_NAME)/src/extra \
                 -I./$(LVGL_DIR_NAME)/src/font \
                 -I./$(LVGL_DIR_NAME)/src/hal \
                 -I./$(LVGL_DIR_NAME)/src/misc \
                 -I./$(LVGL_DIR_NAME)/src/widgets

USER_INC_DIRS := ./App ./App/* ./App/*/* ./BSP ./BSP/* ./BSP/*/*
RAW_DIRS      := $(wildcard $(USER_INC_DIRS))
CLEAN_DIRS    := $(sort $(foreach dir, $(RAW_DIRS), $(if $(wildcard $(dir)/.), $(dir))))
USER_INCLUDES := $(addprefix -I, $(CLEAN_DIRS))

INCLUDES := $(ST_INCLUDES) $(USER_INCLUDES) $(RTOS_INCLUDES) $(LVGL_INCLUDES)

LDSCRIPT = ./Platform/STM32F4/stm32f407vg_flash.ld

# 1. 检索标准库、RTOS、用户源码
ST_SRCS   := ./Platform/STM32F4/CMSIS/Device/Source/system_stm32f4xx.c \
             $(wildcard ./Platform/STM32F4/StdPeriph/src/*.c)
ST_ASMS   := ./Platform/STM32F4/startup_stm32f407xx.s
RTOS_SRCS := $(wildcard ./Middlewares/FreeRTOS/src/*.c) \
             $(wildcard ./Middlewares/FreeRTOS/port/*.c)
USER_SRCS := $(wildcard ./App/*.c) $(wildcard ./App/*/*.c) $(wildcard ./App/*/*/*.c) \
             $(wildcard ./BSP/*.c) $(wildcard ./BSP/*/*.c) $(wildcard ./BSP/*/*/*.c)

# 2. 【核心大招】：不依赖官方脚本，由主Makefile物理全量扫描LVGL带相对路径的源码
# 这样能精准抓取到所有组件，同时完美避开了nxp、sdl、renesas等无用硬件加速文件
LVGL_SRCS := $(wildcard $(LVGL_DIR_NAME)/src/core/*.c) \
             $(wildcard $(LVGL_DIR_NAME)/src/draw/*.c) \
             $(wildcard $(LVGL_DIR_NAME)/src/draw/sw/*.c) \
             $(wildcard $(LVGL_DIR_NAME)/src/draw/stm32_dma2d/*.c) \
             $(wildcard $(LVGL_DIR_NAME)/src/extra/*.c) \
             $(wildcard $(LVGL_DIR_NAME)/src/extra/layouts/*/*.c) \
             $(wildcard $(LVGL_DIR_NAME)/src/extra/libs/*/*.c) \
             $(wildcard $(LVGL_DIR_NAME)/src/extra/others/*/*.c) \
             $(wildcard $(LVGL_DIR_NAME)/src/extra/themes/*/*.c) \
             $(wildcard $(LVGL_DIR_NAME)/src/extra/widgets/*/*.c) \
             $(wildcard $(LVGL_DIR_NAME)/src/font/*.c) \
             $(wildcard $(LVGL_DIR_NAME)/src/hal/*.c) \
             $(wildcard $(LVGL_DIR_NAME)/src/misc/*.c) \
             $(wildcard $(LVGL_DIR_NAME)/src/widgets/*.c) \
             $(wildcard $(LVGL_DIR_NAME)/porting/*.c)

# 汇总所有纯相对路径源码
ALL_C_SRCS := $(ST_SRCS) $(RTOS_SRCS) $(USER_SRCS) $(LVGL_SRCS)

CFLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -O2 $(DEFINES) $(INCLUDES) -g -gdwarf-2
CFLAGS += -Wall -ffunction-sections -fdata-sections -std=gnu99

LDFLAGS = -T $(LDSCRIPT) -Wl,--gc-sections -specs=nano.specs -lc -lm -lnosys -lgcc

# ====================================================================
# 【影子目录映射逻辑】：100% 相对路径严格对齐，彻底废除 VPATH 机制
# ====================================================================
CLEAN_SRCS := $(patsubst ./%, %, $(ALL_C_SRCS))
CLEAN_ASMS := $(patsubst ./%, %, $(ST_ASMS))

OBJECTS := $(patsubst %.c, $(OBJ_DIR)/%.o, $(CLEAN_SRCS))
OBJECTS += $(patsubst %.s, $(OBJ_DIR)/%.o, $(CLEAN_ASMS))

# ==========================================
# 编译链接规则
# ==========================================
all: $(TARGET).hex

$(TARGET).hex: $(TARGET).elf
	@$(OBJCOPY) -O ihex $< $@
	@$(SIZE) $<
	@echo "==============================> [SUCCESS] Build Complete!"

$(TARGET).elf: $(OBJECTS)
	@echo "Linking object files into $@..."
	@$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

# 像素级严格对齐编译：源码在什么子目录下，.o 就在 build/obj/ 下自动创建什么子目录
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	@echo "Assembling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -rf build

flash: all
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program $(TARGET).hex verify reset exit"

.PHONY: all clean flash
CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

TARGET    = build/firmware
OBJ_DIR   = build/obj

C_INCLUDES =
C_SOURCES =
ASM_SOURCES =

include ./Platform/STM32F4/stm32f407.mk
include ./App/app.mk
include ./BSP/bsp.mk
include ./Middlewares/FreeRTOS/freertos.mk
include ./Middlewares/LVGL/lvgl_port.mk
include ./Middlewares/MAVLink/mavlink.mk

DEFINES = \
-DUSE_STDPERIPH_DRIVER \
-DSTM32F40_41xxx \
-DHSE_VALUE=8000000 \
-DLV_CONF_INCLUDE_SIMPLE

CFLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -O2 $(DEFINES) $(C_INCLUDES) -g -gdwarf-2
CFLAGS += -Wall -ffunction-sections -fdata-sections -std=gnu99

LDSCRIPT = ./Platform/STM32F4/stm32f407vg_flash.ld
LDFLAGS = -T $(LDSCRIPT) -Wl,--gc-sections -specs=nano.specs --specs=nosys.specs -u _printf_float -lc -lm -lnosys -lgcc

CLEAN_SRCS := $(patsubst ./%, %, $(C_SOURCES))
CLEAN_ASMS := $(patsubst ./%, %, $(ASM_SOURCES))

OBJECTS := $(patsubst %.c, $(OBJ_DIR)/%.o, $(CLEAN_SRCS))
OBJECTS += $(patsubst %.s, $(OBJ_DIR)/%.o, $(CLEAN_ASMS))

# 编译链接规则
all: $(TARGET).hex

$(TARGET).hex: $(TARGET).elf
	@$(OBJCOPY) -O ihex $< $@
	@$(SIZE) $<
	@echo "==============================> [SUCCESS] Build Complete!"

$(TARGET).elf: $(OBJECTS)
	@echo "Linking object files into $@..."
	@$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

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
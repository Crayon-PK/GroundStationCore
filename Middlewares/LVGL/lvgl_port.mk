LVGL_DIR_NAME := Middlewares/LVGL

C_INCLUDES += \
    -I./$(LVGL_DIR_NAME) \
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

C_SOURCES += \
    $(wildcard ./$(LVGL_DIR_NAME)/src/core/*.c) \
    $(wildcard ./$(LVGL_DIR_NAME)/src/draw/*.c) \
    $(wildcard ./$(LVGL_DIR_NAME)/src/draw/sw/*.c) \
    $(wildcard ./$(LVGL_DIR_NAME)/src/draw/stm32_dma2d/*.c) \
    $(wildcard ./$(LVGL_DIR_NAME)/src/extra/*.c) \
    $(wildcard ./$(LVGL_DIR_NAME)/src/extra/layouts/*/*.c) \
    $(wildcard ./$(LVGL_DIR_NAME)/src/extra/libs/*/*.c) \
    $(wildcard ./$(LVGL_DIR_NAME)/src/extra/others/*/*.c) \
    $(wildcard ./$(LVGL_DIR_NAME)/src/extra/themes/*/*.c) \
    $(wildcard ./$(LVGL_DIR_NAME)/src/extra/widgets/*/*.c) \
    $(wildcard ./$(LVGL_DIR_NAME)/src/font/*.c) \
    $(wildcard ./$(LVGL_DIR_NAME)/src/hal/*.c) \
    $(wildcard ./$(LVGL_DIR_NAME)/src/misc/*.c) \
    $(wildcard ./$(LVGL_DIR_NAME)/src/widgets/*.c) \
    $(wildcard ./$(LVGL_DIR_NAME)/porting/*.c)
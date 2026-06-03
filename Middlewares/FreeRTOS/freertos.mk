FREERTOS_DIR := ./Middlewares/FreeRTOS

C_INCLUDES += \
    -I$(FREERTOS_DIR)/include \
    -I$(FREERTOS_DIR)/port

C_SOURCES += \
    $(wildcard $(FREERTOS_DIR)/src/*.c) \
    $(wildcard $(FREERTOS_DIR)/port/*.c)
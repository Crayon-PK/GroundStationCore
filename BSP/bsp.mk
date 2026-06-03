BSP_DIR := ./BSP

C_INCLUDES += \
    -I$(BSP_DIR)/COMMUNICATION \
    -I$(BSP_DIR)/DISPLAY \
    -I$(BSP_DIR)/INFRASTRUCTURE

C_SOURCES += \
    $(wildcard $(BSP_DIR)/COMMUNICATION/*.c) \
    $(wildcard $(BSP_DIR)/DISPLAY/*.c) \
    $(wildcard $(BSP_DIR)/INFRASTRUCTURE/*.c)

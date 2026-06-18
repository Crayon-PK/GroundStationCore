APP_DIR := ./App

C_INCLUDES += \
    -I$(APP_DIR) \
    -I$(APP_DIR)/widgets \

C_SOURCES += \
    $(wildcard $(APP_DIR)/*.c) \
    $(wildcard $(APP_DIR)/widgets/*.c)

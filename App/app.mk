APP_DIR := ./App

C_INCLUDES += \
    -I$(APP_DIR) \
    -I$(APP_DIR)/ui \
    -I$(APP_DIR)/ui/widgets \

C_SOURCES += \
    $(wildcard $(APP_DIR)/*.c) \
    $(wildcard $(APP_DIR)/ui/*.c) \
    $(wildcard $(APP_DIR)/ui/widgets/*.c)

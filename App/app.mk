APP_DIR := ./App

C_INCLUDES += \
    -I$(APP_DIR)/Main\
    -I$(APP_DIR)

C_SOURCES += \
    $(wildcard $(APP_DIR)/Main/*.c) \
    $(wildcard $(APP_DIR)/*.c) \

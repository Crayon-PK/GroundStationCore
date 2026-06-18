TASKS_DIR := ./Tasks

C_INCLUDES += \
    -I$(TASKS_DIR)

C_SOURCES += \
    $(wildcard $(TASKS_DIR)/*.c)
EXTRA_FILES := $(wildcard $(LVGL_DIR)/$(LVGL_DIR_NAME)/src/extra/*.c) \
               $(wildcard $(LVGL_DIR)/$(LVGL_DIR_NAME)/src/extra/*/*.c) \
               $(wildcard $(LVGL_DIR)/$(LVGL_DIR_NAME)/src/extra/*/*/*.c) \
               $(wildcard $(LVGL_DIR)/$(LVGL_DIR_NAME)/src/extra/*/*/*/*.c)

CSRCS += $(EXTRA_FILES)
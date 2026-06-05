MAVLINK_DIR := ./Middlewares/MAVLink

C_INCLUDES += \
    -I$(MAVLINK_DIR)/common \
	-I$(MAVLINK_DIR)/standard \
	-I$(MAVLINK_DIR)/minimal \
    -I$(MAVLINK_DIR)/
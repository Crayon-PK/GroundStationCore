#ifndef __WIDGET_POSE_H
#define __WIDGET_POSE_H

#include "lvgl.h"

lv_obj_t* Widget_Pose_Init(lv_obj_t* parent);
void Widget_Pose_Update(float roll, float pitch, float yaw);

#endif

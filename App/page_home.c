#include "page_home.h"
#include "widget_pose.h"
#include "data_pool.h"

extern FlightData_t g_flight_data;

static lv_obj_t * s_page_home = NULL;
static lv_obj_t * s_pose_ball = NULL;
static lv_obj_t * s_label_telemetry = NULL;

void Page_Home_Create(void)
{
    if (s_page_home != NULL) return; 

    s_page_home = lv_obj_create(NULL); 
    lv_obj_set_style_bg_color(s_page_home, lv_color_white(), 0); 
    
    s_pose_ball = Widget_Pose_Init(s_page_home);
    lv_obj_set_pos(s_pose_ball, 500, 200); 
    
    lv_obj_update_layout(s_pose_ball); 

    s_label_telemetry = lv_label_create(s_page_home);
    lv_obj_align(s_label_telemetry, LV_ALIGN_TOP_LEFT, 20, 20);
    lv_obj_set_style_text_color(s_label_telemetry, lv_color_make(0xAA, 0x55, 0x00), 0);
    lv_label_set_text(s_label_telemetry, "MAVLink: Connecting...");

    lv_scr_load(s_page_home);
}

// 主界面数据刷新
void Page_Home_Update(void)
{
    Widget_Pose_Update(g_flight_data.attitude.roll,
                       g_flight_data.attitude.pitch,
                       g_flight_data.attitude.yaw);

    lv_label_set_text_fmt(s_label_telemetry, "Packets: %lu\nHeartbeats: %lu",
                          g_flight_data.stats.total_packets,
                          g_flight_data.stats.heartbeat_count);
}
#include "widget_pose.h"
#include <stdlib.h>
#include <stdio.h>

// =============================================================
// 核心配置与比例宏定义
#define WIDGET_POSE_SIZE            200                                // 唯一全局控制：姿态球的外环直径

#define INNER_BALL_SIZE             ((int)(WIDGET_POSE_SIZE * 0.708f))  // 内环姿态球直径
#define PX_PER_DEG                  ((int)(WIDGET_POSE_SIZE * 0.025f))  // 每度代表的像素数
#define LEN_1DEG                    ((int)(WIDGET_POSE_SIZE * 0.066f))  // 1度刻度线长
#define LEN_5DEG                    ((int)(WIDGET_POSE_SIZE * 0.200f))  // 5度刻度线长

#define R_TICK_INNER                ((int)(WIDGET_POSE_SIZE * 0.354f))  // 罗盘刻度内径
#define R_TICK_OUTER                ((int)(WIDGET_POSE_SIZE * 0.395f))  // 罗盘刻度外径
#define R_TEXT                      ((int)(WIDGET_POSE_SIZE * 0.437f))  // 罗盘文字半径

// 颜色配置宏
#define COLOR_OUTER_BG              lv_palette_darken(LV_PALETTE_GREY, 3) // 外层大圆盘的深灰背景色
#define COLOR_OUTER_BORDER          lv_color_white()                            // 内外圈交界处的圆环边框颜色
#define COLOR_TICK_WHITE            lv_color_white()                            // 外层罗盘刻度线颜色
#define COLOR_TEXT_WHITE            lv_color_white()                            // 外层罗盘方位文字与度数颜色

#define COLOR_SKY                   lv_palette_main(LV_PALETTE_BLUE)    // 天空颜色
#define COLOR_GROUND                lv_palette_main(LV_PALETTE_BROWN)   // 地面颜色
#define COLOR_T_POINTER             lv_palette_main(LV_PALETTE_ORANGE)  // T型指针颜色
#define COLOR_YAW_ARROW             lv_palette_main(LV_PALETTE_RED)     // 偏航箭头颜色
#define COLOR_REF_LINE              lv_palette_main(LV_PALETTE_RED)     // 基准参考线颜色

// 线宽配置宏
#define WIDTH_PITCH_5DEG            2                                   // 5度俯仰线宽
#define WIDTH_PITCH_1DEG            1                                   // 1度俯仰线宽
#define WIDTH_T_POINTER             2                                   // T型指针线宽
#define WIDTH_REF_LINE              2                                   // 静态参考线线宽
// =============================================================

static float s_roll = 0.0f;
static float s_pitch = 0.0f;
static float s_yaw = 0.0f;

static lv_obj_t* s_roll_container = NULL;
static lv_obj_t* s_attitude_container = NULL;
static lv_obj_t* s_attitude_bg = NULL;

// 最内层：绘制蓝褐背景与俯仰刻度线 (Pitch)
static void draw_horizon_line_cb(lv_event_t* e){
    lv_obj_t* obj = lv_event_get_target(e);
    lv_draw_ctx_t* draw_ctx = lv_event_get_draw_ctx(e);

    lv_area_t obj_area;
    lv_obj_get_coords(obj, &obj_area);

    lv_coord_t center_x = obj_area.x1 + lv_area_get_width(&obj_area) / 2;
    lv_coord_t center_y = obj_area.y1 + lv_area_get_height(&obj_area) / 2;

    lv_coord_t pitch_offset_y = s_pitch * PX_PER_DEG;
    lv_coord_t horizon_y = center_y + pitch_offset_y;

    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.border_width = 0;

    rect_dsc.bg_color = COLOR_SKY;
    lv_area_t sky_area = {obj_area.x1, obj_area.y1, obj_area.x2, horizon_y};
    lv_draw_rect(draw_ctx, &rect_dsc, &sky_area);

    rect_dsc.bg_color = COLOR_GROUND;
    lv_area_t ground_area = {obj_area.x1, horizon_y, obj_area.x2, obj_area.y2};
    lv_draw_rect(draw_ctx, &rect_dsc, &ground_area);

    lv_draw_line_dsc_t line_1deg;
    lv_draw_line_dsc_init(&line_1deg);
    line_1deg.color = lv_palette_main(LV_PALETTE_GREY);
    line_1deg.width = WIDTH_PITCH_1DEG;

    lv_draw_line_dsc_t line_5deg;
    lv_draw_line_dsc_init(&line_5deg);
    line_5deg.color = lv_color_white();
    line_5deg.width = WIDTH_PITCH_5DEG;

    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    label_dsc.color = lv_color_white();

    for(int angle = -90; angle <= 90; angle++){
        lv_coord_t y_pos = center_y - (angle * PX_PER_DEG) + pitch_offset_y;

        if(y_pos < obj_area.y1 || y_pos > obj_area.y2) continue;

        lv_point_t p1 = { .y = y_pos };
        lv_point_t p2 = { .y = y_pos };

        if(angle % 5 == 0){
            p1.x = center_x - LEN_5DEG / 2;
            p2.x = center_x + LEN_5DEG / 2;
            lv_draw_line(draw_ctx, &line_5deg, &p1, &p2);

            char angle_text[16];
            lv_snprintf(angle_text, sizeof(angle_text), "%d", angle);

            // 字号偏移同样自适应缩放
            lv_area_t label_area = {p2.x + 5, y_pos - (LEN_1DEG/2), p2.x + 40, y_pos + LEN_1DEG};
            lv_draw_label(draw_ctx, &label_dsc, &label_area, angle_text, NULL);
        } else {
             p1.x = center_x - LEN_1DEG / 2;
             p2.x = center_x + LEN_1DEG / 2;
             lv_draw_line(draw_ctx, &line_1deg, &p1, &p2);
        }
    }
}

// 内环的静止俯仰基准
static void draw_static_pitch_ref_cb(lv_event_t* e){
    lv_obj_t* obj = lv_event_get_target(e);
    lv_draw_ctx_t* draw_ctx = lv_event_get_draw_ctx(e);
    lv_area_t obj_area;
    lv_obj_get_coords(obj, &obj_area);
    lv_coord_t cy = obj_area.y1 + lv_area_get_height(&obj_area) / 2;

    lv_draw_line_dsc_t line_red;
    lv_draw_line_dsc_init(&line_red);
    line_red.color = COLOR_REF_LINE;
    line_red.width = WIDTH_REF_LINE;

    // 基准参考线长自适应
    lv_point_t p_left[2] = {{obj_area.x1, cy}, {obj_area.x1 + LEN_1DEG, cy}};
    lv_draw_line(draw_ctx, &line_red, &p_left[0], &p_left[1]);

    lv_point_t p_right[2] = {{obj_area.x2, cy}, {obj_area.x2 - LEN_1DEG, cy}};
    lv_draw_line(draw_ctx, &line_red, &p_right[0], &p_right[1]);
}

// Roll 刻度、断层 T 型指针、方位字符与 Yaw 旋转箭头
static void draw_roll_yaw_overlay_cb(lv_event_t* e){
    lv_obj_t* obj = lv_event_get_target(e);
    lv_draw_ctx_t* draw_ctx = lv_event_get_draw_ctx(e);
    lv_area_t obj_area;
    lv_obj_get_coords(obj, &obj_area);

    lv_coord_t cx = obj_area.x1 + lv_area_get_width(&obj_area) / 2;
    lv_coord_t cy = obj_area.y1 + lv_area_get_height(&obj_area) / 2;
    lv_point_t pivot = {cx, cy};

    lv_draw_line_dsc_t line_tick;
    lv_draw_line_dsc_init(&line_tick);
    line_tick.color = COLOR_TICK_WHITE;

    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    label_dsc.color = COLOR_TEXT_WHITE;
    label_dsc.align = LV_TEXT_ALIGN_CENTER;

    for(int angle = -180; angle < 180; angle += 30) {
        lv_point_t p_in  = {cx, cy - R_TICK_INNER};
        lv_point_t p_out = {cx, cy - R_TICK_OUTER};
        lv_point_t p_txt = {cx, cy - R_TEXT};

        lv_point_transform(&p_in,  angle * 10, 256, &pivot);
        lv_point_transform(&p_out, angle * 10, 256, &pivot);
        lv_point_transform(&p_txt, angle * 10, 256, &pivot);

        line_tick.width = (angle % 90 == 0) ? 4 : 2;
        lv_draw_line(draw_ctx, &line_tick, &p_in, &p_out);

        char text_buf[8];
        if (angle == 0)        lv_snprintf(text_buf, sizeof(text_buf), "N");
        else if (angle == 90)  lv_snprintf(text_buf, sizeof(text_buf), "E");
        else if (angle == -180) lv_snprintf(text_buf, sizeof(text_buf), "S");
        else if (angle == -90) lv_snprintf(text_buf, sizeof(text_buf), "W");
        else lv_snprintf(text_buf, sizeof(text_buf), "%d", angle);

        lv_area_t txt_area = {p_txt.x - 20, p_txt.y - 8, p_txt.x + 20, p_txt.y + 8};
        lv_draw_label(draw_ctx, &label_dsc, &txt_area, text_buf, NULL);
    }

    lv_draw_line_dsc_t line_orange;
    lv_draw_line_dsc_init(&line_orange);
    line_orange.color = COLOR_T_POINTER;
    line_orange.width = WIDTH_T_POINTER;

    // T型指针关键点位自适应缩放（控制垂线长度，防内侵过于靠近中心）
    lv_point_t t_left    = {cx - (int)(WIDGET_POSE_SIZE * 0.125f), cy};
    lv_point_t t_right   = {cx + (int)(WIDGET_POSE_SIZE * 0.125f), cy};
    // 让垂线起点（内端）咬住外圈半径再往下减去总直径的 5%，控制垂直短线长短
    lv_point_t t_ptr_in  = {cx, cy - R_TICK_INNER + (int)(WIDGET_POSE_SIZE * 0.050f)};
    lv_point_t t_ptr_out = {cx, cy - R_TICK_INNER};

    int32_t roll_angle = s_roll * 10;
    lv_point_transform(&t_left,  roll_angle, 256, &pivot);
    lv_point_transform(&t_right, roll_angle, 256, &pivot);
    lv_point_transform(&t_ptr_in,  roll_angle, 256, &pivot);
    lv_point_transform(&t_ptr_out, roll_angle, 256, &pivot);

    lv_draw_line(draw_ctx, &line_orange, &t_left, &pivot);
    lv_draw_line(draw_ctx, &line_orange, &pivot, &t_right);
    lv_draw_line(draw_ctx, &line_orange, &t_ptr_in, &t_ptr_out);

    lv_draw_line_dsc_t line_red;
    lv_draw_line_dsc_init(&line_red);
    line_red.color = COLOR_YAW_ARROW;
    line_red.width = 3;

    // 航向红色三角形指示头等比例自适应
    int y_tip = (int)(WIDGET_POSE_SIZE * 0.466f);
    int y_base = (int)(WIDGET_POSE_SIZE * 0.500f);
    lv_point_t y_ptr[3] = {
        {cx, cy - y_tip},
        {cx - 6, cy - y_base},
        {cx + 6, cy - y_base}
    };

    int32_t yaw_angle = s_yaw * 10;
    lv_point_transform(&y_ptr[0], yaw_angle, 256, &pivot);
    lv_point_transform(&y_ptr[1], yaw_angle, 256, &pivot);
    lv_point_transform(&y_ptr[2], yaw_angle, 256, &pivot);

    lv_draw_line(draw_ctx, &line_red, &y_ptr[0], &y_ptr[1]);
    lv_draw_line(draw_ctx, &line_red, &y_ptr[1], &y_ptr[2]);
    lv_draw_line(draw_ctx, &line_red, &y_ptr[2], &y_ptr[0]);
}

// 初始化接口
lv_obj_t* Widget_Pose_Init(lv_obj_t* parent){
    // 创建外环
    s_roll_container = lv_obj_create(parent);
    lv_obj_set_size(s_roll_container, WIDGET_POSE_SIZE, WIDGET_POSE_SIZE);
    lv_obj_set_style_radius(s_roll_container, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(s_roll_container, COLOR_OUTER_BG, 0);
    lv_obj_set_style_border_width(s_roll_container, 0, 0);
    lv_obj_set_style_pad_all(s_roll_container, 0, 0);
    // 明确清除外盘的滚动条，防止影响子组件布局
    lv_obj_set_scrollbar_mode(s_roll_container, LV_SCROLLBAR_MODE_OFF); 
    lv_obj_add_event_cb(s_roll_container, draw_roll_yaw_overlay_cb, LV_EVENT_DRAW_POST, NULL);

    // 创建内环
    s_attitude_container = lv_obj_create(s_roll_container);
    lv_obj_set_size(s_attitude_container, INNER_BALL_SIZE, INNER_BALL_SIZE);
    lv_obj_align(s_attitude_container, LV_ALIGN_CENTER, 0, 0); 
    lv_obj_set_style_radius(s_attitude_container, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(s_attitude_container, true, 0);
    lv_obj_set_style_border_width(s_attitude_container, 2, 0);
    lv_obj_set_style_border_color(s_attitude_container, COLOR_OUTER_BORDER, 0);
    lv_obj_set_style_pad_all(s_attitude_container, 0, 0);
    lv_obj_set_scrollbar_mode(s_attitude_container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(s_attitude_container, draw_static_pitch_ref_cb, LV_EVENT_DRAW_POST, NULL);

    // 创建最内层背景
    s_attitude_bg = lv_obj_create(s_attitude_container);
    lv_obj_set_size(s_attitude_bg, INNER_BALL_SIZE, INNER_BALL_SIZE);
    lv_obj_align(s_attitude_bg, LV_ALIGN_CENTER, 0, 0); 
    lv_obj_set_style_bg_opa(s_attitude_bg, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_attitude_bg, 0, 0);
    lv_obj_set_style_pad_all(s_attitude_bg, 0, 0);
    lv_obj_set_scrollbar_mode(s_attitude_bg, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(s_attitude_bg, draw_horizon_line_cb, LV_EVENT_DRAW_MAIN, NULL);

    return s_roll_container;
}

// 数据更新接口
void Widget_Pose_Update(float roll, float pitch, float yaw) {
    bool need_update = false;

    if (s_roll != roll || s_yaw != yaw) {
        s_roll = roll;
        s_yaw = yaw;
        if (s_roll_container) lv_obj_invalidate(s_roll_container);
        need_update = true;
    }

    if (s_pitch != pitch) {
        s_pitch = pitch;
        if (s_attitude_bg) lv_obj_invalidate(s_attitude_bg);
        need_update = true;
    }
}

#include "widget_pose.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// =============================================================
// 核心配置与比例宏定义
#define WIDGET_POSE_SIZE            200

#define INNER_BALL_SIZE             ((int)(WIDGET_POSE_SIZE * 0.708f))
#define PX_PER_DEG                  ((int)(WIDGET_POSE_SIZE * 0.025f))
#define LEN_1DEG                    ((int)(WIDGET_POSE_SIZE * 0.066f))
#define LEN_5DEG                    ((int)(WIDGET_POSE_SIZE * 0.200f))

#define R_TICK_INNER                ((int)(WIDGET_POSE_SIZE * 0.354f))
#define R_TICK_OUTER                ((int)(WIDGET_POSE_SIZE * 0.395f))
#define R_TEXT                      ((int)(WIDGET_POSE_SIZE * 0.437f))

// 浮点比较阈值
#define POSE_EPSILON                0.001f

// 颜色配置宏
#define COLOR_OUTER_BG              lv_palette_darken(LV_PALETTE_GREY, 3)
#define COLOR_OUTER_BORDER          lv_color_white()
#define COLOR_TICK_WHITE            lv_color_white()
#define COLOR_TEXT_WHITE            lv_color_white()

#define COLOR_SKY                   lv_palette_main(LV_PALETTE_BLUE)
#define COLOR_GROUND                lv_palette_main(LV_PALETTE_BROWN)
#define COLOR_T_POINTER             lv_palette_main(LV_PALETTE_ORANGE)
#define COLOR_YAW_ARROW             lv_palette_main(LV_PALETTE_RED)
#define COLOR_REF_LINE              lv_palette_main(LV_PALETTE_RED)

// 线宽配置宏
#define WIDTH_PITCH_5DEG            2
#define WIDTH_PITCH_1DEG            1
#define WIDTH_T_POINTER             2
#define WIDTH_REF_LINE              2
// =============================================================

static float s_roll  = 0.0f;
static float s_pitch = 0.0f;
static float s_yaw   = 0.0f;

static lv_obj_t* s_roll_container    = NULL;
static lv_obj_t* s_attitude_container = NULL;
static lv_obj_t* s_attitude_bg       = NULL;

// ---------------------------------------------------------------
// 最内层：天地背景 + 俯仰刻度线（DRAW_MAIN，最先执行）
// ---------------------------------------------------------------
static void draw_horizon_line_cb(lv_event_t* e)
{
    lv_obj_t*      obj      = lv_event_get_target(e);
    lv_draw_ctx_t* draw_ctx = lv_event_get_draw_ctx(e);

    lv_area_t obj_area;
    lv_obj_get_coords(obj, &obj_area);

    lv_coord_t cx = obj_area.x1 + lv_area_get_width(&obj_area)  / 2;
    lv_coord_t cy = obj_area.y1 + lv_area_get_height(&obj_area) / 2;

    // 统一用 pitch_offset_y 表示"地平线相对中心的偏移量"
    lv_coord_t pitch_offset_y = (lv_coord_t)(s_pitch * PX_PER_DEG);
    lv_coord_t horizon_y      = cy + pitch_offset_y;

    // --- 天空 / 地面背景 ---
    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.border_width = 0;

    rect_dsc.bg_color = COLOR_SKY;
    lv_area_t sky_area = { obj_area.x1, obj_area.y1, obj_area.x2, horizon_y };
    lv_draw_rect(draw_ctx, &rect_dsc, &sky_area);

    rect_dsc.bg_color = COLOR_GROUND;
    lv_area_t ground_area = { obj_area.x1, horizon_y, obj_area.x2, obj_area.y2 };
    lv_draw_rect(draw_ctx, &rect_dsc, &ground_area);

    // --- 俯仰刻度线 ---
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

    for (int angle = -90; angle <= 90; angle++) {
        // 刻度线的屏幕 Y 位置：俯仰角越大，线越靠上（cy - angle * px）
        // pitch_offset_y 将整个刻度系统随机体俯仰平移
        lv_coord_t y_pos = cy - (lv_coord_t)(angle * PX_PER_DEG) + pitch_offset_y;

        if (y_pos < obj_area.y1 || y_pos > obj_area.y2) continue;

        if (angle % 5 == 0) {
            lv_point_t p1 = { cx - LEN_5DEG / 2, y_pos };
            lv_point_t p2 = { cx + LEN_5DEG / 2, y_pos };
            lv_draw_line(draw_ctx, &line_5deg, &p1, &p2);

            char angle_text[8];
            lv_snprintf(angle_text, sizeof(angle_text), "%d", angle);

            lv_area_t label_area = { p2.x + 5, y_pos - LEN_1DEG / 2,
                                     p2.x + 40, y_pos + LEN_1DEG };
            lv_draw_label(draw_ctx, &label_dsc, &label_area, angle_text, NULL);
        } else {
            lv_point_t p1 = { cx - LEN_1DEG / 2, y_pos };
            lv_point_t p2 = { cx + LEN_1DEG / 2, y_pos };
            lv_draw_line(draw_ctx, &line_1deg, &p1, &p2);
        }
    }
}

// ---------------------------------------------------------------
// 内环静止参考线（DRAW_POST，晚于 DRAW_MAIN，始终压在俯仰背景上方）
// ---------------------------------------------------------------
static void draw_static_pitch_ref_cb(lv_event_t* e)
{
    lv_obj_t*      obj      = lv_event_get_target(e);
    lv_draw_ctx_t* draw_ctx = lv_event_get_draw_ctx(e);

    lv_area_t obj_area;
    lv_obj_get_coords(obj, &obj_area);
    lv_coord_t cy = obj_area.y1 + lv_area_get_height(&obj_area) / 2;

    lv_draw_line_dsc_t line_red;
    lv_draw_line_dsc_init(&line_red);
    line_red.color = COLOR_REF_LINE;
    line_red.width = WIDTH_REF_LINE;

    lv_point_t p_left_a  = { obj_area.x1,           cy };
    lv_point_t p_left_b  = { obj_area.x1 + LEN_1DEG, cy };
    lv_draw_line(draw_ctx, &line_red, &p_left_a, &p_left_b);

    lv_point_t p_right_a = { obj_area.x2,            cy };
    lv_point_t p_right_b = { obj_area.x2 - LEN_1DEG, cy };
    lv_draw_line(draw_ctx, &line_red, &p_right_a, &p_right_b);
}

// ---------------------------------------------------------------
// 外环：罗盘刻度 / T型指针 / 偏航箭头（DRAW_POST，最后执行）
// ---------------------------------------------------------------
static void draw_roll_yaw_overlay_cb(lv_event_t* e)
{
    lv_obj_t*      obj      = lv_event_get_target(e);
    lv_draw_ctx_t* draw_ctx = lv_event_get_draw_ctx(e);

    lv_area_t obj_area;
    lv_obj_get_coords(obj, &obj_area);

    lv_coord_t  cx    = obj_area.x1 + lv_area_get_width(&obj_area)  / 2;
    lv_coord_t  cy    = obj_area.y1 + lv_area_get_height(&obj_area) / 2;
    lv_point_t  pivot = { cx, cy };

    // --- 罗盘刻度与方位文字 ---
    lv_draw_line_dsc_t line_tick;
    lv_draw_line_dsc_init(&line_tick);
    line_tick.color = COLOR_TICK_WHITE;

    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    label_dsc.color = COLOR_TEXT_WHITE;
    label_dsc.align = LV_TEXT_ALIGN_CENTER;

    for (int angle = -180; angle < 180; angle += 30) {
        lv_point_t p_in  = { cx, cy - R_TICK_INNER };
        lv_point_t p_out = { cx, cy - R_TICK_OUTER };
        lv_point_t p_txt = { cx, cy - R_TEXT };

        lv_point_transform(&p_in,  angle * 10, 256, &pivot);
        lv_point_transform(&p_out, angle * 10, 256, &pivot);
        lv_point_transform(&p_txt, angle * 10, 256, &pivot);

        line_tick.width = (angle % 90 == 0) ? 4 : 2;
        lv_draw_line(draw_ctx, &line_tick, &p_in, &p_out);

        char text_buf[8];
        // angle == -180 是循环中 S 方位的唯一触达值（180 不可达）
        if      (angle ==    0) lv_snprintf(text_buf, sizeof(text_buf), "N");
        else if (angle ==   90) lv_snprintf(text_buf, sizeof(text_buf), "E");
        else if (angle == -180) lv_snprintf(text_buf, sizeof(text_buf), "S");
        else if (angle ==  -90) lv_snprintf(text_buf, sizeof(text_buf), "W");
        else                    lv_snprintf(text_buf, sizeof(text_buf), "%d", angle);

        lv_area_t txt_area = { p_txt.x - 20, p_txt.y - 8,
                                p_txt.x + 20, p_txt.y + 8 };
        lv_draw_label(draw_ctx, &label_dsc, &txt_area, text_buf, NULL);
    }

    // --- T型指针（橙色），合并水平线为一次绘制 ---
    lv_draw_line_dsc_t line_orange;
    lv_draw_line_dsc_init(&line_orange);
    line_orange.color = COLOR_T_POINTER;
    line_orange.width = WIDTH_T_POINTER;

    lv_point_t t_left   = { cx - (int)(WIDGET_POSE_SIZE * 0.125f), cy };
    lv_point_t t_right  = { cx + (int)(WIDGET_POSE_SIZE * 0.125f), cy };
    lv_point_t t_ptr_in = { cx, cy - R_TICK_INNER + (int)(WIDGET_POSE_SIZE * 0.050f) };
    lv_point_t t_ptr_out= { cx, cy - R_TICK_INNER };

    int32_t roll_angle = (int32_t)(s_roll * 10);
    lv_point_transform(&t_left,    roll_angle, 256, &pivot);
    lv_point_transform(&t_right,   roll_angle, 256, &pivot);
    lv_point_transform(&t_ptr_in,  roll_angle, 256, &pivot);
    lv_point_transform(&t_ptr_out, roll_angle, 256, &pivot);

    // 水平臂：左端 → 右端（途经中心，合并为一条线）
    lv_draw_line(draw_ctx, &line_orange, &t_left,   &t_right);
    // 垂直短线：指向外圈
    lv_draw_line(draw_ctx, &line_orange, &t_ptr_in, &t_ptr_out);

    // --- 偏航三角箭头（红色）---
    lv_draw_line_dsc_t line_red;
    lv_draw_line_dsc_init(&line_red);
    line_red.color = COLOR_YAW_ARROW;
    line_red.width = 3;

    lv_point_t y_ptr[3] = {
        { cx,     cy - (int)(WIDGET_POSE_SIZE * 0.466f) },
        { cx - 6, cy - (int)(WIDGET_POSE_SIZE * 0.500f) },
        { cx + 6, cy - (int)(WIDGET_POSE_SIZE * 0.500f) }
    };

    int32_t yaw_angle = (int32_t)(s_yaw * 10);
    lv_point_transform(&y_ptr[0], yaw_angle, 256, &pivot);
    lv_point_transform(&y_ptr[1], yaw_angle, 256, &pivot);
    lv_point_transform(&y_ptr[2], yaw_angle, 256, &pivot);

    lv_draw_line(draw_ctx, &line_red, &y_ptr[0], &y_ptr[1]);
    lv_draw_line(draw_ctx, &line_red, &y_ptr[1], &y_ptr[2]);
    lv_draw_line(draw_ctx, &line_red, &y_ptr[2], &y_ptr[0]);
}

// ---------------------------------------------------------------
// 初始化
// ---------------------------------------------------------------
lv_obj_t* Widget_Pose_Init(lv_obj_t* parent)
{
    // 外环：承载罗盘/T型指针/偏航箭头
    s_roll_container = lv_obj_create(parent);
    lv_obj_set_size(s_roll_container, WIDGET_POSE_SIZE, WIDGET_POSE_SIZE);
    lv_obj_align(s_roll_container, LV_ALIGN_CENTER, 0, 6);
    lv_obj_set_style_radius(s_roll_container, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(s_roll_container, COLOR_OUTER_BG, 0);
    lv_obj_set_style_border_width(s_roll_container, 0, 0);
    lv_obj_set_style_pad_all(s_roll_container, 0, 0);
    lv_obj_set_scrollbar_mode(s_roll_container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(s_roll_container, draw_roll_yaw_overlay_cb, LV_EVENT_DRAW_POST, NULL);
    lv_obj_clear_flag(s_roll_container,    LV_OBJ_FLAG_SCROLLABLE);

    // 内环：clip_corner 将子对象天地背景裁切为圆形；承载静止参考线
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
    lv_obj_clear_flag(s_attitude_container, LV_OBJ_FLAG_SCROLLABLE);
    
    // 透明罩：与内环等大，背景透明，用于隔离绘制顺序
    // DRAW_MAIN 先于内环的 DRAW_POST，确保天地背景在参考线之下
    s_attitude_bg = lv_obj_create(s_attitude_container);
    lv_obj_set_size(s_attitude_bg, INNER_BALL_SIZE, INNER_BALL_SIZE);
    lv_obj_align(s_attitude_bg, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(s_attitude_bg, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_attitude_bg, 0, 0);
    lv_obj_set_style_pad_all(s_attitude_bg, 0, 0);
    lv_obj_set_scrollbar_mode(s_attitude_bg, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(s_attitude_bg, draw_horizon_line_cb, LV_EVENT_DRAW_MAIN, NULL);
    lv_obj_clear_flag(s_attitude_bg,        LV_OBJ_FLAG_SCROLLABLE);

    return s_roll_container;
}

// ---------------------------------------------------------------
// 数据更新：仅在值发生变化时触发重绘
// ---------------------------------------------------------------
void Widget_Pose_Update(float roll, float pitch, float yaw)
{
    if (fabsf(s_roll - roll) > POSE_EPSILON || fabsf(s_yaw - yaw) > POSE_EPSILON) {
        s_roll = roll;
        s_yaw  = yaw;
        if (s_roll_container) lv_obj_invalidate(s_roll_container);
    }

    if (fabsf(s_pitch - pitch) > POSE_EPSILON) {
        s_pitch = pitch;
        if (s_attitude_bg) lv_obj_invalidate(s_attitude_bg);
    }
}
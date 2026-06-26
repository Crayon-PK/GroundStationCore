#include "page_home.h"
#include "widget_pose.h"
#include "data_pool.h"
#include "lvgl.h"
#include <stdio.h>

// 屏幕与颜色常量
#define SCREEN_W   800
#define SCREEN_H   480
#define NAV_H       45

#define CLR_BG     0x0D1117
#define CLR_CARD   0x161B22
#define CLR_TEXT   0xE6EDF3
#define CLR_MUTED  0x6B7280
#define CLR_BLUE   0x58A6FF
#define CLR_GREEN  0x3FB950
#define CLR_RED    0xFF4444
#define CLR_ORANGE 0xFF8C00
#define CLR_AMBER  0xFFC300

// 卡片结构体（外壳 + 纵向 Flex 内容区）
typedef struct {
    lv_obj_t *card;
    lv_obj_t *body;
} ui_card_t;

// 共享样式
static lv_style_t s_sty_card;
static lv_style_t s_sty_title;
static lv_style_t s_sty_value;
static lv_style_t s_sty_sub;       // 二级小数据（爬升率、电流等）

// 页面根句柄
static lv_obj_t *s_tabview     = NULL;  // 全局 tabview 根

// HOME Tab 动态标签句柄（供 Update 高频刷新）
// 行1：状态
static lv_obj_t *s_val_mode    = NULL;
static lv_obj_t *s_val_armed   = NULL;
static lv_obj_t *s_val_heading = NULL;

// 行2：高度 + 速度
static lv_obj_t *s_val_alt     = NULL;  // "48.3 m"
static lv_obj_t *s_val_climb   = NULL;  // "▲ +0.2 m/s"
static lv_obj_t *s_val_gspd    = NULL;  // "3.2 m/s"
static lv_obj_t *s_val_aspd    = NULL;  // "Air 3.1 m/s"

// 行3：电池 + GPS
static lv_obj_t *s_val_volt    = NULL;  // "23.4 V"
static lv_obj_t *s_val_curr    = NULL;  // "6.2 A  ·  38%"
static lv_obj_t *s_bar_bat     = NULL;  // 进度条
static lv_obj_t *s_val_gps_fix = NULL;  // "● 3D Fix"
static lv_obj_t *s_val_gps_sub = NULL;  // "14 sats · HDOP 0.8"

// 行4：RPY
static lv_obj_t *s_val_rpy     = NULL;

// 右侧：油门 Arc
static lv_obj_t *s_arc_throttle   = NULL;
static lv_obj_t *s_val_throttle   = NULL;  // 数字百分比

// 样式初始化
static void style_init(void)
{
    // 卡片背景
    lv_style_init(&s_sty_card);
    lv_style_set_bg_color(&s_sty_card, lv_color_hex(CLR_CARD));
    lv_style_set_bg_opa(&s_sty_card, LV_OPA_COVER);
    lv_style_set_border_width(&s_sty_card, 0);
    lv_style_set_radius(&s_sty_card, 8);
    lv_style_set_pad_all(&s_sty_card, 8);

    // 灰色小标题
    lv_style_init(&s_sty_title);
    lv_style_set_text_color(&s_sty_title, lv_color_hex(CLR_MUTED));
    lv_style_set_text_font(&s_sty_title, &lv_font_montserrat_10);

    // 主数值
    lv_style_init(&s_sty_value);
    lv_style_set_text_color(&s_sty_value, lv_color_hex(CLR_TEXT));
    lv_style_set_text_font(&s_sty_value, &lv_font_montserrat_14);

    // 二级小数据
    lv_style_init(&s_sty_sub);
    lv_style_set_text_color(&s_sty_sub, lv_color_hex(CLR_MUTED));
    lv_style_set_text_font(&s_sty_sub, &lv_font_montserrat_10);
}

// 工厂函数
static ui_card_t make_card(lv_obj_t *parent, lv_coord_t w, lv_coord_t h)
{
    ui_card_t c;

    c.card = lv_obj_create(parent);
    lv_obj_add_style(c.card, &s_sty_card, 0);
    lv_obj_set_size(c.card, w, h);
    lv_obj_set_scrollbar_mode(c.card, LV_SCROLLBAR_MODE_OFF);

    c.body = lv_obj_create(c.card);
    lv_obj_set_style_bg_opa(c.body, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(c.body, 0, 0);
    lv_obj_set_style_pad_all(c.body, 0, 0);
    lv_obj_set_size(c.body, lv_pct(100), lv_pct(100));
    lv_obj_set_scrollbar_mode(c.body, LV_SCROLLBAR_MODE_OFF);

    lv_obj_set_layout(c.body, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(c.body, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(c.body, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(c.body, 2, 0);

    return c;
}

static lv_obj_t *make_title(lv_obj_t *body, const char *text)
{
    lv_obj_t *o = lv_label_create(body);
    lv_obj_add_style(o, &s_sty_title, 0);
    lv_label_set_text(o, text);
    return o;
}

static lv_obj_t *make_value(lv_obj_t *body, const char *text)
{
    lv_obj_t *o = lv_label_create(body);
    lv_obj_add_style(o, &s_sty_value, 0);
    lv_label_set_text(o, text);
    return o;
}

static lv_obj_t *make_sub(lv_obj_t *body, const char *text)
{
    lv_obj_t *o = lv_label_create(body);
    lv_obj_add_style(o, &s_sty_sub, 0);
    lv_label_set_text(o, text);
    return o;
}

// 各卡片业务创建
/* 行1：MODE / STATUS / HEADING（3 × 31%，高 60px） */
static void create_card_status(lv_obj_t *parent)
{
    ui_card_t c_mode = make_card(parent, lv_pct(31), 60);
    make_title(c_mode.body, "MODE");
    s_val_mode = make_value(c_mode.body, "---");

    ui_card_t c_arm = make_card(parent, lv_pct(31), 60);
    make_title(c_arm.body, "STATUS");
    s_val_armed = make_value(c_arm.body, "DISARMED");

    ui_card_t c_hdg = make_card(parent, lv_pct(31), 60);
    make_title(c_hdg.body, "HEADING");
    s_val_heading = make_value(c_hdg.body, "---°");
}

/* 行2：ALTITUDE（含爬升率）/ GROUND SPEED（含空速）（2 × 48%，高 85px） */
static void create_card_move(lv_obj_t *parent)
{
    ui_card_t c_alt = make_card(parent, lv_pct(48), 85);
    make_title(c_alt.body, "ALTITUDE (REL)");
    s_val_alt   = make_value(c_alt.body, "--- m");
    s_val_climb = make_sub(c_alt.body,   "-- m/s");

    ui_card_t c_spd = make_card(parent, lv_pct(48), 85);
    make_title(c_spd.body, "GROUND SPEED");
    s_val_gspd = make_value(c_spd.body, "--- m/s");
    s_val_aspd = make_sub(c_spd.body,   "Air --- m/s");
}

/* 行3：BATTERY（电压 + 电流 + 进度条）/ GPS（fix + 卫星 + HDOP）（2 × 48%，高 90px） */
static void create_card_sys(lv_obj_t *parent)
{
    // --- 电池卡片 ---
    ui_card_t c_bat = make_card(parent, lv_pct(48), 90);
    make_title(c_bat.body, "BATTERY");
    s_val_volt = make_value(c_bat.body, "--- V");
    s_val_curr = make_sub(c_bat.body, "-- A  ·  --%");

    s_bar_bat = lv_bar_create(c_bat.body);
    lv_obj_set_size(s_bar_bat, lv_pct(100), 8);
    lv_obj_set_style_bg_color(s_bar_bat, lv_color_hex(0x2A3A2A), LV_PART_MAIN);              // 背景槽
    lv_obj_set_style_bg_color(s_bar_bat, lv_color_hex(CLR_GREEN), LV_PART_INDICATOR);
    lv_bar_set_range(s_bar_bat, 0, 100);
    lv_bar_set_value(s_bar_bat, 0, LV_ANIM_OFF);

    // --- GPS 卡片 ---
    ui_card_t c_gps = make_card(parent, lv_pct(48), 90);
    make_title(c_gps.body, "GPS");
    s_val_gps_fix = make_value(c_gps.body, "No Fix");
    s_val_gps_sub = make_sub(c_gps.body, "-- sats  ·  HDOP --");
}

/* 行4：ROLL / PITCH / YAW 底条（100%，高 55px） */
static void create_card_rpy(lv_obj_t *parent)
{
    ui_card_t c_rpy = make_card(parent, lv_pct(100), 55);
    make_title(c_rpy.body, "ROLL / PITCH / YAW");
    s_val_rpy = make_value(c_rpy.body, "R: +0.0°      P: +0.0°      Y: 000.0°");
}

/* 右侧上区：姿态仪表盘卡片（flex_grow=3，占约 60%） */
static void create_card_pose(lv_obj_t *parent)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_add_style(card, &s_sty_card, 0);
    lv_obj_set_flex_grow(card, 3);
    lv_obj_set_width(card, lv_pct(100));
    lv_obj_set_style_pad_all(card, 6, 0);
    lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *title = lv_label_create(card);
    lv_obj_add_style(title, &s_sty_title, 0);
    lv_label_set_text(title, "INSTRUMENT PANEL");
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, 0);

    Widget_Pose_Init(card); 
}

/* 右侧下区：油门 Arc 卡片（flex_grow=2，占约 40%） */
static void create_throttle_arc(lv_obj_t *parent)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_add_style(card, &s_sty_card, 0);
    lv_obj_set_flex_grow(card, 2);
    lv_obj_set_width(card, lv_pct(100));
    lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_all(card, 6, 0);

    // 小标题
    lv_obj_t *title = lv_label_create(card);
    lv_obj_add_style(title, &s_sty_title, 0);
    lv_label_set_text(title, "THROTTLE");
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, 0);

    // Arc 仪表盘（半圆形，从 135° 到 405°）
    s_arc_throttle = lv_arc_create(card);
    lv_arc_set_rotation(s_arc_throttle, 135);
    lv_arc_set_bg_angles(s_arc_throttle, 0, 270);
    lv_arc_set_range(s_arc_throttle, 0, 100);
    lv_arc_set_value(s_arc_throttle, 0);
    lv_obj_remove_style(s_arc_throttle, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(s_arc_throttle, LV_OBJ_FLAG_CLICKABLE);

    // Arc 尺寸与位置
    lv_coord_t arc_size = 90;
    lv_obj_set_size(s_arc_throttle, arc_size, arc_size);
    lv_obj_align(s_arc_throttle, LV_ALIGN_CENTER, 0, 4);

    // Arc 颜色
    lv_obj_set_style_arc_color(s_arc_throttle, lv_color_hex(0x2A2F3A), LV_PART_MAIN);      // 背景弧
    lv_obj_set_style_arc_color(s_arc_throttle, lv_color_hex(CLR_ORANGE), LV_PART_INDICATOR); // 前景弧
    lv_obj_set_style_arc_width(s_arc_throttle, 8, LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_arc_throttle, 8, LV_PART_INDICATOR);

    // 中心数字百分比
    s_val_throttle = lv_label_create(card);
    lv_obj_add_style(s_val_throttle, &s_sty_value, 0);
    lv_obj_set_style_text_font(s_val_throttle, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(s_val_throttle, lv_color_hex(CLR_ORANGE), 0);
    lv_label_set_text(s_val_throttle, "0%");
    lv_obj_align(s_val_throttle, LV_ALIGN_CENTER, 0, 4);
}

// 主界面初始化函数
void Page_Home_Create(void)
{
    if (s_tabview != NULL) return;

    style_init();

    // ── 全屏 tabview，Tab Bar 固定在底部，高度 NAV_H ──────────────
    s_tabview = lv_tabview_create(lv_scr_act(), LV_DIR_BOTTOM, NAV_H);
    lv_obj_set_size(s_tabview, SCREEN_W, SCREEN_H);
    lv_obj_set_style_bg_color(s_tabview, lv_color_hex(CLR_BG), 0);
    lv_obj_set_style_bg_opa(s_tabview, LV_OPA_COVER, 0);

    // Tab Bar 样式
    lv_obj_t *tab_btns = lv_tabview_get_tab_btns(s_tabview);
    lv_obj_set_style_bg_color(tab_btns, lv_color_hex(CLR_CARD), 0);
    lv_obj_set_style_text_color(tab_btns, lv_color_hex(CLR_MUTED), 0);
    lv_obj_set_style_text_color(tab_btns, lv_color_hex(CLR_BLUE), LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_border_side(tab_btns, LV_BORDER_SIDE_TOP, LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_border_color(tab_btns, lv_color_hex(CLR_BLUE), LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_border_width(tab_btns, 2, LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_text_font(tab_btns, &lv_font_montserrat_10, 0);

    // ── Tab 0：HOME ────────────────────────────────────────────────
    lv_obj_t *tab_home = lv_tabview_add_tab(s_tabview, "HOME");
    lv_obj_set_style_bg_color(tab_home, lv_color_hex(CLR_BG), 0);
    lv_obj_set_style_pad_all(tab_home, 8, 0);
    lv_obj_set_style_pad_column(tab_home, 12, 0);
    lv_obj_set_scrollbar_mode(tab_home, LV_SCROLLBAR_MODE_OFF);

    // tab_home 内部横向分割：left_panel | right_panel
    lv_obj_set_layout(tab_home, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(tab_home, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(tab_home, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    // ── left_panel（ROW_WRAP，数据卡片区）─────────────────────────
    lv_obj_t *left_panel = lv_obj_create(tab_home);
    lv_obj_set_size(left_panel, lv_pct(48), lv_pct(100));
    lv_obj_set_style_bg_opa(left_panel, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(left_panel, 0, 0);
    lv_obj_set_style_pad_all(left_panel, 0, 0);
    lv_obj_set_style_pad_row(left_panel, 8, 0);
    lv_obj_set_style_pad_column(left_panel, 8, 0);
    lv_obj_set_scrollbar_mode(left_panel, LV_SCROLLBAR_MODE_OFF);

    lv_obj_set_layout(left_panel, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(left_panel, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(left_panel, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    // ── right_panel（COLUMN，上下两区由各卡片的 flex_grow 自动分配比例）
    lv_obj_t *right_panel = lv_obj_create(tab_home);
    lv_obj_set_flex_grow(right_panel, 1);
    lv_obj_set_size(right_panel, LV_SIZE_CONTENT, lv_pct(100));
    lv_obj_set_style_bg_opa(right_panel, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(right_panel, 0, 0);
    lv_obj_set_style_pad_all(right_panel, 0, 0);
    lv_obj_set_style_pad_row(right_panel, 8, 0);
    lv_obj_set_scrollbar_mode(right_panel, LV_SCROLLBAR_MODE_OFF);

    lv_obj_set_layout(right_panel, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(right_panel, LV_FLEX_FLOW_COLUMN);

    // ── Tab 1：RC CHANNELS（预留，后续填充）───────────────────────
    lv_obj_t *tab_rc = lv_tabview_add_tab(s_tabview, "RC CH");
    lv_obj_set_style_bg_color(tab_rc, lv_color_hex(CLR_BG), 0);
    lv_obj_set_style_pad_all(tab_rc, 8, 0);
    lv_obj_set_scrollbar_mode(tab_rc, LV_SCROLLBAR_MODE_OFF);

    // RC 页预留空白，后续填充内容时在此处添加控件
    (void)tab_rc;   // 消除未使用警告

    // 禁用 tabview 左右滑动手势，改为纯按键切换
    lv_obj_clear_flag(lv_tabview_get_content(s_tabview), LV_OBJ_FLAG_SCROLLABLE);

    // ── 注入各数据卡片 ─────────────────────────────────────────────
    create_card_status(left_panel);
    create_card_move(left_panel);
    create_card_sys(left_panel);
    create_card_rpy(left_panel);
    create_card_pose(right_panel);
    create_throttle_arc(right_panel);
}

void Page_Home_Update(void)
{
    char buf[64];
    // ── 行1：MODE / STATUS / HEADING ──────────────────────────────
    {
        LinkStatus_t st;
        DataPool_GetStatus(&st);

        lv_label_set_text(s_val_mode, st.mode_str);
        lv_label_set_text(s_val_armed, st.is_armed ? "ARMED" : "DISARMED");
        lv_obj_set_style_text_color(s_val_armed,
            st.is_armed ? lv_color_hex(CLR_RED) : lv_color_hex(CLR_GREEN), 0);
    }
    {
        LinkVfrHud_t hud;
        DataPool_GetVfrHud(&hud);
        
        // 航向角为整数，直接输出
        snprintf(buf, sizeof(buf), "%03u°", hud.heading);
        lv_label_set_text(s_val_heading, buf);

        // ── 行2：ALTITUDE + 爬升率 ─────────────────────────────
        snprintf(buf, sizeof(buf), "%.1f m", hud.alt);
        lv_label_set_text(s_val_alt, buf);

        if (hud.climb_rate >= 0.0f) {
            snprintf(buf, sizeof(buf), LV_SYMBOL_UP " +%.1f m/s", hud.climb_rate);
            lv_obj_set_style_text_color(s_val_climb, lv_color_hex(CLR_GREEN), 0);
        } else {
            snprintf(buf, sizeof(buf), LV_SYMBOL_DOWN " %.1f m/s", hud.climb_rate);
            lv_obj_set_style_text_color(s_val_climb, lv_color_hex(CLR_RED), 0);
        }
        lv_label_set_text(s_val_climb, buf);

        // ── 行2：GROUND SPEED + 空速 ────────────────────────────
        snprintf(buf, sizeof(buf), "%.1f m/s", hud.groundspeed);
        lv_label_set_text(s_val_gspd, buf);

        snprintf(buf, sizeof(buf), "Air %.1f m/s", hud.airspeed);
        lv_label_set_text(s_val_aspd, buf);

        // ── 右侧：油门 Arc ──────────────────────────────────────
        int32_t thr = (int32_t)hud.throttle_pct;
        if (thr < 0)   thr = 0;
        if (thr > 100) thr = 100;
        lv_arc_set_value(s_arc_throttle, thr);
        
        snprintf(buf, sizeof(buf), "%d%%", (int)thr);
        lv_label_set_text(s_val_throttle, buf);
        
        lv_obj_set_style_arc_color(s_arc_throttle,
            thr > 80 ? lv_color_hex(CLR_RED) : lv_color_hex(CLR_ORANGE),
            LV_PART_INDICATOR);
        lv_obj_set_style_text_color(s_val_throttle,
            thr > 80 ? lv_color_hex(CLR_RED) : lv_color_hex(CLR_ORANGE), 0);
    }

    // ── 行3：BATTERY ──────────────────────────────────────────────
    {
        LinkBattery_t bat;
        DataPool_GetBattery(&bat);

        snprintf(buf, sizeof(buf), "%.1f V", bat.voltage_mv / 1000.0f);
        lv_label_set_text(s_val_volt, buf);

        float curr_a = (bat.current_ca >= 0) ? bat.current_ca / 10.0f : 0.0f;
        int   pct    = (bat.remaining_pct >= 0) ? (int)bat.remaining_pct : 0;
        snprintf(buf, sizeof(buf), "%.1f A  \xB7  %d%%", curr_a, pct);
        lv_label_set_text(s_val_curr, buf);

        lv_color_t bar_clr = lv_color_hex(CLR_GREEN);
        if (pct <= 20)      bar_clr = lv_color_hex(CLR_RED);
        else if (pct <= 40) bar_clr = lv_color_hex(CLR_AMBER);
        lv_obj_set_style_bg_color(s_bar_bat, bar_clr, LV_PART_INDICATOR);
        lv_bar_set_value(s_bar_bat, pct, LV_ANIM_OFF);

        uint8_t warn = DataPool_GetBatteryWarnLevel();
        lv_obj_set_style_text_color(s_val_volt,
            warn == 2 ? lv_color_hex(CLR_RED)    :
            warn == 1 ? lv_color_hex(CLR_AMBER)  :
                        lv_color_hex(CLR_TEXT),  0);
    }

    // ── 行3：GPS ──────────────────────────────────────────────────
    {
        LinkGps_t gps;
        DataPool_GetGps(&gps);

        const char *fix_str;
        lv_color_t  fix_clr;
        switch (gps.fix_type) {
            case 0:
            case 1:  fix_str = "No Fix";  fix_clr = lv_color_hex(CLR_RED);   break;
            case 2:  fix_str = "2D Fix";  fix_clr = lv_color_hex(CLR_AMBER); break;
            case 3:  fix_str = "3D Fix";  fix_clr = lv_color_hex(CLR_GREEN); break;
            default: fix_str = "DGPS";    fix_clr = lv_color_hex(CLR_BLUE);  break;
        }
        lv_label_set_text(s_val_gps_fix, fix_str);
        lv_obj_set_style_text_color(s_val_gps_fix, fix_clr, 0);

        snprintf(buf, sizeof(buf), "%u sats  \xB7  HDOP %.1f",
                 gps.satellites_visible, gps.hdop);
        lv_label_set_text(s_val_gps_sub, buf);
    }

    // ── 行4：ROLL / PITCH / YAW（欧拉角） ────────────────────────────
    {
        LinkAttitude_t att;
        DataPool_GetAttitude(&att);
        
        Widget_Pose_Update(att.roll, att.pitch, att.yaw);
        
        snprintf(buf, sizeof(buf), "R: %+.1f°    P: %+.1f°    Y: %+.1f°",
                 att.roll, att.pitch, att.yaw);
        lv_label_set_text(s_val_rpy, buf);
    }
}
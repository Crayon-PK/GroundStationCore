#include "page_home.h"
#include "widget_pose.h"
#include "data_pool.h"
#include "ui_manager.h"
#include "lvgl.h"
#include <stdio.h>

typedef struct {
    lv_obj_t *card;
    lv_obj_t *body;
} ui_card_t;

// 共享样式
static lv_style_t s_sty_card;
static lv_style_t s_sty_title;
static lv_style_t s_sty_value;
static lv_style_t s_sty_sub;

static lv_obj_t *s_page_root = NULL;

// 动态标签句柄
static lv_obj_t *s_val_mode    = NULL;
static lv_obj_t *s_val_armed   = NULL;
static lv_obj_t *s_val_heading = NULL;
static lv_obj_t *s_val_alt     = NULL;  
static lv_obj_t *s_val_climb   = NULL;  
static lv_obj_t *s_val_gspd    = NULL;  
static lv_obj_t *s_val_aspd    = NULL;  
static lv_obj_t *s_val_volt    = NULL;  
static lv_obj_t *s_val_curr    = NULL;  
static lv_obj_t *s_bar_bat     = NULL;  
static lv_obj_t *s_val_gps_fix = NULL;  
static lv_obj_t *s_val_gps_sub = NULL;  
static lv_obj_t *s_val_rpy     = NULL;
static lv_obj_t *s_arc_throttle   = NULL;
static lv_obj_t *s_val_throttle   = NULL;  

static void style_init(void)
{
    lv_style_init(&s_sty_card);
    lv_style_set_bg_color(&s_sty_card, lv_color_hex(CLR_CARD));
    lv_style_set_bg_opa(&s_sty_card, LV_OPA_COVER);
    lv_style_set_border_width(&s_sty_card, 0);
    lv_style_set_radius(&s_sty_card, 8);
    lv_style_set_pad_all(&s_sty_card, 8);

    lv_style_init(&s_sty_title);
    lv_style_set_text_color(&s_sty_title, lv_color_hex(CLR_MUTED));
    lv_style_set_text_font(&s_sty_title, &lv_font_montserrat_10);

    lv_style_init(&s_sty_value);
    lv_style_set_text_color(&s_sty_value, lv_color_hex(CLR_TEXT));
    lv_style_set_text_font(&s_sty_value, &lv_font_montserrat_14);

    lv_style_init(&s_sty_sub);
    lv_style_set_text_color(&s_sty_sub, lv_color_hex(CLR_MUTED));
    lv_style_set_text_font(&s_sty_sub, &lv_font_montserrat_10);
}

static ui_card_t make_card(lv_obj_t *parent, lv_coord_t w, lv_coord_t h)
{
    ui_card_t c;
    c.card = lv_obj_create(parent);
    lv_obj_add_style(c.card, &s_sty_card, 0);
    lv_obj_set_size(c.card, w, h);
    lv_obj_set_scrollbar_mode(c.card, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(c.card, LV_OBJ_FLAG_SCROLLABLE); // 禁用滚动

    c.body = lv_obj_create(c.card);
    lv_obj_set_style_bg_opa(c.body, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(c.body, 0, 0);
    lv_obj_set_style_pad_all(c.body, 0, 0);
    lv_obj_set_size(c.body, lv_pct(100), lv_pct(100));
    lv_obj_set_scrollbar_mode(c.body, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(c.body, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_layout(c.body, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(c.body, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(c.body, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(c.body, 2, 0);
    return c;
}

static lv_obj_t *make_title(lv_obj_t *body, const char *text) {
    lv_obj_t *o = lv_label_create(body);
    lv_obj_add_style(o, &s_sty_title, 0);
    lv_label_set_text(o, text);
    return o;
}

static lv_obj_t *make_value(lv_obj_t *body, const char *text) {
    lv_obj_t *o = lv_label_create(body);
    lv_obj_add_style(o, &s_sty_value, 0);
    lv_label_set_text(o, text);
    return o;
}

static lv_obj_t *make_sub(lv_obj_t *body, const char *text) {
    lv_obj_t *o = lv_label_create(body);
    lv_obj_add_style(o, &s_sty_sub, 0);
    lv_label_set_text(o, text);
    return o;
}

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

static void create_card_sys(lv_obj_t *parent)
{
    ui_card_t c_bat = make_card(parent, lv_pct(48), 90);
    make_title(c_bat.body, "BATTERY");
    s_val_volt = make_value(c_bat.body, "--- V");
    s_val_curr = make_sub(c_bat.body, "-- A  ·  --%");

    s_bar_bat = lv_bar_create(c_bat.body);
    lv_obj_set_size(s_bar_bat, lv_pct(100), 8);
    lv_obj_set_style_bg_color(s_bar_bat, lv_color_hex(0x2A3A2A), LV_PART_MAIN);              
    lv_obj_set_style_bg_color(s_bar_bat, lv_color_hex(CLR_GREEN), LV_PART_INDICATOR);
    lv_bar_set_range(s_bar_bat, 0, 100);
    lv_bar_set_value(s_bar_bat, 0, LV_ANIM_OFF);

    ui_card_t c_gps = make_card(parent, lv_pct(48), 90);
    make_title(c_gps.body, "GPS");
    s_val_gps_fix = make_value(c_gps.body, "No Fix");
    s_val_gps_sub = make_sub(c_gps.body, "-- sats  ·  HDOP --");
}

static void create_card_rpy(lv_obj_t *parent)
{
    ui_card_t c_rpy = make_card(parent, lv_pct(100), 55);
    make_title(c_rpy.body, "ROLL / PITCH / YAW");
    s_val_rpy = make_value(c_rpy.body, "R: +0.0°      P: +0.0°      Y: 000.0°");
}

static void create_card_pose(lv_obj_t *parent)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_add_style(card, &s_sty_card, 0);
    lv_obj_set_flex_grow(card, 3);
    lv_obj_set_width(card, lv_pct(100));
    lv_obj_set_style_pad_all(card, 6, 0);
    lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(card);
    lv_obj_add_style(title, &s_sty_title, 0);
    lv_label_set_text(title, "INSTRUMENT PANEL");
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, 0);

    Widget_Pose_Init(card); 
}

static void create_throttle_arc(lv_obj_t *parent)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_add_style(card, &s_sty_card, 0);
    lv_obj_set_flex_grow(card, 2);
    lv_obj_set_width(card, lv_pct(100));
    lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_all(card, 6, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(card);
    lv_obj_add_style(title, &s_sty_title, 0);
    lv_label_set_text(title, "THROTTLE");
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, 0);

    s_arc_throttle = lv_arc_create(card);
    lv_arc_set_rotation(s_arc_throttle, 135);
    lv_arc_set_bg_angles(s_arc_throttle, 0, 270);
    lv_arc_set_range(s_arc_throttle, 0, 100);
    lv_arc_set_value(s_arc_throttle, 0);
    lv_obj_remove_style(s_arc_throttle, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(s_arc_throttle, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_set_size(s_arc_throttle, 90, 90);
    lv_obj_align(s_arc_throttle, LV_ALIGN_CENTER, 0, 4);

    lv_obj_set_style_arc_color(s_arc_throttle, lv_color_hex(0x2A2F3A), LV_PART_MAIN);      
    lv_obj_set_style_arc_color(s_arc_throttle, lv_color_hex(CLR_ORANGE), LV_PART_INDICATOR); 
    lv_obj_set_style_arc_width(s_arc_throttle, 8, LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_arc_throttle, 8, LV_PART_INDICATOR);

    s_val_throttle = lv_label_create(card);
    lv_obj_add_style(s_val_throttle, &s_sty_value, 0);
    lv_obj_set_style_text_color(s_val_throttle, lv_color_hex(CLR_ORANGE), 0);
    lv_label_set_text(s_val_throttle, "0%");
    lv_obj_align(s_val_throttle, LV_ALIGN_CENTER, 0, 4);
}

void Page_Home_Create(void)
{
    if (s_page_root != NULL) return;
    style_init();

    s_page_root = lv_obj_create(lv_scr_act());
    lv_obj_set_size(s_page_root, SCREEN_W, PAGE_H);
    lv_obj_align(s_page_root, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(s_page_root, lv_color_hex(CLR_BG), 0);
    lv_obj_set_style_bg_opa(s_page_root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(s_page_root, 0, 0);
    lv_obj_set_style_radius(s_page_root, 0, 0);
    lv_obj_set_style_pad_all(s_page_root, 8, 0);
    lv_obj_set_style_pad_column(s_page_root, 12, 0);
    lv_obj_set_scrollbar_mode(s_page_root, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(s_page_root, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_layout(s_page_root, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(s_page_root, LV_FLEX_FLOW_ROW);

    // 左侧面板
    lv_obj_t *left_panel = lv_obj_create(s_page_root);
    lv_obj_set_size(left_panel, lv_pct(48), lv_pct(100));
    lv_obj_set_style_bg_opa(left_panel, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(left_panel, 0, 0);
    lv_obj_set_style_pad_all(left_panel, 0, 0);
    lv_obj_set_style_pad_row(left_panel, 8, 0);
    lv_obj_set_style_pad_column(left_panel, 8, 0);
    lv_obj_set_scrollbar_mode(left_panel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(left_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_layout(left_panel, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(left_panel, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(left_panel, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    // 右侧面板
    lv_obj_t *right_panel = lv_obj_create(s_page_root);
    lv_obj_set_flex_grow(right_panel, 1);
    lv_obj_set_size(right_panel, LV_SIZE_CONTENT, lv_pct(100));
    lv_obj_set_style_bg_opa(right_panel, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(right_panel, 0, 0);
    lv_obj_set_style_pad_all(right_panel, 0, 0);
    lv_obj_set_style_pad_row(right_panel, 8, 0);
    lv_obj_set_scrollbar_mode(right_panel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(right_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_layout(right_panel, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(right_panel, LV_FLEX_FLOW_COLUMN);

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
        lv_obj_set_style_text_color(s_val_armed, st.is_armed ? lv_color_hex(CLR_RED) : lv_color_hex(CLR_GREEN), 0);
    }
    {
        LinkVfrHud_t hud;
        DataPool_GetVfrHud(&hud);
        
        snprintf(buf, sizeof(buf), "%03u°", hud.heading);
        lv_label_set_text(s_val_heading, buf);

        // ── 行2：ALTITUDE ─────────────────────────────
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

        // ── 行2：SPEED ────────────────────────────
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
        
        lv_color_t arc_clr = (thr > 80) ? lv_color_hex(CLR_RED) : lv_color_hex(CLR_ORANGE);
        lv_obj_set_style_arc_color(s_arc_throttle, arc_clr, LV_PART_INDICATOR);
        lv_obj_set_style_text_color(s_val_throttle, arc_clr, 0);
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

        lv_color_t bar_clr = (pct <= 20) ? lv_color_hex(CLR_RED) : ((pct <= 40) ? lv_color_hex(CLR_AMBER) : lv_color_hex(CLR_GREEN));
        lv_obj_set_style_bg_color(s_bar_bat, bar_clr, LV_PART_INDICATOR);
        lv_bar_set_value(s_bar_bat, pct, LV_ANIM_OFF);

        uint8_t warn = DataPool_GetBatteryWarnLevel();
        lv_obj_set_style_text_color(s_val_volt, (warn == 2) ? lv_color_hex(CLR_RED) : ((warn == 1) ? lv_color_hex(CLR_AMBER) : lv_color_hex(CLR_TEXT)),  0);
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

        snprintf(buf, sizeof(buf), "%u sats  \xB7  HDOP %.1f", gps.satellites_visible, gps.hdop);
        lv_label_set_text(s_val_gps_sub, buf);
    }

    // ── 行4：姿态 ────────────────────────────────────────────
    {
        LinkAttitude_t att;
        DataPool_GetAttitude(&att);
        Widget_Pose_Update(att.roll, att.pitch, att.yaw);
        
        snprintf(buf, sizeof(buf), "R: %+.1f°    P: %+.1f°    Y: %+.1f°", att.roll, att.pitch, att.yaw);
        lv_label_set_text(s_val_rpy, buf);
    }
}

void Page_Home_Destroy(void)
{
    if (s_page_root == NULL) return;
    lv_obj_del(s_page_root);
    s_page_root = NULL;

    s_val_mode = s_val_armed = s_val_heading = NULL;
    s_val_alt  = s_val_climb = s_val_gspd   = s_val_aspd = NULL;
    s_val_volt = s_val_curr  = s_bar_bat    = NULL;
    s_val_gps_fix = s_val_gps_sub = NULL;
    s_val_rpy      = NULL;
    s_arc_throttle = s_val_throttle = NULL;
}
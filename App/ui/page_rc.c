#include "page_rc.h"
#include "data_pool.h"
#include "ui_manager.h" 
#include "lvgl.h"
#include <stdio.h>

#define HEADER_H          40
#define CONTENT_H        (PAGE_H - HEADER_H) 
#define COL_W            375
#define CH_ROW_H          95    

#define PWM_MIN         1000
#define PWM_MID         1500
#define PWM_MAX         2000
#define PWM_WARN_LOW    1050
#define PWM_WARN_HIGH   1950
#define PWM_NEUTRAL_LOW  1450
#define PWM_NEUTRAL_HIGH 1550

static const char * const CH_LABELS[8] = {
    "Roll", "Pitch", "Throttle", "Yaw", "CH 5", "CH 6", "CH 7", "CH 8"
};

typedef struct {
    lv_obj_t *bar;      
    lv_obj_t *val_lbl;  
} ChWidget_t;

static lv_obj_t  *s_page_root   = NULL;   
static lv_obj_t  *s_rssi_bar    = NULL;   
static lv_obj_t  *s_rssi_lbl    = NULL;   
static ChWidget_t s_ch[8];                

static lv_color_t bar_color_by_pwm(uint16_t pwm)
{
    if (pwm <= PWM_WARN_LOW || pwm >= PWM_WARN_HIGH) return lv_color_hex(CLR_RED);
    if (pwm >= PWM_NEUTRAL_LOW && pwm <= PWM_NEUTRAL_HIGH) return lv_color_hex(CLR_MUTED);
    return lv_color_hex(CLR_BLUE);
}

static void create_header(lv_obj_t *parent)
{
    // 创建状态栏主容器
    lv_obj_t *hdr = lv_obj_create(parent);
    lv_obj_set_size(hdr, SCREEN_W, HEADER_H);
    lv_obj_set_style_bg_color(hdr, lv_color_hex(CLR_CARD), 0);
    lv_obj_set_style_bg_opa(hdr, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(hdr, 0, 0);
    lv_obj_set_style_radius(hdr, 0, 0);
    lv_obj_set_style_pad_hor(hdr, 16, 0);
    lv_obj_set_scrollbar_mode(hdr, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(hdr, LV_OBJ_FLAG_SCROLLABLE);
    
    // 状态栏自身配置为横向 Flex 布局，且内部子控件全部靠右对齐
    lv_obj_set_layout(hdr, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(hdr, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(hdr, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(hdr, 8, 0); 

    // 信号文本
    lv_obj_t *rssi_title = lv_label_create(hdr);
    lv_label_set_text(rssi_title, "RSSI");
    lv_obj_set_style_text_font(rssi_title, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(rssi_title, lv_color_hex(CLR_MUTED), 0);

    // 信号进度条
    s_rssi_bar = lv_bar_create(hdr);
    lv_obj_set_size(s_rssi_bar, 80, 8);
    lv_bar_set_range(s_rssi_bar, 0, 255);
    lv_obj_set_style_bg_color(s_rssi_bar, lv_color_hex(CLR_BAR_BG), LV_PART_MAIN);
    lv_obj_set_style_bg_color(s_rssi_bar, lv_color_hex(CLR_GREEN), LV_PART_INDICATOR);
    lv_obj_set_style_radius(s_rssi_bar, 4, LV_PART_MAIN);
    lv_obj_set_style_radius(s_rssi_bar, 4, LV_PART_INDICATOR);

    // 信号数字标签
    s_rssi_lbl = lv_label_create(hdr);
    lv_label_set_text(s_rssi_lbl, "---");
    lv_obj_set_style_text_font(s_rssi_lbl, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(s_rssi_lbl, lv_color_hex(CLR_TEXT), 0);
}

static void create_channel_row(lv_obj_t *parent, uint8_t ch_idx, uint8_t row_in_col)
{
    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_set_size(row, COL_W, CH_ROW_H);
    lv_obj_set_style_bg_color(row, (row_in_col % 2 == 0) ? lv_color_hex(CLR_CARD) : lv_color_hex(CLR_CARD2), 0);
    lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_radius(row, 6, 0);
    lv_obj_set_style_pad_hor(row, 12, 0);
    lv_obj_set_style_pad_ver(row, 8, 0);
    lv_obj_set_scrollbar_mode(row, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_layout(row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_obj_t *top_row = lv_obj_create(row);
    lv_obj_set_size(top_row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(top_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(top_row, 0, 0);
    lv_obj_set_style_pad_all(top_row, 0, 0);
    lv_obj_set_scrollbar_mode(top_row, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(top_row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_layout(top_row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(top_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    char ch_name[24];
    snprintf(ch_name, sizeof(ch_name), "CH%d  %s", ch_idx + 1, CH_LABELS[ch_idx]);
    lv_obj_t *name_lbl = lv_label_create(top_row);
    lv_label_set_text(name_lbl, ch_name);
    lv_obj_set_style_text_font(name_lbl, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(name_lbl, lv_color_hex(CLR_MUTED), 0);

    s_ch[ch_idx].val_lbl = lv_label_create(top_row);
    lv_label_set_text(s_ch[ch_idx].val_lbl, "----");
    lv_obj_set_style_text_font(s_ch[ch_idx].val_lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(s_ch[ch_idx].val_lbl, lv_color_hex(CLR_TEXT), 0);

    s_ch[ch_idx].bar = lv_bar_create(row);
    lv_obj_set_size(s_ch[ch_idx].bar, lv_pct(100), 10);
    lv_bar_set_range(s_ch[ch_idx].bar, PWM_MIN, PWM_MAX);
    lv_bar_set_value(s_ch[ch_idx].bar, PWM_MID, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(s_ch[ch_idx].bar, lv_color_hex(CLR_BAR_BG), LV_PART_MAIN);
    lv_obj_set_style_radius(s_ch[ch_idx].bar, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_color(s_ch[ch_idx].bar, lv_color_hex(CLR_BLUE), LV_PART_INDICATOR);
    lv_obj_set_style_radius(s_ch[ch_idx].bar, 5, LV_PART_INDICATOR);

    lv_obj_t *tick = lv_obj_create(row);
    lv_obj_set_size(tick, 2, 6);
    lv_obj_set_style_bg_color(tick, lv_color_hex(CLR_MUTED), 0);
    lv_obj_set_style_bg_opa(tick, LV_OPA_50, 0);
    lv_obj_set_style_border_width(tick, 0, 0);
    lv_obj_align_to(tick, s_ch[ch_idx].bar, LV_ALIGN_OUT_BOTTOM_MID, 0, -4);
}

static void create_content(lv_obj_t *parent)
{
    lv_obj_t *content = lv_obj_create(parent);
    lv_obj_set_size(content, SCREEN_W, CONTENT_H);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(content, 0, 0);
    
    // 设置左右边界内边距各15px，两列中间间距20px
    lv_obj_set_style_pad_hor(content, 15, 0);
    lv_obj_set_style_pad_ver(content, 8, 0);
    lv_obj_set_style_pad_column(content, 20, 0);
    
    lv_obj_set_scrollbar_mode(content, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_layout(content, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_obj_t *col_left = lv_obj_create(content);
    lv_obj_set_size(col_left, COL_W, CONTENT_H - 16);
    lv_obj_set_style_bg_opa(col_left, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(col_left, 0, 0);
    lv_obj_set_style_pad_all(col_left, 0, 0);
    lv_obj_set_style_pad_row(col_left, 6, 0);
    lv_obj_set_scrollbar_mode(col_left, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(col_left, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_layout(col_left, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(col_left, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *col_right = lv_obj_create(content);
    lv_obj_set_size(col_right, COL_W, CONTENT_H - 16);
    lv_obj_set_style_bg_opa(col_right, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(col_right, 0, 0);
    lv_obj_set_style_pad_all(col_right, 0, 0);
    lv_obj_set_style_pad_row(col_right, 6, 0);
    lv_obj_set_scrollbar_mode(col_right, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(col_right, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_layout(col_right, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(col_right, LV_FLEX_FLOW_COLUMN);

    for (uint8_t i = 0; i < 4; i++) create_channel_row(col_left, i, i);
    for (uint8_t i = 4; i < 8; i++) create_channel_row(col_right, i, i - 4);
}

void Page_RC_Create(void)
{
    if (s_page_root != NULL) return;

    s_page_root = lv_obj_create(lv_scr_act());
    lv_obj_set_size(s_page_root, SCREEN_W, PAGE_H);
    lv_obj_align(s_page_root, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(s_page_root, lv_color_hex(CLR_BG), 0);
    lv_obj_set_style_bg_opa(s_page_root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(s_page_root, 0, 0);
    lv_obj_set_style_radius(s_page_root, 0, 0);
    lv_obj_set_style_pad_all(s_page_root, 0, 0);
    lv_obj_set_scrollbar_mode(s_page_root, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(s_page_root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_layout(s_page_root, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(s_page_root, LV_FLEX_FLOW_COLUMN);

    create_header(s_page_root);
    create_content(s_page_root);
}

void Page_RC_Update(void)
{
    if (s_page_root == NULL) return;

    char buf[16];
    LinkRcChannels_t rc;
    DataPool_GetRcChannels(&rc);

    for (uint8_t i = 0; i < 8; i++) {
        uint16_t pwm = rc.chan[i];
        if (pwm == 0) {
            lv_label_set_text(s_ch[i].val_lbl, "----");
            lv_bar_set_value(s_ch[i].bar, PWM_MID, LV_ANIM_OFF);
            lv_obj_set_style_bg_color(s_ch[i].bar, lv_color_hex(CLR_MUTED), LV_PART_INDICATOR);
            continue;
        }

        if (pwm < PWM_MIN) pwm = PWM_MIN;
        if (pwm > PWM_MAX) pwm = PWM_MAX;

        lv_bar_set_value(s_ch[i].bar, (int32_t)pwm, LV_ANIM_OFF);
        lv_obj_set_style_bg_color(s_ch[i].bar, bar_color_by_pwm(pwm), LV_PART_INDICATOR);

        snprintf(buf, sizeof(buf), "%u", pwm);
        lv_label_set_text(s_ch[i].val_lbl, buf);

        lv_color_t val_clr = (pwm <= PWM_WARN_LOW || pwm >= PWM_WARN_HIGH) ? lv_color_hex(CLR_RED) : lv_color_hex(CLR_TEXT);
        lv_obj_set_style_text_color(s_ch[i].val_lbl, val_clr, 0);
    }

    uint8_t rssi = rc.rssi;
    lv_bar_set_value(s_rssi_bar, (int32_t)rssi, LV_ANIM_OFF);
    lv_color_t rssi_clr = (rssi > 150) ? lv_color_hex(CLR_GREEN) : ((rssi > 80) ? lv_color_hex(CLR_AMBER) : lv_color_hex(CLR_RED));
    lv_obj_set_style_bg_color(s_rssi_bar, rssi_clr, LV_PART_INDICATOR);

    snprintf(buf, sizeof(buf), "%u", rssi);
    lv_label_set_text(s_rssi_lbl, buf);
    lv_obj_set_style_text_color(s_rssi_lbl, rssi_clr, 0);
}

void Page_RC_Destroy(void)
{
    if (s_page_root == NULL) return;
    lv_obj_del(s_page_root);
    s_page_root = NULL;
    s_rssi_bar  = NULL;
    s_rssi_lbl  = NULL;

    for (uint8_t i = 0; i < 8; i++) {
        s_ch[i].bar     = NULL;
        s_ch[i].val_lbl = NULL;
    }
}
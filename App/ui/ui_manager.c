#include "ui_manager.h"
#include "page_home.h"
#include "page_rc.h"
#include "lvgl.h"

#define NAVBAR_BTN_W 390

// 颜色常量
#define CLR_BG          0x0D1117
#define CLR_CARD        0x161B22
#define CLR_TEXT        0xE6EDF3
#define CLR_MUTED       0x6B7280
#define CLR_BLUE        0x58A6FF
#define CLR_ACTIVE_BG   0x1A3A5C

// 页面描述符
typedef struct {
    void (*create)(void);
    void (*update)(void);
    void (*destroy)(void);
} UiPage_t;

static const UiPage_t s_pages[UI_PAGE_COUNT] = {
    [UI_PAGE_HOME] = {
        .create  = Page_Home_Create,
        .update  = Page_Home_Update,
        .destroy = Page_Home_Destroy,
    },
    [UI_PAGE_RC] = {
        .create  = Page_RC_Create,
        .update  = Page_RC_Update,
        .destroy = Page_RC_Destroy,
    },
};

// 内部状态
static UiPageId_t  s_current_page = UI_PAGE_HOME;

// 常驻底栏及两个导航按钮
static lv_obj_t   *s_navbar     = NULL;
static lv_obj_t   *s_btn_home   = NULL;
static lv_obj_t   *s_btn_rc     = NULL;

// 底栏按钮样式
static lv_style_t  s_sty_btn_normal;
static lv_style_t  s_sty_btn_active;

// 更新底栏高亮
static void navbar_update_highlight(UiPageId_t active)
{
    lv_obj_remove_style(s_btn_home, &s_sty_btn_active, 0);
    lv_obj_remove_style(s_btn_rc,   &s_sty_btn_active, 0);

    lv_obj_t *active_btn = (active == UI_PAGE_HOME) ? s_btn_home : s_btn_rc;
    lv_obj_add_style(active_btn, &s_sty_btn_active, 0);

    // 同步标签颜色
    lv_obj_t *lbl_home = lv_obj_get_child(s_btn_home, 0);
    lv_obj_t *lbl_rc   = lv_obj_get_child(s_btn_rc,   0);
    if (lbl_home) lv_obj_set_style_text_color(lbl_home,
        (active == UI_PAGE_HOME) ? lv_color_hex(CLR_TEXT) : lv_color_hex(CLR_MUTED), 0);
    if (lbl_rc)   lv_obj_set_style_text_color(lbl_rc,
        (active == UI_PAGE_RC)   ? lv_color_hex(CLR_TEXT) : lv_color_hex(CLR_MUTED), 0);

    lv_obj_invalidate(s_navbar);
}

// LVGL 按钮事件回调
static void btn_home_cb(lv_event_t *e)
{
    (void)e;
    if (s_current_page != UI_PAGE_HOME) {
        UI_Manager_SwitchPage(UI_PAGE_HOME);
    }
}

static void btn_rc_cb(lv_event_t *e)
{
    (void)e;
    if (s_current_page != UI_PAGE_RC) {
        UI_Manager_SwitchPage(UI_PAGE_RC);
    }
}

// 创建常驻底栏
static void navbar_create(void)
{
    // 按钮通用样式
    lv_style_init(&s_sty_btn_normal);
    lv_style_set_bg_color(&s_sty_btn_normal,    lv_color_hex(CLR_CARD));
    lv_style_set_bg_opa(&s_sty_btn_normal,      LV_OPA_COVER);
    lv_style_set_border_width(&s_sty_btn_normal, 0);
    lv_style_set_radius(&s_sty_btn_normal,       6);
    lv_style_set_shadow_width(&s_sty_btn_normal, 0);
    lv_style_set_pad_all(&s_sty_btn_normal,      0);

    // 按钮激活样式
    lv_style_init(&s_sty_btn_active);
    lv_style_set_bg_color(&s_sty_btn_active,     lv_color_hex(CLR_ACTIVE_BG));
    lv_style_set_bg_opa(&s_sty_btn_active,       LV_OPA_COVER);
    lv_style_set_border_width(&s_sty_btn_active,  2);
    lv_style_set_border_color(&s_sty_btn_active,  lv_color_hex(CLR_BLUE));
    lv_style_set_border_side(&s_sty_btn_active,   LV_BORDER_SIDE_TOP);
    lv_style_set_radius(&s_sty_btn_active,        6);
    lv_style_set_shadow_width(&s_sty_btn_active,  0);
    lv_style_set_pad_all(&s_sty_btn_active,       0);

    // 底栏容器
    s_navbar = lv_obj_create(lv_scr_act());
    lv_obj_set_size(s_navbar, SCREEN_W, NAVBAR_H);
    lv_obj_align(s_navbar, LV_ALIGN_BOTTOM_MID, 0, 0);
    
    // 清空内边距，关闭滚动拦截
    lv_obj_set_style_pad_all(s_navbar, 0, 0); 
    lv_obj_set_style_pad_column(s_navbar, 10, 0);
    lv_obj_set_scrollbar_mode(s_navbar, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(s_navbar, LV_OBJ_FLAG_SCROLLABLE);

    // Flex 横向布局
    lv_obj_set_layout(s_navbar, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(s_navbar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(s_navbar, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // HOME 按钮
    s_btn_home = lv_btn_create(s_navbar);
    lv_obj_set_size(s_btn_home, NAVBAR_BTN_W, NAVBAR_H); 
    lv_obj_add_style(s_btn_home, &s_sty_btn_normal, 0);
    lv_obj_add_event_cb(s_btn_home, btn_home_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *lbl_home = lv_label_create(s_btn_home);
    lv_label_set_text(lbl_home, "HOME");
    lv_obj_set_style_text_font(lbl_home, &lv_font_montserrat_14, 0);
    lv_obj_center(lbl_home);

    // RC CHANNELS 按钮
    s_btn_rc = lv_btn_create(s_navbar);
    lv_obj_set_size(s_btn_rc, NAVBAR_BTN_W, NAVBAR_H); 
    lv_obj_add_style(s_btn_rc, &s_sty_btn_normal, 0);
    lv_obj_add_event_cb(s_btn_rc, btn_rc_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *lbl_rc = lv_label_create(s_btn_rc);
    lv_label_set_text(lbl_rc, "RC CHANNELS");
    lv_obj_set_style_text_font(lbl_rc, &lv_font_montserrat_14, 0);
    lv_obj_center(lbl_rc);

    navbar_update_highlight(UI_PAGE_HOME);
}

// 公共接口实现
void UI_Manager_Init(void)
{
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(CLR_BG), 0);
    lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0);

    // 创建页面
    s_current_page = UI_PAGE_HOME;
    s_pages[UI_PAGE_HOME].create();

    // 创建底栏
    navbar_create();
    lv_obj_move_foreground(s_navbar);
}

void UI_Manager_SwitchPage(UiPageId_t page_id)
{
    if (page_id >= UI_PAGE_COUNT) return;
    if (page_id == s_current_page) return;

    s_pages[s_current_page].destroy();
    s_current_page = page_id;
    s_pages[s_current_page].create();

    lv_obj_move_foreground(s_navbar);
    navbar_update_highlight(s_current_page);
}

void UI_Manager_Update(void)
{
    if (s_pages[s_current_page].update != NULL) {
        s_pages[s_current_page].update();
    }
}

UiPageId_t UI_Manager_GetCurrentPage(void)
{
    return s_current_page;
}
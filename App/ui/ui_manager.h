#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "lvgl.h"

// 全局 UI 布局配置
#define SCREEN_W         800
#define SCREEN_H         480
#define NAVBAR_H          30
#define PAGE_H           450

// 全局暗色主题颜色板
#define CLR_BG          0x0D1117   // 深黑背景
#define CLR_CARD        0x161B22   // 卡片背景 1
#define CLR_CARD2       0x1C2333   // 卡片背景 2（交替色）
#define CLR_TEXT        0xE6EDF3   // 主文字
#define CLR_MUTED       0x6B7280   // 暗淡灰色（标题、副文本）
#define CLR_BLUE        0x58A6FF   // 强调蓝
#define CLR_GREEN       0x3FB950   // 正常/安全绿
#define CLR_RED         0xFF4444   // 警告红
#define CLR_ORANGE      0xFF8C00   // 油门/警告橙
#define CLR_AMBER       0xFFC300   // 琥珀黄（中度警告）
#define CLR_BAR_BG      0x444C56   // 进度条未填充槽背景
#define CLR_ACTIVE_BG   0x1A3A5C   // 激活态按钮背景

// 页面管理框架接口
typedef enum {
    UI_PAGE_HOME = 0,
    UI_PAGE_RC,
    UI_PAGE_COUNT
} UiPageId_t;

void UI_Manager_Init(void);
void UI_Manager_SwitchPage(UiPageId_t page_id);
void UI_Manager_Update(void);
UiPageId_t UI_Manager_GetCurrentPage(void);

#endif // UI_MANAGER_H
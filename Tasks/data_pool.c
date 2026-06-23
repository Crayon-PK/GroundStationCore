#include "data_pool.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "mavlink.h"
#include <string.h>

static FlightData_t      s_pool  = {0};
static SemaphoreHandle_t s_mutex = NULL;

// 6S 锂电警告阈值（毫伏）
#define BATT_WARN_MV     21600
#define BATT_DANGER_MV   19800

// 加锁宏
#define LOCK_W()  (s_mutex && xSemaphoreTake(s_mutex, pdMS_TO_TICKS(5))  == pdTRUE)
#define LOCK_R()  (s_mutex && xSemaphoreTake(s_mutex, pdMS_TO_TICKS(10)) == pdTRUE)
#define UNLOCK()  xSemaphoreGive(s_mutex)

// 飞行模式字符串表
static const char* const s_mode_table[] = {
    "Stabilize", "Acro",    "AltHold", "Auto",     "Guided",
    "Loiter",    "RTL",     "Circle",  "Land",     "Drift",
    "Sport",     "Flip",    "AutoTune","PosHold",  "Brake",
    "Throw",     "Avoid",   "GuidedNG","SmartRTL", "FlowHold",
    "Follow",    "ZigZag",  "SystemID","Autorot",  "AutoRTL"
};
#define MODE_TABLE_SIZE ((uint32_t)(sizeof(s_mode_table) / sizeof(s_mode_table[0])))

void DataPool_Init(void)
{
    memset(&s_pool, 0, sizeof(s_pool));
    // 初始化状态字符串，避免UI读到空字符串
    strncpy(s_pool.status.mode_str, "Unknown", sizeof(s_pool.status.mode_str) - 1);
    s_pool.battery.remaining_pct = -1;
    s_pool.battery.current_ca    = -1;
 
    if (s_mutex == NULL) {
        s_mutex = xSemaphoreCreateMutex();
    }
}

// ==================== 写入接口  ====================

void DataPool_SetAttitude(const LinkAttitude_t *att)
{
    if (!att) return;
    if (LOCK_W()) { s_pool.attitude = *att; UNLOCK(); }
}
 
void DataPool_SetGps(const LinkGps_t *gps)
{
    if (!gps) return;
    if (LOCK_W()) { s_pool.gps = *gps; UNLOCK(); }
}
 
void DataPool_SetVfrHud(const LinkVfrHud_t *hud)
{
    if (!hud) return;
    if (LOCK_W()) { s_pool.vfr_hud = *hud; UNLOCK(); }
}
 
void DataPool_SetBattery(const LinkBattery_t *bat)
{
    if (!bat) return;
    if (LOCK_W()) { s_pool.battery = *bat; UNLOCK(); }
}
 
void DataPool_SetStatus(const LinkStatus_t *status)
{
    if (!status) return;
    if (LOCK_W()) { s_pool.status = *status; UNLOCK(); }
}
 
void DataPool_SetRcChannels(const LinkRcChannels_t *rc)
{
    if (!rc) return;
    if (LOCK_W()) { s_pool.rc = *rc; UNLOCK(); }
}

// 统计计数
void DataPool_IncStats_ByMsgId(uint32_t msgid)
{
    s_pool.stats.total_packets++;
    switch (msgid) {
        case MAVLINK_MSG_ID_HEARTBEAT:           s_pool.stats.heartbeat_count++; break;
        case MAVLINK_MSG_ID_ATTITUDE:            s_pool.stats.attitude_count++;  break;
        case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
        case MAVLINK_MSG_ID_GPS_RAW_INT:         s_pool.stats.gps_count++;       break;
        case MAVLINK_MSG_ID_VFR_HUD:             s_pool.stats.vfr_hud_count++;   break;
        case MAVLINK_MSG_ID_SYS_STATUS:          s_pool.stats.battery_count++;   break;
        case MAVLINK_MSG_ID_RC_CHANNELS:         s_pool.stats.rc_count++;        break;
        default: break;
    }
}

// ==================== 读取接口 ====================
void DataPool_GetAttitude(LinkAttitude_t *out)
{
    if (!out) return;
    if (LOCK_R()) { *out = s_pool.attitude; UNLOCK(); }
}
 
void DataPool_GetGps(LinkGps_t *out)
{
    if (!out) return;
    if (LOCK_R()) { *out = s_pool.gps; UNLOCK(); }
}
 
void DataPool_GetVfrHud(LinkVfrHud_t *out)
{
    if (!out) return;
    if (LOCK_R()) { *out = s_pool.vfr_hud; UNLOCK(); }
}
 
void DataPool_GetBattery(LinkBattery_t *out)
{
    if (!out) return;
    if (LOCK_R()) { *out = s_pool.battery; UNLOCK(); }
}
 
void DataPool_GetStatus(LinkStatus_t *out)
{
    if (!out) return;
    if (LOCK_R()) { *out = s_pool.status; UNLOCK(); }
}
 
void DataPool_GetRcChannels(LinkRcChannels_t *out)
{
    if (!out) return;
    if (LOCK_R()) { *out = s_pool.rc; UNLOCK(); }
}
 
void DataPool_GetStats(LinkStats_t *out)
{
    if (!out) return;
    if (LOCK_R()) { *out = s_pool.stats; UNLOCK(); }
}

uint8_t DataPool_GetBatteryWarnLevel(void)
{
    uint16_t mv = 0;
    if (LOCK_R()) { mv = s_pool.battery.voltage_mv; UNLOCK(); }
    if (mv == 0)               return 0;
    if (mv <= BATT_DANGER_MV)  return 2;
    if (mv <= BATT_WARN_MV)    return 1;
    return 0;
}

void DataPool_ParseAndSetStatus(uint8_t base_mode, uint32_t custom_mode)
{
    LinkStatus_t st;
    memset(&st, 0, sizeof(st));
 
    // MAV_MODE_FLAG_SAFETY_ARMED = 0x80
    st.is_armed = (base_mode & 0x80u) ? 1u : 0u;
 
    if (custom_mode < MODE_TABLE_SIZE) {
        strncpy(st.mode_str, s_mode_table[custom_mode], sizeof(st.mode_str) - 1);
    } else {
        // 未知模式，显示原始数字
        snprintf(st.mode_str, sizeof(st.mode_str), "Mode%lu", custom_mode);
    }
 
    DataPool_SetStatus(&st);
}
#ifndef __DATA_POOL_H
#define __DATA_POOL_H

#include "stdint.h"

// 姿态
typedef struct {
    float roll;                 // 滚转角 (度)
    float pitch;                // 俯仰角 (度)
    float yaw;                  // 偏航角 (度)
} LinkAttitude_t;

// GPS
typedef struct {
    int32_t lat;                 // 纬度 (度 * 1E7)
    int32_t lon;                 // 经度 (度 * 1E7)
    float   alt_rel;             // 相对起飞点高度
    uint8_t fix_type;            // 定位类型: 0=无 2=2D 3=3D 4=DGPS 6=RTK
    uint8_t satellites_visible;  // 可见卫星数
    float   hdop;                // 水平精度因子 (越小越好，<1.0优秀)
} LinkGps_t;

// VFR 飞行仪表
typedef struct {
    float    airspeed;      // 空速 (m/s)
    float    groundspeed;   // 地速 (m/s)
    float    climb_rate;    // 爬升率 (m/s, 正=爬升 负=下降)
    float    throttle_pct;  // 油门百分比 (0~100)
    float    alt;           // 气压融合高度 (米，相对起飞点)
    uint16_t heading;       // 航向 (度, 0~359)
} LinkVfrHud_t;

// 电池
typedef struct {
    uint16_t voltage_mv;        // 电池电压 (毫伏)
    int16_t  current_ca;        // 电池电流 (厘安, -1=不支持)
    int8_t   remaining_pct;     // 飞控估算剩余电量 (-1=不支持)
} LinkBattery_t;

// 飞行状态
typedef struct {
    char    mode_str[16];   // 飞行模式可读字符串，如 "Loiter"
    uint8_t is_armed;       // 解锁状态: 1=ARMED 0=DISARMED
} LinkStatus_t;

// 遥控通道
typedef struct {
    uint16_t chan[8];   // 8通道 PWM 值 (微秒, 1000~2000, 0=无效)
    uint8_t  rssi;      // 信号强度 (0~254, 255=不支持)
} LinkRcChannels_t;

// 诊断统计
typedef struct {
    uint32_t total_packets;
    uint32_t heartbeat_count;
    uint32_t attitude_count;
    uint32_t gps_count;
    uint32_t vfr_hud_count;
    uint32_t battery_count;
    uint32_t rc_count;
} LinkStats_t;

// 全局数据池
typedef struct {
    LinkAttitude_t   attitude;
    LinkGps_t        gps;
    LinkVfrHud_t     vfr_hud;
    LinkBattery_t    battery;
    LinkStatus_t     status;
    LinkRcChannels_t rc;
    LinkStats_t      stats;
} FlightData_t;

// --- API 接口声明 ---
void DataPool_Init(void);

// 写入接口
void DataPool_SetAttitude(const LinkAttitude_t *att);
void DataPool_SetGps(const LinkGps_t *gps);
void DataPool_SetVfrHud(const LinkVfrHud_t *hud);
void DataPool_SetBattery(const LinkBattery_t *bat);
void DataPool_SetStatus(const LinkStatus_t *status);
void DataPool_SetRcChannels(const LinkRcChannels_t *rc);

// 统计计数
void DataPool_IncStats_ByMsgId(uint32_t msgid);

// 读取接口
void DataPool_GetAttitude(LinkAttitude_t *out);
void DataPool_GetGps(LinkGps_t *out);
void DataPool_GetVfrHud(LinkVfrHud_t *out);
void DataPool_GetBattery(LinkBattery_t *out);
void DataPool_GetStatus(LinkStatus_t *out);
void DataPool_GetRcChannels(LinkRcChannels_t *out);
void DataPool_GetStats(LinkStats_t *out);

// 电池警告等级 (返回: 0=正常  1=警告(橙色)  2=危险(红色闪烁))
uint8_t DataPool_GetBatteryWarnLevel(void);
void DataPool_ParseAndSetStatus(uint8_t base_mode, uint32_t custom_mode);

#endif /* __DATA_POOL_H */
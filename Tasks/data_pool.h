#ifndef __DATA_POOL_H
#define __DATA_POOL_H

#include "stdint.h"

// 姿态子结构体
typedef struct {
    float roll;                 // 滚转角 (度)
    float pitch;                // 俯仰角 (度)
    float yaw;                  // 偏航角 (度)
} LinkAttitude_t;

// GPS与位置子结构体
typedef struct {
    int32_t lat;                // 纬度 (角度 * 1E7, MAVLink标准)
    int32_t lon;                // 经度 (角度 * 1E7, MAVLink标准)
    float alt;                  // 高度 (米)
    uint8_t satellites_visible; // 可见星数
} LinkGps_t;

// 电池与系统状态子结构体
typedef struct {
    uint16_t voltage_battery;   // 电池电压 (毫伏 mV)
    int16_t  current_battery;   // 电池电流 (厘安 cA)
    int8_t   battery_remaining; // 剩余电量百分比 (0-100)
} LinkSysStatus_t;

// 地面站本地通信诊断统计
typedef struct {
    uint32_t total_packets;     // 成功接收的 MAVLink 总包数
    uint32_t heartbeat_count;   // 接收到的心跳包总数
    uint32_t attitude_count;    // 接收到的姿态包总数
    uint32_t gps_count;         // 接收到的 GPS 包总数
} LinkStats_t;

// 地面站全局数据池总结构体
typedef struct {
    LinkAttitude_t  attitude;   // 姿态板块
    LinkGps_t       gps;        // GPS板块
    LinkSysStatus_t status;     // 状态板块
    LinkStats_t     stats;      // 诊断统计板块
} FlightData_t;

extern FlightData_t g_flight_data;

#endif /* __DATA_POOL_H */
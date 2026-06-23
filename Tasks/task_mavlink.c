#include "telemetry_uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "mavlink.h"
#include "data_pool.h"

extern QueueHandle_t g_mavlink_queue;

void Telemetry_Rx_Callback(uint8_t* pData, uint16_t len)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    for (uint16_t i = 0; i < len; i++)
    {
        xQueueSendFromISR(g_mavlink_queue, &pData[i], &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void handle_message(const mavlink_message_t *msg)
{
    DataPool_IncStats_ByMsgId(msg->msgid);
 
    switch (msg->msgid)
    {
        // HEARTBEAT：飞行模式 + 解锁状态
        case MAVLINK_MSG_ID_HEARTBEAT:
        {
            mavlink_heartbeat_t hb;
            mavlink_msg_heartbeat_decode(msg, &hb);
            if (hb.type == 6) break;
            DataPool_ParseAndSetStatus(hb.base_mode, hb.custom_mode);
            break;
        }
 
        // ATTITUDE：欧拉角，弧度转度
        case MAVLINK_MSG_ID_ATTITUDE:
        {
            mavlink_attitude_t att;
            mavlink_msg_attitude_decode(msg, &att);
            LinkAttitude_t local = {
                .roll  = att.roll  * 57.29578f,
                .pitch = att.pitch * 57.29578f,
                .yaw   = att.yaw   * 57.29578f,
            };
            DataPool_SetAttitude(&local);
            break;
        }
 
        // GLOBAL_POSITION_INT：相对高度 + 经纬度
        case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
        {
            mavlink_global_position_int_t pos;
            mavlink_msg_global_position_int_decode(msg, &pos);
 
            // 先读出当前 GPS 数据，保留 fix_type 和卫星数不变
            LinkGps_t local;
            DataPool_GetGps(&local);
            local.lat     = pos.lat;
            local.lon     = pos.lon;
            local.alt_rel = pos.relative_alt / 1000.0f;  // 毫米转米
            DataPool_SetGps(&local);
            break;
        }
 
        // GPS_RAW_INT：定位质量 + 卫星数
        case MAVLINK_MSG_ID_GPS_RAW_INT:
        {
            mavlink_gps_raw_int_t gps;
            mavlink_msg_gps_raw_int_decode(msg, &gps);
 
            LinkGps_t local;
            DataPool_GetGps(&local);
            local.fix_type          = gps.fix_type;
            local.satellites_visible = gps.satellites_visible;
            // eph 单位是 cm*100，转成常见的 HDOP 值（除以 100）
            local.hdop = gps.eph / 100.0f;
            DataPool_SetGps(&local);
            break;
        }
 
        // VFR_HUD：空速/地速/航向/油门/高度/爬升率
        case MAVLINK_MSG_ID_VFR_HUD:
        {
            mavlink_vfr_hud_t hud;
            mavlink_msg_vfr_hud_decode(msg, &hud);
            LinkVfrHud_t local = {
                .airspeed    = hud.airspeed,
                .groundspeed = hud.groundspeed,
                .climb_rate  = hud.climb,
                .throttle_pct = (float)hud.throttle,
                .alt         = hud.alt,
                .heading     = (uint16_t)hud.heading,
            };
            DataPool_SetVfrHud(&local);
            break;
        }
 
        // SYS_STATUS：电池电压 + 电流 + 飞控估算剩余电量
        case MAVLINK_MSG_ID_SYS_STATUS:
        {
            mavlink_sys_status_t sys;
            mavlink_msg_sys_status_decode(msg, &sys);
            LinkBattery_t local = {
                .voltage_mv    = sys.voltage_battery,
                .current_ca    = sys.current_battery,
                .remaining_pct = sys.battery_remaining,
            };
            DataPool_SetBattery(&local);
            break;
        }
 
        // RC_CHANNELS：遥控通道 PWM + RSSI
        case MAVLINK_MSG_ID_RC_CHANNELS:
        {
            mavlink_rc_channels_t rc;
            mavlink_msg_rc_channels_decode(msg, &rc);
            LinkRcChannels_t local;
            local.chan[0] = rc.chan1_raw;
            local.chan[1] = rc.chan2_raw;
            local.chan[2] = rc.chan3_raw;
            local.chan[3] = rc.chan4_raw;
            local.chan[4] = rc.chan5_raw;
            local.chan[5] = rc.chan6_raw;
            local.chan[6] = rc.chan7_raw;
            local.chan[7] = rc.chan8_raw;
            local.rssi    = rc.rssi;
            DataPool_SetRcChannels(&local);
            break;
        }
 
        default:
            break;
    }
}
 
// MAVLink 解析任务
void vTask_MAVLink_Parser(void *pvParameters)
{
    (void)pvParameters;
    uint8_t           byte;
    mavlink_message_t msg;
    mavlink_status_t  status;
 
    while (1)
    {
        if (xQueueReceive(g_mavlink_queue, &byte, portMAX_DELAY) == pdTRUE)
        {
            if (mavlink_parse_char(MAVLINK_COMM_0, byte, &msg, &status))
            {
                handle_message(&msg);
                taskYIELD();
            }
        }
    }
}


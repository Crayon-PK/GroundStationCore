#include "telemetry_uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "mavlink.h"
#include "data_pool.h"

extern QueueHandle_t g_mavlink_queue;

FlightData_t g_flight_data = {0};

void Telemetry_Rx_Callback(uint8_t* pData, uint16_t len)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    for (uint16_t i = 0; i < len; i++)
    {
        xQueueSendFromISR(g_mavlink_queue, &pData[i], &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void vTask_MAVLink_Parser(void *pvParameters)
{
    uint8_t byte;
    mavlink_message_t msg;
    mavlink_status_t status;
    
    while (1)
    {
        if (xQueueReceive(g_mavlink_queue, &byte, portMAX_DELAY) == pdTRUE)
        {
            if (mavlink_parse_char(MAVLINK_COMM_0, byte, &msg, &status))
            {
                g_flight_data.stats.total_packets++;

                switch (msg.msgid)
                {
                    case MAVLINK_MSG_ID_HEARTBEAT:
                        g_flight_data.stats.heartbeat_count++;
                        break;
                        
                    case MAVLINK_MSG_ID_ATTITUDE:
                    {
                        g_flight_data.stats.attitude_count++;
                        mavlink_attitude_t att;
                        mavlink_msg_attitude_decode(&msg, &att);
                        g_flight_data.attitude.roll  = att.roll * 57.29578f;
                        g_flight_data.attitude.pitch = att.pitch * 57.29578f;
                        g_flight_data.attitude.yaw   = att.yaw * 57.29578f;
                        break;
                    }
                    
                    case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
                    {
                        g_flight_data.stats.gps_count++;
                        mavlink_global_position_int_t pos;
                        mavlink_msg_global_position_int_decode(&msg, &pos);
                        g_flight_data.gps.lat = pos.lat; // 角度 * 1E7
                        g_flight_data.gps.lon = pos.lon; // 角度 * 1E7
                        g_flight_data.gps.alt = pos.alt / 1000.0f; // 毫米转米
                        break;
                    }
                    
                    default:
                        break;
                }
            }
        }
    }
}


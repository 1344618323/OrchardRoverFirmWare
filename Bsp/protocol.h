#ifndef __PROTOCOL__
#define __PROTOCOL__

#include "crc_transmit.h"
#include "orchard_rover_sys.h"
#include "bsp_uart.h"

/*************传输协议*************/
//  帧头(0xA5) -> pack_len -> data_id -> data_content -> CRC32(总数据包的校验)
//  底盘信息 0x01
//  底盘命令 0x03

#define SOF 0xA5
#define HEADER_LEN sizeof(FrameHeader)
#define CMD_LEN 1
#define CRC_DATA_LEN 4
#define TX_MAX_SIZE_PC 50
#define RX_MAX_SIZE_PC 50

#define CMD_PUSH_CHASSIS_INFO (0X01u)
#define CMD_SET_CHASSIS_SPEED (0X03u)
#define CMD_SET_CAM_ANGLE (0x05u)
#define CMD_PUSH_OBSERVE_INFO (0X07u)

typedef enum
{
        STEP_HEADER_SOF = 0,
        STEP_LENGTH_LOW = 1,
        STEP_LENGTH_HIGH = 2,
        STEP_DATA_CRC16 = 3,
} UnpackStep;

#pragma pack(push, 1)
// 对齐系数为1

typedef struct
{
        uint8_t sof;
        uint16_t pack_len;
} FrameHeader;

typedef struct
{
        int16_t gyro_angle;
        int32_t position_x_mm;
        int32_t position_y_mm;
        // int16_t gyro_rate;
        // int16_t angle_deg;
        // int16_t v_x_mm;
        // int16_t v_y_mm;
} cmd_chassis_info;

typedef struct
{
        int16_t vx;
        int16_t vy;
        int16_t vw;
} cmd_chassis_speed;

typedef struct
{
        uint8_t valid[2];  //下位机到转角命令后是否测量，1不测量，2测量，0无效数据包
        int16_t angle[2];  //上位机控制转角
        uint8_t exec_flag; //只有下位机有这个标志位
} cmd_cam_angle;

typedef struct
{
        uint8_t valid[2];   //0无效包，1返回角度包，2返回测距包
        int16_t bearing[2]; //当前转角
        int16_t range[2];   //当前测量距离
} cmd_observe_info;

#pragma pack(pop)

int16_t ProtocolPack(uint8_t *data, uint16_t len, uint8_t cmd_id, uint8_t *pack);
void ProtocolFillPack(uint8_t *topack_data,
                      uint8_t *packed_data,
                      uint16_t len,
                      uint8_t cmd_id);
void ReceiveHandle(uint8_t *rx_buf, uint16_t read_len);

void DataHandle(uint8_t *protocol_packet);

extern cmd_cam_angle cam_angle;

#endif

#include "protocol.h"

cmd_chassis_info test_info;
cmd_cam_angle cam_angle;

/*************************** 对数据打包 ****************************/

int16_t ProtocolPack(uint8_t *data, uint16_t len, uint8_t cmd_id, uint8_t *pack)
{
    memset(pack, 0, TX_MAX_SIZE_PC);
    int16_t pack_length = HEADER_LEN + CMD_LEN + len + CRC_DATA_LEN;
    if (pack_length > TX_MAX_SIZE_PC)
        return -1;
    ProtocolFillPack(data, pack, len, cmd_id);
    return pack_length;
}

void ProtocolFillPack(uint8_t *topack_data,
                      uint8_t *packed_data,
                      uint16_t len,
                      uint8_t cmd_id)
{
    FrameHeader *p_header = (FrameHeader *)packed_data;
    uint16_t pack_length = HEADER_LEN + CMD_LEN + len + CRC_DATA_LEN;

    p_header->sof = SOF;
    p_header->pack_len = pack_length;

    memcpy(packed_data + HEADER_LEN, (uint8_t *)&cmd_id, CMD_LEN);
    memcpy(packed_data + HEADER_LEN + CMD_LEN, topack_data, len);

    uint32_t crc_data = CRC32Calc(packed_data, pack_length - CRC_DATA_LEN);
    memcpy(packed_data + len + HEADER_LEN + CMD_LEN, &crc_data, CRC_DATA_LEN);
}

/*************************** Receive DATA ****************************/
uint8_t protocol_packet[RX_MAX_SIZE_PC];
void ReceiveHandle(uint8_t *rx_buf, uint16_t read_len)
{
    uint16_t read_buff_index = 0;
    UnpackStep unpack_step_e = STEP_HEADER_SOF;
    uint16_t index = 0;
    FrameHeader header;
    memset(protocol_packet, 0, RX_MAX_SIZE_PC);
    if (read_len > 0)
    {
        while (read_len--)
        {
            uint8_t read_byte = rx_buf[read_buff_index++];
            switch (unpack_step_e)
            {
            case STEP_HEADER_SOF:
            {
                if (read_byte == SOF)
                {
                    protocol_packet[index++] = read_byte;
                    unpack_step_e = STEP_LENGTH_LOW;
                }
                else
                {
                    index = 0;
                }
            }
            break;
            case STEP_LENGTH_LOW:
            {
                header.pack_len = read_byte;
                protocol_packet[index++] = read_byte;
                unpack_step_e = STEP_LENGTH_HIGH;
            }
            break;
            case STEP_LENGTH_HIGH:
            {
                header.pack_len |= (read_byte << 8);
                protocol_packet[index++] = read_byte;
                if (header.pack_len < (RX_MAX_SIZE_PC))
                {
                    unpack_step_e = STEP_DATA_CRC16;
                }
                else
                {
                    unpack_step_e = STEP_HEADER_SOF;
                    index = 0;
                }
            }
            break;

            case STEP_DATA_CRC16:
            {
                if (index < header.pack_len)
                {
                    protocol_packet[index++] = read_byte;
                }
                else if (index > header.pack_len)
                {
                    unpack_step_e = STEP_HEADER_SOF;
                    index = 0;
                }
                if (index == header.pack_len)
                {
                    unpack_step_e = STEP_HEADER_SOF;
                    index = 0;
                    if (CRCTailCheck(protocol_packet, header.pack_len))
                    {
                        DataHandle(protocol_packet);
                    }
                }
            }
            break;
            default:
            {
                unpack_step_e = STEP_HEADER_SOF;
                index = 0;
            }
            break;
            }
        }
    }
}

void DataHandle(uint8_t *protocol_packet)
{
    FrameHeader *p_header = (FrameHeader *)protocol_packet;
    uint16_t data_length = p_header->pack_len - HEADER_LEN - CMD_LEN - CRC_DATA_LEN;
    uint8_t cmd_id = *(uint16_t *)(protocol_packet + HEADER_LEN);
    uint8_t *data_addr = protocol_packet + HEADER_LEN + CMD_LEN;

    switch (cmd_id)
    {
    case CMD_PUSH_CHASSIS_INFO:
    {
        test_info.gyro_angle = *(uint16_t *)(data_addr);
        test_info.position_x_mm = *(uint32_t *)(data_addr + 2);
        test_info.position_y_mm = *(uint32_t *)(data_addr + 6);
    }
    break;
    case CMD_SET_CHASSIS_SPEED:
    {
    }
    break;
    case CMD_SET_CAM_ANGLE:
    {
        cam_angle.valid[0] = *(uint8_t *)(data_addr);
        cam_angle.valid[1] = *(uint8_t *)(data_addr + 1);
        cam_angle.angle[0] = *(int16_t *)(data_addr + 2);
        cam_angle.angle[1] = *(int16_t *)(data_addr + 4);
        cam_angle.exec_flag = 1;
    }
    break;
    default:
        break;
    }
}

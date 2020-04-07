#ifndef __CRC_TRANSMIT__
#define __CRC_TRANSMIT__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>


uint16_t CRC16Update(uint16_t crc, uint8_t ch);

uint32_t CRC32Update(uint32_t crc, uint8_t ch);

uint16_t CRC16Calc(const uint8_t *data_ptr, uint32_t length);

uint32_t CRC32Calc(const uint8_t *data_ptr, uint32_t length);

bool CRCHeadCheck(uint8_t *data_ptr, uint32_t length);

bool CRCTailCheck(uint8_t *data_ptr, uint32_t length);

#endif

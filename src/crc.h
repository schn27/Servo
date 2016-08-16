#ifndef CRC_H
#define CRC_H

#include <stdint.h>


#define USE_CRC8		0
#define USE_CRC8_TABLE	1
#define USE_CRC16		0
#define USE_CHECKSUM	0


#if (USE_CRC8 == 1) || (USE_CRC8_TABLE == 1)
uint8_t crc8(uint8_t *pcBlock, int len);
#endif

#if USE_CRC16 == 1
uint16_t crc16(uint8_t *pcBlock, int len);
#endif

#if USE_CHECKSUM == 1
uint8_t checkSum(uint8_t *pcBlock, int len);
#endif


#endif

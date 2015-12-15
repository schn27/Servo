// rs485.h

#ifndef RS485_H
#define RS485_H

#include <stdint.h>

void rs485_init(void);
uint8_t rs485_put(const uint8_t *buf, uint8_t bufsize);
uint8_t rs485_get(uint8_t *buf, uint8_t bufsize);

#endif

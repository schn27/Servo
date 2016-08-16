// rs485.h

#ifndef RS485_H
#define RS485_H

#include <stdint.h>

void rs485_init(void);
char rs485_put(uint8_t value);
char rs485_get(uint8_t *value);

#endif

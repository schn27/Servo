#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

void motor_init(void);
void motor_set(int16_t value, uint16_t uin);
void motor_enable(uint8_t enable);

#endif

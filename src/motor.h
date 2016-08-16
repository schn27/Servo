// motor.h

#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

void Motor_Init(void);
void Motor_Set(int16_t value, uint16_t uin);
void Motor_Enable(uint8_t enable);

#endif

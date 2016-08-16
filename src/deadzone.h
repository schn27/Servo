#ifndef DEADZONE_H
#define DEADZONE_H

#include <stdint.h>

void MoveDeadZone(int16_t *center, int16_t value, uint16_t deadZone);
int16_t GetValueFromDeadZone(int16_t center, int16_t refvalue, uint16_t deadZone);

#endif

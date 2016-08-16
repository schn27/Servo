// convvalues.h

#ifndef CONVVALUES_H
#define CONVVALUES_H

#include <stdint.h>

int16_t conv_position_to_abs(int16_t value);
int16_t conv_position_from_abs(int16_t value);
uint16_t conv_current_to_mA(uint16_t value);
uint16_t conv_current_from_mA(uint16_t value_mA);
uint16_t conv_voltage_to_mV(uint16_t value);
uint16_t conv_speed_from_deg(uint16_t value_deg);

#endif


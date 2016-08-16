#ifndef LOWPASSFILTER_H
#define LOWPASSFILTER_H

#include <stdint.h>

void LowPassFilter(int16_t *cur_value, int16_t value, int16_t alpha);

#endif

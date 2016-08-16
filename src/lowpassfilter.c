#include "lowpassfilter.h"

void lowPassFilter(int16_t *cur_value, int16_t value, int16_t alpha) {
	*cur_value = ((int32_t)value * alpha + (int32_t)*cur_value * (256 - alpha)) >> 8;
}

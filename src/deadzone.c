#include "deadzone.h"


static void getMinMax(int16_t center, uint16_t deadZone, int16_t *min, int16_t *max) {
	int32_t min_ = center - (int32_t)deadZone;
	int32_t max_ = center + (int32_t)deadZone;

	if (min_ < -32767) {
		min_ = -32767;
	}

	if (max_ > 32767) {
		max_ = 32767;
	}

	*min = (int16_t)min_;
	*max = (int16_t)max_;
}


void deadZone_move(int16_t *center, int16_t value, uint16_t deadZone) {
	int16_t min = 0;
	int16_t max = 0;

	getMinMax(*center, deadZone, &min, &max);

	if (value > (int16_t)max) {
		*center = value - deadZone;
	}

	if (value < (int16_t)min) {
		*center = value + deadZone;
	}
}


int16_t deadZone_getValue(int16_t center, int16_t refvalue, uint16_t deadZone) {
	int16_t min = 0;
	int16_t max = 0;

	getMinMax(center, deadZone, &min, &max);

	if (refvalue < min) {
		return min;
	}

	if (refvalue > max) {
		return max;
	}

	return refvalue;
}



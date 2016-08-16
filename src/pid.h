#ifndef PID_H
#define PID_H

#include <stdint.h>

typedef struct {
	uint8_t fixedPointPos;
	int16_t kP;
	int16_t kI;
	int16_t kD;
	int16_t outMin;
	int16_t outMax;

	int16_t ref;
	int16_t fdb;
	int16_t fdb1;
	int16_t out;

	int32_t integr;
} pidreg_t;

void pid_init(pidreg_t *pid);
void pid_calc(pidreg_t *pid);


#endif

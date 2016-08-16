#ifndef PID_H
#define PID_H

#include <stdint.h>

typedef struct tag_PID
{
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
}
PID;

//#define PIDX PID xdata

void PID_Init(PID *pid);
void PID_Calc(PID *pid);


#endif

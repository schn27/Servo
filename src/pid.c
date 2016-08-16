#include "common.h"
#include "pid.h"

// инициализация ПИД-регулятора
void PID_Init(PID *pid)
{
	pid->ref = 0;
	pid->fdb = 0;
	pid->integr = 0;
	pid->fdb1 = 0;
}


// вычисление управляющего воздействия
void PID_Calc(PID *pid)
{
	int32_t error;
	int32_t prop;
	int32_t direv;
	int32_t outPreSat;
	int32_t tmp;

	// Compute the error
	error = (int32_t)pid->ref - (int32_t)pid->fdb;

	// Compute the proportional output
	prop = pid->kP * error;

	// Compute the integral output
	pid->integr += pid->kI * error;

	// Integrator anti windup
	tmp = pid->integr >> pid->fixedPointPos;

	if (tmp > pid->outMax)
		pid->integr = (int32_t)pid->outMax << pid->fixedPointPos;
	if (tmp < pid->outMin)
		pid->integr = (int32_t)pid->outMin << pid->fixedPointPos;

	// Compute the derivative output
	direv = pid->kD * ((int32_t)pid->fdb - (int32_t)pid->fdb1);
	pid->fdb1 = pid->fdb;

	// Compute the presaturated output
	outPreSat = prop + pid->integr + direv;
	outPreSat >>= pid->fixedPointPos;

	// Saturate the output
	if (outPreSat > pid->outMax)
		pid->out = pid->outMax;
	else if (outPreSat < pid->outMin)
		pid->out = pid->outMin;
	else
		pid->out = (int16_t)outPreSat;
}



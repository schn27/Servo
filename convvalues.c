// convvalues.c

#include <convvalues.h>
#include <config.h>

int16_t conv_position_to_abs(int16_t value)
{
	int32_t data x = config.reversed ? -value : value;

	int32_t data k = 
		(x < 0) ? 
		((int32_t)config.centerOfs - config.endPoint1) :
		((int32_t)config.endPoint2 - config.centerOfs);

	return (int16_t)((int32_t)config.centerOfs + ((x * k) >> 15));
}

int16_t conv_position_from_abs(int16_t value)
{
	int32_t data x = (int32_t)value - (int32_t)config.centerOfs;

	if (x > 0)
		x = (x * 32767) / ((int32_t)config.endPoint2 - (int32_t)config.centerOfs);
	else
		x = (x * 32767) / ((int32_t)config.centerOfs - (int32_t)config.endPoint1);

	if (x > 32767)
		x = 32767;
	else if (x < -32767)
		x = -32767;

	return (int16_t)(config.reversed ? -x : x);
}

// преобразование тока из попугаев в милиамперы
uint16_t conv_current_to_mA(uint16_t value)
{
	uint32_t t = ((uint32_t)value * 3928UL) / 4095UL;
	return (t < 0x10000UL) ? (uint16_t)t : 0xFFFF;
}

// преобразование тока из миллиампер в попугаи
uint16_t conv_current_from_mA(uint16_t value_mA)
{
	uint32_t t = ((uint32_t)value_mA * 4095UL) / 3928UL;
	return (t < 0x10000UL) ? (uint16_t)t : 0xFFFF;
}


// преобразование напряжения из попугаев в миливольты
uint16_t conv_voltage_to_mV(uint16_t value)
{
	uint32_t t = ((uint32_t)value * 23739UL) / 4095UL;
	return (t < 0x10000UL) ? (uint16_t)t : 0xFFFF;
}


#if 0
// преобразование скорости из градусов/с в попугаи
uint16_t conv_speed_from_deg(uint16_t value_deg)
{
	uint32_t t = ((uint32_t)value_deg * 6898UL) / 1000UL;
	return (t < 0x10000UL) ? (uint16_t)t : 0xFFFF;
}
#endif



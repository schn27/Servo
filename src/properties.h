#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <stdint.h>

typedef struct tag_properties_t
{
	int16_t original_command;
	int16_t current_command;
	int16_t command;
	int16_t original_position;
	int16_t position_ndz;
	int16_t position;
	int16_t speed;
	uint16_t uin;
	uint16_t iout;
}
properties_t;

extern properties_t properties;

#endif

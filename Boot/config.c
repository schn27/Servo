// config.c

#include <common.h>
#include <config.h>
#include <F410_FlashPrimitives.h>

// адрес rs-485
static uint8_t addr = 0;

void Config_Init(void)
{
	addr = FLASH_ByteRead(0x7a00);
}

uint8_t Config_GetAddr(void)
{
	return addr;
}
#include "common.h"
#include "config.h"
#include "F410_FlashPrimitives.h"
#include "crc.h"


#define FIXED8P8(v) ((v) * 256)

#define CONFIG_MAXPOS	32600


// настройки во Flash
static CONFIG code configStored _at_ 0x7a00;

// копия настроек
CONFIG data config;

static uint8_t modified_timer = 0;


void SetDefault(void);
void CheckBounds(void);
char ApplyBounds(int16_t *value, int16_t low_bound, int16_t high_bound);


// копирование настроек из Flash
void Config_Init(void)
{
	uint16_t i = sizeof(config);
	uint8_t *ptr = (uint8_t *)&config;
	uint16_t addr = (uint16_t)&configStored;

	while (i--)
	{
		*ptr++ = FLASH_ByteRead(addr++);
	}

	if (Crc8((uint8_t *)&config, sizeof(config) - 1) == config.crc)
		CheckBounds();
	else
		SetDefault();
}


// сохранение настроек во Flash и рестарт
void Config_Store(void)
{
	uint16_t i = sizeof(CONFIG);
	uint8_t *ptr = (uint8_t *)&config;
	uint16_t addr = (uint16_t)&configStored;

	WDT_DISABLE();
	DRIVER_DISABLE();

	EA = 0;			// прерывания запрещены

	config.crc = Crc8((uint8_t *)&config, sizeof(config) - 1);	// update crc

	FLASH_PageErase(addr);

	while (i--)
	{
		FLASH_ByteWrite(addr++, *ptr++);
	}	

	RESET();
}


// проверка таймера сохранения настроек после модификации
void Config_CheckModified(void)
{
	if (modified_timer)
	{
		if (--modified_timer == 0)
			Config_Store();
	}
}


// запуск таймера сохранения настроек после модификации
void Config_SetModified(void)
{
	modified_timer = 100;
}


// применение параметров режима ручной настройки
void Config_ApplyManual(void)
{
	config.reversed = config.endPoint1 > config.endPoint2;

	if (config.reversed)
	{
		int16_t t = config.endPoint1;
		config.endPoint1 = config.endPoint2;
		config.endPoint2 = t;
	}

	Config_SetModified();
}


static void SetDefault(void)
{
	config.addr = 255;
	config.addr_alias = 0;

	config.calibrated = 0;

	config.posMul = 0x4000;		// == +1
	config.posOfs = 0;
	config.motorReversed = 0;

	config.reversed = 0;
	config.speed = 2200;
	config.endPoint1 = -CONFIG_MAXPOS;
	config.endPoint2 = CONFIG_MAXPOS;
	config.centerOfs = 0;
	config.failSafeMode = 2;
	config.failSafePos = 0;
	config.deadZone = 50;
	config.ioutMax = 1500;

	config.kP1 = FIXED8P8(1.75);
	config.kI1 = FIXED8P8(0.0);
	config.kD1 = FIXED8P8(-5.0);
	config.kP2 = FIXED8P8(9.0);
	config.kI2 = FIXED8P8(0.44);
	config.kD2 = FIXED8P8(-20.0);

	Config_Store();
}


static void CheckBounds(void)
{
	char changed = 
		ApplyBounds(&config.centerOfs, -CONFIG_MAXPOS, CONFIG_MAXPOS) +
		ApplyBounds(&config.endPoint1, -CONFIG_MAXPOS, config.centerOfs) +
		ApplyBounds(&config.endPoint2, config.centerOfs, CONFIG_MAXPOS) +
		ApplyBounds(&config.failSafePos, config.endPoint1, config.endPoint2);

	if (changed)
		Config_Store();
}


static char ApplyBounds(int16_t *value, int16_t low_bound, int16_t high_bound)
{
	char changed = 0;

	if (*value < low_bound)
	{
		*value = low_bound;
		changed = 1;
	}

	if (*value > high_bound)
	{
		*value = high_bound;
		changed = 1;
	}

	return changed;
}
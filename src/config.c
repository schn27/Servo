#include "common.h"
#include "config.h"
#include "F410_FlashPrimitives.h"
#include "crc.h"


#define FIXED8P8(v) ((v) * 256)

#define CONFIG_MAXPOS	32600


// настройки во Flash
static config_t code configStored _at_ 0x7a00;

// копия настроек
config_t data config;

static uint8_t modified_timer = 0;


void setDefault(void);
void checkBounds(void);
char applyBounds(int16_t *value, int16_t low_bound, int16_t high_bound);


// копирование настроек из Flash
void config_init(void) {
	uint16_t i = sizeof(config);
	uint8_t *ptr = (uint8_t *)&config;
	uint16_t addr = (uint16_t)&configStored;

	while (i--) {
		*ptr++ = flash_byteRead(addr++);
	}

	if (crc8((uint8_t *)&config, sizeof(config) - 1) == config.crc) {
		checkBounds();
	} else {
		setDefault();
	}
}


// сохранение настроек во Flash и рестарт
void config_store(void) {
	uint16_t i = sizeof(config_t);
	uint8_t *ptr = (uint8_t *)&config;
	uint16_t addr = (uint16_t)&configStored;

	WDT_DISABLE();
	DRIVER_DISABLE();

	EA = 0;			// прерывания запрещены

	config.crc = crc8((uint8_t *)&config, sizeof(config) - 1);	// update crc

	flash_pageErase(addr);

	while (i--) {
		flash_byteWrite(addr++, *ptr++);
	}	

	RESET();
}


// проверка таймера сохранения настроек после модификации
void config_checkModified(void) {
	if (modified_timer) {
		if (--modified_timer == 0) {
			config_store();
		}
	}
}


// запуск таймера сохранения настроек после модификации
void config_setModified(void) {
	modified_timer = 100;
}


// применение параметров режима ручной настройки
void config_applyManual(void) {
	config.reversed = config.endPoint1 > config.endPoint2;

	if (config.reversed) {
		int16_t t = config.endPoint1;
		config.endPoint1 = config.endPoint2;
		config.endPoint2 = t;
	}

	config_setModified();
}


static void setDefault(void) {
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

	config_store();
}


static void checkBounds(void) {
	char changed = 
		applyBounds(&config.centerOfs, -CONFIG_MAXPOS, CONFIG_MAXPOS) +
		applyBounds(&config.endPoint1, -CONFIG_MAXPOS, config.centerOfs) +
		applyBounds(&config.endPoint2, config.centerOfs, CONFIG_MAXPOS) +
		applyBounds(&config.failSafePos, config.endPoint1, config.endPoint2);

	if (changed) {
		config_store();
	}
}


static char applyBounds(int16_t *value, int16_t low_bound, int16_t high_bound) {
	char changed = 0;

	if (*value < low_bound) {
		*value = low_bound;
		changed = 1;
	}

	if (*value > high_bound) {
		*value = high_bound;
		changed = 1;
	}

	return changed;
}

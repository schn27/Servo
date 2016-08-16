#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

typedef struct {
	uint8_t addr;			// адрес на шине rs485
	uint8_t addr_alias;		// групповой адрес на шине rs485

	uint8_t calibrated;
	int16_t posMul;			// -
	int16_t posOfs;			// - определяются автоматически при калибровке
	uint8_t motorReversed;	// -

	uint8_t reversed;		// реверс позиции
	uint16_t speed;			// скорость перемещения актуатора (в градусах/с)
	int16_t endPoint1;		// ограничение требуемой позиции снизу (в единицах нормированного положения)
	int16_t endPoint2;		// ограничение требуемой позиции сверху (в единицах нормированного положения)
	int16_t centerOfs;		// смещение центральной точки (в единицах нормированного положения)
	uint8_t failSafeMode;	// режим при отсутствии команд: 0 - сохранять позицию, 1 - failSafePos, 2 - свободный ход
	int16_t failSafePos;	// позиция при отсутствии команд (в единицах нормированного положения)
	uint16_t deadZone;		// мёртвая зона (в единицах нормированного положения)
	uint16_t ioutMax;		// ограничение тока (в миллиамперах)

	int16_t kP1;			// Kp для регулятора положения (8 бит дробная часть)
	int16_t kI1;			// Ki для регулятора положения (8 бит дробная часть)
	int16_t kD1;			// Kd для регулятора положения (8 бит дробная часть)
	int16_t kP2;			// Kp для регулятора скорости (8 бит дробная часть)
	int16_t kI2;			// Ki для регулятора скорости (8 бит дробная часть)
	int16_t kD2;			// Kd для регулятора скорости (8 бит дробная часть)

	uint8_t crc;
} config_t;

extern config_t data config;

void config_init(void);
void config_store(void);
void config_checkModified(void);
void config_setModified(void);
void config_applyManual(void);

#endif

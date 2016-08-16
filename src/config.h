#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

typedef struct tag_CONFIG
{
	uint8_t addr;			// ����� �� ���� rs485
	uint8_t addr_alias;		// ��������� ����� �� ���� rs485

	uint8_t calibrated;
	int16_t posMul;			// -
	int16_t posOfs;			// - ������������ ������������� ��� ����������
	uint8_t motorReversed;	// -

	uint8_t reversed;		// ������ �������
	uint16_t speed;			// �������� ����������� ��������� (� ��������/�)
	int16_t endPoint1;		// ����������� ��������� ������� ����� (� �������� �������������� ���������)
	int16_t endPoint2;		// ����������� ��������� ������� ������ (� �������� �������������� ���������)
	int16_t centerOfs;		// �������� ����������� ����� (� �������� �������������� ���������)
	uint8_t failSafeMode;	// ����� ��� ���������� ������: 0 - ��������� �������, 1 - failSafePos, 2 - ��������� ���
	int16_t failSafePos;	// ������� ��� ���������� ������ (� �������� �������������� ���������)
	uint16_t deadZone;		// ������ ���� (� �������� �������������� ���������)
	uint16_t ioutMax;		// ����������� ���� (� ������������)

	int16_t kP1;			// Kp ��� ���������� ��������� (8 ��� ������� �����)
	int16_t kI1;			// Ki ��� ���������� ��������� (8 ��� ������� �����)
	int16_t kD1;			// Kd ��� ���������� ��������� (8 ��� ������� �����)
	int16_t kP2;			// Kp ��� ���������� �������� (8 ��� ������� �����)
	int16_t kI2;			// Ki ��� ���������� �������� (8 ��� ������� �����)
	int16_t kD2;			// Kd ��� ���������� �������� (8 ��� ������� �����)

	uint8_t crc;
} CONFIG;

extern CONFIG data config;

void Config_Init(void);
void Config_Store(void);
void Config_CheckModified(void);
void Config_SetModified(void);
void Config_ApplyManual(void);

#endif

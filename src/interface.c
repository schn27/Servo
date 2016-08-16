#include <string.h>
#include "common.h"
#include "interface.h"
#include "rs485.h"
#include "crc.h"
#include "config.h"
#include "properties.h"
#include "convvalues.h"
#include "arraysize.h"
#include "version.h"


#define FAILSAFE_TIMEOUT	1000

static uint8_t buffer[16];
static uint8_t data bytecnt = 0;
static uint8_t data free_mode = 1;

void checkTimeout(uint8_t reset);
void processPacket(void);
void sendResponse(void);
char isValidAddr(void);
int callHandler(void);

int handler_getver(void);
int handler_getstate(void);
int handler_setpos(void);
int handler_setrs485(void);
int handler_getparam(void);
int handler_setparam(void);
int handler_manualcfg(void);
int handler_loaderrequest(void);


void put_uint16(uint8_t *buf, uint16_t value);
uint16_t get_uint16(uint8_t *buf);


void interface_init(void) {
	rs485_init();
}


void interface_update(void) {
	char data anydata = 0;

	while (rs485_get(buffer + bytecnt)) {
		++bytecnt;
		anydata = 1;

		if (bytecnt < 3) {
			continue;
		}

		if (buffer[2] > ARRAYSIZE(buffer) || buffer[2] < 4) {
			bytecnt = 0;	// invalid size
		} else if (bytecnt >= buffer[2]) {
			WDT_RESET();
			processPacket();
			bytecnt = 0;
		}
	}

	if (!anydata) {
		bytecnt = 0;
	}

	checkTimeout(0);
}


uint8_t interface_isFreeMode(void) {
	return free_mode;
}


static void checkTimeout(uint8_t reset) {
	static uint16_t timeout_cnt = 0;

	if (reset) {
		timeout_cnt = 0;
		free_mode &= 0xFE;
	}

	if (++timeout_cnt < FAILSAFE_TIMEOUT) {
		return;
	}

	timeout_cnt = FAILSAFE_TIMEOUT;

	if (config.failSafeMode == 2) {
		free_mode |= 1;
		properties.original_command = properties.position;
	}

	if (config.failSafeMode == 1) {
		properties.original_command = config.failSafePos;
	}
}


static void processPacket(void) {
	int resp = -1;

	if (!isValidAddr()) {
		return;
	}

	if (crc8(buffer, buffer[2] - 1) != buffer[buffer[2] - 1]) {
		return;		// invalid CRC
	}

	checkTimeout(1);

	if ((resp = callHandler()) < 0) {
		return;		// no response
	}

	buffer[0] = config.addr;
	buffer[1] += 1;
	buffer[2] = resp + 4;
	buffer[buffer[2] - 1] = crc8(buffer, buffer[2] - 1);

	// send response
	sendResponse();
}


static void sendResponse(void) {
	int i = 0;
	while (i < buffer[2]) {
		rs485_put(buffer[i++]);
	}
}


static char isValidAddr(void) {
	return buffer[0] == 0 
		|| buffer[0] == config.addr
		|| buffer[0] == config.addr_alias;
}


static int callHandler(void) {
	switch (buffer[1]) {
	case 0x00: return handler_getver();
	case 0x02: return handler_getstate();
	case 0x10: handler_setpos(); return -1;
	case 0x12: return handler_setpos();
	case 0x20: return handler_setrs485();
	case 0x30: return handler_getparam();
	case 0x32: return handler_setparam();
	case 0x38: return handler_manualcfg();
	case 0xF0: return handler_loaderrequest();
	default: return -1;
	}
}


static int handler_getver(void) {
	const char *ver = DEVICE_STRING;

	uint8_t n = strlen(ver);
	const uint8_t bufsize = ARRAYSIZE(buffer) - 4;

	if (n < bufsize) {
		n = bufsize;
	}

	strncpy(buffer + 3, ver, n);
	
	return n;
}


static int handler_getstate(void) {
	put_uint16(buffer + 3, conv_position_from_abs(properties.position));
	put_uint16(buffer + 5, properties.speed);
	put_uint16(buffer + 7, conv_voltage_to_mV(properties.uin));
	put_uint16(buffer + 9, conv_current_to_mA(properties.iout));
	return 8;
}


static int handler_setpos(void) {
	properties.original_command = conv_position_to_abs(get_uint16(buffer + 3));
	return 0;
}


static int handler_setrs485(void) {
	config.addr = buffer[7];
	config.addr_alias = buffer[8];
	config_setModified();
	return 0;
}


static int handler_getparam(void) {
	int16_t value = 0;

	switch (buffer[3]) {
	case  0: value = config.calibrated; break;
	case  1: value = config.reversed; break;
	case  2: value = config.speed; break;
	case  3: value = config.endPoint1; break;
	case  4: value = config.endPoint2; break;
	case  5: value = config.centerOfs; break;
	case  6: value = config.failSafeMode; break;
	case  7: value = config.failSafePos; break;
	case  8: value = config.deadZone; break;
	case  9: value = config.ioutMax; break;
	case 10: value = config.kP1; break;
	case 11: value = config.kI1; break;
	case 12: value = config.kD1; break;
	case 13: value = config.kP2; break;
	case 14: value = config.kI2; break;
	case 15: value = config.kD2; break;
	default: return -1;
	}

	put_uint16(buffer + 3, value);

	return 2;
}


static int handler_setparam(void) {
	int16_t value = get_uint16(buffer + 4);

	switch (buffer[3]) {
	case  0: config.calibrated = value; break;
	case  1: config.reversed = value; break;
	case  2: config.speed = value; break;
	case  3: config.endPoint1 = value; break;
	case  4: config.endPoint2 = value; break;
	case  5: config.centerOfs = value; break;
	case  6: config.failSafeMode = value; break;
	case  7: config.failSafePos = value; break;
	case  8: config.deadZone = value; break;
	case  9: config.ioutMax = value; break;
	case 10: config.kP1 = value; break;
	case 11: config.kI1 = value; break;
	case 12: config.kD1 = value; break;
	case 13: config.kP2 = value; break;
	case 14: config.kI2 = value; break;
	case 15: config.kD2 = value; break;
	default: return -1;
	}

	config_setModified();

	return 0;
}


static int handler_manualcfg(void) {
	switch (buffer[3]) {
	case 0: 
		free_mode |= 2; 
		if (config.reversed) {
			int16_t t = config.endPoint2;
			config.endPoint2 = config.endPoint1;
			config.endPoint1 = t;
		}
		break;
	case 1: config.endPoint1 = properties.position; break;
	case 2: config.centerOfs = properties.position; break;
	case 3: config.endPoint2 = properties.position; break;
	case 4: config_applyManual(); break;
	}

	return 0;
}


static int handler_loaderrequest(void) {
	DRIVER_DISABLE();
	RESET();
	return -1;
}



static void put_uint16(uint8_t *buf, uint16_t value) {
	buf[0] = value;
	buf[1] = value >> 8;
}

static uint16_t get_uint16(uint8_t *buf) {
	return buf[0] + buf[1] * 256;
}



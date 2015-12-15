// mainloop.c - главный цикл

#include <common.h>
#include <config.h>
#include <arraysize.h>
#include <rs485.h>
#include <crc.h>
#include <F410_FlashPrimitives.h>

#define START_ADDR 0x800
#define KEY_ADDR 0x79FF
#define KEY_VALUE 0x55

static uint8_t buffer[16];

static uint8_t activated = 0;
static uint8_t activate_cnt = 0;
static uint8_t execute_req = 0;

void Execute(void);
char ProcessPacket(void);
int CallHandler(void);
int handler_activate(void);
int handler_erase(void);
int handler_write(void);
int handler_read(void);
int handler_execute(void);

uint16_t get_uint16(uint8_t *buf);


// главный цикл
void MainLoop(void)
{
	rs485_init();

	TMR3CN = 0x20;

	for (;;)
	{
		char res = ProcessPacket();

		if (!activated)
		{
			if (++activate_cnt > 10 || res)
			{
				activate_cnt = 10;
				Execute();		// таймаут либо пришёл пакет с ID != 0xF0
			}
		}

		if (execute_req)
			Execute();			// запрос клиента на передачу управления
	}
}


static void Execute(void)
{
	if (FLASH_ByteRead(KEY_ADDR) == KEY_VALUE)
	{
		void (*f)(void) = (void code *)START_ADDR;
		f();
	}
}


static char ProcessPacket(void)
{
	int resp = -1;

	if (rs485_get(buffer, 4) != 4)
		return 0;		// timeout
		
	if (buffer[2] > ARRAYSIZE(buffer) || buffer[2] < 4)
		return 0;		// invalid size

	if (buffer[2] > 4 && 
		rs485_get(buffer + 4, buffer[2] - 4) != buffer[2] - 4)
		return 0;		// timeout

	if (buffer[0] != Config_GetAddr())
		return 0;		// invalid addr

	if (Crc8(buffer, buffer[2] - 1) != buffer[buffer[2] - 1])
		return 0;		// invalid CRC

	if ((resp = CallHandler()) < 0)
		return 1;		// no response

	// send response
	buffer[0] = Config_GetAddr();
	buffer[1] += 1;
	buffer[2] = resp + 4;
	buffer[buffer[2] - 1] = Crc8(buffer, buffer[2] - 1);
	rs485_put(buffer, buffer[2]);

	return 1;
}


static int CallHandler(void)
{
	switch (buffer[1])
	{
	case 0xF0: return handler_activate();
	case 0xF2: return handler_erase();
	case 0xF4: return handler_write();
	case 0xF6: return handler_read();
	case 0xF8: return handler_execute();
	default: return -1;
	}
}


int handler_activate(void)
{
	activated = 1;
	return 0;
}


int handler_erase(void)
{
	uint16_t addr = get_uint16(buffer + 3);

	if (!activated)
		return -1;

	if (addr < START_ADDR)
		return -1;	// invalid address

	FLASH_PageErase(addr);
	return 0;
}


int handler_write(void)
{
	uint16_t addr = get_uint16(buffer + 3);
	uint8_t cnt = buffer[2] - 6;
	uint8_t i;

	if (!activated)
		return -1;

	for (i = 0; i < cnt; ++i)
	{
		if (addr >= START_ADDR)
			FLASH_ByteWrite(addr, buffer[5 + i]);

		++addr;
	}

	return 0;
}


int handler_read(void)
{
	uint16_t addr = get_uint16(buffer + 3);
	uint8_t cnt = buffer[5];
	uint8_t i;

	if (!activated)
		return -1;

	for (i = 0; i < cnt; ++i)
	{
		buffer[3 + i] = (addr >= START_ADDR) ? FLASH_ByteRead(addr) : 0;
		++addr;
	}

	return cnt;
}


int handler_execute(void)
{
	if (!activated)
		return -1;

	FLASH_ByteWrite(KEY_ADDR, KEY_VALUE);

	execute_req = 1;
	return 0;
}


static uint16_t get_uint16(uint8_t *buf)
{
	return buf[0] + buf[1] * 256;
}

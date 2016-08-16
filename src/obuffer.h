// obuffer.h

#ifndef OBUFFER_H
#define OBUFFER_H

#include <stdint.h>


#define OBUFFER_MACRO	1


typedef struct tag_OBUFFER
{
	uint8_t *buf;
	uint8_t in;
	uint8_t out;
	uint8_t size;
	uint8_t maxSize;
}
OBUFFER;


// standart variant (inoperable)
#if OBUFFER_MACRO == 0

#error "OBUFFER_MACRO must be defined as 1!"

void obuffer_init_(OBUFFER *obuf, uint8_t *buf, uint8_t maxSize);
uint8_t obuffer_put_(OBUFFER *obuf, uint8_t value);
uint8_t obuffer_get_(OBUFFER *obuf, uint8_t *value);

#define obuffer_init(obuf, buffer, buffersize) obuffer_init_(&obuf, buffer, buffersize)
#define obuffer_put(obuf, value) obuffer_put_(&obuf, value)
#define obuffer_get(obuf, value) obuffer_get_(&obuf, &value)

#endif


// macro variant
#if OBUFFER_MACRO != 0

#define obuffer_init(obuf, buffer, buffersize) \
	(obuf.buf = buffer, obuf.in = obuf.out = obuf.size = 0, obuf.maxSize = buffersize)

#define obuffer_put(obuf, value) \
	((obuf.size < obuf.maxSize) ? \
	(obuf.buf[obuf.in] = value, ++obuf.size, (obuf.in = (++obuf.in >= obuf.maxSize) ? 0 : obuf.in), 1) : \
	(0))

#define obuffer_get(obuf, value) \
	((obuf.size > 0) ? \
	(value = obuf.buf[obuf.out], --obuf.size, (obuf.out = (++obuf.out >= obuf.maxSize) ? 0 : obuf.out), 1) : \
	(0))

#endif

#endif


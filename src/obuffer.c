// obuffer.c

#include <obuffer.h>

#if OBUFFER_MACRO == 0

void obuffer_init_(OBUFFER *obuf, uint8_t *buf, uint8_t maxSize)
{
	obuf->buf = buf;
	obuf->in = 0;
	obuf->out = 0;
	obuf->size = 0;
	obuf->maxSize = maxSize;
}


uint8_t obuffer_put_(OBUFFER *obuf, uint8_t value)
{
	if (obuf->size < obuf->maxSize)
	{
		obuf->buf[obuf->in] = value;

		++obuf->size;

		if (++obuf->in >= obuf->maxSize)
			obuf->in = 0;

		return 1;
	}
	else
		return 0;
}


uint8_t obuffer_get_(OBUFFER *obuf, uint8_t *value)
{
	if (obuf->size > 0)
	{
		*value = obuf->buf[obuf->out];

		--obuf->size;

		if (++obuf->out >= obuf->maxSize)
			obuf->out = 0;

		return 1;
	}
	else
		return 0;
}

#endif


// (C) Aleksandr Dikarev, 2013-2014

#include <ff.h>

void ff_add_accum_f32(float* acc, float* item, unsigned int size)
{
	unsigned int blkCnt = size >> 2u;

	while(blkCnt > 0u)
	{
		*acc++ += (*item++);
		*acc++ += (*item++);
		*acc++ += (*item++);
		*acc++ += (*item++);

		blkCnt--;
	}

	blkCnt = size % 0x4u;

	while(blkCnt > 0u)
	{
	    *acc++ += (*item++);
	    blkCnt--;
	}
}

void ff_fill_f32(float* dst, float value, unsigned int size)
{
	unsigned int blkCnt = size >> 2u;

	while(blkCnt > 0u)
	{
		*dst++ = value;
		*dst++ = value;
		*dst++ = value;
		*dst++ = value;
		blkCnt--;
	}

	blkCnt = size % 0x4u;

	while(blkCnt > 0u)
	{
		*dst++ = value;
		blkCnt--;
	}
}

void ff_fill_u16(unsigned short* dst, unsigned short value, unsigned int size)
{
	unsigned int blkCnt = size >> 2u;
	while (blkCnt > 0u)
	{
		*dst++ = value;
		*dst++ = value;
		*dst++ = value;
		*dst++ = value;

		blkCnt--;
	}

	blkCnt = size % 0x4u;

	while (blkCnt > 0u)
	{
		*dst++ = value;
		blkCnt--;
	}
}

void ff_fill_s16(short* dst, short value, unsigned int size)
{
	unsigned int blkCnt = size >> 2u;
	while (blkCnt > 0u)
	{
		*dst++ = value;
		*dst++ = value;
		*dst++ = value;
		*dst++ = value;

		blkCnt--;
	}

	blkCnt = size % 0x4u;

	while (blkCnt > 0u)
	{
		*dst++ = value;
		blkCnt--;
	}
}

void ff_fill_s32(int* dst, int value, unsigned int size)
{
	unsigned int blkCnt = size >> 2u;
	while (blkCnt > 0u)
	{
		*dst++ = value;
		*dst++ = value;
		*dst++ = value;
		*dst++ = value;

		blkCnt--;
	}

	blkCnt = size % 0x4u;

	while(blkCnt > 0u)
	{
		*dst++ = value;
		blkCnt--;
	}
}

void ff_fill_u32(unsigned int* dst, unsigned int value, unsigned int size)
{
	unsigned int blkCnt = size >> 2u;
	while (blkCnt > 0u)
	{
		*dst++ = value;
		*dst++ = value;
		*dst++ = value;
		*dst++ = value;

		blkCnt--;
	}

	blkCnt = size % 0x4u;

	while(blkCnt > 0u)
	{
		*dst++ = value;
		blkCnt--;
	}
}

void ff_fill_s8(char* dst, char value, unsigned int size)
{
	unsigned int blkCnt = size >> 2u;
	while (blkCnt > 0u)
	{
		*dst++ = value;
		*dst++ = value;
		*dst++ = value;
		*dst++ = value;

		blkCnt--;
	}

	blkCnt = size % 0x4u;

	while(blkCnt > 0u)
	{
		*dst++ = value;
		blkCnt--;
	}
}

void ff_fill_u8(unsigned char* dst, unsigned char value, unsigned int size)
{
	unsigned int blkCnt = size >> 2u;
	while (blkCnt > 0u)
	{
		*dst++ = value;
		*dst++ = value;
		*dst++ = value;
		*dst++ = value;

		blkCnt--;
	}

	blkCnt = size % 0x4u;

	while(blkCnt > 0u)
	{
		*dst++ = value;
		blkCnt--;
	}
}

void ff_copy_u8(unsigned char* src, unsigned char* dst, unsigned int size)
{
	unsigned int blkCnt = size >> 2u;
	while (blkCnt > 0u)
	{
		*dst++ = (*src++);
		*dst++ = (*src++);
		*dst++ = (*src++);
		*dst++ = (*src++);

		blkCnt--;
	}

	blkCnt = size % 0x4u;

	while(blkCnt > 0u)
	{
		*dst++ = (*src++);
		blkCnt--;
	}
}

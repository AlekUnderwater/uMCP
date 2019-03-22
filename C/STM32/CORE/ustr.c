// (C) Aleksandr Dikarev, 2015-2019

#include <ustr.h>
#include <ff.h>

void Str_WriterInit(unsigned char* buffer, unsigned int* srcIdx, unsigned int size)
{
	*srcIdx = 0;
	ff_fill_u8(buffer, 0, size);
}

void Str_WriteByte(unsigned char* buffer,unsigned  int* srcIdx, unsigned char c)
{
	buffer[*srcIdx] = c;
	(*srcIdx)++;
}

void Str_WriteHexByte(unsigned char* buffer, unsigned int* srcIdx, unsigned char c)
{
	unsigned char b1 = c / 16;
	unsigned char b2 = c % 16;
	if (b1 > 9) b1 += ('A' - 10); else b1 += '0';
	if (b2 > 9) b2 += ('A' - 10); else b2 += '0';
	buffer[*srcIdx] = b1;
	(*srcIdx)++;
	buffer[*srcIdx] = b2;
	(*srcIdx)++;
}

void Str_WriteIntDec(unsigned char* buffer, unsigned int* srcIdx, int src, unsigned int zPad)
{
	int x = src, len = 0, i;

	do { x /= 10; len++; } while (x >= 1);

	x = 1;
	for (i = 1; i < len; i++) x *= 10;

	if (zPad > 0) i = zPad;
	else i = len;

	do
	{
		if (i > len) buffer[*srcIdx] = '0';
		else
		{
			buffer[*srcIdx] = (unsigned char)((src / x) + '0');
			src -= (src / x) * x;
			x /= 10;
		}
		(*srcIdx)++;
	} while (--i > 0);
}

void Str_WriteFloat(unsigned char* buffer, unsigned int* srcIdx, float f, unsigned int dPlaces, unsigned int zPad)
{
	float ff = f;

	if (ff < 0)
	{
		Str_WriteByte(buffer, srcIdx, '-');
		ff = -f;
	}

	int dec = (int)ff, mult = 1, i;
	for (i = 0; i < dPlaces; i++) mult *= 10;
	int frac = (int)((ff - dec) * (float)mult);

	Str_WriteIntDec(buffer, srcIdx, dec, zPad);
	Str_WriteByte(buffer, srcIdx, '.');
	Str_WriteIntDec(buffer, srcIdx, frac, dPlaces);
}

void Str_WriteStr(unsigned char* buffer, unsigned int* srcIdx, char* src)
{
	unsigned char c;
	c = *src;
	while (c != '\0') { buffer[(*srcIdx)++] = c; c = *++src; }
}

void Str_WriteHexStr(unsigned char* buffer, unsigned int* srcIdx, unsigned char* src, unsigned int srcSize)
{
	int i;
	for (i = 0; i < srcSize; i++) { Str_WriteHexByte(buffer, srcIdx, src[i]); }
}

float Str_ParseFloat(const unsigned char* buffer, unsigned int stIdx, unsigned int ndIdx)
{
	int i, dotIdx = ndIdx + 1;
	for (i = stIdx; i <= ndIdx; i++) { if (buffer[i] == '.') dotIdx = i; }

	float result = 0.0f;
	float multiplier = 1.0f;

	for (i = dotIdx - 1; i >= stIdx; i--)
	{
		result += ((float)((buffer[i] - '0'))) * multiplier;
		multiplier *= 10.0f;
	}

	multiplier = 0.1f;
	for (i = dotIdx + 1; i <= ndIdx; i++)
	{
		result += ((float)((buffer[i] - '0'))) * multiplier;
		multiplier /= 10.0f;
	}

	return result;
}

float Str_ReadFloat(const unsigned char* buffer, unsigned int stIdx, unsigned int size, unsigned int* ndIdx)
{
	int i = stIdx;
	*ndIdx = -1;
	while ((*ndIdx < 0) && (i < size)) { if (buffer[i] == '.') { *ndIdx = i; }  i++; }
	if (*ndIdx < 0) *ndIdx = size;
	return Str_ParseFloat(buffer, stIdx, *ndIdx);
}

unsigned char Str_ParseHexByte(const unsigned char* buffer, unsigned int stIdx)
{
	unsigned char c1 = buffer[stIdx];
	unsigned char c2 = buffer[stIdx + 1];
	if (c1 >= 0x41) { c1 -= 'A'; c1 += 10; } else c1 -= '0';
	if (c2 >= 0x41) { c2 -= 'A'; c2 += 10; } else c2 -= '0';
	return c1 * 16 + c2;
}


void StrB_WriteBytes(unsigned char* buffer, unsigned int* srcIdx, unsigned char* bytes, unsigned int size)
{
	int i;
	for (i = 0; i < size; i++)
	{
		buffer[*srcIdx] = bytes[i];
		(*srcIdx)++;
	}
}

void StrB_WriterInit(unsigned char* buffer, unsigned int* srcIdx, unsigned int size)
{
	*srcIdx = 0;
	ff_fill_u8(buffer, 0, size);
}

void StrB_WriteByte(unsigned char* buffer, unsigned int* srcIdx, unsigned char c)
{
	buffer[*srcIdx] = c;
	(*srcIdx)++;
}

void StrB_WriteInt(unsigned char* buffer, unsigned int* srcIdx, int src)
{
	buffer[*srcIdx] = (src & 0xff); (*srcIdx)++;
	buffer[*srcIdx] = ((src & 0xff00) >> 8); (*srcIdx)++;
	buffer[*srcIdx] = ((src & 0xff0000) >> 16); (*srcIdx)++;
	buffer[*srcIdx] = ((src & 0xff000000) >> 24); (*srcIdx)++;
}

void StrB_WriteUInt(unsigned char* buffer, unsigned int* srcIdx, unsigned int src)
{
	buffer[*srcIdx] = (src & 0xff); (*srcIdx)++;
	buffer[*srcIdx] = ((src & 0xff00) >> 8); (*srcIdx)++;
	buffer[*srcIdx] = ((src & 0xff0000) >> 16); (*srcIdx)++;
	buffer[*srcIdx] = ((src & 0xff000000) >> 24); (*srcIdx)++;
}

void StrB_WriteFloat(unsigned char* buffer, unsigned int* srcIdx, float src)
{
	int tmp = *((unsigned int*)&src);

	buffer[*srcIdx] = (tmp & 0xff); (*srcIdx)++;
	buffer[*srcIdx] = ((tmp & 0xff00) >> 8); (*srcIdx)++;
	buffer[*srcIdx] = ((tmp & 0xff0000) >> 16); (*srcIdx)++;
	buffer[*srcIdx] = ((tmp & 0xff000000) >> 24); (*srcIdx)++;
}

void StrB_WriteShort(unsigned char* buffer, unsigned int* srcIdx, short src)
{
	buffer[*srcIdx] = (src & 0xff); (*srcIdx)++;
	buffer[*srcIdx] = ((src & 0xff00) >> 8); (*srcIdx)++;
}

void StrB_WriteUShort(unsigned char* buffer, unsigned int* srcIdx, unsigned short src)
{
	buffer[*srcIdx] = (src & 0xff); (*srcIdx)++;
	buffer[*srcIdx] = ((src & 0xff00) >> 8); (*srcIdx)++;
}

unsigned char StrB_ReadByte(const unsigned char* buffer,unsigned  int* srcIdx)
{
	return buffer[(*srcIdx)++];
}

int StrB_ReadInt(const unsigned char* buffer, unsigned int* srcIdx)
{
	int result = buffer[*srcIdx]; (*srcIdx)++;
	result += (buffer[*srcIdx] << 8); (*srcIdx)++;
	result += (buffer[*srcIdx] << 16); (*srcIdx)++;
	result += (buffer[*srcIdx] << 24); (*srcIdx)++;
	return result;
}

unsigned int StrB_ReadUInt(const unsigned char* buffer, unsigned int* srcIdx)
{
	int result = buffer[*srcIdx]; (*srcIdx)++;
	result += (buffer[*srcIdx] << 8); (*srcIdx)++;
	result += (buffer[*srcIdx] << 16); (*srcIdx)++;
	result += (buffer[*srcIdx] << 24); (*srcIdx)++;
	return result;
}

float StrB_ReadFloat(const unsigned char* buffer, unsigned int* srcIdx)
{
	int result = buffer[*srcIdx]; (*srcIdx)++;
	result += (buffer[*srcIdx] << 8); (*srcIdx)++;
	result += (buffer[*srcIdx] << 16); (*srcIdx)++;
	result += (buffer[*srcIdx] << 24); (*srcIdx)++;
	return *((float*)&result);
}

short StrB_ReadShort(const unsigned char* buffer, unsigned int* srcIdx)
{
	short result = buffer[*srcIdx]; (*srcIdx)++;
	result += (buffer[*srcIdx] << 8); (*srcIdx)++;
	return result;
}

unsigned short StrB_ReadUShort(const unsigned char* buffer, unsigned int* srcIdx)
{
	unsigned short result = buffer[*srcIdx]; (*srcIdx)++;
	result += (buffer[*srcIdx] << 8); (*srcIdx)++;
	return result;
}



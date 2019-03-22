// (C) Aleksandr Dikarev, 2015-2019

#ifndef _USTR_H_
#define _USTR_H_

void Str_WriterInit(unsigned char* buffer, unsigned int* srcIdx, unsigned int size);
void Str_WriteByte(unsigned char* buffer, unsigned int* srcIdx, unsigned char c);
void Str_WriteHexByte(unsigned char* buffer, unsigned int* srcIdx, unsigned char c);
void Str_WriteIntDec(unsigned char* buffer, unsigned int* srcIdx, int src, unsigned int zPad);
void Str_WriteFloat(unsigned char* buffer, unsigned int* srcIdx, float f, unsigned int dPlaces, unsigned int zPad);
void Str_WriteStr(unsigned char* buffer, unsigned int* srcIdx, char* src);
void Str_WriteHexStr(unsigned char* buffer, unsigned int* srcIdx, unsigned char* src, unsigned int srcSize);
float Str_ParseFloat(const unsigned char* buffer, unsigned int stIdx, unsigned int ndIdx);
float Str_ReadFloat(const unsigned char* buffer, unsigned int stIdx, unsigned int size, unsigned int* ndIdx);
unsigned char Str_ParseHexByte(const unsigned char* buffer, unsigned int stIdx);


void StrB_WriteBytes(unsigned char* buffer, unsigned int* srcIdx, unsigned char* bytes, unsigned int size);
void StrB_WriterInit(unsigned char* buffer, unsigned int* srcIdx, unsigned int size);
void StrB_WriteByte(unsigned char* buffer, unsigned int* srcIdx, unsigned char c);
void StrB_WriteInt(unsigned char* buffer, unsigned int* srcIdx, int src);
void StrB_WriteUInt(unsigned char* buffer, unsigned int* srcIdx, unsigned int src);
void StrB_WriteFloat(unsigned char* buffer, unsigned int* srcIdx, float src);
void StrB_WriteShort(unsigned char* buffer, unsigned int* srcIdx, short src);
void StrB_WriteUShort(unsigned char* buffer, unsigned int* srcIdx, unsigned short src);
unsigned char StrB_ReadByte(const unsigned char* buffer, unsigned int* srcIdx);
int StrB_ReadInt(const unsigned char* buffer, unsigned int* srcIdx);
unsigned int StrB_ReadUInt(const unsigned char* buffer, unsigned int* srcIdx);
float StrB_ReadFloat(const unsigned char* buffer, unsigned int* srcIdx);
short StrB_ReadShort(const unsigned char* buffer, unsigned int* srcIdx);
unsigned short StrB_ReadUShort(const unsigned char* buffer, unsigned int* srcIdx);

#endif

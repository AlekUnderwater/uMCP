// (C) Aleksandr Dikarev, 2015

#ifndef _FF_H_
#define _FF_H_


void ff_add_accum_f32(float* acc, float* item, unsigned int size);
void ff_fill_f32(float* dst, float value, unsigned int size);
void ff_fill_u16(unsigned short* dst, unsigned short value, unsigned int size);
void ff_fill_s16(short* dst, short value, unsigned int size);
void ff_fill_u32(unsigned int* dst, unsigned int value, unsigned int size);
void ff_fill_s32(int* dst, int value, unsigned int size);
void ff_fill_s8(char* dst, char value, unsigned int size);
void ff_fill_u8(unsigned char* dst, unsigned char value, unsigned int size);

void ff_copy_u8(unsigned char* src, unsigned char* dst, unsigned int size);

#endif

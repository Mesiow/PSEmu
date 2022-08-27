#include "Common.h"

u8 sext_8(s8 value, u8 start)
{
	u8 result = ((value << (8 - start)) >> (8 - start));
	return result;
}

u16 sext_16(s16 value, u8 start)
{
	u16 result = ((value << (16 - start)) >> (16 - start));
	return result;
}

u32 sext_32(s32 value, u8 start)
{
	u32 result = ((value << (32 - start)) >> (32 - start));
	return result;
}

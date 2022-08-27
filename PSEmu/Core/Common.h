#pragma once
#include <cstdint>
#include <cstdlib>
#include <string>
#include <iostream>
#include <assert.h>

using u8 = uint8_t;
using s8 = int8_t;
using u16 = uint16_t;
using s16 = int16_t;
using u32 = uint32_t;
using s32 = int32_t;
using u64 = uint64_t;
using s64 = int64_t;

u8 sext_8(s8 value, u8 start);
u16 sext_16(s16 value, u8 start);
u32 sext_32(s32 value, u8 start);
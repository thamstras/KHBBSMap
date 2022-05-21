#pragma once
#include "..\BTypes.h"

constexpr uint32_t MagicCode(char a, char b, char c, char d)
{
    return (a << 0) | (b << 8) | (c << 16) | (d << 24);
}
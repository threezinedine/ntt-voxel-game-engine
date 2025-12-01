#include "MEEDEngine/core/string/string.h"
#include <string.h>

u32 mdGetStringLength(const char* str)
{
	return strlen(str);
}

i32 mdStringCompare(const char* str1, const char* str2)
{
	return strcmp(str1, str2);
}
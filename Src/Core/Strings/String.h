#pragma once

#include "Core/Platform.h"
#include "Core/Types.h"

#include <ctype.h> // isspace
#include <stdarg.h>
#include <stdio.h> // sscanf, vsnprintf
#include <cstring>

namespace Rio
{

inline int32_t vsnPrintF(char* str, size_t num, const char* format, va_list args)
{
#if RIO_COMPILER_MSVC
	int32_t len = _vsnprintf_s(str, num, _TRUNCATE, format, args);
	return (len == 1) ? _vscprintf(format, args) : len;
#else
	return ::vsnprintf(str, num, format, args);
#endif // RIO_COMPILER_MSVC
}

inline int32_t snPrintF(char* str, size_t n, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	int32_t len = Rio::vsnPrintF(str, n, format, args);
	va_end(args);
	return len;
}

inline uint32_t getStrLen32(const char* str)
{
	return (uint32_t)(std::strlen(str));
}

inline const char* skipSpaces(const char* str)
{
	while (isspace(*str)) ++str;
	return str;
}

inline const char* skipBlock(const char* str, char a, char b)
{
	uint32_t num = 0;

	for (char ch = *str++; ch != '\0'; ch = *str++)
	{
		if (ch == a)
		{
			++num;
		}
		else if (ch == b)
		{
			if (--num == 0)
			{
				return str;
			}
		}
	}

	return nullptr;
}

// Returns pointer after EOL
inline const char* getStrNl(const char* str)
{
	const char* eol = strchr(str, '\n');
	return eol ? eol + 1 : str + strlen(str);
}

inline int wildCmp(const char* wild, const char* str)
{
	const char* cp = nullptr;
	const char* mp = nullptr;

	while (*str && *wild != '*')
	{
		if (*wild != *str && *wild != '?')
		{
			return 0;
		}
		++wild;
		++str;
	}

	while (*str)
	{
		if (*wild == '*')
		{
			if (!*++wild)
			{
				return 1;
			}
		  mp = wild;
		  cp = str + 1;
		}
		else if (*wild == *str || *wild == '?')
		{
			++wild;
			++str;
		}
		else
		{
			wild = mp;
			str = cp++;
		}
	}

	while (*wild == '*')
	{
		++wild;
	}

	return !*wild;
}

} // namespace Rio

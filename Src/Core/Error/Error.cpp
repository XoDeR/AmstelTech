// Copyright (c) 2016 Volodymyr Syvochka
#include <stdarg.h>
#include <stdlib.h> // exit

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h> // isprint

#include "Core/Error/Error.h"
#include "Device/Log.h"

#include "Core/Error/Error.h"

#include "Core/Base/DebugHelper.h"
#include "Core/Strings/StringUtils.h" // snprintf, vsnprintf

void dbgPrintfVargs(const char* format, va_list argList)
{
	char temp[8192];
	char* out = temp;
	int32_t len = RioCore::vsnprintf(out, sizeof(temp), format, argList);
	if ((int32_t)sizeof(temp) < len)
	{
		out = (char*)alloca(len + 1);
		len = RioCore::vsnprintf(out, len, format, argList);
	}
	out[len] = '\0';
	RioCore::debugOutput(out);
}

void dbgPrintf(const char* _format, ...)
{
	va_list argList;
	va_start(argList, _format);
	dbgPrintfVargs(_format, argList);
	va_end(argList);
}

#define DBG_ADDRESS "%" PRIxPTR

void dbgPrintfData(const void* _data, uint32_t _size, const char* _format, ...)
{
#define HEX_DUMP_WIDTH 16
#define HEX_DUMP_SPACE_WIDTH 48
#define HEX_DUMP_FORMAT "%-" DBG_STRINGIZE(HEX_DUMP_SPACE_WIDTH) "." DBG_STRINGIZE(HEX_DUMP_SPACE_WIDTH) "s"

	va_list argList;
	va_start(argList, _format);
	dbgPrintfVargs(_format, argList);
	va_end(argList);

	dbgPrintf("\ndata: " DBG_ADDRESS ", size: %d\n", _data, _size);

	if (NULL != _data)
	{
		const uint8_t* data = reinterpret_cast<const uint8_t*>(_data);
		char hex[HEX_DUMP_WIDTH * 3 + 1];
		char ascii[HEX_DUMP_WIDTH + 1];
		uint32_t hexPos = 0;
		uint32_t asciiPos = 0;
		for (uint32_t ii = 0; ii < _size; ++ii)
		{
			RioCore::snprintf(&hex[hexPos], sizeof(hex) - hexPos, "%02x ", data[asciiPos]);
			hexPos += 3;

			ascii[asciiPos] = isprint(data[asciiPos]) ? data[asciiPos] : '.';
			asciiPos++;

			if (HEX_DUMP_WIDTH == asciiPos)
			{
				ascii[asciiPos] = '\0';
				dbgPrintf("\t" DBG_ADDRESS "\t" HEX_DUMP_FORMAT "\t%s\n", data, hex, ascii);
				data += asciiPos;
				hexPos = 0;
				asciiPos = 0;
			}
		}

		if (0 != asciiPos)
		{
			ascii[asciiPos] = '\0';
			dbgPrintf("\t" DBG_ADDRESS "\t" HEX_DUMP_FORMAT "\t%s\n", data, hex, ascii);
		}
	}

#undef HEX_DUMP_WIDTH
#undef HEX_DUMP_SPACE_WIDTH
#undef HEX_DUMP_FORMAT
}


namespace RioCore
{

namespace ErrorFn
{
	static void abort(const char* file, int line, const char* format, va_list args)
	{
		RIO_LOGEV(format, args);
		RIO_LOGE("\tIn: %s:%d\n\nStacktrace:", file, line);
		printCallstack();
		exit(EXIT_FAILURE);
	}

	void abort(const char* file, int line, const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		abort(file, line, format, args);
		va_end(args);
	}
} // namespace ErrorFn

} // namespace RioCore
  // Copyright (c) 2016 Volodymyr Syvochka
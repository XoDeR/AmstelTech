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

#include "Core/Error/DebugHelper.h"
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

void dbgPrintf(const char* format, ...)
{
	va_list argList;
	va_start(argList, format);
	dbgPrintfVargs(format, argList);
	va_end(argList);
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
#pragma once

#include "Core/Types.h"

#include <stdarg.h>

namespace Rio
{

struct LogSeverity
{
	enum Enum
	{
		LOG_INFO,
		LOG_WARN,
		LOG_ERROR,

		COUNT
	};
};

namespace LogInternal
{
	struct System
	{
		const char* name = nullptr;
	};

	void logExtended(LogSeverity::Enum sev, System system, const char* message, va_list args);
	void logExtendedAux(LogSeverity::Enum sev, System system, const char* message, char* buf, const uint32_t bufSize, va_list args);
	
	void logExtended(LogSeverity::Enum sev, System system, const char* message, ...);

} // namespace LogInternal

} // namespace Rio

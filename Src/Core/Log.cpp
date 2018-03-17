#include "Core/Log.h"

#include "Core/Memory/TempAllocator.h"
#include "Core/Os.h"
#include "Core/Platform.h"
#include "Core/Strings/String.h"
#include "Core/Strings/StringStream.h"
#include "Core/Thread/Mutex.h"
#include "Core/ConsoleServer.h"

namespace Rio
{

namespace LogInternal
{
	static Mutex mutexStatic;

	void logExtendedAux(LogSeverity::Enum logSeverityType, System system, const char* message, char* buffer, const uint32_t bufferSize, va_list args)
	{
		int32_t length = Rio::vsnPrintF(buffer, bufferSize, message, args);

		if (length > 0 && length < static_cast<int32_t>(bufferSize))
		{
			buffer[length] = '\0';
		}
		else
		{
			RIO_FATAL("Bad log message");
		}

#if RIO_PLATFORM_POSIX
#define ANSI_RESET  "\x1b[0m"
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_RED    "\x1b[31m"

		static const char* stt[] =
		{
			ANSI_RESET,
			ANSI_YELLOW,
			ANSI_RED,
			ANSI_RESET
		};

		OsFn::log(stt[logSeverityType]);
		OsFn::log(buffer);
		OsFn::log(ANSI_RESET);
#else
		OsFn::log(buffer);
#endif
		OsFn::log("\n");

		if (getConsoleServerGlobal())
		{
			static const char* logSeverityStringMap[] = 
			{ 
				"info",
				"warning",
				"error" 
			};
			RIO_STATIC_ASSERT(countof(logSeverityStringMap) == LogSeverity::COUNT);

			TempAllocator4096 ta;
			StringStream jsonStringStream(ta);

			jsonStringStream << "{\"type\":\"message\",";
			jsonStringStream << "\"severity\":\"";
			jsonStringStream << logSeverityStringMap[logSeverityType];
			jsonStringStream << "\",";
			jsonStringStream << "\"system\":\"";
			jsonStringStream << system.name;
			jsonStringStream << "\",";
			jsonStringStream << "\"message\":\"";

			// Sanitize buf
			const char* ch = buffer;
			for (; *ch; ch++)
			{
				if (*ch == '"' || *ch == '\\')
				{
					jsonStringStream << "\\";
				}
				jsonStringStream << *ch;
			}

			jsonStringStream << "\"}";

			getConsoleServerGlobal()->send(StringStreamFn::getCStr(jsonStringStream));
		}
	}

	void logExtended(LogSeverity::Enum logSeverityType, System system, const char* message, va_list args)
	{
		ScopedMutex scopedMutex(mutexStatic);

		const uint32_t bufferSize = 8192;
		char buffer[bufferSize];
		logExtendedAux(logSeverityType, system, message, buffer, bufferSize, args);
	}

	void logExtended(LogSeverity::Enum logSeverityType, System system, const char* message, ...)
	{
		va_list args;
		va_start(args, message);
		logExtended(logSeverityType, system, message, args);
		va_end(args);
	}

} // namespace log

} // namespace Rio

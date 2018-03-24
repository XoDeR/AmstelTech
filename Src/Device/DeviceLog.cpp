#include "Device/DeviceLog.h"

#include "Core/Memory/TempAllocator.h"
#include "Core/Os.h"
#include "Core/Platform.h"
#include "Core/Strings/String.h"
#include "Core/Strings/StringStream.h"
#include "Core/Thread/Mutex.h"

#include "Device/Device.h"

namespace Rio
{

namespace LogInternal
{
	static Mutex mutexStatic;

	void logX(LogSeverity::Enum logSeverityType, System system, const char* message, va_list argumentList)
	{
		ScopedMutex scopedMutex(mutexStatic);
		
		const uint32_t bufferSize = 8192;
		char buffer[bufferSize];
		logExtendedAux(logSeverityType, system, message, buffer, bufferSize, argumentList);
		
		// Log to file
		if (getDevice())
		{
			getDevice()->logToFile(buffer);
		}
	}

	void logX(LogSeverity::Enum logSeverityType, System system, const char* message, ...)
	{
		va_list argumentList;
		va_start(argumentList, message);
		logX(logSeverityType, system, message, argumentList);
		va_end(argumentList);
	}

} // namespace LogInternal

} // namespace Rio

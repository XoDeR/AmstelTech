#pragma once

#include "Core/Log.h"

namespace Rio
{

namespace LogInternal
{
	void logX(LogSeverity::Enum logSeverityType, System system, const char* message, va_list argumentList);
	void logX(LogSeverity::Enum logSeverityType, System system, const char* message, ...);

} // namespace LogInternal

} // namespace Rio

#define logInfoVariadic(system, message, va_list) Rio::LogInternal::logX(Rio::LogSeverity::LOG_INFO, system, message, va_list)
#define logErrorVariadic(system, message, va_list) Rio::LogInternal::logX(Rio::LogSeverity::LOG_ERROR, system, message, va_list)
#define logWarningVariadic(system, message, va_list) Rio::LogInternal::logX(Rio::LogSeverity::LOG_WARN, system, message, va_list)

#define logInfo(system, message, ...) Rio::LogInternal::logX(Rio::LogSeverity::LOG_INFO, system, message, ## __VA_ARGS__)
#define logError(system, message, ...) Rio::LogInternal::logX(Rio::LogSeverity::LOG_ERROR, system, message, ## __VA_ARGS__)
#define logWarning(system, message, ...) Rio::LogInternal::logX(Rio::LogSeverity::LOG_WARN, system, message, ## __VA_ARGS__)

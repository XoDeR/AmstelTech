#include "Core/Error/Error.h"

#include "Core/Error/Callstack.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Strings/StringStream.h"
#include "Core/Log.h"

#include <stdarg.h>
#include <stdlib.h> // exit, EXIT_FAILURE

namespace 
{ 
	const Rio::LogInternal::System ERROR = { "Error" }; 
}

namespace Rio
{

namespace ErrorFn
{
	static void abort(const char* format, va_list args)
	{
		LogInternal::logExtended(Rio::LogSeverity::LOG_ERROR, ERROR, format, args);

		TempAllocator4096 tempAllocator4096;
		StringStream stringStream(tempAllocator4096);
		callstack(stringStream);

		LogInternal::logExtended(Rio::LogSeverity::LOG_ERROR, ERROR, "Stacktrace:\n%s", StringStreamFn::getCStr(stringStream));
		
		exit(EXIT_FAILURE); // EXIT_FAILURE == 1
	}

	void abort(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		abort(format, args);
		va_end(args);
	}

} // namespace ErrorFn

} // namespace Rio

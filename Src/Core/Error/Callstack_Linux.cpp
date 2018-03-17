#include "Core/Platform.h"

#if RIO_PLATFORM_LINUX && RIO_COMPILER_GCC

#include "Core/Strings/String.h"
#include "Core/Strings/StringStream.h"

#include <cxxabi.h>
#include <execinfo.h>
#include <stdlib.h>
#include <cstring> // strchr
#include <unistd.h> // getpid

namespace Rio
{

namespace ErrorFn
{
	static const char* addr2line(const char* addr, char* line, int len)
	{
		char buffer[256];
		Rio::snPrintF(buffer, sizeof(buffer), "addr2line -e /proc/%u/exe %s", getpid(), addr);
		FILE* file = popen(buffer, "r");
		if (file)
		{
			fgets(line, len, file);
			line[getStrLen32(line) - 1] = '\0';
			pclose(file);
			return line;
		}
		return "<addr2line missing>";
	}

	void callstack(StringStream& stringStream)
	{
		void* arrayTemp[64];
		int size = backtrace(array, countof(arrayTemp));
		char** messages = backtrace_symbols(arrayTemp, size);

		// skip first stack frame (points here)
		for (int i = 1; i < size && messages != nullptr; ++i)
		{
			char* message = messages[i];
			char* mangledName = strchr(message, '(');
			char* offsetBegin = strchr(message, '+');
			char* offsetEnd = strchr(message, ')');
			char* addressBegin = strchr(message, '[');
			char* addressEnd = strchr(message, ']');

			char outputBuffer[512];

			// Attempt to demangle the symbol
			if (mangledName && offsetBegin && offsetEnd && mangledName < offsetBegin)
			{
				*mangledName++ = '\0';
				*offsetBegin++ = '\0';
				*offsetEnd++ = '\0';
				*addressBegin++ = '\0';
				*addressEnd++ = '\0';

				int demangleResultSuccess = 0;
				char* realName = abi::__cxa_demangle(mangledName, 0, 0, &demangleResultSuccess);
				char line[256];
				memset(line, 0, sizeof(line));

				Rio::snPrintF(outputBuffer
					, sizeof(outputBuffer)
					, "    [%2d] %s: (%s)+%s in %s\n"
					, i
					, message
					, (demangleResultSuccess == 0 ? realName : mangledName)
					, offsetBegin
					, addr2line(addressBegin, line, sizeof(line))
					);

				free(realName);
			}
			else
			{
				Rio::snPrintF(outputBuffer, sizeof(outputBuffer), "    [%2d] %s\n", i, message);
			}

			stringStream << outputBuffer;
		}
		free(messages);
	}

} // namespace ErrorFn

} // namespace Rio

#endif // RIO_PLATFORM_LINUX && RIO_COMPILER_GCC

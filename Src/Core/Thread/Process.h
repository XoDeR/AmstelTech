#pragma once

#include "Core/Strings/StringUtils.h" // strlen

#include "Core/Math/Uint32T.h"

#if RIO_PLATFORM_LINUX || RIO_PLATFORM_HURD
#include <unistd.h>
#elif RIO_PLATFORM_WINDOWS
#include <process.h>
#endif // RIO_PLATFORM_LINUX || RIO_PLATFORM_HURD

namespace RioCore
{
	inline void* exec(const char* const* _argv)
	{
#if RIO_PLATFORM_LINUX || RIO_PLATFORM_HURD
		pid_t pid = fork();

		if (0 == pid)
		{
			int result = execvp(_argv[0], const_cast<char* const*>(&_argv[1]) );
			RIO_UNUSED(result);
			return NULL;
		}

		return (void*)uintptr_t(pid);
#elif RIO_PLATFORM_WINDOWS
		STARTUPINFO si;
		memset(&si, 0, sizeof(STARTUPINFO) );
		si.cb = sizeof(STARTUPINFO);

		PROCESS_INFORMATION pi;
		memset(&pi, 0, sizeof(PROCESS_INFORMATION) );

		int32_t total = 0;
		for (uint32_t ii = 0; NULL != _argv[ii]; ++ii)
		{
			total += (int32_t)strlen(_argv[ii]) + 1;
		}

		char* temp = (char*)alloca(total);
		int32_t len = 0;
		for(uint32_t ii = 0; NULL != _argv[ii]; ++ii)
		{
			len += snprintf(&temp[len], RioCore::uint32_imax(0, total-len)
						, "%s "
						, _argv[ii]
						);
		}

		bool ok = CreateProcessA(_argv[0]
					, temp
					, NULL
					, NULL
					, false
					, 0
					, NULL
					, NULL
					, &si
					, &pi
					);
		if (ok)
		{
			return pi.hProcess;
		}

		return NULL;
#else
		return NULL;
#endif // RIO_PLATFORM_LINUX || RIO_PLATFORM_HURD
	}

} // namespace RioCore

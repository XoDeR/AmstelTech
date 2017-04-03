#pragma once

#include "Core/Base/RioCommon.h"

#if RIO_PLATFORM_ANDROID
#include <time.h> // clock, clock_gettime
#elif RIO_PLATFORM_EMSCRIPTEN
#include <emscripten.h>
#elif RIO_PLATFORM_WINDOWS || RIO_PLATFORM_XBOXONE || RIO_PLATFORM_WINRT
#include <windows.h>
#else
#include <sys/time.h> // gettimeofday
#endif // RIO_PLATFORM_

namespace RioCore
{
	inline int64_t getHighPerformanceCounter()
	{
#if RIO_PLATFORM_WINDOWS || RIO_PLATFORM_XBOX360 || RIO_PLATFORM_XBOXONE || RIO_PLATFORM_WINRT
		LARGE_INTEGER timeValue;
		QueryPerformanceCounter(&timeValue);
		int64_t result = timeValue.QuadPart;
#elif RIO_PLATFORM_ANDROID
		struct timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		int64_t result = now.tv_sec * int64_t(1000000000) + now.tv_nsec;
#elif RIO_PLATFORM_EMSCRIPTEN
		int64_t result = int64_t(1000.0f * emscripten_get_now() );
#else
		struct timeval now;
		gettimeofday(&now, 0);
		int64_t result = now.tv_sec * int64_t(1000000) + now.tv_usec;
#endif // RIO_PLATFORM_
		return result;
	}

	inline int64_t getHighPerformanceFrequency()
	{
#if RIO_PLATFORM_WINDOWS || RIO_PLATFORM_XBOX360 || RIO_PLATFORM_XBOXONE || RIO_PLATFORM_WINRT
		LARGE_INTEGER frequencyValue;
		QueryPerformanceFrequency(&frequencyValue);
		return frequencyValue.QuadPart;
#elif RIO_PLATFORM_ANDROID
		return int64_t(1000000000);
#elif RIO_PLATFORM_EMSCRIPTEN
		return int64_t(1000000);
#else
		return int64_t(1000000);
#endif // RIO_PLATFORM_
	}
} // namespace RioCore

#pragma once

#include "Core/Platform.h"
#include "Core/Thread/Mutex.h"

#if RIO_PLATFORM_POSIX
	#include <pthread.h>
#elif RIO_PLATFORM_WINDOWS
	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
	#include <limits.h>
#endif

namespace Rio
{

struct Semaphore
{

#if RIO_PLATFORM_POSIX
	Mutex mutex;
	pthread_cond_t condition;
	int32_t count = 0;
#elif RIO_PLATFORM_WINDOWS
	HANDLE handle = INVALID_HANDLE_VALUE;
#endif

	Semaphore();
	~Semaphore();
	Semaphore(const Semaphore&) = delete;
	Semaphore& operator=(const Semaphore&) = delete;

	void post(uint32_t count = 1);
	void wait();
};

} // namespace Rio

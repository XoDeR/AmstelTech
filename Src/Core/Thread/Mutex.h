#pragma once

#include "Core/Error/Error.h"
#include "Core/Platform.h"
#include "Core/Types.h"

#if RIO_PLATFORM_POSIX
	#include <pthread.h>
#elif RIO_PLATFORM_WINDOWS
	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
#endif

namespace Rio
{

struct Mutex
{
#if RIO_PLATFORM_POSIX
	pthread_mutex_t mutex;
#elif RIO_PLATFORM_WINDOWS
	CRITICAL_SECTION criticalSection;
#endif

	Mutex();
	~Mutex();
	Mutex(const Mutex&) = delete;
	Mutex& operator=(const Mutex&) = delete;

	void lock();
	void unlock();
};

// Automatically locks a mutex when created and unlocks when destroyed
struct ScopedMutex
{
	Mutex& mutex;

	// Locks the mutex <m>
	ScopedMutex(Mutex& m)
		: mutex(m)
	{
		this->mutex.lock();
	}

	// Unlocks the mutex passed to ScopedMutex::ScopedMutex()
	~ScopedMutex()
	{
		this->mutex.unlock();
	}

	ScopedMutex(const ScopedMutex&) = delete;
	ScopedMutex& operator=(const ScopedMutex&) = delete;
};

} // namespace Rio

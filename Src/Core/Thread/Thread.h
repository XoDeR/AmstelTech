#pragma once

#include "Core/Platform.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Types.h"

#if RIO_PLATFORM_POSIX
	#include <pthread.h>
#elif RIO_PLATFORM_WINDOWS
	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
	#include <process.h>
#endif

namespace Rio
{

struct Thread
{
	using ThreadFunction = int32_t (*)(void* data);

	ThreadFunction threadFunction = nullptr;
	void* userData = nullptr;
	Semaphore semaphore;
	bool isRunning = false;
#if RIO_PLATFORM_POSIX
	pthread_t handle = 0;
#elif RIO_PLATFORM_WINDOWS
	HANDLE handle = INVALID_HANDLE_VALUE;
#endif // RIO_PLATFORM_POSIX | RIO_PLATFORM_WINDOWS

	Thread();
	~Thread();
	Thread(const Thread&) = delete;
	Thread& operator=(const Thread&) = delete;

	void start(ThreadFunction threadFunction, void* userData = nullptr, uint32_t stackSize = 0);
	void stop();
	bool isThreadRunning();
	// Do not call explicitly
	int32_t run();
};

} // namespace Rio

#include "Core/Thread/Thread.h"

#include "Core/Error/Error.h"

namespace Rio
{

#if RIO_PLATFORM_POSIX
static void* threadProcedure(void* arg)
{
	static int32_t result = -1;
	result = ((Thread*)arg)->run();
	return (void*)&result;
}
#elif RIO_PLATFORM_WINDOWS
static DWORD WINAPI threadProcedure(void* arg)
{
	int32_t result = ((Thread*)arg)->run();
	return result;
}
#endif

Thread::Thread()
{
}

Thread::~Thread()
{
	if (isRunning)
	{
		stop();
	}
}

void Thread::start(ThreadFunction threadFunction, void* userData, uint32_t stackSize)
{
	RIO_ASSERT(!isRunning, "Thread is already running");
	RIO_ASSERT(threadFunction != nullptr, "Function must be != NULL");
	this->threadFunction = threadFunction;
	this->userData = userData;

#if RIO_PLATFORM_POSIX
	pthread_attr_t attr;
	int err = pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	RIO_ASSERT(err == 0, "pthread_attr_init: errno = %d", err);

	if (stackSize != 0)
	{
		err = pthread_attr_setstacksize(&attr, stackSize);
		RIO_ASSERT(err == 0, "pthread_attr_setstacksize: errno = %d", err);
	}

	err = pthread_create(&handle, &attr, threadProcedure, this);
	RIO_ASSERT(err == 0, "pthread_create: errno = %d", err);

	err = pthread_attr_destroy(&attr);
	RIO_ASSERT(err == 0, "pthread_attr_destroy: errno = %d", err);
	RIO_UNUSED(err);
#elif RIO_PLATFORM_WINDOWS
	handle = CreateThread(NULL, stackSize, threadProcedure, this, 0, NULL);
	RIO_ASSERT(handle != nullptr, "CreateThread: GetLastError = %d", GetLastError());
#endif

	isRunning = true;
	semaphore.wait();
}

void Thread::stop()
{
	RIO_ASSERT(isRunning, "Thread is not running");

#if RIO_PLATFORM_POSIX
	int err = pthread_join(handle, NULL);
	RIO_ASSERT(err == 0, "pthread_join: errno = %d", err);
	RIO_UNUSED(err);
	handle = 0;
#elif RIO_PLATFORM_WINDOWS
	WaitForSingleObject(handle, INFINITE);
	CloseHandle(handle);
	handle = INVALID_HANDLE_VALUE;
#endif

	isRunning = false;
}

bool Thread::isThreadRunning()
{
	return isRunning;
}

int32_t Thread::run()
{
	semaphore.post();
	return threadFunction(userData);
}

} // namespace Rio

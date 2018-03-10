#include "Core/Thread/Semaphore.h"

#include "Core/Error/Error.h"

namespace Rio
{

Semaphore::Semaphore()
{
#if RIO_PLATFORM_POSIX
	int err = pthread_cond_init(&(this->cond), NULL);
	RIO_ASSERT(err == 0, "pthread_cond_init: errno = %d", err);
	RIO_UNUSED(err);
#elif RIO_PLATFORM_WINDOWS
	this->handle = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
	RIO_ASSERT(this->handle != nullptr, "CreateSemaphore: GetLastError = %d", GetLastError());
	RIO_UNUSED(this->handle);
#endif
}

Semaphore::~Semaphore()
{
#if RIO_PLATFORM_POSIX
	int err = pthread_cond_destroy(&(this->cond));
	RIO_ASSERT(err == 0, "pthread_cond_destroy: errno = %d", err);
	RIO_UNUSED(err);
#elif RIO_PLATFORM_WINDOWS
	BOOL err = CloseHandle(this->handle);
	RIO_ASSERT(err != 0, "CloseHandle: GetLastError = %d", GetLastError());
	RIO_UNUSED(err);
#endif
}

void Semaphore::post(uint32_t count)
{
#if RIO_PLATFORM_POSIX
	ScopedMutex scopedMutex(this->mutex);

	for (uint32_t i = 0; i < count; ++i)
	{
		int err = pthread_cond_signal(&(this->cond));
		RIO_ASSERT(err == 0, "pthread_cond_signal: errno = %d", err);
		RIO_UNUSED(err);
	}

	this->count += count;
#elif RIO_PLATFORM_WINDOWS
	BOOL err = ReleaseSemaphore(this->handle, count, NULL);
	RIO_ASSERT(err != 0, "ReleaseSemaphore: GetLastError = %d", GetLastError());
	RIO_UNUSED(err);
#endif
}

void Semaphore::wait()
{
#if RIO_PLATFORM_POSIX
	ScopedMutex scopedMutex(this->mutex);

	while (this->count <= 0)
	{
		int err = pthread_cond_wait(&(this->cond), &(this->mutex.mutex));
		RIO_ASSERT(err == 0, "pthread_cond_wait: errno = %d", err);
		RIO_UNUSED(err);
	}

	this->count--;
#elif RIO_PLATFORM_WINDOWS
	DWORD err = WaitForSingleObject(this->handle, INFINITE);
	RIO_ASSERT(err == WAIT_OBJECT_0, "WaitForSingleObject: GetLastError = %d", GetLastError());
	RIO_UNUSED(err);
#endif
}

} // namespace Rio

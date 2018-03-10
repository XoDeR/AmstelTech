#include "Core/Thread/Mutex.h"

namespace Rio
{

Mutex::Mutex()
{
#if RIO_PLATFORM_POSIX
	pthread_mutexattr_t attr;
	int err = pthread_mutexattr_init(&attr);
	RIO_ASSERT(err == 0, "pthread_mutexattr_init: errno = %d", err);
	err = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
	RIO_ASSERT(err == 0, "pthread_mutexattr_settype: errno = %d", err);
	err = pthread_mutex_init(&(this->mutex), &attr);
	RIO_ASSERT(err == 0, "pthread_mutex_init: errno = %d", err);
	err = pthread_mutexattr_destroy(&attr);
	RIO_ASSERT(err == 0, "pthread_mutexattr_destroy: errno = %d", err);
	RIO_UNUSED(err);
#elif RIO_PLATFORM_WINDOWS
	InitializeCriticalSection(&(this->criticalSection));
#endif
}

Mutex::~Mutex()
{
#if RIO_PLATFORM_POSIX
	int err = pthread_mutex_destroy(&(this->mutex));
	RIO_ASSERT(err == 0, "pthread_mutex_destroy: errno = %d", err);
	RIO_UNUSED(err);
#elif RIO_PLATFORM_WINDOWS
	DeleteCriticalSection(&(this->criticalSection));
#endif
}

void Mutex::lock()
{
#if RIO_PLATFORM_POSIX
	int err = pthread_mutex_lock(&(this->mutex));
	RIO_ASSERT(err == 0, "pthread_mutex_lock: errno = %d", err);
	RIO_UNUSED(err);
#elif RIO_PLATFORM_WINDOWS
	EnterCriticalSection(&(this->criticalSection));
#endif
}

void Mutex::unlock()
{
#if RIO_PLATFORM_POSIX
	int err = pthread_mutex_unlock(&(this->mutex));
	RIO_ASSERT(err == 0, "pthread_mutex_unlock: errno = %d", err);
	RIO_UNUSED(err);
#elif RIO_PLATFORM_WINDOWS
	LeaveCriticalSection(&(this->criticalSection));
#endif
}

} // namespace Rio


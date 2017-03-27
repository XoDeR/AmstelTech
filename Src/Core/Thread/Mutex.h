// Copyright (c) 2016, 2017 Volodymyr Syvochka
#pragma once

#include "Core/Base/RioCommon.h"
#include "Core/Base/Cpu.h"
#include "Core/Base/Os.h"
#include "Core/Thread/Semaphore.h"

#if RIO_CONFIG_SUPPORTS_THREADING

#if RIO_PLATFORM_NACL || RIO_PLATFORM_LINUX || RIO_PLATFORM_ANDROID || RIO_PLATFORM_OSX
#include <pthread.h>
#elif RIO_PLATFORM_WINDOWS || RIO_PLATFORM_XBOX360 || RIO_PLATFORM_WINRT
#include <errno.h>
#endif // RIO_PLATFORM_

namespace RioCore
{

#if RIO_PLATFORM_WINDOWS || RIO_PLATFORM_XBOX360 || RIO_PLATFORM_XBOXONE || RIO_PLATFORM_WINRT
    typedef CRITICAL_SECTION pthread_mutex_t;
    typedef unsigned pthread_mutexattr_t;

    inline int pthread_mutex_lock(pthread_mutex_t* _mutex)
    {
        EnterCriticalSection(_mutex);
        return 0;
    }

    inline int pthread_mutex_unlock(pthread_mutex_t* _mutex)
    {
        LeaveCriticalSection(_mutex);
        return 0;
    }

    inline int pthread_mutex_trylock(pthread_mutex_t* _mutex)
    {
        return TryEnterCriticalSection(_mutex) ? 0 : EBUSY;
    }

    inline int pthread_mutex_init(pthread_mutex_t* _mutex, pthread_mutexattr_t* /*_attr*/)
    {
#if RIO_PLATFORM_WINRT
        InitializeCriticalSectionEx(_mutex, 4000, 0);   // docs recommend 4000 spincount as sane default
#else
        InitializeCriticalSection(_mutex);
#endif
        return 0;
    }

    inline int pthread_mutex_destroy(pthread_mutex_t* _mutex)
    {
        DeleteCriticalSection(_mutex);
        return 0;
    }
#endif // RIO_PLATFORM_

    class Mutex
    {
    private:
        Mutex(const Mutex& rhs) = delete;
        Mutex& operator=(const Mutex& rhs) = delete;
    public:
        Mutex()
        {
            pthread_mutexattr_t attr;
#if RIO_PLATFORM_WINDOWS || RIO_PLATFORM_XBOX360 || RIO_PLATFORM_XBOXONE || RIO_PLATFORM_WINRT
#else
            pthread_mutexattr_init(&attr);
            pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
#endif // RIO_PLATFORM_WINDOWS || RIO_PLATFORM_XBOX360 || RIO_PLATFORM_WINRT
            pthread_mutex_init(&m_handle, &attr);
        }

        ~Mutex()
        {
            pthread_mutex_destroy(&m_handle);
        }

        void lock()
        {
            pthread_mutex_lock(&m_handle);
        }

        void unlock()
        {
            pthread_mutex_unlock(&m_handle);
        }

    private:
        pthread_mutex_t m_handle;
    };

    // Automatically locks a mutex when created and unlocks when destroyed
    // RAII wrapper
    struct ScopedMutex
    {
        // Locks the mutex
        ScopedMutex(Mutex& m)
            : mutex(m)
        {
            mutex.lock();
        }

        // Unlocks the mutex
        ~ScopedMutex()
        {
            mutex.unlock();
        }

    private:
        // Disable copying
        ScopedMutex(const ScopedMutex&) = delete;
        ScopedMutex& operator=(const ScopedMutex&) = delete;
        ScopedMutex() = delete;
    public:
        Mutex& mutex;
    };

} // namespace RioCore

#endif // RIO_CONFIG_SUPPORTS_THREADING

// Copyright (c) 2016, 2017 Volodymyr Syvochka
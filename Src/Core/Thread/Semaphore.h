// Copyright (c) 2016, 2017 Volodymyr Syvochka
#pragma once

#include "Core/Base/RioCommon.h"
#include "Core/Error/Error.h"
#include "Core/Base/Platform.h"

#if RIO_CONFIG_SUPPORTS_THREADING

#if RIO_PLATFORM_POSIX
#include <errno.h>
#include <semaphore.h>
#include <time.h>
#include <pthread.h>
#elif RIO_PLATFORM_XBOXONE
#include <synchapi.h>
#elif RIO_PLATFORM_XBOX360 || RIO_PLATFORM_WINDOWS || RIO_PLATFORM_WINRT
#include <windows.h>
#include <limits.h>
#endif // RIO_PLATFORM_

namespace RioCore
{
#if RIO_PLATFORM_POSIX
#if RIO_CONFIG_SEMAPHORE_PTHREAD

	class Semaphore
	{
	public:
		pthread_mutex_t m_mutex;
		pthread_cond_t m_cond;
		int32_t m_count = 0;
	private:
		Semaphore(const Semaphore& rhs) = delete;
		Semaphore& operator=(const Semaphore& rhs) = delete;
	public:
		Semaphore()
		{
			int result = 0;
			result = pthread_mutex_init(&m_mutex, NULL);
			RIO_CHECK(0 == result, "pthread_mutex_init %d", result);

			result = pthread_cond_init(&m_cond, NULL);
			RIO_CHECK(0 == result, "pthread_cond_init %d", result);

			RIO_UNUSED(result);
		}

		~Semaphore()
		{
			int result;
			result = pthread_cond_destroy(&m_cond);
			RIO_CHECK(0 == result, "pthread_cond_destroy %d", result);

			result = pthread_mutex_destroy(&m_mutex);
			RIO_CHECK(0 == result, "pthread_mutex_destroy %d", result);

			RIO_UNUSED(result);
		}

		void post(uint32_t count = 1)
		{
			int result = pthread_mutex_lock(&m_mutex);
			RIO_CHECK(0 == result, "pthread_mutex_lock %d", result);

			for (uint32_t i = 0; i < count; ++i)
			{
				result = pthread_cond_signal(&m_cond);
				RIO_CHECK(0 == result, "pthread_cond_signal %d", result);
			}

			this->m_count += count;

			result = pthread_mutex_unlock(&m_mutex);
			RIO_CHECK(0 == result, "pthread_mutex_unlock %d", result);

			RIO_UNUSED(result);
		}

		bool wait(int32_t milliseconds = -1)
		{
			int result = pthread_mutex_lock(&m_mutex);
			RIO_CHECK(0 == result, "pthread_mutex_lock %d", result);

#if RIO_PLATFORM_NACL || RIO_PLATFORM_OSX
			RIO_UNUSED(milliseconds);
			RIO_CHECK(-1 == milliseconds, "NaCl and OSX don't support pthread_cond_timedwait at this moment.");
			while (0 == result
				&& 0 >= m_count)
			{
				result = pthread_cond_wait(&m_cond, &m_mutex);
			}
#elif RIO_PLATFORM_IOS
			if (-1 == milliseconds)
			{
				while (0 == result && 0 >= m_count)
				{
					result = pthread_cond_wait(&m_cond, &m_mutex);
				}
			}
			else
			{
				timespec ts;
				ts.tv_sec = milliseconds / 1000;
				ts.tv_nsec = (milliseconds % 1000) * 1000;

				while (0 == result && 0 >= m_count)
				{
					result = pthread_cond_timedwait_relative_np(&m_cond, &m_mutex, &ts);
				}
			}
#else
			timespec ts;
			clock_gettime(CLOCK_REALTIME, &ts);
			ts.tv_sec += milliseconds / 1000;
			ts.tv_nsec += (milliseconds % 1000) * 1000;

			while (0 == result && 0 >= m_count)
			{
				result = pthread_cond_timedwait(&m_cond, &m_mutex, &ts);
			}
#endif // RIO_PLATFORM_NACL || RIO_PLATFORM_OSX
			bool ok = 0 == result;

			if (ok)
			{
				--m_count;
			}

			result = pthread_mutex_unlock(&m_mutex);
			RIO_CHECK(0 == result, "pthread_mutex_unlock %d", result);

			RIO_UNUSED(result);

			return ok;
		}
	};
#else
	class Semaphore
	{
	public:
		sem_t semaphoreHandle;
	private:
		Semaphore(const Semaphore& s) = delete;
		Semaphore& operator=(const Semaphore& s) = delete;
	public:
		Semaphore()
		{
			int32_t result = sem_init(&semaphoreHandle, 0, 0);
			RIO_CHECK(0 == result, "sem_init failed. errno %d", errno);
			RIO_UNUSED(result);
		}

		~Semaphore()
		{
			int32_t result = sem_destroy(&semaphoreHandle);
			RIO_CHECK(0 == result, "sem_destroy failed. errno %d", errno);
			RIO_UNUSED(result);
		}

		void post(uint32_t count = 1)
		{
			int32_t result = 0;
			for (uint32_t i = 0; i < count; ++i)
			{
				result = sem_post(&semaphoreHandle);
				RIO_CHECK(0 == result, "sem_post failed. errno %d", errno);
			}
			RIO_UNUSED(result);
		}

		bool wait(int32_t milliseconds = -1)
		{
#if RIO_PLATFORM_NACL || RIO_PLATFORM_OSX
			RIO_CHECK(-1 == milliseconds, "NaCl and OSX don't support sem_timedwait at this moment."); RIO_UNUSED(milliseconds);
			return 0 == sem_wait(&semaphoreHandle);
#else
			if (0 > milliseconds)
			{
				int32_t result;
				do
				{
					result = sem_wait(&semaphoreHandle);
				} // keep waiting when interrupted by a signal handler
				while (-1 == result && EINTR == errno);
				RIO_CHECK(0 == result, "sem_wait failed. errno %d", errno);
				return 0 == result;
			}

			timespec ts;
			clock_gettime(CLOCK_REALTIME, &ts);
			ts.tv_sec += milliseconds / 1000;
			ts.tv_nsec += (milliseconds % 1000) * 1000;
			return 0 == sem_timedwait(&semaphoreHandle, &ts);
#endif // RIO_PLATFORM_
		}


	};
#endif // RIO_CONFIG_SEMAPHORE_PTHREAD

#elif RIO_PLATFORM_XBOX360 || RIO_PLATFORM_XBOXONE || RIO_PLATFORM_WINDOWS || RIO_PLATFORM_WINRT

	class Semaphore
	{
	public:
		HANDLE semaphoreHandle = INVALID_HANDLE_VALUE;
	private:
		// Disable copying
		Semaphore(const Semaphore& s) = delete;
		Semaphore& operator=(const Semaphore& s) = delete;
	public:
		Semaphore()
		{
#if RIO_PLATFORM_XBOXONE || RIO_PLATFORM_WINRT
			semaphoreHandle = CreateSemaphoreExW(NULL, 0, LONG_MAX, NULL, 0, SEMAPHORE_ALL_ACCESS);
#else
			semaphoreHandle = CreateSemaphoreA(NULL, 0, LONG_MAX, NULL);
#endif
			RIO_CHECK(NULL != semaphoreHandle, "Failed to create Semaphore!");
		}

		~Semaphore()
		{
			CloseHandle(semaphoreHandle);
		}

		void post(uint32_t count = 1) const
		{
			ReleaseSemaphore(semaphoreHandle, count, nullptr);
		}

		bool wait(int32_t milliseconds = -1) const
		{
			DWORD millisecondsLocal = (0 > milliseconds) ? INFINITE : milliseconds;
#if RIO_PLATFORM_XBOXONE || RIO_PLATFORM_WINRT
			return WAIT_OBJECT_0 == WaitForSingleObjectEx(semaphoreHandle, millisecondsLocal, FALSE);
#else
			return WAIT_OBJECT_0 == WaitForSingleObject(semaphoreHandle, millisecondsLocal);
#endif
		}
	};

#endif // RIO_PLATFORM_

} // namespace RioCore

#endif // RIO_CONFIG_SUPPORTS_THREADING
  // Copyright (c) 2016, 2017 Volodymyr Syvochka
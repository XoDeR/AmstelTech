// Copyright (c) 2016, 2017 Volodymyr Syvochka
#pragma once

#if RIO_PLATFORM_POSIX
#include <pthread.h>
#if defined(__FreeBSD__)
#include <pthread_np.h>
#endif
#if RIO_PLATFORM_LINUX && (RIO_CRT_GLIBC < 21200)
#include <sys/prctl.h>
#endif // RIO_PLATFORM_
#elif RIO_PLATFORM_WINRT
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
#endif // RIO_PLATFORM_

#include "Core/Thread/Semaphore.h"

#if RIO_CONFIG_SUPPORTS_THREADING

namespace RioCore
{
	using ThreadFunction = int32_t(*)(void* data);

	class Thread
	{
	public:
#if RIO_PLATFORM_WINDOWS || RIO_PLATFORM_XBOX360 || RIO_PLATFORM_XBOXONE || RIO_PLATFORM_WINRT
		HANDLE threadHandle = INVALID_HANDLE_VALUE;
		DWORD m_threadId = UINT32_MAX;
#elif RIO_PLATFORM_POSIX
		pthread_t threadHandle = 0;
#endif // RIO_PLATFORM_

		ThreadFunction threadFunction = nullptr;
		void* m_userData = nullptr;
		Semaphore m_sem;
		uint32_t m_stackSize = 0;
		int32_t m_exitCode = 0; // EXIT_SUCCESS
		bool isRunning = false;
	private:
		// Disable copying
		Thread(const Thread& t) = delete;
		Thread& operator=(const Thread& t) = delete;
	public:
		Thread()
		{
		}

		virtual ~Thread()
		{
			if (isRunning)
			{
				shutdown();
			}
		}

		void init(ThreadFunction _fn, void* _userData = NULL, uint32_t _stackSize = 0, const char* _name = nullptr)
		{
			RIO_CHECK(!isRunning, "Already running!");

			this->threadFunction = _fn;
			this->m_userData = _userData;
			this->m_stackSize = _stackSize;
			this->isRunning = true;

#if RIO_PLATFORM_WINDOWS || RIO_PLATFORM_XBOX360 || RIO_PLATFORM_XBOXONE
			threadHandle = ::CreateThread(NULL
				, m_stackSize
				, (LPTHREAD_START_ROUTINE)threadProcedure
				, this
				, 0
				, NULL
			);
#elif RIO_PLATFORM_WINRT
			threadHandle = CreateEventEx(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
			auto workItemHandler = ref new WorkItemHandler([=](IAsyncAction^)
			{
				m_exitCode = threadProcedure(this);
				SetEvent(threadHandle);
			}, CallbackContext::Any);

			ThreadPool::RunAsync(workItemHandler, WorkItemPriority::Normal, WorkItemOptions::TimeSliced);
#elif RIO_PLATFORM_POSIX
			int result;
			RIO_UNUSED(result);

			pthread_attr_t attr;
			result = pthread_attr_init(&attr);
			RIO_CHECK(0 == result, "pthread_attr_init failed! %d", result);

			if (0 != m_stackSize)
			{
				result = pthread_attr_setstacksize(&attr, m_stackSize);
				RIO_CHECK(0 == result, "pthread_attr_setstacksize failed! %d", result);
			}

			result = pthread_create(&threadHandle, &attr, &threadProcedure, this);
			RIO_CHECK(0 == result, "pthread_attr_setschedparam failed! %d", result);
#else
#error "Not implemented!"
#endif // RIO_PLATFORM_

			m_sem.wait();

			if (NULL != _name)
			{
				setThreadName(_name);
			}
		}

		void shutdown()
		{
			RIO_ASSERT(isRunning, "Not running!");
#if RIO_PLATFORM_WINDOWS || RIO_PLATFORM_XBOX360
			WaitForSingleObject(threadHandle, INFINITE);
			GetExitCodeThread(threadHandle, (DWORD*)&m_exitCode);
			CloseHandle(threadHandle);
			threadHandle = INVALID_HANDLE_VALUE;
#elif RIO_PLATFORM_WINRT
			WaitForSingleObjectEx(threadHandle, INFINITE, FALSE);
			CloseHandle(threadHandle);
			threadHandle = INVALID_HANDLE_VALUE;
#elif RIO_PLATFORM_POSIX
			union
			{
				void* ptr;
				int32_t i;
			} cast;
			pthread_join(threadHandle, &cast.ptr);
			m_exitCode = cast.i;
			threadHandle = 0;
#endif // RIO_PLATFORM_
			isRunning = false;
		}

		bool getIsRunning() const
		{
			return isRunning;
		}

		int32_t getExitCode() const
		{
			return m_exitCode;
		}

		void setThreadName(const char* _name)
		{
#if RIO_PLATFORM_OSX || RIO_PLATFORM_IOS
			pthread_setname_np(_name);
#elif (RIO_CRT_GLIBC >= 21200) && ! RIO_PLATFORM_HURD
			pthread_setname_np(threadHandle, _name);
#elif RIO_PLATFORM_LINUX
			prctl(PR_SET_NAME, _name, 0, 0, 0);
#elif RIO_PLATFORM_BSD
#ifdef __NetBSD__
			pthread_setname_np(threadHandle, "%s", (void*)_name);
#else
			pthread_set_name_np(threadHandle, _name);
#endif // __NetBSD__
#elif RIO_PLATFORM_WINDOWS && RIO_COMPILER_MSVC
#pragma pack(push, 8)
			struct ThreadName
			{
				DWORD  type;
				LPCSTR name;
				DWORD  id;
				DWORD  flags;
			};
#pragma pack(pop)
			ThreadName threadName;
			threadName.type = 0x1000;
			threadName.name = _name;
			threadName.id = m_threadId;
			threadName.flags = 0;

			__try
			{
				RaiseException(0x406d1388
					, 0
					, sizeof(threadName) / 4
					, reinterpret_cast<ULONG_PTR*>(&threadName)
				);
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
			}
#else
			RIO_UNUSED(_name);
#endif // RIO_PLATFORM_
		}

	private:
		int32_t entry()
		{
#if RIO_PLATFORM_WINDOWS
			m_threadId = ::GetCurrentThreadId();
#endif // RIO_PLATFORM_WINDOWS

			m_sem.post();
			return this->threadFunction(m_userData);
		}

#if RIO_PLATFORM_WINDOWS || RIO_PLATFORM_XBOX360 || RIO_PLATFORM_WINRT
		static DWORD WINAPI threadProcedure(LPVOID argumentList)
		{
			Thread* thread = (Thread*)argumentList;
			int32_t result = thread->entry();
			return result;
		}
#else
		static void* threadProcedure(void* argumentList)
		{
			Thread* thread = (Thread*)argumentList;
			union
			{
				void* ptr;
				int32_t i;
			} cast;
			cast.i = thread->entry();
			return cast.ptr;
		}
#endif // RIO_PLATFORM_


	};

#if RIO_PLATFORM_WINDOWS
	class TlsData
	{
	private:
		uint32_t m_id;
	public:
		TlsData()
		{
			m_id = TlsAlloc();
			RIO_CHECK(TLS_OUT_OF_INDEXES != m_id, "Failed to allocated TLS index (err: 0x%08x).", GetLastError());
		}

		~TlsData()
		{
			BOOL result = TlsFree(m_id);
			RIO_CHECK(0 != result, "Failed to free TLS index (err: 0x%08x).", GetLastError()); RIO_UNUSED(result);
		}

		void* get() const
		{
			return TlsGetValue(m_id);
		}

		void set(void* ptr)
		{
			TlsSetValue(m_id, ptr);
		}
	};

#elif !(RIO_PLATFORM_XBOXONE || RIO_PLATFORM_WINRT)

	class TlsData
	{
	private:
		pthread_key_t m_id;
	public:
		TlsData()
		{
			int result = pthread_key_create(&m_id, NULL);
			RIO_CHECK(0 == result, "pthread_key_create failed %d.", result); RIO_UNUSED(result);
		}

		~TlsData()
		{
			int result = pthread_key_delete(m_id);
			RIO_CHECK(0 == result, "pthread_key_delete failed %d.", result); RIO_UNUSED(result);
		}

		void* get() const
		{
			return pthread_getspecific(m_id);
		}

		void set(void* ptr)
		{
			int result = pthread_setspecific(m_id, ptr);
			RIO_CHECK(0 == result, "pthread_setspecific failed %d.", result); RIO_UNUSED(result);
		}
	};
#endif // RIO_PLATFORM_*

} // namespace RioCore

#endif // RIO_CONFIG_SUPPORTS_THREADING
  // Copyright (c) 2016, 2017 Volodymyr Syvochka
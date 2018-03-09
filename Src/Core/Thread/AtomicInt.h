#pragma once

#include "Core/Platform.h"

#if RIO_PLATFORM_WINDOWS
	#include "Core/Types.h"
	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
#endif

namespace Rio
{

struct AtomicInt
{
#if RIO_PLATFORM_POSIX && RIO_COMPILER_GCC
	mutable int32_t val;
#elif RIO_PLATFORM_WINDOWS
	mutable LONG val;
#endif

	AtomicInt(int32_t val)
	{
		store(val);
	}

	int load() const
	{
#if RIO_PLATFORM_POSIX && RIO_COMPILER_GCC
		__sync_fetch_and_add(&(this->val), 0);
		return this->val;
#elif RIO_PLATFORM_WINDOWS
		InterlockedExchangeAdd(&(this->val), (int32_t)0);
		return this->val;
#endif
	}

	void store(int32_t val)
	{
#if RIO_PLATFORM_POSIX && RIO_COMPILER_GCC
		__sync_lock_test_and_set(&(this->val), val);
#elif RIO_PLATFORM_WINDOWS
		InterlockedExchange(&(this->val), val);
#endif
	}
};

} // namespace Rio

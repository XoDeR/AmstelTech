#pragma once

#include "Core/Base/RioCommon.h"

#if RIO_COMPILER_MSVC
#if RIO_PLATFORM_XBOX360
#include <ppcintrinsics.h>
#include <xtl.h>
#else
#include <math.h> 
// math.h is included :
// warning C4985: 'ceil': attributes not present on previous declaration.
// must be included before intrin.h.
#include <intrin.h>
#include <windows.h>
#endif // !RIO_PLATFORM_XBOX360
#if RIO_PLATFORM_WINRT
#define _InterlockedExchangeAdd64 InterlockedExchangeAdd64
#endif // RIO_PLATFORM_WINRT
extern "C" void _ReadBarrier();
extern "C" void _WriteBarrier();
extern "C" void _ReadWriteBarrier();
#pragma intrinsic(_ReadBarrier)
#pragma intrinsic(_WriteBarrier)
#pragma intrinsic(_ReadWriteBarrier)
#pragma intrinsic(_InterlockedExchangeAdd)
#pragma intrinsic(_InterlockedCompareExchange)
#endif // RIO_COMPILER_MSVC

namespace RioCore
{
	inline void readBarrier()
	{
#if RIO_COMPILER_MSVC
		_ReadBarrier();
#else
		asm volatile("":::"memory");
#endif // RIO_COMPILER
	}

	inline void writeBarrier()
	{
#if RIO_COMPILER_MSVC
		_WriteBarrier();
#else
		asm volatile("":::"memory");
#endif // RIO_COMPILER
	}

	inline void readWriteBarrier()
	{
#if RIO_COMPILER_MSVC
		_ReadWriteBarrier();
#else
		asm volatile("":::"memory");
#endif // RIO_COMPILER
	}

	inline void memoryBarrier()
	{
#if RIO_PLATFORM_XBOX360
		__lwsync();
#elif RIO_PLATFORM_WINRT
		MemoryBarrier();
#elif RIO_COMPILER_MSVC
		_mm_mfence();
#else
		__sync_synchronize();
//		asm volatile("mfence":::"memory");
#endif // RIO_COMPILER
	}

	template<typename Ty>
	inline Ty atomicFetchAndAdd(volatile Ty* ptr, Ty currentValue);

	template<typename Ty>
	inline Ty atomicAddAndFetch(volatile Ty* ptr, Ty currentValue);

	template<typename Ty>
	inline Ty atomicFetchAndSub(volatile Ty* ptr, Ty currentValue);

	template<typename Ty>
	inline Ty atomicSubAndFetch(volatile Ty* ptr, Ty currentValue);

	template<typename Ty>
	inline Ty atomicCompareAndSwap(volatile void* ptr, Ty oldValue, Ty newValue);

	template<>
	inline int32_t atomicCompareAndSwap(volatile void* ptr, int32_t oldValue, int32_t newValue);

	template<>
	inline int64_t atomicCompareAndSwap(volatile void* ptr, int64_t oldValue, int64_t newValue);

	template<>
	inline int32_t atomicFetchAndAdd<int32_t>(volatile int32_t* ptr, int32_t add)
	{
#if RIO_COMPILER_MSVC
		return _InterlockedExchangeAdd((volatile long*)ptr, add);
#else
		return __sync_fetch_and_add(ptr, add);
#endif // RIO_COMPILER_
	}

	template<>
	inline int64_t atomicFetchAndAdd<int64_t>(volatile int64_t* ptr, int64_t add)
	{
#if RIO_COMPILER_MSVC
#if _WIN32_WINNT >= 0x600
		return _InterlockedExchangeAdd64( (volatile int64_t*)ptr, add);
#else
		int64_t oldValue;
		int64_t newValue = *(int64_t volatile*)ptr;
		do
		{
            oldValue = newVal;
			newVal = atomicCompareAndSwap(ptr, oldValue, newValue + add);

		} 
        while (oldValue != newValue);

		return oldValue;
#endif
#else
		return __sync_fetch_and_add(ptr, add);
#endif // RIO_COMPILER_
	}

	template<>
	inline uint32_t atomicFetchAndAdd<uint32_t>(volatile uint32_t* ptr, uint32_t add)
	{
		return uint32_t(atomicFetchAndAdd<int32_t>((volatile int32_t*)ptr, int32_t(add)));
	}

	template<>
	inline uint64_t atomicFetchAndAdd<uint64_t>(volatile uint64_t* ptr, uint64_t add)
	{
		return uint64_t(atomicFetchAndAdd<int64_t>((volatile int64_t*)ptr, int64_t(add)));
	}

	template<>
	inline int32_t atomicAddAndFetch<int32_t>(volatile int32_t* ptr, int32_t add)
	{
#if RIO_COMPILER_MSVC
		return atomicFetchAndAdd(ptr, add) + add;
#else
		return __sync_add_and_fetch(ptr, add);
#endif // RIO_COMPILER_
	}

	template<>
	inline int64_t atomicAddAndFetch<int64_t>(volatile int64_t* ptr, int64_t add)
	{
#if RIO_COMPILER_MSVC
		return atomicFetchAndAdd(ptr, add) + add;
#else
		return __sync_add_and_fetch(ptr, add);
#endif // RIO_COMPILER_
	}

	template<>
	inline uint32_t atomicAddAndFetch<uint32_t>(volatile uint32_t* ptr, uint32_t add)
	{
		return uint32_t(atomicAddAndFetch<int32_t>( (volatile int32_t*)ptr, int32_t(add)));
	}

	template<>
	inline uint64_t atomicAddAndFetch<uint64_t>(volatile uint64_t* ptr, uint64_t add)
	{
		return uint64_t(atomicAddAndFetch<int64_t>( (volatile int64_t*)ptr, int64_t(add)));
	}

	template<>
	inline int32_t atomicFetchAndSub<int32_t>(volatile int32_t* ptr, int32_t sub)
	{
#if RIO_COMPILER_MSVC
		return atomicFetchAndAdd(ptr, -sub);
#else
		return __sync_fetch_and_sub(ptr, sub);
#endif // RIO_COMPILER_
	}

	template<>
	inline int64_t atomicFetchAndSub<int64_t>(volatile int64_t* ptr, int64_t sub)
	{
#if RIO_COMPILER_MSVC
		return atomicFetchAndAdd(ptr, -sub);
#else
		return __sync_fetch_and_sub(ptr, sub);
#endif // RIO_COMPILER_
	}

	template<>
	inline uint32_t atomicFetchAndSub<uint32_t>(volatile uint32_t* ptr, uint32_t add)
	{
		return uint32_t(atomicFetchAndSub<int32_t>((volatile int32_t*)ptr, int32_t(add)));
	}

	template<>
	inline uint64_t atomicFetchAndSub<uint64_t>(volatile uint64_t* ptr, uint64_t add)
	{
		return uint64_t(atomicFetchAndSub<int64_t>((volatile int64_t*)ptr, int64_t(add)));
	}

	template<>
	inline int32_t atomicSubAndFetch<int32_t>(volatile int32_t* ptr, int32_t sub)
	{
#if RIO_COMPILER_MSVC
		return atomicFetchAndAdd(ptr, -sub) - sub;
#else
		return __sync_sub_and_fetch(ptr, sub);
#endif // RIO_COMPILER_
	}

	template<>
	inline int64_t atomicSubAndFetch<int64_t>(volatile int64_t* ptr, int64_t sub)
	{
#if RIO_COMPILER_MSVC
		return atomicFetchAndAdd(ptr, -sub) - sub;
#else
		return __sync_sub_and_fetch(ptr, sub);
#endif // RIO_COMPILER_
	}

	template<>
	inline uint32_t atomicSubAndFetch<uint32_t>(volatile uint32_t* ptr, uint32_t add)
	{
		return uint32_t(atomicSubAndFetch<int32_t>((volatile int32_t*)ptr, int32_t(add)));
	}

	template<>
	inline uint64_t atomicSubAndFetch<uint64_t>(volatile uint64_t* ptr, uint64_t add)
	{
		return uint64_t(atomicSubAndFetch<int64_t>((volatile int64_t*)ptr, int64_t(add)));
	}

	// Returns the resulting incremented value
	template<typename Ty>
	inline Ty atomicInc(volatile Ty* ptr)
	{
		return atomicAddAndFetch(ptr, Ty(1));
	}

	// Returns the resulting decremented value
	template<typename Ty>
	inline Ty atomicDec(volatile Ty* ptr)
	{
		return atomicSubAndFetch(ptr, Ty(1));
	}

	template<>
	inline int32_t atomicCompareAndSwap(volatile void* ptr, int32_t oldValue, int32_t newValue)
	{
#if RIO_COMPILER_MSVC
		return _InterlockedCompareExchange( (volatile LONG*)(ptr), newValue, oldValue);
#else
		return __sync_val_compare_and_swap( (volatile int32_t*)ptr, oldValue, newValue);
#endif // RIO_COMPILER
	}

	template<>
	inline int64_t atomicCompareAndSwap(volatile void* ptr, int64_t oldValue, int64_t newValue)
	{
#if RIO_COMPILER_MSVC
		return _InterlockedCompareExchange64((volatile LONG64*)(ptr), newValue, oldValue);
#else
		return __sync_val_compare_and_swap( (volatile int64_t*)ptr, oldValue, newValue);
#endif // RIO_COMPILER
	}

	inline void* atomicExchangePtr(void** ptr, void* newPtr)
	{
#if RIO_COMPILER_MSVC
		return InterlockedExchangePointer(ptr, newPtr);
#else
		return __sync_lock_test_and_set(ptr, newPtr);
#endif // RIO_COMPILER
	}

	inline int32_t atomicTestAndInc(volatile void* ptr, int32_t test)
	{
		int32_t oldVal = 0;
		int32_t newVal = *(int32_t volatile*)ptr;
		do
		{
			oldVal = newVal;
			newVal = atomicCompareAndSwap(ptr, oldVal, newVal >= test ? test : newVal + 1);

		} 
        while (oldVal != newVal);

		return oldVal;
	}

	inline int32_t atomicTestAndDec(volatile void* ptr, int32_t test)
	{
		int32_t oldVal = 0;
		int32_t newVal = *(int32_t volatile*)ptr;
		do
		{
			oldVal = newVal;
			newVal = atomicCompareAndSwap(ptr, oldVal, newVal <= test ? test : newVal-1);

		} 
        while (oldVal != newVal);

		return oldVal;
	}

} // namespace RioCore

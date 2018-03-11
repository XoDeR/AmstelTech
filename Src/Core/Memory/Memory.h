#pragma once

#include "Core/Error/Error.h"
#include "Core/Memory/Allocator.h"
#include "Core/Types.h"

#include <new>

namespace Rio
{

Allocator& getDefaultAllocator();
Allocator& getDefaultScratchAllocator();

namespace Memory
{
	// Returns the pointer [p] aligned to the desired [align] byte
	inline void* getAlignedToTop(void* p, uint32_t align)
	{
		RIO_ASSERT(align >= 1, "Alignment must be > 1");
		RIO_ASSERT(align % 2 == 0 || align == 1, "Alignment must be a power of two");

		uintptr_t ptr = (uintptr_t)p;
		const uint32_t mod = ptr % align;

		if (mod)
		{
			ptr += align - mod;
		}

		return (void*)ptr;
	}

	// Respects standard behavior when calling on NULL [ptr]
	template <typename T>
	inline void callDestructorAndDeallocate(Allocator& a, T* ptr)
	{
		if (!ptr)
		{
			return;
		}

		ptr->~T();
		a.deallocate(ptr);
	}

} // namespace Memory

#define ALLOCATOR_AWARE typedef int32_t allocatorAwareMarker

// Convert integer to type
template <int32_t v>
struct Int2Type 
{ 
	enum 
	{
		value = v
	}; 
};

// Determines if a type is allocator aware
template <typename T>
struct IsAllocatorAware 
{
	template <typename C>
	static char testFunction(typename C::allocatorAwareMarker*);

	template <typename C>
	static int testFunction(...);

	enum 
	{
		value = (sizeof(testFunction<T>(0)) == sizeof(char))
	};
};

#define IS_ALLOCATOR_AWARE(T) IsAllocatorAware<T>::value
#define IS_ALLOCATOR_AWARE_TYPE(T) Int2Type<IS_ALLOCATOR_AWARE(T)>

// Allocator aware constuction
template <typename T> inline T& construct(void* p, Allocator& a, Int2Type<true>) 
{
	new (p) T(a);
	return *(T*)p;
}

template <typename T> inline T& construct(void* p, Allocator& /*a*/, Int2Type<false>) 
{
	new (p) T;
	return *(T*)p;
}

template <typename T> inline T& construct(void* p, Allocator& a) 
{
	return construct<T>(p, a, IS_ALLOCATOR_AWARE_TYPE(T)());
}

namespace MemoryGlobalFn
{
	// Constructs the initial default allocators
	// Has to be called before anything else during the application startup
	void init();

	// Destroys the allocators created with MemoryGlobalFn::init()
	// Should be the last call of the program
	void shutdown();

} // namespace MemoryGlobalFn

} // namespace Rio

// Allocates memory with <allocator> for the given <T> type and calls constructor on it
// <allocator> must be a reference to an existing allocator
#define RIO_NEW(allocator, T) new ((allocator).allocate(sizeof(T), alignof(T))) T

// Calls destructor on <ptr> and deallocates memory using the given <allocator>
// <allocator> must be a reference to an existing allocator
#define RIO_DELETE(allocator, ptr) Rio::Memory::callDestructorAndDeallocate(allocator, ptr)

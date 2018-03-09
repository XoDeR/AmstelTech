#pragma once

#include "Core/Types.h"

namespace Rio
{

// Interface for memory allocators
struct Allocator
{
	Allocator() 
	{
	}

	virtual	~Allocator() 
	{
	}

	Allocator(const Allocator&) = delete;
	Allocator& operator=(const Allocator&) = delete;

	// Allocates <size> bytes of memory aligned to the specified <align> byte 
	// Returns a pointer to the first allocated byte
	virtual void* allocate(uint32_t size, uint32_t align = DEFAULT_ALIGN) = 0;

	// Deallocates a previously allocated block of memory pointed by <data>
	virtual void deallocate(void* data) = 0;

	// Returns the size of the memory block pointed by <ptr> or SIZE_NOT_TRACKED if the allocator does not support memory tracking
	// <ptr> must be a pointer returned by Allocator::allocate()
	virtual uint32_t getAllocatedSize(const void* ptr) = 0;

	// Returns the total number of bytes allocated
	virtual uint32_t getTotalAllocatedBytes() = 0;

	// Default memory alignment in bytes
	static const uint32_t DEFAULT_ALIGN = 4;
	static const uint32_t SIZE_NOT_TRACKED = 0xffffffffu;
};

} // namespace Rio

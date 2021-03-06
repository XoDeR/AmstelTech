#pragma once

#include "Core/Memory/Allocator.h"

namespace Rio
{

// Allocates memory linearly from a fixed chunk of memory
// and frees all the allocations with a single call to clear()
struct LinearAllocator : public Allocator
{
	Allocator* backingAllocator = nullptr;
	void* physicalStart = nullptr;
	uint32_t totalSize = 0;
	uint32_t offset = 0;

	// Allocates <size> bytes from <backing>
	LinearAllocator(Allocator& backing, uint32_t size);

	// Uses <size> bytes of memory from <start>
	LinearAllocator(void* start, uint32_t size);
	~LinearAllocator();

	void* allocate(uint32_t size, uint32_t align = Allocator::DEFAULT_ALIGN);

	// The linear allocator does not support deallocating individual allocations
	// You have to call clear() to free all allocated memory at once
	void deallocate(void* data);

	// Frees all the allocations made by allocate()
	void clear();

	uint32_t getAllocatedSize(const void* /*ptr*/)
	{ 
		return SIZE_NOT_TRACKED; 
	}

	uint32_t getTotalAllocatedBytes() 
	{ 
		return this->offset; 
	}
};

} // namespace Rio

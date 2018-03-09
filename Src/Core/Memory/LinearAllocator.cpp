#include "Core/Memory/LinearAllocator.h"
#include "Core/Memory/Memory.h"

namespace Rio
{

LinearAllocator::LinearAllocator(Allocator& backingAllocator, uint32_t size)
	: backingAllocator(&backingAllocator)
	, totalSize(size)
{
	this->physicalStart = backingAllocator.allocate(size);
}

LinearAllocator::LinearAllocator(void* start, uint32_t size)
	: physicalStart(start)
	, totalSize(size)
{
}

LinearAllocator::~LinearAllocator()
{
	if (this->backingAllocator)
	{
		this->backingAllocator->deallocate(this->physicalStart);
	}

	RIO_ASSERT(this->offset == 0
		, "Memory leak of %d bytes, maybe you forgot to call clear()?"
		, this->offset
		);
}

void* LinearAllocator::allocate(uint32_t size, uint32_t align)
{
	const uint32_t actualSize = size + align;

	// Out of memory
	if (this->offset + actualSize > this->totalSize)
	{
		return nullptr;
	}

	void* userPtr = Memory::getAlignedToTop((char*)(this->physicalStart) + this->offset, align);

	this->offset += actualSize;

	return userPtr;
}

void LinearAllocator::deallocate(void* /*data*/)
{
	// Single deallocations not supported. Use clear()
}

void LinearAllocator::clear()
{
	this->offset = 0;
}

} // namespace Rio

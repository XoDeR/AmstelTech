#pragma once

#include "Core/Memory/Allocator.h"
#include "Core/Memory/Memory.h"

namespace Rio
{
	// A temporary memory allocator that primarily allocates memory from a local stack buffer of size BUFFER_SIZE
	// If that memory is exhausted it will use the backing allocator (typically a scratch allocator)
	// Memory allocated with a TempAllocator does not have to be deallocated
	// It is automatically deallocated when the TempAllocator is destroyed
	template <int BUFFER_SIZE>
	struct TempAllocator : public Allocator
	{
		// Local stack buffer for allocations
		char stackBuffer[BUFFER_SIZE];

		// Backing allocator if local memory is exhausted
		Allocator& backingAllocator;
		
		// Start of current allocation region
		char* currentStart = nullptr;
		
		// Current allocation pointer
		char* currentPtr = nullptr;					
		
		// End of current allocation region
		char* currentEnd = nullptr;
		
		// Chunks to allocate from backing allocator
		unsigned chunkSize;		

		// Creates a new temporary allocator using the specified backing allocator
		TempAllocator(Allocator& backing = getDefaultScratchAllocator());
		virtual ~TempAllocator();

		virtual void* allocate(uint32_t size, uint32_t align = DEFAULT_ALIGN);

		// Deallocation is a NOP for the TempAllocator
		// The memory is automatically deallocated when the TempAllocator is destroyed
		virtual void deallocate(void*) 
		{
		}

		// Returns SIZE_NOT_TRACKED
		virtual uint32_t getAllocatedSize(const void*)
		{
			return SIZE_NOT_TRACKED;
		}

		// Returns SIZE_NOT_TRACKED
		virtual uint32_t getTotalAllocatedBytes() 
		{
			return SIZE_NOT_TRACKED;
		}
	};

	// If possible, use one of these predefined sizes for the TempAllocator to avoid unnecessary template instantiation
	typedef TempAllocator<64> TempAllocator64;
	typedef TempAllocator<128> TempAllocator128;
	typedef TempAllocator<256> TempAllocator256;
	typedef TempAllocator<512> TempAllocator512;
	typedef TempAllocator<1024> TempAllocator1024;
	typedef TempAllocator<2048> TempAllocator2048;
	typedef TempAllocator<4096> TempAllocator4096;

	template <int BUFFER_SIZE>
	TempAllocator<BUFFER_SIZE>::TempAllocator(Allocator& backing) 
		: backingAllocator(backing)
		, chunkSize(4*1024)
	{
		this->currentPtr = currentStart = stackBuffer;
		this->currentEnd = currentStart + BUFFER_SIZE;
		*(void**)currentStart = 0;
		this->currentPtr += sizeof(void*);
	}

	template <int BUFFER_SIZE>
	TempAllocator<BUFFER_SIZE>::~TempAllocator()
	{
		char* start = this->stackBuffer;
		void* p = *(void**)start;
		while (p) 
		{
			void* next = *(void**)p;
			this->backingAllocator.deallocate(p);
			p = next;
		}
	}

	template <int BUFFER_SIZE>
	void* TempAllocator<BUFFER_SIZE>::allocate(uint32_t size, uint32_t align)
	{
		currentPtr = (char*)Memory::getAlignedToTop(currentPtr, align);
		if ((int)size > currentEnd - currentPtr)
		{
			uint32_t toAllocate = sizeof(void *) + size + align;
			if (toAllocate < chunkSize)
			{
				toAllocate = chunkSize;
			}
			chunkSize *= 2;
			void* p = backingAllocator.allocate(toAllocate);
			*(void**)currentStart = p;
			currentPtr = currentStart = (char*)p;
			currentEnd = currentStart + toAllocate;
			*(void**)currentStart = 0;
			currentPtr += sizeof(void*);
			currentPtr = (char*)Memory::getAlignedToTop(currentPtr, align);
		}
		void* result = currentPtr;
		currentPtr += size;
		return result;
	}

} // namespace Rio

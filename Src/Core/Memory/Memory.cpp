#include "Core/Memory/Allocator.h"
#include "Core/Memory/Memory.h"
#include "Core/Thread/Mutex.h"

#include <stdlib.h> // malloc

namespace Rio
{

namespace Memory
{
	// Header stored at the beginning of a memory allocation to indicate the size of the allocated data
	struct Header
	{
		uint32_t size;
	};

	// If we need to align the memory allocation we pad the header with this value after storing the size
	const uint32_t HEADER_PAD_VALUE = 0xffffffffu;

	// Given a pointer to the header, returns a pointer to the data that follows it
	inline void* getDataPointer(Header* header, uint32_t align)
	{
		void* p = header + 1;
		return Memory::getAlignedToTop(p, align);
	}

	// Given a pointer to the data, returns a pointer to the header before it
	inline Header* getHeader(const void* data)
	{
		uint32_t* p = (uint32_t*)data;
		while (p[-1] == HEADER_PAD_VALUE)
		{
			--p;
		}
		return (Header*)p - 1;
	}

	// Stores the size in the header and pads with HEADER_PAD_VALUE up to the data pointer
	inline void fill(Header* header, void* data, uint32_t size)
	{
		header->size = size;
		uint32_t *p = (uint32_t*)(header + 1);
		while (p < data)
		{
			*p++ = HEADER_PAD_VALUE;
		}
	}

	inline uint32_t getActualAllocationSize(uint32_t size, uint32_t align)
	{
		return size + align + sizeof(Header);
	}

	inline void pad(Header* header, void* data)
	{
		uint32_t* p = (uint32_t*)(header + 1);

		while (p != data)
		{
			*p = HEADER_PAD_VALUE;
			p++;
		}
	}

	// Allocator based on C malloc()
	struct HeapAllocator : public Allocator
	{
		Mutex mutex;
		uint32_t allocatedSize = 0;
		uint32_t allocationCount = 0;

		HeapAllocator()
		{
		}

		~HeapAllocator()
		{
			RIO_ASSERT(allocationCount == 0 && getTotalAllocatedBytes() == 0
				, "Missing %d deallocations causing a leak of %d bytes"
				, allocationCount
				, getTotalAllocatedBytes()
				);
		}

		void* allocate(uint32_t size, uint32_t align = Allocator::DEFAULT_ALIGN)
		{
			ScopedMutex scopedMutex(mutex);

			uint32_t actualSize = getActualAllocationSize(size, align);

			Header* h = (Header*)malloc(actualSize);
			h->size = actualSize;

			void* data = Memory::getAlignedToTop(h + 1, align);

			pad(h, data);

			allocatedSize += actualSize;
			++allocationCount;

			return data;
		}

		void deallocate(void* data)
		{
			ScopedMutex scopedMutex(mutex);

			if (!data)
			{
				return;
			}

			Header* h = getHeader(data);

			allocatedSize -= h->size;
			--allocationCount;

			free(h);
		}

		uint32_t getAllocatedSize(const void* ptr)
		{
			return getSizeOfBlock(ptr);
		}

		uint32_t getTotalAllocatedBytes()
		{
			ScopedMutex scopedMutex(mutex);
			return allocatedSize;
		}

		// Returns the size in bytes of the block of memory pointed by <data>
		uint32_t getSizeOfBlock(const void* data)
		{
			ScopedMutex scopedMutex(mutex);
			Header* h = getHeader(data);
			return h->size;
		}
	};

	// An allocator used to allocate temporary "scratch" memory
	// The allocator uses a fixed size ring buffer to services the requests

	// Memory is always always allocated linearly
	// An allocation pointer is advanced through the buffer 
	// as memory is allocated and wraps around at the end of the buffer
	// Similarly, a free pointer is advanced as memory is freed

	// It is important that the scratch allocator is only used for short-lived memory allocations
	// A long lived allocator will lock the "free" pointer and prevent the "allocate" pointer from proceeding past it, 
	// which means the ring buffer can't be used

	// If the ring buffer is exhausted, the scratch allocator will use its 
	// backing allocator to allocate memory instead
	struct ScratchAllocator : public Allocator
	{
		Mutex mutex;
		Allocator& backingAllocator;

		// Start and end of the ring buffer
		char* bufferBegin = nullptr;
		char* bufferEnd = nullptr;

		// Pointers to where to allocate memory and where to free memory
		char* whereToAllocate = nullptr;
		char* whereToFree = nullptr;

		// Creates a ScratchAllocator
		// The allocator will use the <backingAllocator> to create the ring buffer 
		// and to service any requests that don't fit in the ring buffer
		// <size> specifies the size of the ring buffer
		ScratchAllocator(Allocator& backingAllocator, uint32_t size)
			: backingAllocator(backingAllocator)
		{
			bufferBegin = (char*)(this->backingAllocator).allocate(size);
			bufferEnd = bufferBegin + size;
			whereToAllocate = bufferBegin;
			whereToFree = bufferBegin;
		}

		~ScratchAllocator()
		{
			RIO_ASSERT(whereToFree == whereToAllocate, "Memory leak");
			backingAllocator.deallocate(bufferBegin);
		}

		bool getIsInUse(void* p)
		{
			if (whereToFree == whereToAllocate)
			{
				return false;
			}
			if (whereToAllocate > whereToFree)
			{
				return p >= whereToFree && p < whereToAllocate;
			}
			return p >= whereToFree || p < whereToAllocate;
		}

		void* allocate(uint32_t size, uint32_t align)
		{
			ScopedMutex scopedMutex(mutex);

			RIO_ASSERT(align % 4 == 0, "Must be 4-byte aligned");
			size = ((size + 3)/4)*4;

			char* p = whereToAllocate;
			Header* h = (Header*)p;
			char* data = (char*)getDataPointer(h, align);
			p = data + size;

			// Reached the end of the buffer, wrap around to the beginning
			if (p > bufferEnd)
			{
				h->size = uint32_t(bufferEnd - (char*)h) | 0x80000000u;

				p = bufferBegin;
				h = (Header*)p;
				data = (char*)getDataPointer(h, align);
				p = data + size;
			}

			// If the buffer is exhausted use the backing allocator instead
			if (getIsInUse(p))
			{
				return backingAllocator.allocate(size, align);
			}

			fill(h, data, uint32_t(p - (char*)h));
			whereToAllocate = p;
			return data;
		}

		void deallocate(void *p)
		{
			ScopedMutex scopedMutex(mutex);

			if (!p)
			{
				return;
			}

			if (p < bufferBegin || p >= bufferEnd)
			{
				backingAllocator.deallocate(p);
				return;
			}

			// Mark this slot as free
			Header* h = getHeader(p);
			RIO_ASSERT((h->size & 0x80000000u) == 0, "Not free");
			h->size = h->size | 0x80000000u;

			// Advance the free pointer past all free slots
			while (whereToFree != whereToAllocate)
			{
				Header* h = (Header*)whereToFree;
				if ((h->size & 0x80000000u) == 0)
				{
					break;
				}

				whereToFree += h->size & 0x7fffffffu;
				if (whereToFree == bufferEnd)
				{
					whereToFree = bufferBegin;
				}
			}
		}

		uint32_t getAllocatedSize(const void* p)
		{
			ScopedMutex scopedMutex(mutex);
			Header* h = getHeader(p);
			return h->size - uint32_t((char*)p - (char*)h);
		}

		uint32_t getTotalAllocatedBytes()
		{
			ScopedMutex scopedMutex(mutex);
			return uint32_t(bufferEnd - bufferBegin);
		}
	};

} // namespace Memory

namespace MemoryGlobalFn
{
	using namespace Memory;

	static const uint32_t auxBufferSize = sizeof(HeapAllocator) + sizeof(ScratchAllocator);
	char auxBuffer[auxBufferSize];
	HeapAllocator* defaultAllocator = nullptr;
	ScratchAllocator* defaultScratchAllocator = nullptr;

	void init()
	{
		defaultAllocator = new (auxBuffer) HeapAllocator();
		defaultScratchAllocator = new (auxBuffer + sizeof(HeapAllocator)) ScratchAllocator(*defaultAllocator, 1024*1024);
	}

	void shutdown()
	{
		defaultScratchAllocator->~ScratchAllocator();
		defaultAllocator->~HeapAllocator();
	}

} // namespace MemoryGlobalFn

Allocator& getDefaultAllocator()
{
	return *(MemoryGlobalFn::defaultAllocator);
}

Allocator& getDefaultScratchAllocator()
{
	return *(MemoryGlobalFn::defaultScratchAllocator);
}

} // namespace Rio

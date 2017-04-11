// Copyright (c) 2016, 2017 Volodymyr Syvochka
#pragma once

#include "Core/Base/Types.h"
#include "Core/Base/RioCommon.h"

//#include <memory.h>
#include <string.h> //::memmove
#include <new>

#if RIO_CONFIG_ALLOCATOR_DEBUG
#define RIO_ALLOC(allocator, size) RioCore::alloc(allocator, size, 0, __FILE__, __LINE__)
#define RIO_REALLOC(allocator, ptr, size) RioCore::realloc(allocator, ptr, size, 0, __FILE__, __LINE__)
#define RIO_FREE(_allocator, _ptr) RioCore::free(_allocator, _ptr, 0, __FILE__, __LINE__)
#define RIO_ALIGNED_ALLOC(_allocator, _size, _align) RioCore::alloc(_allocator, _size, _align, __FILE__, __LINE__)
#define RIO_ALIGNED_REALLOC(_allocator, _ptr, _size, _align) RioCore::realloc(_allocator, _ptr, _size, _align, __FILE__, __LINE__)
#define RIO_ALIGNED_FREE(_allocator, _ptr, _align) RioCore::free(_allocator, _ptr, _align, __FILE__, __LINE__)
#define RIO_NEW(_allocator, _type) ::new(RIO_ALLOC(_allocator, sizeof(_type))) _type
#define RIO_DELETE(_allocator, _ptr) RioCore::deleteObject(_allocator, _ptr, 0, __FILE__, __LINE__)
#define RIO_ALIGNED_NEW(_allocator, _type, _align) ::new(RIO_ALIGNED_ALLOC(_allocator, sizeof(_type), _align)) _type
#define RIO_ALIGNED_DELETE(_allocator, _ptr, _align) RioCore::deleteObject(_allocator, _ptr, _align, __FILE__, __LINE__)

#else
#define RIO_ALLOC(allocator, size) RioCore::alloc(allocator, size, 0)
#define RIO_REALLOC(_allocator, _ptr, _size) RioCore::realloc(_allocator, _ptr, _size, 0)
#define RIO_FREE(_allocator, _ptr) RioCore::free(_allocator, _ptr, 0)
#define RIO_ALIGNED_ALLOC(_allocator, _size, _align) RioCore::alloc(_allocator, _size, _align)
#define RIO_ALIGNED_REALLOC(_allocator, _ptr, _size, _align) RioCore::realloc(_allocator, _ptr, _size, _align)
#define RIO_ALIGNED_FREE(_allocator, _ptr, _align) RioCore::free(_allocator, _ptr, _align)
#define RIO_NEW(_allocator, _type) ::new(RIO_ALLOC(_allocator, sizeof(_type))) _type
#define RIO_DELETE(_allocator, _ptr) RioCore::deleteObject(_allocator, _ptr, 0)
#define RIO_ALIGNED_NEW(_allocator, _type, _align) ::new(RIO_ALIGNED_ALLOC(_allocator, sizeof(_type), _align)) _type
#define RIO_ALIGNED_DELETE(_allocator, _ptr, _align) RioCore::deleteObject(_allocator, _ptr, _align)
#endif // RIO_CONFIG_DEBUG_ALLOC

// Allocates memory with <allocator> for the given <T> type and calls constructor on it
// <allocator> must be a reference to an existing allocator
#define AMSTEL_NEW(allocator, T) new ((allocator).allocate(sizeof(T), alignof(T))) T

// Calls destructor on the <ptr> and deallocates memory using the <allocator>
// The allocator must be a reference to an existing allocator
#define AMSTEL_DELETE(allocator, ptr) RioCore::MemoryFn::callDestructorAndDeallocate(allocator, ptr)

#ifndef RIO_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT
#define RIO_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT 8
#endif // RIO_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT

namespace RioCore
{

    class Allocator
    {
    public:
        Allocator() 
		{}
        virtual	~Allocator() 
		{}

        virtual void* allocate(uint32_t size, uint32_t align = DEFAULT_ALIGN) = 0;
        virtual void deallocate(void* data) = 0;
        virtual uint32_t getAllocatedSize(const void* ptr) = 0;
        virtual uint32_t getTotalAllocatedBytes() = 0;

        // Default memory alignment in bytes
        static const uint32_t DEFAULT_ALIGN = 4;
        static const uint32_t SIZE_NOT_TRACKED = 0xffffffffu;
    private:
        // Disable copying
        Allocator(const Allocator&) = delete;
        Allocator& operator=(const Allocator&) = delete;
    };

    // Aligns pointer to nearest next aligned address. _align must be power of two
    inline void* alignPtr(void* _ptr, size_t extra, size_t _align = RIO_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT)
    {
        union 
		{ 
			void* ptr; 
			size_t addr; 
		} un;
        un.ptr = _ptr;
        size_t unaligned = un.addr + extra; // space for header
        size_t mask = _align - 1;
        size_t aligned = RIO_ALIGN_MASK(unaligned, mask);
        un.addr = aligned;
        return un.ptr;
    }

    struct RIO_NO_VTABLE AllocatorI
    {
        virtual ~AllocatorI() = 0;

        // Allocated, resizes memory block or frees memory.
        // ptr If _ptr is NULL new block will be allocated.
        // size If _ptr is set, and _size is 0, memory will be freed.
        // align Alignment.
        // file Debug file path info.
        // line Debug file line info.
        virtual void* realloc(void* ptr, size_t size, size_t align, const char* file, uint32_t line) = 0;
    };

    inline AllocatorI::~AllocatorI()
    {
    }

    inline void* alloc(AllocatorI* allocator, size_t size, size_t align = 0, const char* file = nullptr, uint32_t line = 0)
    {
        return allocator->realloc(nullptr, size, align, file, line);
    }

    inline void free(AllocatorI* allocator, void* ptr, size_t align = 0, const char* file = nullptr, uint32_t line = 0)
    {
        allocator->realloc(ptr, 0, align, file, line);
    }

    inline void* realloc(AllocatorI* _allocator, void* _ptr, size_t _size, size_t _align = 0, const char* _file = nullptr, uint32_t _line = 0)
    {
        return _allocator->realloc(_ptr, _size, _align, _file, _line);
    }

    static inline void* alignedAlloc(AllocatorI* _allocator, size_t _size, size_t _align, const char* _file = nullptr, uint32_t _line = 0)
    {
        size_t total = _size + _align;
        uint8_t* ptr = (uint8_t*)alloc(_allocator, total, 0, _file, _line);
        uint8_t* aligned = (uint8_t*)alignPtr(ptr, sizeof(uint32_t), _align);
        uint32_t* header = (uint32_t*)aligned - 1;
        *header = uint32_t(aligned - ptr);
        return aligned;
    }

    static inline void alignedFree(AllocatorI* _allocator, void* _ptr, size_t /*_align*/, const char* _file = nullptr, uint32_t _line = 0)
    {
        uint8_t* aligned = (uint8_t*)_ptr;
        uint32_t* header = (uint32_t*)aligned - 1;
        uint8_t* ptr = aligned - *header;
        free(_allocator, ptr, 0, _file, _line);
    }

    static inline void* alignedRealloc(AllocatorI* _allocator, void* _ptr, size_t _size, size_t _align, const char* _file = nullptr, uint32_t _line = 0)
    {
        if (nullptr == _ptr)
        {
            return alignedAlloc(_allocator, _size, _align, _file, _line);
        }

        uint8_t* aligned = (uint8_t*)_ptr;
        uint32_t offset = *((uint32_t*)aligned - 1);
        uint8_t* ptr = aligned - offset;
        size_t total = _size + _align;
        ptr = (uint8_t*)realloc(_allocator, ptr, total, 0, _file, _line);
        uint8_t* newAligned = (uint8_t*)alignPtr(ptr, sizeof(uint32_t), _align);

        if (newAligned == aligned)
        {
            return aligned;
        }

        aligned = ptr + offset;
        ::memmove(newAligned, aligned, _size);
        uint32_t* header = (uint32_t*)newAligned - 1;
        *header = uint32_t(newAligned - ptr);
        return newAligned;
    }

    template <typename ObjectT>
    inline void deleteObject(AllocatorI* allocator, ObjectT* object, size_t align = 0, const char* file = nullptr, uint32_t line = 0)
    {
        if (nullptr != object)
        {
            object->~ObjectT();
            free(allocator, object, align, file, line);
        }
    }

} // namespace RioCore
// Copyright (c) 2016, 2017 Volodymyr Syvochka
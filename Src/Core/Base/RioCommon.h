#pragma once

#include <stdint.h> // uint32_t
#include <stdlib.h> // size_t
#include <string.h> // memcpy

#include "Core/Base/RioCoreConfig.h"
#include "Core/Base/Macros.h"

namespace RioCore
{
    template<typename T, size_t N> char(&COUNTOF_REQUIRES_ARRAY_ARGUMENT(const T(&)[N]))[N];
#define RIO_COUNTOF(_x) sizeof(RioCore::COUNTOF_REQUIRES_ARRAY_ARGUMENT(_x))

    // Template for avoiding MSVC: C4127: conditional expression is constant
    template<bool>
    inline bool isEnabled()
    {
        return true;
    }

    template<>
    inline bool isEnabled<false>()
    {
        return false;
    }
#define RIO_ENABLED(_x) RioCore::isEnabled<!!(_x)>()

    inline bool ignoreC4127(bool _x)
    {
        return _x;
    }
#define RIO_IGNORE_C4127(_x) RioCore::ignoreC4127(!!(_x))

    template<typename Ty>
    inline void xchg(Ty& a, Ty& b)
    {
        Ty tmp = a;
		a = b;
		b = tmp;
    }

    // Check if pointer is aligned. align must be power of two
    inline bool isPtrAligned(const void* ptr, size_t align)
    {
        union
        {
            const void* ptr;
            size_t addr;
        } un;
        un.ptr = ptr;
        return 0 == (un.addr & (align - 1));
    }

    // Scatter/gather memcpy
    inline void memCopy(void* dst, const void* src, uint32_t size, uint32_t count, uint32_t srcPitch, uint32_t dstPitch)
    {
        const uint8_t* srcLocal = (const uint8_t*)src;
        uint8_t* dstLocal = (uint8_t*)dst;

        for (uint32_t ii = 0; ii < count; ++ii)
        {
            memcpy(dstLocal, srcLocal, size);
			srcLocal += srcPitch;
			dstLocal += dstPitch;
        }
    }

    inline void gather(void* dst, const void* src, uint32_t size, uint32_t num, uint32_t srcPitch)
    {
        memCopy(dst, src, size, num, srcPitch, size);
    }

    inline void scatter(void* dst, const void* src, uint32_t size, uint32_t num, uint32_t dstPitch)
    {
        memCopy(dst, src, size, num, size, dstPitch);
    }

} // namespace RioCore

// For deprecated C++0x stuff
namespace std
{
    namespace tr1
    {
    };
    using namespace tr1;
}

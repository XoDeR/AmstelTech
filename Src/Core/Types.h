#pragma once

#include <cstdint>

#if defined(_MSC_VER)
	#define _ALLOW_KEYWORD_MACROS
#endif

#if !defined(alignof)
	#define alignof(x) __alignof(x)
#endif

#if !defined(__va_copy)
	#define __va_copy(dest, src) (dest = src)
#endif

#ifndef NULL
	#define NULL 0
#endif

#define countof(arr) (sizeof(arr)/sizeof(arr[0]))

#define RIO_NOOP(...) do { (void)0; } while (0)
#define RIO_UNUSED(x) do { (void)(x); } while (0)
#define RIO_STATIC_ASSERT(condition, ...) static_assert(condition, "" # __VA_ARGS__)

#if defined(__GNUC__)
	#define RIO_THREAD __thread
#elif defined(_MSC_VER)
	#define RIO_THREAD __declspec(thread)
#else
	#error "Compiler not supported"
#endif

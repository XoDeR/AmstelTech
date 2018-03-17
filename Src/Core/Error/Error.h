#pragma once

#include "Core/Config.h"
#include "Core/Types.h"

namespace Rio
{

// Error management
namespace ErrorFn
{
	// Aborts the program execution logging an error message and a stacktrace if the current platform supports it
	void abort(const char* format, ...);

} // namespace ErrorFn

} // namespace Rio

#if RIO_DEBUG
	#define RIO_ASSERT(condition, message, ...)              \
		do                                                   \
		{                                                    \
			if (!(condition))                                \
			{                                                \
				Rio::ErrorFn::abort("Assertion failed: %s\n" \
					"    In: %s:%d\n"                        \
					"    " message                           \
					, # condition                            \
					, __FILE__                               \
					, __LINE__                               \
					, ## __VA_ARGS__                         \
					);                                       \
			}                                                \
		} while (0)
#else
	#define RIO_ASSERT(...) RIO_NOOP()
#endif // RIO_DEBUG

#define RIO_FATAL(message, ...) RIO_ASSERT(false, message, ## __VA_ARGS__)
#define RIO_ENSURE(condition) RIO_ASSERT(condition, "")

// Copyright (c) 2016, 2017 Volodymyr Syvochka
#pragma once

#if RIO_COMPILER_MSVC
// Workaround MSVS bug
#define RIO_VA_ARGS_PASS(...) RIO_VA_ARGS_PASS_1_ __VA_ARGS__ RIO_VA_ARGS_PASS_2_
#define RIO_VA_ARGS_PASS_1_ (
#define RIO_VA_ARGS_PASS_2_ )
#else
#define RIO_VA_ARGS_PASS(...) (__VA_ARGS__)
#endif // RIO_COMPILER_MSVC

#define RIO_VA_ARGS_COUNT(...) RIO_VA_ARGS_COUNT_ RIO_VA_ARGS_PASS(__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
#define RIO_VA_ARGS_COUNT_(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10, _a11, _a12, _a13, _a14, _a15, _a16, _last, ...) _last

#define RIO_MACRO_DISPATCHER(_func, ...) RIO_MACRO_DISPATCHER_1_(_func, RIO_VA_ARGS_COUNT(__VA_ARGS__) )
#define RIO_MACRO_DISPATCHER_1_(_func, _argCount) RIO_MACRO_DISPATCHER_2_(_func, _argCount)
#define RIO_MACRO_DISPATCHER_2_(_func, _argCount) RIO_CONCATENATE(_func, _argCount)

#define RIO_MAKEFOURCC(_a, _b, _c, _d) ( ( (uint32_t)(_a) | ( (uint32_t)(_b) << 8) | ( (uint32_t)(_c) << 16) | ( (uint32_t)(_d) << 24) ) )

#define RIO_STRINGIZE(_x) RIO_STRINGIZE_(_x)
#define RIO_STRINGIZE_(_x) #_x

#define RIO_CONCATENATE(_x, _y) RIO_CONCATENATE_(_x, _y)
#define RIO_CONCATENATE_(_x, _y) _x ## _y

#define RIO_FILE_LINE_LITERAL "" __FILE__ "(" RIO_STRINGIZE(__LINE__) "): "

#define RIO_ALIGN_MASK(_value, _mask) ( ( (_value)+(_mask) ) & ( (~0)&(~(_mask) ) ) )
#define RIO_ALIGN_16(_value) RIO_ALIGN_MASK(_value, 0xf)
#define RIO_ALIGN_256(_value) RIO_ALIGN_MASK(_value, 0xff)
#define RIO_ALIGN_4096(_value) RIO_ALIGN_MASK(_value, 0xfff)

#define RIO_ALIGNOF(_type) __alignof(_type)

#if defined(__has_feature)
#define RIO_CLANG_HAS_FEATURE(_x) __has_feature(_x)
#else
#define RIO_CLANG_HAS_FEATURE(_x) 0
#endif // defined(__has_feature)

#if defined(__has_extension)
#define RIO_CLANG_HAS_EXTENSION(_x) __has_extension(_x)
#else
#define RIO_CLANG_HAS_EXTENSION(_x) 0
#endif // defined(__has_extension)

#if RIO_COMPILER_GCC || RIO_COMPILER_CLANG
#define RIO_ALIGN_DECL(_align, _decl) _decl __attribute__((aligned(_align)))
#define RIO_ALLOW_UNUSED __attribute__( (unused) )
#define RIO_FORCE_INLINE inline __attribute__((__always_inline__))
#define RIO_FUNCTION __PRETTY_FUNCTION__
#define RIO_LIKELY(_x)   __builtin_expect(!!(_x), 1)
#define RIO_UNLIKELY(_x) __builtin_expect(!!(_x), 0)
#define RIO_NO_INLINE __attribute__( (noinline) )
#define RIO_NO_RETURN __attribute__( (noreturn) )
#define RIO_NO_VTABLE
#define RIO_OVERRIDE
#define RIO_PRINTF_ARGS(_format, _args) __attribute__( (format(__printf__, _format, _args)))
#if RIO_CLANG_HAS_FEATURE(cxx_thread_local)
#define RIO_THREAD_LOCAL __thread
#endif // RIO_COMPILER_CLANG
#if (!RIO_PLATFORM_OSX && (RIO_COMPILER_GCC >= 40200)) || (RIO_COMPILER_GCC >= 40500)
#define RIO_THREAD_LOCAL __thread
#endif // RIO_COMPILER_GCC
#define RIO_ATTRIBUTE(_x) __attribute__( (_x) )
#if RIO_CRT_MSVC
#define __stdcall
#endif // RIO_CRT_MSVC
#elif RIO_COMPILER_MSVC
#define RIO_ALIGN_DECL(_align, _decl) __declspec(align(_align) ) _decl
#define RIO_ALLOW_UNUSED
#define RIO_FORCE_INLINE __forceinline
#define RIO_FUNCTION __FUNCTION__
#define RIO_LIKELY(_x)   (_x)
#define RIO_UNLIKELY(_x) (_x)
#define RIO_NO_INLINE __declspec(noinline)
#define RIO_NO_RETURN
#define RIO_NO_VTABLE __declspec(novtable)
#define RIO_OVERRIDE override
#define RIO_PRINTF_ARGS(_format, _args)
#define RIO_THREAD_LOCAL __declspec(thread)
#define RIO_ATTRIBUTE(_x)
#else
#error "Unknown RIO_COMPILER_?"
#endif

// #define RIO_STATIC_ASSERT(_condition, ...) static_assert(_condition, "" __VA_ARGS__)
#define RIO_STATIC_ASSERT(_condition, ...) typedef char RIO_CONCATENATE(RIO_STATIC_ASSERT, __LINE__)[1][(_condition)] RIO_ATTRIBUTE(unused)

#define RIO_ALIGN_DECL_16(_decl) RIO_ALIGN_DECL(16, _decl)
#define RIO_ALIGN_DECL_256(_decl) RIO_ALIGN_DECL(256, _decl)
#define RIO_ALIGN_DECL_CACHE_LINE(_decl) RIO_ALIGN_DECL(RIO_CACHE_LINE_SIZE, _decl)

#define RIO_MACRO_BLOCK_BEGIN for(;;) {
#define RIO_MACRO_BLOCK_END break; }
#define RIO_NOOP(...) RIO_MACRO_BLOCK_BEGIN RIO_MACRO_BLOCK_END

#define RIO_UNUSED_1(_a1) RIO_MACRO_BLOCK_BEGIN (void)(true ? (void)0 : ((void)(_a1))); RIO_MACRO_BLOCK_END
#define RIO_UNUSED_2(_a1, _a2) RIO_UNUSED_1(_a1); RIO_UNUSED_1(_a2)
#define RIO_UNUSED_3(_a1, _a2, _a3) RIO_UNUSED_2(_a1, _a2); RIO_UNUSED_1(_a3)
#define RIO_UNUSED_4(_a1, _a2, _a3, _a4) RIO_UNUSED_3(_a1, _a2, _a3); RIO_UNUSED_1(_a4)
#define RIO_UNUSED_5(_a1, _a2, _a3, _a4, _a5) RIO_UNUSED_4(_a1, _a2, _a3, _a4); RIO_UNUSED_1(_a5)
#define RIO_UNUSED_6(_a1, _a2, _a3, _a4, _a5, _a6) RIO_UNUSED_5(_a1, _a2, _a3, _a4, _a5); RIO_UNUSED_1(_a6)
#define RIO_UNUSED_7(_a1, _a2, _a3, _a4, _a5, _a6, _a7) RIO_UNUSED_6(_a1, _a2, _a3, _a4, _a5, _a6); RIO_UNUSED_1(_a7)
#define RIO_UNUSED_8(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8) RIO_UNUSED_7(_a1, _a2, _a3, _a4, _a5, _a6, _a7); RIO_UNUSED_1(_a8)
#define RIO_UNUSED_9(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9) RIO_UNUSED_8(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8); RIO_UNUSED_1(_a9)
#define RIO_UNUSED_10(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10) RIO_UNUSED_9(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9); RIO_UNUSED_1(_a10)
#define RIO_UNUSED_11(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10, _a11) RIO_UNUSED_10(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10); RIO_UNUSED_1(_a11)
#define RIO_UNUSED_12(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10, _a11, _a12) RIO_UNUSED_11(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10, _a11); RIO_UNUSED_1(_a12)

#if RIO_COMPILER_MSVC
// Workaround MSVS bug...
#define RIO_UNUSED(...) RIO_MACRO_DISPATCHER(RIO_UNUSED_, __VA_ARGS__) RIO_VA_ARGS_PASS(__VA_ARGS__)
#else
#define RIO_UNUSED(...) RIO_MACRO_DISPATCHER(RIO_UNUSED_, __VA_ARGS__)(__VA_ARGS__)
#endif // RIO_COMPILER_MSVC

#if RIO_COMPILER_CLANG
#define RIO_PRAGMA_DIAGNOSTIC_PUSH_CLANG_()     _Pragma("clang diagnostic push")
#define RIO_PRAGMA_DIAGNOSTIC_POP_CLANG_()      _Pragma("clang diagnostic pop")
#define RIO_PRAGMA_DIAGNOSTIC_IGNORED_CLANG(_x) _Pragma(RIO_STRINGIZE(clang diagnostic ignored _x))
#else
#define RIO_PRAGMA_DIAGNOSTIC_PUSH_CLANG_()
#define RIO_PRAGMA_DIAGNOSTIC_POP_CLANG_()
#define RIO_PRAGMA_DIAGNOSTIC_IGNORED_CLANG(_x)
#endif // RIO_COMPILER_CLANG

#if RIO_COMPILER_GCC && RIO_COMPILER_GCC >= 40600
#define RIO_PRAGMA_DIAGNOSTIC_PUSH_GCC_() _Pragma("GCC diagnostic push")
#define RIO_PRAGMA_DIAGNOSTIC_POP_GCC_() _Pragma("GCC diagnostic pop")
#define RIO_PRAGMA_DIAGNOSTIC_IGNORED_GCC(_x) _Pragma(RIO_STRINGIZE(GCC diagnostic ignored _x))
#else
#define RIO_PRAGMA_DIAGNOSTIC_PUSH_GCC_()
#define RIO_PRAGMA_DIAGNOSTIC_POP_GCC_()
#define RIO_PRAGMA_DIAGNOSTIC_IGNORED_GCC(_x)
#endif // RIO_COMPILER_GCC

#if RIO_COMPILER_MSVC
#define RIO_PRAGMA_DIAGNOSTIC_PUSH_MSVC_() __pragma(warning(push))
#define RIO_PRAGMA_DIAGNOSTIC_POP_MSVC_() __pragma(warning(pop))
#define RIO_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(_x) __pragma(warning(disable:_x))
#else
#define RIO_PRAGMA_DIAGNOSTIC_PUSH_MSVC_()
#define RIO_PRAGMA_DIAGNOSTIC_POP_MSVC_()
#define RIO_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(_x)
#endif // RIO_COMPILER_CLANG

#if RIO_COMPILER_CLANG
#define RIO_PRAGMA_DIAGNOSTIC_PUSH RIO_PRAGMA_DIAGNOSTIC_PUSH_CLANG_
#define RIO_PRAGMA_DIAGNOSTIC_POP RIO_PRAGMA_DIAGNOSTIC_POP_CLANG_
#define RIO_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC RIO_PRAGMA_DIAGNOSTIC_IGNORED_CLANG
#elif RIO_COMPILER_GCC
#define RIO_PRAGMA_DIAGNOSTIC_PUSH RIO_PRAGMA_DIAGNOSTIC_PUSH_GCC_
#define RIO_PRAGMA_DIAGNOSTIC_POP RIO_PRAGMA_DIAGNOSTIC_POP_GCC_
#define RIO_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC RIO_PRAGMA_DIAGNOSTIC_IGNORED_GCC
#elif RIO_COMPILER_MSVC
#define RIO_PRAGMA_DIAGNOSTIC_PUSH RIO_PRAGMA_DIAGNOSTIC_PUSH_MSVC_
#define RIO_PRAGMA_DIAGNOSTIC_POP RIO_PRAGMA_DIAGNOSTIC_POP_MSVC_
#define RIO_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC(_x)
#endif // RIO_COMPILER_

#if RIO_COMPILER_GCC && defined(__is_pod)
#define RIO_TYPE_IS_POD(t) __is_pod(t)
#elif RIO_COMPILER_MSVC
#define RIO_TYPE_IS_POD(t) (!__is_class(t) || __is_pod(t))
#else
#define RIO_TYPE_IS_POD(t) false
#endif

#ifndef RIO_CHECK
#define RIO_CHECK(_condition, ...) RIO_NOOP()
#endif // RIO_CHECK

#ifndef RIO_TRACE
#define RIO_TRACE(...) RIO_NOOP()
#endif // RIO_TRACE

#ifndef RIO_WARN
#define RIO_WARN(_condition, ...) RIO_NOOP()
#endif // RIO_CHECK

// Copyright (c) 2016, 2017 Volodymyr Syvochka
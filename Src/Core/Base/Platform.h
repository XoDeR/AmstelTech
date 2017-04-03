// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

// Compiler
#define RIO_COMPILER_CLANG 0
#define RIO_COMPILER_CLANG_ANALYZER 0
#define RIO_COMPILER_GCC 0
#define RIO_COMPILER_MSVC 1

// Endianess
#define RIO_CPU_ENDIAN_BIG 0
#define RIO_CPU_ENDIAN_LITTLE 0

// CPU
#define RIO_CPU_ARM   0
#define RIO_CPU_JIT   0
#define RIO_CPU_MIPS  0
#define RIO_CPU_PPC   0
#define RIO_CPU_RISCV 0
#define RIO_CPU_X86   0

// C Runtime
#define RIO_CRT_MSVC   0
#define RIO_CRT_GLIBC  0
#define RIO_CRT_NEWLIB 0
#define RIO_CRT_MINGW  0
#define RIO_CRT_MUSL   0

// Platform
#define RIO_PLATFORM_ANDROID 0
#define RIO_PLATFORM_EMSCRIPTEN 0
#define RIO_PLATFORM_BSD 0
#define RIO_PLATFORM_HURD 0
#define RIO_PLATFORM_IOS 0
#define RIO_PLATFORM_LINUX 0
#define RIO_PLATFORM_NACL 0
#define RIO_PLATFORM_OSX 0
#define RIO_PLATFORM_PS4 0
#define RIO_PLATFORM_QNX 0
#define RIO_PLATFORM_RPI 0
#define RIO_PLATFORM_STEAMLINK 0
#define RIO_PLATFORM_WINDOWS 0
#define RIO_PLATFORM_WINRT 0
#define RIO_PLATFORM_XBOX360 0
#define RIO_PLATFORM_XBOXONE 0

// Clang
#if defined(__clang__)
// clang defines __GNUC__ or _MSC_VER
#undef  RIO_COMPILER_CLANG
#define RIO_COMPILER_CLANG (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#if defined(__clang_analyzer__)
#undef  RIO_COMPILER_CLANG_ANALYZER
#define RIO_COMPILER_CLANG_ANALYZER 1
#endif // defined(__clang_analyzer__)
#if defined(_MSC_VER)
#undef  RIO_CRT_MSVC
#define RIO_CRT_MSVC 1
#elif defined(__GLIBC__)
#undef  RIO_CRT_GLIBC
#define RIO_CRT_GLIBC (__GLIBC__ * 10000 + __GLIBC_MINOR__ * 100)
#endif // defined(__GLIBC__)
// MSVC
#elif defined(_MSC_VER)
#undef  RIO_COMPILER_MSVC
#define RIO_COMPILER_MSVC _MSC_VER
#undef  RIO_CRT_MSVC
#define RIO_CRT_MSVC 1
// GCC
#elif defined(__GNUC__)
#undef  RIO_COMPILER_GCC
#define RIO_COMPILER_GCC (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#if defined(__GLIBC__)
#undef  RIO_CRT_GLIBC
#define RIO_CRT_GLIBC (__GLIBC__ * 10000 + __GLIBC_MINOR__ * 100)
#elif defined(__MINGW32__) || defined(__MINGW64__)
#undef RIO_CRT_MINGW
#define RIO_CRT_MINGW 1
#endif
#else
#error "RIO_COMPILER_* is not defined!"
#endif

#if defined(__arm__)     || \
	defined(__aarch64__) || \
	defined(_M_ARM)
#undef  RIO_CPU_ARM
#define RIO_CPU_ARM 1
#define RIO_CACHE_LINE_SIZE 64
#elif defined(__MIPSEL__)     || \
	  defined(__mips_isa_rev) || \
	  defined(__mips64)
#undef  RIO_CPU_MIPS
#define RIO_CPU_MIPS 1
#define RIO_CACHE_LINE_SIZE 64
#elif defined(_M_PPC)        || \
	  defined(__powerpc__)   || \
	  defined(__powerpc64__)
#undef  RIO_CPU_PPC
#define RIO_CPU_PPC 1
#define RIO_CACHE_LINE_SIZE 128
#elif defined(__riscv)   || \
	  defined(__riscv__) || \
	  defined(RISCVEL)
#undef  RIO_CPU_RISCV
#define RIO_CPU_RISCV 1
#define RIO_CACHE_LINE_SIZE 64
#elif defined(_M_IX86)    || \
	  defined(_M_X64)     || \
	  defined(__i386__)   || \
	  defined(__x86_64__)
#undef  RIO_CPU_X86
#define RIO_CPU_X86 1
#define RIO_CACHE_LINE_SIZE 64
#else // PNaCl doesn't have CPU defined.
#undef  RIO_CPU_JIT
#define RIO_CPU_JIT 1
#define RIO_CACHE_LINE_SIZE 64
#endif

#if defined(__x86_64__)    || \
	defined(_M_X64)        || \
	defined(__aarch64__)   || \
	defined(__64BIT__)     || \
	defined(__mips64)      || \
	defined(__powerpc64__) || \
	defined(__ppc64__)     || \
	defined(__LP64__)
#undef  RIO_ARCH_64BIT
#define RIO_ARCH_64BIT 64
#else
#undef RIO_ARCH_32BIT
#define RIO_ARCH_32BIT 32
#endif

#if RIO_CPU_PPC
#undef RIO_CPU_ENDIAN_BIG
#define RIO_CPU_ENDIAN_BIG 1
#else
#undef RIO_CPU_ENDIAN_LITTLE
#define RIO_CPU_ENDIAN_LITTLE 1
#endif // RIO_PLATFORM_

#if defined(_XBOX_VER)
#undef RIO_PLATFORM_XBOX360
#define RIO_PLATFORM_XBOX360 1
#elif defined(_DURANGO) || defined(_XBOX_ONE)
#undef RIO_PLATFORM_XBOXONE
#define RIO_PLATFORM_XBOXONE 1
#elif defined(_WIN32) || defined(_WIN64)

#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX
// If _USING_V110_SDK71_ is defined it means we are using the v110_xp or v120_xp toolset.
#if defined(_MSC_VER) && (_MSC_VER >= 1700) && (!_USING_V110_SDK71_)
#include <winapifamily.h>
#endif // defined(_MSC_VER) && (_MSC_VER >= 1700) && (!_USING_V110_SDK71_)
#if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
#undef  RIO_PLATFORM_WINDOWS
#if !defined(WINVER) && !defined(_WIN32_WINNT)
#if RIO_ARCH_64BIT
// When building 64-bit target Win7 and above
#define WINVER 0x0601
#define _WIN32_WINNT 0x0601
#else
// Windows Server 2003 with SP1, Windows XP with SP2 and above
#define WINVER 0x0502
#define _WIN32_WINNT 0x0502
#endif // RIO_ARCH_64BIT
#endif // !defined(WINVER) && !defined(_WIN32_WINNT)
#define RIO_PLATFORM_WINDOWS _WIN32_WINNT
#else
#undef RIO_PLATFORM_WINRT
#define RIO_PLATFORM_WINRT 1
#endif
#elif defined(__ANDROID__)
// Android compiler defines __linux__
#include <android/api-level.h>
#undef  RIO_PLATFORM_ANDROID
#define RIO_PLATFORM_ANDROID __ANDROID_API__
#elif defined(__native_client__)
// NaCl compiler defines __linux__
#include <ppapi/c/pp_macros.h>
#undef  RIO_PLATFORM_NACL
#define RIO_PLATFORM_NACL PPAPI_RELEASE
#elif defined(__STEAMLINK__)
// SteamLink compiler defines __linux__
#undef  RIO_PLATFORM_STEAMLINK
#define RIO_PLATFORM_STEAMLINK 1
#elif defined(__VCCOREVER__)
// RaspberryPi compiler defines __linux__
#undef  RIO_PLATFORM_RPI
#define RIO_PLATFORM_RPI 1
#elif  defined(__linux__) \
	|| defined(__riscv__)
#undef  RIO_PLATFORM_LINUX
#define RIO_PLATFORM_LINUX 1
#elif  defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__) \
	|| defined(__ENVIRONMENT_TV_OS_VERSION_MIN_REQUIRED__)
#undef  RIO_PLATFORM_IOS
#define RIO_PLATFORM_IOS 1
#elif defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__)
#undef  RIO_PLATFORM_OSX
#if defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__)
#define RIO_PLATFORM_OSX __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
#else
#define RIO_PLATFORM_OSX 1
#endif // defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__)
#elif defined(__EMSCRIPTEN__)
#undef RIO_PLATFORM_EMSCRIPTEN
#define RIO_PLATFORM_EMSCRIPTEN 1
#elif defined(__ORBIS__)
#undef RIO_PLATFORM_PS4
#define RIO_PLATFORM_PS4 1
#elif defined(__QNX__)
#undef RIO_PLATFORM_QNX
#define RIO_PLATFORM_QNX 1
#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
#undef RIO_PLATFORM_BSD
#define RIO_PLATFORM_BSD 1
#elif defined(__GNU__)
#undef RIO_PLATFORM_HURD
#define RIO_PLATFORM_HURD 1
#else
#error "RIO_PLATFORM_* is not defined!"
#endif

#define RIO_PLATFORM_POSIX (0 \
						|| RIO_PLATFORM_ANDROID \
						|| RIO_PLATFORM_EMSCRIPTEN \
						|| RIO_PLATFORM_BSD \
						|| RIO_PLATFORM_HURD \
						|| RIO_PLATFORM_IOS \
						|| RIO_PLATFORM_LINUX \
						|| RIO_PLATFORM_NACL \
						|| RIO_PLATFORM_OSX \
						|| RIO_PLATFORM_QNX \
						|| RIO_PLATFORM_STEAMLINK \
						|| RIO_PLATFORM_PS4 \
						|| RIO_PLATFORM_RPI \
						)

#ifndef RIO_CONFIG_ENABLE_MSVC_LEVEL4_WARNINGS
#define RIO_CONFIG_ENABLE_MSVC_LEVEL4_WARNINGS 0
#endif // RIO_CONFIG_ENABLE_MSVC_LEVEL4_WARNINGS

#if RIO_COMPILER_GCC
#define RIO_COMPILER_NAME "GCC " \
				RIO_STRINGIZE(__GNUC__) "." \
				RIO_STRINGIZE(__GNUC_MINOR__) "." \
				RIO_STRINGIZE(__GNUC_PATCHLEVEL__)
#elif RIO_COMPILER_CLANG
#define RIO_COMPILER_NAME "Clang " \
				RIO_STRINGIZE(__clang_major__) "." \
				RIO_STRINGIZE(__clang_minor__) "." \
				RIO_STRINGIZE(__clang_patchlevel__)
#elif RIO_COMPILER_MSVC
#if RIO_COMPILER_MSVC >= 1900 // Visual Studio 2015
#define RIO_COMPILER_NAME "MSVC 14.0"
#elif RIO_COMPILER_MSVC >= 1800 // Visual Studio 2013
#define RIO_COMPILER_NAME "MSVC 12.0"
#elif RIO_COMPILER_MSVC >= 1700 // Visual Studio 2012
#define RIO_COMPILER_NAME "MSVC 11.0"
#elif RIO_COMPILER_MSVC >= 1600 // Visual Studio 2010
#define RIO_COMPILER_NAME "MSVC 10.0"
#elif RIO_COMPILER_MSVC >= 1500 // Visual Studio 2008
#define RIO_COMPILER_NAME "MSVC 9.0"
#else
#define RIO_COMPILER_NAME "MSVC"
#endif
#endif // RIO_COMPILER_

#if RIO_PLATFORM_ANDROID
#define RIO_PLATFORM_NAME "Android " \
				RIO_STRINGIZE(RIO_PLATFORM_ANDROID)
#elif RIO_PLATFORM_EMSCRIPTEN
#define RIO_PLATFORM_NAME "asm.js " \
				RIO_STRINGIZE(__EMSCRIPTEN_major__) "." \
				RIO_STRINGIZE(__EMSCRIPTEN_minor__) "." \
				RIO_STRINGIZE(__EMSCRIPTEN_tiny__)
#elif RIO_PLATFORM_BSD
#define RIO_PLATFORM_NAME "BSD"
#elif RIO_PLATFORM_HURD
#define RIO_PLATFORM_NAME "Hurd"
#elif RIO_PLATFORM_IOS
#define RIO_PLATFORM_NAME "iOS"
#elif RIO_PLATFORM_LINUX
#define RIO_PLATFORM_NAME "Linux"
#elif RIO_PLATFORM_NACL
#define RIO_PLATFORM_NAME "NaCl " \
				RIO_STRINGIZE(RIO_PLATFORM_NACL)
#elif RIO_PLATFORM_OSX
#define RIO_PLATFORM_NAME "OSX"
#elif RIO_PLATFORM_PS4
#define RIO_PLATFORM_NAME "PlayStation 4"
#elif RIO_PLATFORM_QNX
#define RIO_PLATFORM_NAME "QNX"
#elif RIO_PLATFORM_RPI
#define RIO_PLATFORM_NAME "RaspberryPi"
#elif RIO_PLATFORM_STEAMLINK
#define RIO_PLATFORM_NAME "SteamLink"
#elif RIO_PLATFORM_WINDOWS
#define RIO_PLATFORM_NAME "Windows"
#elif RIO_PLATFORM_WINRT
#define RIO_PLATFORM_NAME "WinRT"
#elif RIO_PLATFORM_XBOX360
#define RIO_PLATFORM_NAME "Xbox 360"
#elif RIO_PLATFORM_XBOXONE
#define RIO_PLATFORM_NAME "Xbox One"
#endif // RIO_PLATFORM_

#if RIO_CPU_ARM
#define RIO_CPU_NAME "ARM"
#elif RIO_CPU_JIT
#define RIO_CPU_NAME "JIT-VM"
#elif RIO_CPU_MIPS
#define RIO_CPU_NAME "MIPS"
#elif RIO_CPU_PPC
#define RIO_CPU_NAME "PowerPC"
#elif RIO_CPU_RISCV
#define RIO_CPU_NAME "RISC-V"
#elif RIO_CPU_X86
#define RIO_CPU_NAME "x86"
#endif // RIO_CPU_

#if RIO_ARCH_32BIT
#define RIO_ARCH_NAME "32-bit"
#elif RIO_ARCH_64BIT
#define RIO_ARCH_NAME "64-bit"
#endif // RIO_ARCH_

#if RIO_CONFIG_ENABLE_MSVC_LEVEL4_WARNINGS && RIO_COMPILER_MSVC
#pragma warning(error:4062) // ENABLE warning C4062: enumerator'...' in switch of enum '...' is not handled
#pragma warning(error:4100) // ENABLE warning C4100: '' : unreferenced formal parameter
#pragma warning(error:4189) // ENABLE warning C4189: '' : local variable is initialized but not referenced
#pragma warning(error:4121) // ENABLE warning C4121: 'symbol' : alignment of a member was sensitive to packing
#pragma warning(error:4130) // ENABLE warning C4130: 'operator' : logical operation on address of string constant
#pragma warning(error:4239) // ENABLE warning C4239: nonstandard extension used : 'argument' : conversion from '*' to '* &' A non-const reference may only be bound to an lvalue
#pragma warning(error:4245) // ENABLE warning C4245: 'conversion' : conversion from 'type1' to 'type2', signed/unsigned mismatch
#pragma warning(error:4263) // ENABLE warning C4263: 'function' : member function does not override any base class virtual member function
#pragma warning(error:4265) // ENABLE warning C4265: class has virtual functions, but destructor is not virtual
#pragma warning(error:4431) // ENABLE warning C4431: missing type specifier - int assumed. Note: C no longer supports default-int
#pragma warning(error:4505) // ENABLE warning C4505: '' : unreferenced local function has been removed
#pragma warning(error:4545) // ENABLE warning C4545: expression before comma evaluates to a function which is missing an argument list
#pragma warning(error:4549) // ENABLE warning C4549: 'operator' : operator before comma has no effect; did you intend 'operator'?
#pragma warning(error:4701) // ENABLE warning C4701: potentially uninitialized local variable 'name' used
#pragma warning(error:4706) // ENABLE warning C4706: assignment within conditional expression
#pragma warning(error:4800) // ENABLE warning C4800: '': forcing value to bool 'true' or 'false' (performance warning)
#endif // RIO_CONFIG_ENABLE_MSVC_LEVEL4_WARNINGS && RIO_COMPILER_MSVC

// Copyright (c) 2016 Volodymyr Syvochka
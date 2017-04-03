#pragma once

#include "Core/Base/RioCommon.h"

#if RIO_PLATFORM_ANDROID
#include <android/log.h>
#elif RIO_PLATFORM_WINDOWS || RIO_PLATFORM_WINRT0 || RIO_PLATFORM_XBOX360 || RIO_PLATFORM_XBOXONE
extern "C" __declspec(dllimport) void __stdcall OutputDebugStringA(const char* str);
#elif RIO_PLATFORM_IOS || RIO_PLATFORM_OSX
#if defined(__OBJC__)
#import <Foundation/NSObjCRuntime.h>
#else
#include <CoreFoundation/CFString.h>
extern "C" void NSLog(CFStringRef _format, ...);
#endif // defined(__OBJC__)
#elif 0 // RIO_PLATFORM_EMSCRIPTEN
#include <emscripten.h>
#else
#include <stdio.h>
#endif // RIO_PLATFORM_WINDOWS

namespace RioCore
{
#if RIO_COMPILER_CLANG_ANALYZER
    inline __attribute__((analyzer_noreturn)) void debugBreak();
#endif // RIO_COMPILER_CLANG_ANALYZER

    inline void debugBreak()
    {
#if RIO_COMPILER_MSVC
        __debugbreak();
#elif RIO_CPU_ARM
        __builtin_trap();
        //		asm("bkpt 0");
#elif !RIO_PLATFORM_NACL && RIO_CPU_X86 && (RIO_COMPILER_GCC || RIO_COMPILER_CLANG)
        // NaCl doesn't like int 3:
        // NativeClient: NaCl module load failed: Validation failure. File violates Native Client safety rules.
        __asm__("int $3");
#else // cross platform implementation
        int* int3 = (int*)3L;
        *int3 = 3;
#endif // RIO_COMPILER_
    }

    inline void debugOutput(const char* _out)
    {
#if RIO_PLATFORM_ANDROID
#ifndef RIO_ANDROID_LOG_TAG
#define RIO_ANDROID_LOG_TAG ""
#endif // RIO_ANDROID_LOG_TAG
        __android_log_write(ANDROID_LOG_DEBUG, RIO_ANDROID_LOG_TAG, _out);
#elif RIO_PLATFORM_WINDOWS || RIO_PLATFORM_WINRT || RIO_PLATFORM_XBOX360 || RIO_PLATFORM_XBOXONE
        OutputDebugStringA(_out);
#elif RIO_PLATFORM_IOS || RIO_PLATFORM_OSX
#if defined(__OBJC__)
        NSLog(@"%s", _out);
#else
        NSLog(__CFStringMakeConstantString("%s"), _out);
#endif // defined(__OBJC__)
#elif 0 // RIO_PLATFORM_EMSCRIPTEN
        emscripten_log(EM_LOG_CONSOLE, "%s", _out);
#else
        fputs(_out, stdout);
        fflush(stdout);
#endif // RIO_PLATFORM_
    }

} // namespace RioCore

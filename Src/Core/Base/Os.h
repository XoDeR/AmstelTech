// Copyright (c) 2016, 2017 Volodymyr Syvochka
#pragma once

#include "Core/Base/RioCommon.h"
#include "Core/Error/DebugHelper.h"
#include "Core/Error/Error.h"
#include "Core/Base/Macros.h"
#include "Core/Base/Platform.h"
#include "Core/Strings/StringTypes.h"
#include "Core/Base/Types.h"

#include <sys/stat.h>

#if RIO_PLATFORM_WINDOWS || RIO_PLATFORM_WINRT

#include <windows.h>
#include <psapi.h>
#include <io.h>
#include <stdio.h>

#elif  RIO_PLATFORM_ANDROID \
	|| RIO_PLATFORM_EMSCRIPTEN \
	|| RIO_PLATFORM_BSD \
	|| RIO_PLATFORM_HURD \
	|| RIO_PLATFORM_IOS \
	|| RIO_PLATFORM_LINUX \
	|| RIO_PLATFORM_NACL \
	|| RIO_PLATFORM_OSX \
	|| RIO_PLATFORM_PS4 \
	|| RIO_PLATFORM_RPI \
	|| RIO_PLATFORM_STEAMLINK
#include <sched.h> // sched_yield
#if RIO_PLATFORM_BSD \
	|| RIO_PLATFORM_IOS \
	|| RIO_PLATFORM_NACL \
	|| RIO_PLATFORM_OSX \
	|| RIO_PLATFORM_PS4 \
	|| RIO_PLATFORM_STEAMLINK
#include <pthread.h> // mach_port_t
#endif // RIO_PLATFORM_*

#include <time.h> // nanosleep, clock_gettime
#if !RIO_PLATFORM_PS4 && !RIO_PLATFORM_NACL
#include <dlfcn.h> // dlopen, dlclose, dlsym
#endif // !RIO_PLATFORM_PS4 && !RIO_PLATFORM_NACL

#if RIO_PLATFORM_ANDROID
#include <malloc.h> // mallinfo
#elif   RIO_PLATFORM_LINUX \
		|| RIO_PLATFORM_RPI \
		|| RIO_PLATFORM_STEAMLINK
#include <unistd.h> // syscall, access, unlink, rmdir, getcwd, fork, execv
#include <sys/syscall.h>
#elif RIO_PLATFORM_OSX
#include <mach/mach.h> // mach_task_basic_info
#elif RIO_PLATFORM_HURD
#include <unistd.h> // getpid
#elif RIO_PLATFORM_ANDROID
#include "debug.h" // getCurrentThreadId is not implemented
#include <android/log.h>
#endif // RIO_PLATFORM_ANDROID
#endif // RIO_PLATFORM_

#if RIO_CRT_MSVC
#include <direct.h> // _getcwd
#else
#include <unistd.h> // getcwd
#endif // RIO_CRT_MSVC

#if RIO_PLATFORM_OSX
#define RIO_DL_EXT "dylib"
#elif RIO_PLATFORM_WINDOWS
#define RIO_DL_EXT "dll"
#else
#define RIO_DL_EXT "so"
#endif

namespace RioCore
{

    namespace OsFn
    {
        inline void sleep(uint32_t milliSeconds)
        {
#if RIO_PLATFORM_WINDOWS || RIO_PLATFORM_XBOX360
            ::Sleep(milliSeconds);
#elif RIO_PLATFORM_XBOXONE || RIO_PLATFORM_WINRT
            RIO_UNUSED(milliSeconds);
            debugOutput("sleep is not implemented");
            debugBreak();
#else
            timespec req = { (time_t)milliSeconds / 1000, (long)((milliSeconds % 1000) * 1000000) };
            timespec rem = { 0, 0 };
            ::nanosleep(&req, &rem);
#endif // RIO_PLATFORM_
        }

        inline size_t getProcessMemoryUsed()
        {
#if RIO_PLATFORM_ANDROID
            struct mallinfo mi = mallinfo();
            return mi.uordblks;
#elif RIO_PLATFORM_LINUX || RIO_PLATFORM_HURD
            FILE* file = fopen("/proc/self/statm", "r");
            if (NULL == file)
            {
                return 0;
            }

            long pages = 0;
            int items = fscanf(file, "%*s%ld", &pages);
            fclose(file);
            return 1 == items
                ? pages * sysconf(_SC_PAGESIZE)
                : 0
                ;
#elif RIO_PLATFORM_OSX
#if defined(MACH_TASK_BASIC_INFO)
            mach_task_basic_info info;
            mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;

            int const result = task_info(mach_task_self()
                , MACH_TASK_BASIC_INFO
                , (task_info_t)&info
                , &infoCount
            );
#else // MACH_TASK_BASIC_INFO
            task_basic_info info;
            mach_msg_type_number_t infoCount = TASK_BASIC_INFO_COUNT;

            int const result = task_info(mach_task_self()
                , TASK_BASIC_INFO
                , (task_info_t)&info
                , &infoCount
            );
#endif // MACH_TASK_BASIC_INFO
            if (KERN_SUCCESS != result)
            {
                return 0;
            }

            return info.resident_size;
#elif RIO_PLATFORM_WINDOWS
            PROCESS_MEMORY_COUNTERS pmc;
            GetProcessMemoryInfo(GetCurrentProcess()
                , &pmc
                , sizeof(pmc)
            );
            return pmc.WorkingSetSize;
#else
            return 0;
#endif // RIO_PLATFORM_*
        }

        inline void* libraryOpen(const char* filePath)
        {
#if RIO_PLATFORM_WINDOWS
            return (void*)::LoadLibraryA(filePath);
#elif  RIO_PLATFORM_EMSCRIPTEN \
	|| RIO_PLATFORM_NACL \
	|| RIO_PLATFORM_PS4 \
	|| RIO_PLATFORM_XBOXONE \
	|| RIO_PLATFORM_WINRT
            RIO_UNUSED(_filePath);
            return NULL;
#else
            return ::dlopen(filePath, RTLD_LOCAL | RTLD_LAZY);
#endif // RIO_PLATFORM_
        }

        inline void libraryClose(void* libraryHandle)
        {
#if RIO_PLATFORM_WINDOWS
            ::FreeLibrary((HMODULE)libraryHandle);
#elif  RIO_PLATFORM_EMSCRIPTEN \
	|| RIO_PLATFORM_NACL \
	|| RIO_PLATFORM_PS4 \
	|| RIO_PLATFORM_XBOXONE \
	|| RIO_PLATFORM_WINRT
            RIO_UNUSED(libraryHandle);
#else
            ::dlclose(libraryHandle);
#endif // RIO_PLATFORM_
        }

        inline void* libraryLookupSymbol(void* libraryHandle, const char* symbolName)
        {
#if RIO_PLATFORM_WINDOWS
            return (void*)::GetProcAddress((HMODULE)libraryHandle, symbolName);
#elif  RIO_PLATFORM_EMSCRIPTEN \
	|| RIO_PLATFORM_NACL \
	|| RIO_PLATFORM_PS4 \
	|| RIO_PLATFORM_XBOXONE \
	|| RIO_PLATFORM_WINRT
            RIO_UNUSED(libraryHandle, symbolName);
            return NULL;
#else
            return ::dlsym(libraryHandle, symbolName);
#endif // RIO_PLATFORM_
        }

        inline void setEnvironmentVariable(const char* _name, const char* _value)
        {
#if RIO_PLATFORM_WINDOWS
            ::SetEnvironmentVariableA(_name, _value);
#elif  RIO_PLATFORM_PS4 \
	|| RIO_PLATFORM_XBOXONE \
	|| RIO_PLATFORM_WINRT
            RIO_UNUSED(_name, _value);
#else
            ::setenv(_name, _value, 1);
#endif // RIO_PLATFORM_
        }

        inline void unSetEnvironmentVariable(const char* _name)
        {
#if RIO_PLATFORM_WINDOWS
            ::SetEnvironmentVariableA(_name, NULL);
#elif  RIO_PLATFORM_PS4 \
	|| RIO_PLATFORM_XBOXONE \
	|| RIO_PLATFORM_WINRT
            RIO_UNUSED(_name);
#else
            ::unsetenv(_name);
#endif // RIO_PLATFORM_
        }

        inline void log(const char* msg)
        {
#if RIO_PLATFORM_ANDROID
            __android_log_write(ANDROID_LOG_DEBUG, "Rio", msg);
#else
            fputs(msg, stdout);
            fflush(stdout);
#endif // RIO_PLATFORM_
        }

        struct FileInfo
        {
            enum Enum
            {
                Regular,
                Directory,

                Count
        };

            uint64_t m_size;
            Enum m_type;
    };

        inline bool getFileInfo(const char* _filePath, FileInfo& _fileInfo)
        {
            _fileInfo.m_size = 0;
            _fileInfo.m_type = FileInfo::Count;

#if RIO_COMPILER_MSVC
            struct ::_stat64 st;
            int32_t result = ::_stat64(_filePath, &st);

            if (0 != result)
            {
                return false;
            }

            if (0 != (st.st_mode & _S_IFREG))
            {
                _fileInfo.m_type = FileInfo::Regular;
            }
            else if (0 != (st.st_mode & _S_IFDIR))
            {
                _fileInfo.m_type = FileInfo::Directory;
            }
#else
            struct ::stat st;
            int32_t result = ::stat(_filePath, &st);
            if (0 != result)
            {
                return false;
            }

            if (0 != (st.st_mode & S_IFREG))
            {
                _fileInfo.m_type = FileInfo::Regular;
            }
            else if (0 != (st.st_mode & S_IFDIR))
            {
                _fileInfo.m_type = FileInfo::Directory;
            }
#endif // RIO_COMPILER_MSVC

            _fileInfo.m_size = st.st_size;

            return true;
        }

        inline int changeDirectory(const char* path)
        {
#if RIO_PLATFORM_PS4 \
 || RIO_PLATFORM_XBOXONE \
 || RIO_PLATFORM_WINRT
            RIO_UNUSED(path);
            return -1;
#elif RIO_CRT_MSVC
            return ::_chdir(path);
#else
            return ::chdir(_path);
#endif // RIO_COMPILER_
        }

        // Returns whether the <path> exists
        inline bool getDoesExist(const char* path)
        {
#if RIO_PLATFORM_POSIX
            return access(path, F_OK) != -1;
#elif RIO_PLATFORM_WINDOWS
            return _access(path, 0) != -1; // <corecrt_io.h>
#endif // RIO_PLATFORM_
        }

        // Returns whether <path> is a directory
        inline bool getIsDirectory(const char* path)
        {
#if RIO_PLATFORM_POSIX
            struct stat info;
            memset(&info, 0, sizeof(info));
            int err = lstat(path, &info);
            RIO_ASSERT(err == 0, "lstat: errno = %d", errno);
            RIO_UNUSED(err);
            return ((S_ISDIR(info.st_mode) == 1) && (S_ISLNK(info.st_mode) == 0));
#elif RIO_PLATFORM_WINDOWS
            DWORD fileAttributes = GetFileAttributes(path);
            return (fileAttributes != INVALID_FILE_ATTRIBUTES && (fileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
#endif // RIO_PLATFORM_
        }

        // Is a regular file
        inline bool getIsFile(const char* path)
        {
#if RIO_PLATFORM_POSIX
            struct stat info;
            memset(&info, 0, sizeof(info));
            int err = lstat(path, &info);
            RIO_ASSERT(err == 0, "lstat: errno = %d", errno);
            RIO_UNUSED(err);
            return ((S_ISREG(info.st_mode) == 1) && (S_ISLNK(info.st_mode) == 0));
#elif RIO_PLATFORM_WINDOWS
            DWORD fileAttributes = GetFileAttributes(path);
            return (fileAttributes != INVALID_FILE_ATTRIBUTES && (fileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0);
#endif // RIO_PLATFORM_
        }

        // Returns the last modification time of the path
        inline uint64_t getLastModifiedTime(const char* path)
        {
#if RIO_PLATFORM_POSIX
            struct stat info;
            memset(&info, 0, sizeof(info));
            int err = lstat(path, &info);
            RIO_ASSERT(err == 0, "lstat: errno = %d", errno);
            RIO_UNUSED(err);
            return info.st_mtime;
#elif RIO_PLATFORM_WINDOWS
            HANDLE fileHandle = CreateFile(path
                , GENERIC_READ
                , FILE_SHARE_READ
                , NULL
                , OPEN_EXISTING
                , 0
                , NULL
            );
            RIO_ASSERT(fileHandle != INVALID_HANDLE_VALUE, "CreateFile: GetLastError = %d", GetLastError());
            FILETIME fileTimeWrite;
            BOOL err = GetFileTime(fileHandle, NULL, NULL, &fileTimeWrite);
            RIO_ASSERT(err != 0, "GetFileTime: GetLastError = %d", GetLastError());
            RIO_UNUSED(err);
            CloseHandle(fileHandle);
            return (uint64_t)((uint64_t(fileTimeWrite.dwHighDateTime) << 32) | fileTimeWrite.dwLowDateTime);
#endif // RIO_PLATFORM_
        }

        inline void createFile(const char* path)
        {
#if RIO_PLATFORM_POSIX
            // Permission mask: rw-r--r--
            int err = ::mknod(path, S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, 0);
            RIO_ASSERT(err == 0, "mknod: errno = %d", errno);
            RIO_UNUSED(err);
#elif RIO_PLATFORM_WINDOWS
            HANDLE fileHandle = CreateFile(path
                , GENERIC_READ | GENERIC_WRITE
                , 0
                , NULL
                , CREATE_ALWAYS
                , FILE_ATTRIBUTE_NORMAL
                , NULL
            );
            RIO_ASSERT(fileHandle != INVALID_HANDLE_VALUE, "CreateFile: GetLastError = %d", GetLastError());
            CloseHandle(fileHandle);
#endif // RIO_PLATFORM_
        }

        inline void deleteFile(const char* path)
        {
#if RIO_PLATFORM_POSIX
            int err = ::unlink(path);
            RIO_ASSERT(err == 0, "unlink: errno = %d", errno);
            RIO_UNUSED(err);
#elif RIO_PLATFORM_WINDOWS
            BOOL err = DeleteFile(path);
            RIO_ASSERT(err != 0, "DeleteFile: GetLastError = %d", GetLastError());
            RIO_UNUSED(err);
#endif // RIO_PLATFORM_
        }

        inline void createDirectory(const char* path)
        {
#if RIO_PLATFORM_POSIX
            // rwxr-xr-x
            int err = ::mkdir(path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
            RIO_ASSERT(err == 0, "mkdir: errno = %d", errno);
            RIO_UNUSED(err);
#elif RIO_PLATFORM_WINDOWS
            BOOL err = CreateDirectory(path, NULL);
            RIO_ASSERT(err != 0, "CreateDirectory: GetLastError = %d", GetLastError());
            RIO_UNUSED(err);
#endif // RIO_PLATFORM_
        }

        inline void deleteDirectory(const char* path)
        {
#if RIO_PLATFORM_POSIX
            int err = ::rmdir(path);
            RIO_ASSERT(err == 0, "rmdir: errno = %d", errno);
            RIO_UNUSED(err);
#elif RIO_PLATFORM_WINDOWS
            BOOL err = RemoveDirectory(path);
            RIO_ASSERT(err != 0, "RemoveDirectory: GetLastError = %d", GetLastError());
            RIO_UNUSED(err);
#endif // RIO_PLATFORM_
        }

        void getFileList(const char* path, Vector<DynamicString>& files);

        // Returns the current working directory
        inline const char* getCurrentWorkingDirectory(char* buffer, uint32_t size)
        {
#if RIO_PLATFORM_PS4 \
 || RIO_PLATFORM_XBOXONE \
 || RIO_PLATFORM_WINRT
            RIO_UNUSED(buffer, size);
            return nullptr;
#elif RIO_CRT_MSVC
            return ::_getcwd(buffer, (int)size);
#else
            return ::getcwd(buffer, size);
#endif // RIO_COMPILER_
        }

        // Returns the value of the environment variable <name>
        inline const char* getEnvironmentVariable(const char* name)
        {
#if RIO_PLATFORM_POSIX
            return ::getenv(name);
#elif RIO_PLATFORM_WINDOWS
            // TODO
            // GetEnvironmentVariable(name, buffer, size);
#endif // RIO_PLATFORM_
        }

        // Executes the process <path> with the given <args> and returns its exit code
        // Fills <output> with stdout and stderr
        int executeProcess(const char* path, const char* args, StringStream& output);

        inline void yieldCurrentThread()
        {
#if RIO_PLATFORM_WINDOWS
            ::SwitchToThread();
#elif RIO_PLATFORM_XBOX360
            ::Sleep(0);
#elif RIO_PLATFORM_XBOXONE || RIO_PLATFORM_WINRT
            debugOutput("yield is not implemented");
            debugBreak();
#else
            ::sched_yield();
#endif // RIO_PLATFORM_
        }

        inline uint32_t getCurrentThreadId()
        {
#if RIO_PLATFORM_WINDOWS
            return ::GetCurrentThreadId();
#elif RIO_PLATFORM_LINUX || RIO_PLATFORM_RPI || RIO_PLATFORM_STEAMLINK
            return (pid_t)::syscall(SYS_gettid);
#elif RIO_PLATFORM_IOS || RIO_PLATFORM_OSX
            return (mach_port_t)::pthread_mach_thread_np(pthread_self());
#elif RIO_PLATFORM_BSD || RIO_PLATFORM_NACL
            // Casting __nc_basic_thread_data*... need better way to do this.
            return *(uint32_t*)::pthread_self();
#elif RIO_PLATFORM_HURD
            return (pthread_t)::pthread_self();
#else
            debugOutput("getCurrentThreadId is not implemented");
            debugBreak();
            return 0;
#endif
        }
    } // namespace OsFn

} // namespace RioCore
// Copyright (c) 2016 Volodymyr Syvochka
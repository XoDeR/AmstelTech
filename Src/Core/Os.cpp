#include "Core/Containers/Vector.h"
#include "Core/Error/Error.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Os.h"
#include "Core/Platform.h"
#include "Core/Strings/DynamicString.h"
#include "Core/Strings/StringStream.h"

#include <cstring>   // strcmp
#include <sys/stat.h> // stat, mkdir

#if RIO_PLATFORM_POSIX
	#include <dirent.h> // opendir, readdir
	#include <dlfcn.h> // dlopen, dlclose, dlsym
	#include <errno.h>
	#include <stdio.h> // fputs
	#include <stdlib.h> // getenv
	#include <cstring> // memset
	#include <sys/wait.h> // wait
	#include <time.h> // clock_gettime
	#include <unistd.h> // unlink, rmdir, getcwd, fork, execv
#elif RIO_PLATFORM_WINDOWS
	#include <io.h>
	#include <stdio.h>
	#include <windows.h>
#endif // RIO_PLATFORM_POSIX | RIO_PLATFORM_WINDOWS

#if RIO_PLATFORM_ANDROID
	#include <android/log.h>
#endif // RIO_PLATFORM_ANDROID

namespace Rio
{

namespace OsFn
{
	int64_t getClockTime()
	{
#if RIO_PLATFORM_LINUX || RIO_PLATFORM_ANDROID
		timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		return now.tv_sec * int64_t(1000000000) + now.tv_nsec;
#elif RIO_PLATFORM_OSX
		struct timeval now;
		gettimeofday(&now, NULL);
		return now.tv_sec * int64_t(1000000) + now.tv_usec;
#elif RIO_PLATFORM_WINDOWS
		LARGE_INTEGER ttime;
		QueryPerformanceCounter(&ttime);
		return (int64_t)ttime.QuadPart;
#endif
	}

	int64_t getClockFrequency()
	{
#if RIO_PLATFORM_LINUX || RIO_PLATFORM_ANDROID
		return int64_t(1000000000);
#elif RIO_PLATFORM_OSX
		return int64_t(1000000);
#elif RIO_PLATFORM_WINDOWS
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		return (int64_t)frequency.QuadPart;
#endif
	}

	// Suspends execution for <ms> milliseconds
	void sleep(uint32_t ms)
	{
#if RIO_PLATFORM_POSIX
		usleep(ms * 1000);
#elif RIO_PLATFORM_WINDOWS
		Sleep(ms);
#endif
	}

	// Opens the library at <path>
	void* libraryOpen(const char* path)
	{
#if RIO_PLATFORM_POSIX
		return ::dlopen(path, RTLD_LAZY);
#elif RIO_PLATFORM_WINDOWS
		return (void*)LoadLibraryA(path);
#endif
	}

	// Closes a <library> previously opened by libraryOpen
	void libraryClose(void* library)
	{
#if RIO_PLATFORM_POSIX
		dlclose(library);
#elif RIO_PLATFORM_WINDOWS
		FreeLibrary((HMODULE)library);
#endif
	}

	// Returns a pointer to the symbol <name> in the given <library>
	void* getLibrarySymbol(void* library, const char* name)
	{
#if RIO_PLATFORM_POSIX
		return ::dlsym(library, name);
#elif RIO_PLATFORM_WINDOWS
		return (void*)GetProcAddress((HMODULE)library, name);
#endif
	}

	// Logs the message <msg>
	void log(const char* msg)
	{
#if RIO_PLATFORM_ANDROID
		__android_log_write(ANDROID_LOG_DEBUG, "Rio", msg);
#elif RIO_PLATFORM_WINDOWS
		OutputDebugStringA(msg);
#else
		fputs(msg, stdout);
		fflush(stdout);
#endif
	}

	void getFileInfo(Stat& info, const char* path)
	{
		info.fileType = Stat::NO_ENTRY;
		info.size = 0;
		info.lastModifiedTime = 0;

#if RIO_PLATFORM_POSIX
		struct stat buf;
		memset(&buf, 0, sizeof(buf));
		
		int err = ::stat(path, &buf);
		if (err != 0)
		{
			return;
		}

		if (S_ISREG(buf.st_mode) == 1)
		{
			info.fileType = Stat::REGULAR;
		}
		else if (S_ISDIR(buf.st_mode) == 1)
		{
			info.fileType = Stat::DIRECTORY;
		}
#elif RIO_PLATFORM_WINDOWS
		struct _stat64 buf;
		int err = ::_stat64(path, &buf);
		if (err != 0)
		{
			return;
		}

		if ((buf.st_mode & _S_IFREG) != 0)
		{
			info.fileType = Stat::REGULAR;
		}
		else if ((buf.st_mode & _S_IFDIR) != 0)
		{
			info.fileType = Stat::DIRECTORY;
		}
#endif

		info.size = buf.st_size;
		info.lastModifiedTime = buf.st_mtime;
	}

	// Deletes the file at <path>
	void deleteFile(const char* path)
	{
#if RIO_PLATFORM_POSIX
		int err = ::unlink(path);
		RIO_ASSERT(err == 0, "unlink: errno = %d", errno);
		RIO_UNUSED(err);
#elif RIO_PLATFORM_WINDOWS
		BOOL err = DeleteFile(path);
		RIO_ASSERT(err != 0, "DeleteFile: GetLastError = %d", GetLastError());
		RIO_UNUSED(err);
#endif
	}

	// Creates a directory named <path>
	void createDirectory(const char* path)
	{
#if RIO_PLATFORM_POSIX
		int err = ::mkdir(path, 0755);
		RIO_ASSERT(err == 0, "mkdir: errno = %d", errno);
		RIO_UNUSED(err);
#elif RIO_PLATFORM_WINDOWS
		BOOL err = CreateDirectory(path, NULL);
		RIO_ASSERT(err != 0, "CreateDirectory: GetLastError = %d", GetLastError());
		RIO_UNUSED(err);
#endif
	}

	// Deletes the directory at <path>
	void deleteDirectory(const char* path)
	{
#if RIO_PLATFORM_POSIX
		int err = ::rmdir(path);
		RIO_ASSERT(err == 0, "rmdir: errno = %d", errno);
		RIO_UNUSED(err);
#elif RIO_PLATFORM_WINDOWS
		BOOL err = RemoveDirectory(path);
		RIO_ASSERT(err != 0, "RemoveDirectory: GetLastError = %d", GetLastError());
		RIO_UNUSED(err);
#endif
	}

	// Returns the list of <files> at the given <path>
	void getFileList(const char* path, Vector<DynamicString>& files);

	// Returns the current working directory
	const char* getCurrentWorkingDirectory(char* buffer, uint32_t size)
	{
#if RIO_PLATFORM_POSIX
		return ::getcwd(buffer, size);
#elif RIO_PLATFORM_WINDOWS
		GetCurrentDirectory(size, buffer);
		return buffer;
#endif
	}

	// Returns the value of the environment variable <name>
	const char* getEnvironmentVariable(const char* name)
	{
#if RIO_PLATFORM_POSIX
		return ::getenv(name);
#elif RIO_PLATFORM_WINDOWS
		// GetEnvironmentVariable(name, buf, size);
		return nullptr;
#endif
	}

	void getFileList(const char* path, Vector<DynamicString>& files)
	{
#if RIO_PLATFORM_POSIX
		DIR* dir;
		struct dirent* entry;

		if (!(dir = opendir(path)))
		{
			return;
		}

		while ((entry = readdir(dir)))
		{
			const char* dname = entry->d_name;

			if (!strcmp(dname, ".") || !strcmp(dname, ".."))
			{
				continue;
			}

			TempAllocator512 ta;
			DynamicString fname(ta);
			fname.set(dname, getStrLen32(dname));
			VectorFn::pushBack(files, fname);
		}

		closedir(dir);
#elif RIO_PLATFORM_WINDOWS
		TempAllocator1024 ta;
		DynamicString currentPath(ta);
		currentPath += path;
		currentPath += "\\*";

		WIN32_FIND_DATA fileFindData;
		HANDLE file = FindFirstFile(currentPath.getCStr(), &fileFindData);
		if (file == INVALID_HANDLE_VALUE)
		{
			return;
		}

		do
		{
			const char* directoryName = fileFindData.cFileName;

			if (!strcmp(directoryName, ".") || !strcmp(directoryName, ".."))
			{
				continue;
			}

			TempAllocator512 ta;
			DynamicString fileName(ta);
			fileName.set(directoryName, getStrLen32(directoryName));
			VectorFn::pushBack(files, fileName);
		}
		while (FindNextFile(file, &fileFindData) != 0);

		FindClose(file);
#endif
	}

	int executeProcess(const char* const* argv, StringStream& output)
	{
		TempAllocator512 ta;
		StringStream path(ta);

		path << argv[0];
		path << ' ';
#if RIO_PLATFORM_POSIX
		path << "2>&1 ";
#endif
		for (int32_t i = 1; argv[i] != nullptr; ++i)
		{
			const char* arg = argv[i];
			for (; *arg; ++arg)
			{
				if (*arg == ' ')
				{
					path << '\\';
				}
				path << *arg;
			}
			path << ' ';
		}
#if RIO_PLATFORM_POSIX
		FILE* file = popen(StringStreamFn::getCStr(path), "r");

		char buf[1024];
		while (fgets(buf, sizeof(buf), file) != NULL)
		{
			output << buf;
		}

		return pclose(file);
#elif RIO_PLATFORM_WINDOWS
		STARTUPINFO info;
		memset(&info, 0, sizeof(info));
		info.cb = sizeof(info);

		PROCESS_INFORMATION process;
		memset(&process, 0, sizeof(process));

		int err = CreateProcess(argv[0]
			, (LPSTR)StringStreamFn::getCStr(path)
			, NULL
			, NULL
			, FALSE
			, CREATE_NO_WINDOW
			, NULL
			, NULL
			, &info
			, &process
			);
		RIO_ASSERT(err != 0, "CreateProcess: GetLastError = %d", GetLastError());
		RIO_UNUSED(err);

		DWORD exitcode = 1;
		::WaitForSingleObject(process.hProcess, INFINITE);
		GetExitCodeProcess(process.hProcess, &exitcode);
		CloseHandle(process.hProcess);
		CloseHandle(process.hThread);
		return (int)exitcode;
#endif
	}

} // namespace OsFn

} // namespace Rio

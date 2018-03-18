#include "Core/FileSystem/FileSystemDisk.h"

#include "Core/Containers/Vector.h"
#include "Core/FileSystem/File.h"
#include "Core/FileSystem/Path.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Os.h"
#include "Core/Strings/DynamicString.h"

#if RIO_PLATFORM_POSIX
	#include <stdio.h>
	#include <errno.h>
#elif RIO_PLATFORM_WINDOWS
	#include <tchar.h>
	#include <windows.h>
#endif

namespace Rio
{

struct FileDisk : public File
{
#if RIO_PLATFORM_POSIX
	FILE* file = nullptr;
#elif RIO_PLATFORM_WINDOWS
	HANDLE file = INVALID_HANDLE_VALUE;
	bool isEndOfFile = false;
#endif

	FileDisk()
	{
	}

	virtual ~FileDisk()
	{
		close();
	}

	// Opens the file located at <path> with the given <mode>
	void open(const char* path, FileOpenMode::Enum mode)
	{
#if RIO_PLATFORM_POSIX
		this->file = fopen(path, (mode == FileOpenMode::READ) ? "rb" : "wb");
		RIO_ASSERT(_file != NULL, "fopen: errno = %d, path = '%s'", errno, path);
#elif RIO_PLATFORM_WINDOWS
		this->file = CreateFile(path
			, (mode == FileOpenMode::READ) ? GENERIC_READ : GENERIC_WRITE
			, 0
			, NULL
			, OPEN_ALWAYS
			, FILE_ATTRIBUTE_NORMAL
			, NULL
			);
		RIO_ASSERT(this->file != INVALID_HANDLE_VALUE
			, "CreateFile: GetLastError = %d, path = '%s'"
			, GetLastError()
			, path
			);
#endif
	}

	void close()
	{
		if (isFileOpen() == true)
		{
#if RIO_PLATFORM_POSIX
			fclose(this->file);
			this->file = nullptr;
#elif RIO_PLATFORM_WINDOWS
			CloseHandle(this->file);
			this->file = INVALID_HANDLE_VALUE;
#endif
		}
	}

	bool isFileOpen() const
	{
#if RIO_PLATFORM_POSIX
		return this->file != nullptr;
#elif RIO_PLATFORM_WINDOWS
		return this->file != INVALID_HANDLE_VALUE;
#endif
	}

	uint32_t getFileSize()
	{
#if RIO_PLATFORM_POSIX
		long pos = ftell(this->file);
		RIO_ASSERT(pos != -1, "ftell: errno = %d", errno);
		int err = fseek(this->file, 0, SEEK_END);
		RIO_ASSERT(err == 0, "fseek: errno = %d", errno);
		long size = ftell(this->file);
		RIO_ASSERT(size != -1, "ftell: errno = %d", errno);
		err = fseek(this->file, (long)pos, SEEK_SET);
		RIO_ASSERT(err == 0, "fseek: errno = %d", errno);
		RIO_UNUSED(err);
		return (uint32_t)size;
#elif RIO_PLATFORM_WINDOWS
		return GetFileSize(this->file, NULL);
#endif
	}

	uint32_t getFilePosition()
	{
#if RIO_PLATFORM_POSIX
		long pos = ftell(this->file);
		RIO_ASSERT(pos != -1, "ftell: errno = %d", errno);
		return (uint32_t)pos;
#elif RIO_PLATFORM_WINDOWS
		DWORD pos = SetFilePointer(this->file, 0, NULL, FILE_CURRENT);
		RIO_ASSERT(pos != INVALID_SET_FILE_POINTER
			, "SetFilePointer: GetLastError = %d"
			, GetLastError()
			);
		return (uint32_t)pos;
#endif
	}

	bool getIsEndOfFile()
	{
#if RIO_PLATFORM_POSIX
		return feof(this->file) != 0;
#elif RIO_PLATFORM_WINDOWS
		return isEndOfFile;
#endif
	}

	void seek(uint32_t position)
	{
#if RIO_PLATFORM_POSIX
		int err = fseek(this->file, (long)position, SEEK_SET);
		RIO_ASSERT(err == 0, "fseek: errno = %d", errno);
#elif RIO_PLATFORM_WINDOWS
		DWORD err = SetFilePointer(this->file, position, NULL, FILE_BEGIN);
		RIO_ASSERT(err != INVALID_SET_FILE_POINTER
			, "SetFilePointer: GetLastError = %d"
			, GetLastError()
			);
#endif
		RIO_UNUSED(err);
	}

	void seekToEnd()
	{
#if RIO_PLATFORM_POSIX
		int err = fseek(this->file, 0, SEEK_END);
		RIO_ASSERT(err == 0, "fseek: errno = %d", errno);
#elif RIO_PLATFORM_WINDOWS
		DWORD err = SetFilePointer(this->file, 0, NULL, FILE_END);
		RIO_ASSERT(err != INVALID_SET_FILE_POINTER
			, "SetFilePointer: GetLastError = %d"
			, GetLastError()
			);
#endif
		RIO_UNUSED(err);
	}

	void skip(uint32_t bytes)
	{
#if RIO_PLATFORM_POSIX
		int err = fseek(this->file, bytes, SEEK_CUR);
		RIO_ASSERT(err == 0, "fseek: errno = %d", errno);
#elif RIO_PLATFORM_WINDOWS
		DWORD err = SetFilePointer(this->file, bytes, NULL, FILE_CURRENT);
		RIO_ASSERT(err != INVALID_SET_FILE_POINTER
			, "SetFilePointer: GetLastError = %d"
			, GetLastError()
			);
#endif
		RIO_UNUSED(err);
	}

	uint32_t read(void* data, uint32_t size)
	{
		RIO_ASSERT(data != nullptr, "Data must be != nullptr");
#if RIO_PLATFORM_POSIX
		size_t bytesRead = fread(data, 1, size, this->file);
		RIO_ASSERT(ferror(this->file) == 0, "fread error");
		return (uint32_t)bytesRead;
#elif RIO_PLATFORM_WINDOWS
		DWORD bytesRead;
		BOOL err = ReadFile(this->file, data, size, &bytesRead, NULL);
		RIO_ASSERT(err == TRUE, "ReadFile: GetLastError = %d", GetLastError());
		isEndOfFile = err && bytesRead == 0;
		return bytesRead;
#endif
	}

	uint32_t write(const void* data, uint32_t size)
	{
		RIO_ASSERT(data != nullptr, "Data must be != nullptr");
#if RIO_PLATFORM_POSIX
		size_t bytesWritten = fwrite(data, 1, size, this->file);
		RIO_ASSERT(ferror(this->file) == 0, "fwrite error");
		return (uint32_t)bytesWritten;
#elif RIO_PLATFORM_WINDOWS
		DWORD bytesWritten;
		WriteFile(this->file, data, size, &bytesWritten, NULL);
		RIO_ASSERT(size == bytesWritten
			, "WriteFile: GetLastError = %d"
			, GetLastError()
			);
		return bytesWritten;
#endif
	}

	void flush()
	{
#if RIO_PLATFORM_POSIX
		int err = fflush(this->file);
		RIO_ASSERT(err == 0, "fflush: errno = %d", errno);
#elif RIO_PLATFORM_WINDOWS
		BOOL err = FlushFileBuffers(this->file);
		RIO_ASSERT(err != 0
			, "FlushFileBuffers: GetLastError = %d"
			, GetLastError()
			);
#endif
		RIO_UNUSED(err);
	}
};

FileSystemDisk::FileSystemDisk(Allocator& a)
	: allocator(&a)
	, prefix(a)
{
}

void FileSystemDisk::setPrefix(const char* prefix)
{
	this->prefix.set(prefix, getStrLen32(prefix));
}

File* FileSystemDisk::open(const char* path, FileOpenMode::Enum mode)
{
	RIO_ENSURE(nullptr != path);

	TempAllocator256 ta;
	DynamicString absolutePath(ta);
	getAbsolutePath(path, absolutePath);

	FileDisk* file = RIO_NEW(*allocator, FileDisk)();
	file->open(absolutePath.getCStr(), mode);
	return file;
}

void FileSystemDisk::close(File& file)
{
	RIO_DELETE(*allocator, &file);
}

bool FileSystemDisk::exists(const char* path)
{
	RIO_ENSURE(nullptr != path);

	TempAllocator256 ta;
	DynamicString absolutePath(ta);
	getAbsolutePath(path, absolutePath);

	Stat info;
	OsFn::getFileInfo(info, absolutePath.getCStr());
	return info.fileType != Stat::NO_ENTRY;
}

bool FileSystemDisk::getIsDirectory(const char* path)
{
	RIO_ENSURE(nullptr != path);

	TempAllocator256 ta;
	DynamicString absolutePath(ta);
	getAbsolutePath(path, absolutePath);

	Stat info;
	OsFn::getFileInfo(info, absolutePath.getCStr());
	return info.fileType == Stat::DIRECTORY;
}

bool FileSystemDisk::getIsFile(const char* path)
{
	RIO_ENSURE(nullptr != path);

	TempAllocator256 ta;
	DynamicString absolutePath(ta);
	getAbsolutePath(path, absolutePath);

	Stat info;
	OsFn::getFileInfo(info, absolutePath.getCStr());
	return info.fileType == Stat::REGULAR;
}

uint64_t FileSystemDisk::getLastModifiedTime(const char* path)
{
	RIO_ENSURE(nullptr != path);

	TempAllocator256 ta;
	DynamicString absolutePath(ta);
	getAbsolutePath(path, absolutePath);

	Stat info;
	OsFn::getFileInfo(info, absolutePath.getCStr());
	return info.lastModifiedTime;
}

void FileSystemDisk::createDirectory(const char* path)
{
	RIO_ENSURE(nullptr != path);

	TempAllocator256 ta;
	DynamicString absolutePath(ta);
	getAbsolutePath(path, absolutePath);

	Stat info;
	OsFn::getFileInfo(info, absolutePath.getCStr());
	if (info.fileType != Stat::NO_ENTRY)
	{
		return;
	}

	OsFn::createDirectory(absolutePath.getCStr());
}

void FileSystemDisk::deleteDirectory(const char* path)
{
	RIO_ENSURE(nullptr != path);

	TempAllocator256 ta;
	DynamicString absolutePath(ta);
	getAbsolutePath(path, absolutePath);

	OsFn::deleteDirectory(absolutePath.getCStr());
}

void FileSystemDisk::deleteFile(const char* path)
{
	RIO_ENSURE(nullptr != path);

	TempAllocator256 ta;
	DynamicString absolutePath(ta);
	getAbsolutePath(path, absolutePath);

	OsFn::deleteFile(absolutePath.getCStr());
}

void FileSystemDisk::getFileList(const char* path, Vector<DynamicString>& files)
{
	RIO_ENSURE(nullptr != path);

	TempAllocator256 ta;
	DynamicString absolutePath(ta);
	getAbsolutePath(path, absolutePath);

	OsFn::getFileList(absolutePath.getCStr(), files);
}

void FileSystemDisk::getAbsolutePath(const char* path, DynamicString& osPath)
{
	if (PathFn::getIsAbsolute(path))
	{
		osPath = path;
		return;
	}

	TempAllocator1024 ta;
	DynamicString str(ta);
	PathFn::join(str, this->prefix.getCStr(), path);
	PathFn::reduceUnnecessary(osPath, str.getCStr());
}

} // namespace Rio

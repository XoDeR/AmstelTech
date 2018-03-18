#pragma once

#include "Core/FileSystem/FileSystem.h"
#include "Core/Strings/DynamicString.h"

namespace Rio
{

// Access files on disk
// All the file paths can be either relative or absolute
// When a relative path is given, it is automatically translated to absolute based on the file source's root path
// Accessing files using absolute path directly is also possible, but is platform-specific and not recommended
struct FileSystemDisk : public FileSystem
{
	Allocator* allocator = nullptr;
	DynamicString prefix;

	FileSystemDisk(Allocator& a);

	// Sets the root path to the given <prefix>
	// The <prefix> must be absolute
	void setPrefix(const char* prefix);

	File* open(const char* path, FileOpenMode::Enum mode);
	void close(File& file);
	bool exists(const char* path);
	bool getIsDirectory(const char* path);
	bool getIsFile(const char* path);
	uint64_t getLastModifiedTime(const char* path);
	void createDirectory(const char* path);
	void deleteDirectory(const char* path);
	void deleteFile(const char* path);
	void getFileList(const char* path, Vector<DynamicString>& files);
	void getAbsolutePath(const char* path, DynamicString& osPath);
};

} // namespace Rio

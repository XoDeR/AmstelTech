#pragma once

#include "Core/Containers/Types.h"
#include "Core/FileSystem/Types.h"
#include "Core/Strings/Types.h"

namespace Rio
{

// Provides a platform-independent way to access files and directories
struct FileSystem
{
	FileSystem() 
	{
	}

	virtual ~FileSystem() 
	{
	}

	FileSystem(const FileSystem&) = delete;
	FileSystem& operator=(const FileSystem&) = delete;

	// Opens the file at the given <path> with the given <mode>
	virtual File* open(const char* path, FileOpenMode::Enum mode) = 0;

	// Closes the given <file>
	virtual void close(File& file) = 0;

	// Returns whether <path> exists
	virtual bool exists(const char* path) = 0;

	// Returns true if <path> is a directory
	virtual bool getIsDirectory(const char* path) = 0;

	// Returns true if <path> is a regular file
	virtual bool getIsFile(const char* path) = 0;

	// Returns the time of last modification operaton of the <path>
	virtual uint64_t getLastModifiedTime(const char* path) = 0;

	// Creates the directory at the given <path>
	virtual void createDirectory(const char* path) = 0;

	// Deletes the directory at the given <path>
	virtual void deleteDirectory(const char* path) = 0;

	// Deletes the file at the given <path>
	virtual void deleteFile(const char* path) = 0;

	// Returns the relative file names in the given <path>
	virtual void getFileList(const char* path, Vector<DynamicString>& files) = 0;

	// Returns the absolute path of the given <path> based on the root path of the file source
	// If the <path> is absolute, the given path is returned
	virtual void getAbsolutePath(const char* path, DynamicString& osPath) = 0;
};

} // namespace Rio

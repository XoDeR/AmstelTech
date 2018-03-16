#pragma once

#include "Core/Strings/Types.h"
#include "Core/Types.h"

namespace Rio
{

// Information about a file
struct Stat
{
	enum FileType
	{
		REGULAR,
		DIRECTORY,
		NO_ENTRY
	};

	FileType fileType = NO_ENTRY;

	uint64_t size = 0;  // Size in bytes
	uint64_t lastModifiedTime = 0; // Last modified time
};

// Operating system functions
namespace OsFn
{
	// Returns the clock time
	int64_t getClockTime();

	// Returns che clock frequency
	int64_t getClockFrequency();

	// Suspends execution for <ms> milliseconds
	void sleep(uint32_t ms);

	// Opens the library at <path>
	void* libraryOpen(const char* path);

	// Closes a <library> previously opened by libraryOpen
	void libraryClose(void* library);

	// Returns a pointer to the symbol <name> in the given <library>
	void* getLibrarySymbol(void* library, const char* name);

	// Logs the message <message>
	void log(const char* message);

	// Returns information about <path>
	void getFileInfo(Stat& info, const char* path);

	// Deletes the file at <path>
	void deleteFile(const char* path);

	// Creates a directory named <path>
	void createDirectory(const char* path);

	// Deletes the directory at <path>
	void deleteDirectory(const char* path);

	// Returns the list of <files> at the given <path>
	void getFileList(const char* path, Vector<DynamicString>& files);

	// Returns the current working directory
	const char* getCurrentWorkingDirectory(char* buffer, uint32_t size);

	// Returns the value of the environment variable <name>
	const char* getEnvironmentVariable(const char* name);

	// Executes the process described by <argv> and returns its exit code
	// Fills <output> with <stdout> and <stderr>
	int executeProcess(const char* const* argv, StringStream& output);

} // namespace OsFn

} // namespace Rio

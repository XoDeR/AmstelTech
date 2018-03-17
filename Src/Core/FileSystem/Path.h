#pragma once

#include "Core/Platform.h"
#include "Core/Strings/Types.h"

namespace Rio
{

// Operating with strings as file paths
namespace PathFn
{
	// Returns whether the <path> is absolute
	bool getIsAbsolute(const char* path);

	// Returns whether the <path> is relative
	bool getIsRelative(const char* path);

	// Returns whether the <path> is the root path
	bool getIsRootPath(const char* path);

	// Appends <pathB> to <pathA> and fills <path> with the result
	void join(DynamicString& path, const char* pathA, const char* pathB);

	// Returns the basename of the <path>
	const char* getIsFileBaseName(const char* path);

	// Returns the extension of the <path> or nullptr
	const char* getFileExtension(const char* path);

	// Returns whether the <path> has a trailing separator
	bool hasTrailingSeparator(const char* path);

	// Removes unnecessary dots and separators from <path>
	void reduceUnnecessary(DynamicString& clean, const char* path);

} // namespace PathFn

} // namespace Rio

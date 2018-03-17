#include "Core/FileSystem/Path.h"
#include "Core/Strings/DynamicString.h"

#include <ctype.h> // isalpha
#include <cstring> // strrchr

namespace Rio
{

#if RIO_PLATFORM_POSIX
const char PATH_SEPARATOR = '/';
#elif RIO_PLATFORM_WINDOWS
const char PATH_SEPARATOR = '\\';
#endif // RIO_PLATFORM_POSIX

namespace PathFn
{
	bool getIsAbsolute(const char* path)
	{
		RIO_ENSURE(nullptr != path);
#if RIO_PLATFORM_POSIX
		return getStrLen32(path) > 0
			&& path[0] == PATH_SEPARATOR
			;
#elif RIO_PLATFORM_WINDOWS
		return getStrLen32(path) > 2
			&& isalpha(path[0])
			&& path[1] == ':'
			&& path[2] == PATH_SEPARATOR
			;
#endif
	}

	bool getIsRelative(const char* path)
	{
		RIO_ENSURE(nullptr != path);
		return !getIsAbsolute(path);
	}

	bool getIsRootPath(const char* path)
	{
		RIO_ENSURE(nullptr != path);
#if RIO_PLATFORM_POSIX
		return getIsAbsolute(path) && getStrLen32(path) == 1;
#elif RIO_PLATFORM_WINDOWS
		return getIsAbsolute(path) && getStrLen32(path) == 3;
#endif
	}

	void join(DynamicString& path, const char* pathA, const char* pathB)
	{
		RIO_ENSURE(nullptr != pathA);
		RIO_ENSURE(nullptr != pathB);
		const uint32_t lengthA = getStrLen32(pathA);
		const uint32_t lengthB = getStrLen32(pathB);
		path.reserve(lengthA + lengthB + 1);
		path += pathA;
		path += PATH_SEPARATOR;
		path += pathB;
	}

	const char* getIsFileBaseName(const char* path)
	{
		RIO_ENSURE(nullptr != path);
		const char* slashPosition = strrchr(path, '/');
		return slashPosition == nullptr ? path : slashPosition + 1;
	}

	const char* getFileExtension(const char* path)
	{
		RIO_ENSURE(nullptr != path);
		const char* dotPosition = strrchr(path, '.');
		return dotPosition == nullptr ? nullptr : dotPosition + 1;
	}

	bool hasTrailingSeparator(const char* path)
	{
		RIO_ENSURE(nullptr != path);
		return path[getStrLen32(path) - 1] == PATH_SEPARATOR;
	}

	inline bool getIsAnySeparator(char c)
	{
		return c == '/' || c == '\\';
	}

	void reduceUnnecessary(DynamicString& clean, const char* path)
	{
		if (path == nullptr)
		{
			return;
		}

		char currentSymbol = getIsAnySeparator(*path) ? PATH_SEPARATOR : *path;
		clean += currentSymbol;
		++path;

		for (; *path; ++path)
		{
			if (currentSymbol == PATH_SEPARATOR && getIsAnySeparator(*path))
			{
				continue;
			}

			currentSymbol = getIsAnySeparator(*path) ? PATH_SEPARATOR : *path;
			clean += currentSymbol;
		}

		if (hasTrailingSeparator(clean.getCStr()))
		{
			ArrayFn::popBack(clean.dataArray);
		}
	}

} // namespace PathFn

} // namespace Rio

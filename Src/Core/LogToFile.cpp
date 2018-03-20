#include "Core/LogToFile.h"

#include "Core/Strings/String.h"

#include "Core/FileSystem/File.h"
#include "Core/FileSystem/FileSystem.h"

#include "Core/Platform.h"

#if RIO_PLATFORM_ANDROID
#include "Core/FileSystem/FileSystemApk_Android.h"
#endif // RIO_PLATFORM_ANDROID

#include "Core/FileSystem/FileSystemDisk.h"
#include "Core/FileSystem/Path.h"

namespace Rio
{

void LogToFile::init(FileSystem* fileSystem, const char* path)
{
	lastLog = fileSystem->open(path, FileOpenMode::WRITE);
}

void LogToFile::logToFile(const char* message)
{
	if (lastLog != nullptr)
	{
		lastLog->write(message, getStrLen32(message));
		lastLog->write("\n", 1);
		lastLog->flush();
	}
}

void LogToFile::shutdown(FileSystem* fileSystem)
{
	if (lastLog != nullptr)
	{
		fileSystem->close(*lastLog);
	}
}

} // namespace Rio
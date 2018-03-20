#pragma once

#include "Core/FileSystem/Types.h"

namespace Rio
{
	struct LogToFile
	{
		File* lastLog = nullptr;

		void init(FileSystem* fileSystem, const char* path);
		void logToFile(const char* message);
		void shutdown(FileSystem* fileSystem);
	};
	
} // namespace Rio

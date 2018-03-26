#pragma once

#include "Core/Platform.h"
#include "Core/Strings/DynamicString.h"
#include "Core/Types.h"

#include "Config.h"

namespace Rio
{

struct DeviceOptions
{
	int32_t argumentsCount = 0;
	const char** argumentList = nullptr;
	DynamicString sourceDirectory;
	const char* sourceDirectoryToMapName = nullptr;
	DynamicString sourceDirectoryToMapPrefix;
	DynamicString destinationDirectory;
	const char* bootDirectory = nullptr;
	const char* platformName = nullptr;

	bool waitForConsole = false;
	bool compileAllResources = false;
	bool continueAfterCompilation = false;
	bool runEngineAsServer = false;

	uint16_t developersConsolePort = RIO_DEFAULT_CONSOLE_PORT;
	uint16_t windowX = 0;
	uint16_t windowY = 0;
	uint16_t windowWidth = RIO_DEFAULT_WINDOW_WIDTH;
	uint16_t windowHeight = RIO_DEFAULT_WINDOW_HEIGHT;

#if RIO_PLATFORM_ANDROID
	void* assetManager = nullptr;
#endif // RIO_PLATFORM_ANDROID

	DeviceOptions(Allocator& a, int32_t argumentsCount, const char** argumentList);

	// Parses the command line and returns EXIT_SUCCESS if no error is found
	int parse();
};

} // namespace Rio

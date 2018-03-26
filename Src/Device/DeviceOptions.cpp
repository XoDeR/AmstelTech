#include "Device/DeviceOptions.h"

#include "Core/CommandLine.h"
#include "Core/FileSystem/Path.h"

#include <stdlib.h>

namespace Rio
{

DeviceOptions::DeviceOptions(Allocator& a, int32_t argumentsCount, const char** argumentList)
	: argumentsCount(argumentsCount)
	, argumentList(argumentList)
	, sourceDirectory(a)
	, sourceDirectoryToMapPrefix(a)
	, destinationDirectory(a)
{
}

int DeviceOptions::parse()
{
	CommandLine commandLine(argumentsCount, argumentList);

	PathFn::reduceUnnecessary(sourceDirectory, commandLine.getParameter(0, "sourceDataDirectory"));
	PathFn::reduceUnnecessary(destinationDirectory, commandLine.getParameter(0, "destinationDataDirectory"));

	sourceDirectoryToMapName = commandLine.getParameter(0, "sourceDataDirectoryToMap");
	if (sourceDirectoryToMapName != nullptr)
	{
		PathFn::reduceUnnecessary(sourceDirectoryToMapPrefix, commandLine.getParameter(1, "sourceDataDirectoryToMap"));
		if (sourceDirectoryToMapPrefix.getIsEmpty())
		{
			printf("Error: Mapped source directory must be specified\n");
			return EXIT_FAILURE;
		}
	}

	compileAllResources = commandLine.hasOption("compile");
	if (compileAllResources)
	{
		platformName = commandLine.getParameter(0, "platform");

		if (!platformName)
		{
			// Compile for the platform the executable is built for
			platformName = RIO_PLATFORM_NAME;
		}

		if (true
			&& strcmp(platformName, "android") != 0
			&& strcmp(platformName, "linux") != 0
			&& strcmp(platformName, "windows") != 0
			)
		{
			printf("Error: Cannot compile for the given platform\n");
			return EXIT_FAILURE;
		}

		if (sourceDirectory.getIsEmpty())
		{
			printf("Error: Source directory must be specified\n");
			return EXIT_FAILURE;
		}

		// Create data destination directory using data source directory and appending a platform suffix
		if (destinationDirectory.getIsEmpty())
		{
			destinationDirectory += sourceDirectory;
			destinationDirectory += '_';
			destinationDirectory += platformName;
		}
	}

	runEngineAsServer = commandLine.hasOption("serverMode");
	if (runEngineAsServer)
	{
		if (sourceDirectory.getIsEmpty())
		{
			printf("Error: Source directory must be specified\n");
			return EXIT_FAILURE;
		}
	}

	if (!destinationDirectory.getIsEmpty())
	{
		if (!PathFn::getIsAbsolute(destinationDirectory.getCStr()))
		{
			printf("Error: Data directory must be absolute\n");
			return EXIT_FAILURE;
		}
	}

	if (!sourceDirectory.getIsEmpty())
	{
		if (!PathFn::getIsAbsolute(sourceDirectory.getCStr()))
		{
			printf("Error: Source directory must be absolute\n");
			return EXIT_FAILURE;
		}
	}

	if (!sourceDirectoryToMapPrefix.getIsEmpty())
	{
		if (!PathFn::getIsAbsolute(sourceDirectoryToMapPrefix.getCStr()))
		{
			printf("Error: Mapped source directory must be absolute\n");
			return EXIT_FAILURE;
		}
	}

	continueAfterCompilation = commandLine.hasOption("continueAfterCompilation");

	bool bootDirectoryPresent = commandLine.hasOption("bootDirectory");
	if (bootDirectoryPresent == true)
	{
		bootDirectory = commandLine.getParameter(0, "bootDirectory");
		if (bootDirectory != nullptr)
		{
			if (!PathFn::getIsRelative(bootDirectory))
			{
				printf("Error: Boot directory must be relative\n");
				return EXIT_FAILURE;
			}
		}
	}

	waitForConsole = commandLine.hasOption("waitForConsole");

	const char* consolePort = commandLine.getParameter(0, "consolePort");
	if (consolePort != nullptr)
	{
		if (sscanf(consolePort, "%hu", &(this->developersConsolePort)) != 1)
		{
			printf("Error: Console port is invalid\n");
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

} // namespace Rio

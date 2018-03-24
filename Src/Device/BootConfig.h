#pragma once

#include "Core/Strings/DynamicString.h"
#include "Core/Strings/StringId.h"
#include "Core/Types.h"

#include "Config.h"

namespace Rio
{

struct BootConfig
{
#if AMSTEL_ENGINE_SCRIPT_LUA
	StringId64 bootScriptName;
#endif // AMSTEL_ENGINE_SCRIPT_LUA

#if AMSTEL_ENGINE_LOAD_RESOURCES_FROM_BOOT_PACKAGE
	StringId64 bootPackageName;
#endif // AMSTEL_ENGINE_LOAD_RESOURCES_FROM_BOOT_PACKAGE

	DynamicString windowTitle;
	uint16_t windowWidth = RIO_DEFAULT_WINDOW_WIDTH;
	uint16_t windowHeight = RIO_DEFAULT_WINDOW_HEIGHT;
	float aspectRatio = -1.0f;
	bool vSync = true;
	bool isFullscreen = false;

	BootConfig(Allocator& a);
	bool parse(const char* json);
};

} // namespace Rio

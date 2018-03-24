#include "Device/BootConfig.h"

#include "Core/Containers/Map.h"
#include "Core/Json/JsonObject.h"
#include "Core/Json/RJson.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Platform.h"
#include "Core/Strings/DynamicString.h"

namespace Rio
{

BootConfig::BootConfig(Allocator& a)
	: windowTitle(a)
{
}

bool BootConfig::parse(const char* json)
{
	TempAllocator4096 tempAllocator4096;
	JsonObject bootConfigurationRJson(tempAllocator4096);
	RJsonFn::parse(json, bootConfigurationRJson);

	// General configs
#if AMSTEL_ENGINE_SCRIPT_LUA
	bootScriptName  = RJsonFn::parseResourceId(bootConfigurationRJson["bootScript"]);
#endif // AMSTEL_ENGINE_SCRIPT_LUA

#if AMSTEL_ENGINE_LOAD_RESOURCES_FROM_BOOT_PACKAGE
	bootPackageName = RJsonFn::parseResourceId(bootConfigurationRJson["bootPackage"]);
#endif // AMSTEL_ENGINE_LOAD_RESOURCES_FROM_BOOT_PACKAGE

	if (JsonObjectFn::has(bootConfigurationRJson, "windowTitle"))
	{
		RJsonFn::parseString(bootConfigurationRJson["windowTitle"], windowTitle);
	}

	// Platform-specific configs
	if (JsonObjectFn::has(bootConfigurationRJson, RIO_PLATFORM_NAME))
	{
		JsonObject platform(tempAllocator4096);
		RJsonFn::parse(bootConfigurationRJson[RIO_PLATFORM_NAME], platform);

		if (JsonObjectFn::has(platform, "renderer"))
		{
			JsonObject renderer(tempAllocator4096);
			RJsonFn::parse(platform["renderer"], renderer);

			if (JsonObjectFn::has(renderer, "resolution"))
			{
				JsonArray resolution(tempAllocator4096);
				RJsonFn::parseArray(renderer["resolution"], resolution);
				windowWidth = RJsonFn::parseInt32(resolution[0]);
				windowHeight = RJsonFn::parseInt32(resolution[1]);
			}
			if (JsonObjectFn::has(renderer, "aspectRatio"))
			{
				aspectRatio = RJsonFn::parseFloat(renderer["aspectRatio"]);
			}
			if (JsonObjectFn::has(renderer, "vSync"))
			{
				vSync = RJsonFn::parseBool(renderer["vSync"]);
			}
			if (JsonObjectFn::has(renderer, "isFullscreen"))
			{
				isFullscreen = RJsonFn::parseBool(renderer["fullscreen"]);
			}
		}
	}

	return true;
}

} // namespace Rio

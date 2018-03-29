#pragma once

#include "Config.h"

#include "Core/Types.h"
#include "Core/Containers/Types.h"
#include "Core/FileSystem/Types.h"
#include "Core/Memory/Allocator.h"
#include "Core/Memory/LinearAllocator.h"
#include "Core/Strings/StringId.h"
#include "Core/ConsoleServer.h"
#include "Core/LogToFile.h"

#include "Device/BootConfig.h"
#include "Device/DeviceOptions.h"
#include "Device/Display.h"
#include "Device/InputTypes.h"
#include "Device/Pipeline.h"
#include "Device/Window.h"

#if AMSTEL_ENGINE_SCRIPT_LUA
#include "Script/Types.h"
#endif // AMSTEL_ENGINE_SCRIPT_LUA

#if AMSTEL_ENGINE_GAME2D_CPP
#include "../Samples/Game2D/GameBase.h"
#endif // AMSTEL_ENGINE_GAME2D_CPP

#if AMSTEL_ENGINE_RESOURCE_MANAGER
#include "Resource/Types.h"
#endif // AMSTEL_ENGINE_RESOURCE_MANAGER

#if AMSTEL_ENGINE_WORLD
#include "World/Types.h"
#endif // AMSTEL_ENGINE_WORLD

namespace Rio
{

struct RioRendererAllocator;
struct RioRendererCallback;

struct Device
{
	LinearAllocator linearAllocator;

	const DeviceOptions& deviceOptions;
	BootConfig bootConfiguration;

	ConsoleServer* consoleServer = nullptr;
	FileSystem* dataFileSystem = nullptr;

	LogToFile loggerToFile;
	
#if AMSTEL_ENGINE_RESOURCE_MANAGER
	ResourceLoader* resourceLoader = nullptr;
	ResourceManager* resourceManager = nullptr;
#endif // AMSTEL_ENGINE_RESOURCE_MANAGER

	RioRendererAllocator* rioRendererAllocator = nullptr;
	RioRendererCallback* rioRendererCallback = nullptr;
	
	ShaderManager* shaderManager = nullptr;

	MaterialManager* materialManager = nullptr;

	InputManager* inputManager = nullptr;

	UnitManager* unitManager = nullptr;

#if AMSTEL_ENGINE_SCRIPT_LUA
	LuaEnvironment* luaEnvironment = nullptr;
#endif // AMSTEL_ENGINE_SCRIPT_LUA

#if AMSTEL_ENGINE_GAME2D_CPP
	GameBase gameBase;
#endif // AMSTEL_ENGINE_GAME2D_CPP
	
	Pipeline* pipeline = nullptr;
	Display* display = nullptr;
	Window* window = nullptr;
	Array<World*> worldList;

	uint16_t windowWidth = 0;
	uint16_t windowHeight = 0;

	bool quitApplication = false;
	bool isApplicationPaused = false;

	bool processEvents(bool isVSync);

	Device(const DeviceOptions& deviceOptions, ConsoleServer& consoleServer);
	Device(const Device&) = delete;
	Device& operator=(const Device&) = delete;

	// Runs the engine
	void run();

	// Returns the number of command line parameters
	int32_t getCommandLineArgumentsCount() const 
	{ 
		return deviceOptions.argumentsCount;
	}

	// Returns command line parameters
	const char** getCommandLineArgumentList() const
	{ 
		return (const char**)deviceOptions.argumentList;
	}

	void quit();
	void pause();
	void unpause();

	// Returns the main window resolution
	void getMainWindowResolution(uint16_t& width, uint16_t& height);

#if AMSTEL_ENGINE_WORLD
	// Renders <world> using <camera>
	void render(World& world, UnitId cameraUnitId);

	// Creates a new world
	World* createWorld();

	// Destroys the world <world>
	void destroyWorld(World& world);
#endif // AMSTEL_ENGINE_WORLD

#if AMSTEL_ENGINE_RESOURCE_MANAGER
	// Returns the resource package <id>
	ResourcePackage* createResourcePackageById(StringId64 id);

	// Destroys the resource package <resourcePackage>
	// Resources are not automatically unloaded
	// Need to call ResourcePackage::unload() before destroying a package
	void destroyResourcePackage(ResourcePackage& resourcePackage);

	// Reloads the resource <type> <name>
	void reload(StringId64 type, StringId64 name);
#endif // AMSTEL_ENGINE_RESOURCE_MANAGER

	// Logs <message> to log file and console
	void logToFile(const char* message);
};

// Runs the engine
void run(const DeviceOptions& deviceOptions);

// Returns the device
Device* getDevice();

} // namespace Rio

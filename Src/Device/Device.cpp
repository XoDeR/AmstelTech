#include "Device/Device.h"

#include "Config.h"

#include "Core/Types.h"

#include "Core/Containers/Array.h"
#include "Core/Containers/Map.h"

#include "Core/FileSystem/File.h"
#include "Core/FileSystem/FileSystem.h"
#include "Core/FileSystem/FileSystemApk_Android.h"
#include "Core/FileSystem/FileSystemDisk.h"
#include "Core/FileSystem/Path.h"

#include "Core/Json/JsonObject.h"
#include "Core/Json/RJson.h"

#include "Core/Math/Matrix4x4.h"
#include "Core/Math/Vector3.h"

#include "Core/Memory/Memory.h"
#include "Core/Memory/ProxyAllocator.h"
#include "Core/Memory/TempAllocator.h"

#include "Core/Os.h"

#include "Core/Strings/String.h"
#include "Core/Strings/StringStream.h"

#include "Core/ConsoleServer.h"
#include "Core/Profiler.h"

#include "Device/DeviceLog.h"
#include "Device/DeviceEventQueue.h"
#include "Device/InputDevice.h"
#include "Device/InputManager.h"
#include "Device/Pipeline.h"

#include "Resource/ResourceLoader.h"
#include "Resource/ResourceManager.h"

#include "Resource/ConfigResource.h"
#include "Resource/Gui/FontResource.h"

#include "Resource/Renderer/MaterialResource.h"
#include "Resource/Renderer/MeshResource.h"
#include "Resource/Renderer/ShaderResource.h"
#include "Resource/Renderer/TextureResource.h"

#include "Resource/ResourcePackage.h"
#include "Resource/PackageResource.h"

#include "Resource/LevelResource.h"
#include "Resource/UnitResource.h"

#include "Resource/Sprite/SpriteResource.h"
#include "Resource/Sprite/StateMachineResource.h"

#if AMSTEL_ENGINE_SOUND
#include "Resource/Audio/SoundResource.h"
#endif // AMSTEL_ENGINE_SOUND

#if AMSTEL_ENGINE_PHYSICS
#include "Resource/Physics/PhysicsResource.h"
#endif // AMSTEL_ENGINE_PHYSICS

#if AMSTEL_ENGINE_SCRIPT_LUA
#include "Script/LuaEnvironment.h"
#endif // AMSTEL_ENGINE_SCRIPT_LUA

#if AMSTEL_ENGINE_SCRIPT_LUA
#include "Resource/Script/LuaResource.h"
#endif // AMSTEL_ENGINE_SCRIPT_LUA

#include "World/Renderer/ShaderManager.h"
#include "World/Renderer/MaterialManager.h"

#include "World/UnitManager.h"

#if AMSTEL_ENGINE_WORLD
#include "World/World.h"
#endif // AMSTEL_ENGINE_WORLD

#if AMSTEL_ENGINE_SOUND
#include "World/Audio/Audio.h"
#endif // AMSTEL_ENGINE_SOUND

#if AMSTEL_ENGINE_PHYSICS
#include "World/Physics.h"
#endif // AMSTEL_ENGINE_PHYSICS

#include "RioRenderer/RioRenderer.h"

#include "RioCore/Memory/Allocator.h"

#define MAX_SUBSYSTEMS_HEAP 8 * 1024 * 1024

namespace 
{ 
	const Rio::LogInternal::System DEVICE = 
	{ 
		"Device" 
	}; 
}

namespace Rio
{

#if AMSTEL_ENGINE_TOOLS
extern void toolInit();
extern void toolUpdate(float);
extern void toolShutdown();
extern bool toolProcessEvents();
#endif // AMSTEL_ENGINE_TOOLS

extern bool getNextOsEvent(OsEvent& osEvent);

struct RioRendererCallback : public RioRenderer::CallbackI
{
	virtual void fatal(RioRenderer::Fatal::Enum code, const char* str)
	{
		RIO_ASSERT(false, "Fatal error: 0x%08x: %s", code, str);
		RIO_UNUSED(code);
		RIO_UNUSED(str);
	}

	virtual void traceVargs(const char* /*filePath*/, uint16_t /*line*/, const char* formatString, va_list argumentList)
	{
		char buffer[2048];
		strncpy(buffer, formatString, sizeof(buffer));
		buffer[getStrLen32(buffer)-1] = '\0'; // Remove trailing newline

		logInfoVariadic(DEVICE, buffer, argumentList);
	}

	virtual void profilerBegin(const char* /*name*/, uint32_t /*abgr*/, const char* /*filePath*/, uint16_t /*line*/)
	{
	}

	virtual void profilerBeginLiteral(const char* /*name*/, uint32_t /*abgr*/, const char* /*filePath*/, uint16_t /*line*/)
	{
	}

	virtual void profilerEnd()
	{
	}

	virtual uint32_t cacheReadSize(uint64_t /*id*/)
	{
		return 0;
	}

	virtual bool cacheRead(uint64_t /*id*/, void* /*data*/, uint32_t /*size*/)
	{
		return false;
	}

	virtual void cacheWrite(uint64_t /*id*/, const void* /*data*/, uint32_t /*size*/)
	{
	}

	virtual void screenShot(const char* /*filePath*/, uint32_t /*width*/, uint32_t /*height*/, uint32_t /*pitch*/, const void* /*data*/, uint32_t /*size*/, bool /*yFlip*/)
	{
	}

	virtual void captureBegin(uint32_t /*width*/, uint32_t /*height*/, uint32_t /*pitch*/, RioRenderer::TextureFormat::Enum /*format*/, bool /*yFlip*/)
	{
	}

	virtual void captureEnd()
	{
	}

	virtual void captureFrame(const void* /*data*/, uint32_t /*size*/)
	{
	}
};

struct RioRendererAllocator : public RioCore::AllocatorI
{
	ProxyAllocator proxyAllocator;

	RioRendererAllocator(Allocator& a)
		: proxyAllocator(a, "RioRenderer")
	{
	}

	virtual void* realloc(void* ptr, size_t size, size_t align, const char* /*file*/, uint32_t /*line*/)
	{
		if (!ptr)
		{
			return this->proxyAllocator.allocate((uint32_t)size, (uint32_t)align == 0 ? 1 : (uint32_t)align);
		}

		if (size == 0)
		{
			this->proxyAllocator.deallocate(ptr);
			return nullptr;
		}

		// Realloc
		void* resultPtr = this->proxyAllocator.allocate((uint32_t)size, (uint32_t)align == 0 ? 1 : (uint32_t)align);
		this->proxyAllocator.deallocate(ptr);
		return resultPtr;
	}
};

static void executeConsoleCommand(ConsoleServer& consoleServer, TcpSocket client, const char* json, void* userData)
{
	TempAllocator4096 tempAllocator4096;
	JsonObject consoleCommandJsonObject(tempAllocator4096);
	JsonArray argumentList(tempAllocator4096);

	RJsonFn::parse(json, consoleCommandJsonObject);
	RJsonFn::parseArray(consoleCommandJsonObject["argumentList"], argumentList);

	DynamicString commandString(tempAllocator4096);
	RJsonFn::parseString(argumentList[0], commandString);

	if (commandString == "pause")
	{
		getDevice()->pause();
	}
	else if (commandString == "unpause")
	{
		getDevice()->unpause();
	}
	else if (commandString == "reload")
	{
		if (ArrayFn::getCount(argumentList) != 3)
		{
			consoleServer.sendErrorMessage(client, "Usage: reload type name");
			return;
		}

		DynamicString typeString(tempAllocator4096);
		DynamicString nameString(tempAllocator4096);
		RJsonFn::parseString(argumentList[1], typeString);
		RJsonFn::parseString(argumentList[2], nameString);

		((Device*)userData)->reload(ResourceId(typeString.getCStr()), ResourceId(nameString.getCStr()));
	}
}

Device::Device(const DeviceOptions& deviceOptions, ConsoleServer& consoleServer)
	: linearAllocator(getDefaultAllocator(), MAX_SUBSYSTEMS_HEAP)
	, deviceOptions(deviceOptions)
	, bootConfiguration(getDefaultAllocator())
	, consoleServer(&consoleServer)
	, worldList(getDefaultAllocator())
{
}

bool Device::processEvents(bool vsync)
{
#if AMSTEL_ENGINE_TOOLS
	return toolProcessEvents();
#endif // AMSTEL_ENGINE_TOOLS

	bool exit = false;
	bool reset = false;

	OsEvent osEvent;
	while (getNextOsEvent(osEvent))
	{
		if (osEvent.type == OsEventType::NONE)
		{
			continue;
		}

		switch (osEvent.type)
		{
		case OsEventType::BUTTON:
			{
				const ButtonEvent buttonEvent = osEvent.button;
				switch (buttonEvent.deviceId)
				{
				case InputDeviceType::KEYBOARD:
					inputManager->getKeyboardInputDevice()->setButtonPressedState(buttonEvent.buttonIndex, buttonEvent.isPressed);
					break;

				case InputDeviceType::MOUSE:
					inputManager->getMouseInputDevice()->setButtonPressedState(buttonEvent.buttonIndex, buttonEvent.isPressed);
					break;

				case InputDeviceType::TOUCHSCREEN:
					inputManager->getTouchInputDevice()->setButtonPressedState(buttonEvent.buttonIndex, buttonEvent.isPressed);
					break;

				case InputDeviceType::JOYPAD:
					inputManager->getJoypadByIndex(buttonEvent.deviceIndex)->setButtonPressedState(buttonEvent.buttonIndex, buttonEvent.isPressed);
					break;
				}
			}
			break;

		case OsEventType::AXIS:
			{
				const AxisEvent axisEvent = osEvent.axis;
				switch (axisEvent.deviceId)
				{
				case InputDeviceType::MOUSE:
					inputManager->getMouseInputDevice()->setAxis(axisEvent.axisIndex, createVector3(axisEvent.axisX, axisEvent.axisY, axisEvent.axisZ));
					break;

				case InputDeviceType::JOYPAD:
					inputManager->getJoypadByIndex(axisEvent.deviceIndex)->setAxis(axisEvent.axisIndex, createVector3(axisEvent.axisX, axisEvent.axisY, axisEvent.axisZ));
					break;
				}
			}
			break;

		case OsEventType::STATUS:
			{
				const StatusEvent statusEvent = osEvent.status;
				switch (statusEvent.deviceId)
				{
				case InputDeviceType::JOYPAD:
					inputManager->getJoypadByIndex(statusEvent.deviceIndex)->isConnected = statusEvent.isConnected;
					break;
				}
			}
			break;

		case OsEventType::RESOLUTION:
			{
				const ResolutionEvent& resolutionEvent = osEvent.resolution;
				this->windowWidth = resolutionEvent.width;
				this->windowHeight = resolutionEvent.height;
				reset   = true;
			}
			break;

		case OsEventType::EXIT:
			exit = true;
			break;

		case OsEventType::PAUSE:
			pause();
			break;

		case OsEventType::RESUME:
			unpause();
			break;

		case OsEventType::TEXT:
			break;

		default:
			RIO_FATAL("Unknown OS event");
			break;
		}
	}

	if (reset)
	{
		RioRenderer::reset(windowWidth, windowHeight, (vsync ? RIO_RENDERER_RESET_VSYNC : RIO_RENDERER_RESET_NONE));
	}

	return exit;
}

void Device::run()
{
	consoleServer->registerCommand("command", executeConsoleCommand, this);

	consoleServer->listen(deviceOptions.developersConsolePort, deviceOptions.waitForConsole);

#if RIO_PLATFORM_ANDROID
	dataFileSystem = RIO_NEW(linearAllocator, FileSystemApk)(getDefaultAllocator(), const_cast<AAssetManager*>((AAssetManager*)deviceOptions.assetManager));
#else
	dataFileSystem = RIO_NEW(linearAllocator, FileSystemDisk)(getDefaultAllocator());
	{
		char currentWorkingDirectoryName[1024];
		const char* dataDirectory = !deviceOptions.destinationDirectory.getIsEmpty()
			? deviceOptions.destinationDirectory.getCStr()
			: OsFn::getCurrentWorkingDirectory(currentWorkingDirectoryName, sizeof(currentWorkingDirectoryName))
			;
		((FileSystemDisk*)dataFileSystem)->setPrefix(dataDirectory);
	}

	loggerToFile.init(dataFileSystem, AMSTEL_ENGINE_LAST_LOG);
#endif // RIO_PLATFORM_ANDROID

	logInfo(DEVICE, "Initializing RIO Engine %s %s %s", AMSTEL_ENGINE_VERSION, RIO_PLATFORM_NAME, RIO_ARCH_NAME);

	ProfilerGlobalFn::init();

	resourceLoader = RIO_NEW(linearAllocator, ResourceLoader)(*dataFileSystem);

	resourceManager = RIO_NEW(linearAllocator, ResourceManager)(*resourceLoader);

	resourceManager->registerNewResourceType(RESOURCE_TYPE_CONFIG, RESOURCE_VERSION_CONFIG, ConfigResourceInternalFn::load, ConfigResourceInternalFn::unload, nullptr, nullptr);
	resourceManager->registerNewResourceType(RESOURCE_TYPE_FONT, RESOURCE_VERSION_FONT, nullptr, nullptr, nullptr, nullptr);
	resourceManager->registerNewResourceType(RESOURCE_TYPE_LEVEL, RESOURCE_VERSION_LEVEL, nullptr, nullptr, nullptr, nullptr);
	resourceManager->registerNewResourceType(RESOURCE_TYPE_MATERIAL, RESOURCE_VERSION_MATERIAL, MaterialResourceInternalFn::load, MaterialResourceInternalFn::unload, MaterialResourceInternalFn::online, MaterialResourceInternalFn::offline);
	resourceManager->registerNewResourceType(RESOURCE_TYPE_MESH, RESOURCE_VERSION_MESH, MeshResourceInternalFn::load, MeshResourceInternalFn::unload, MeshResourceInternalFn::online, MeshResourceInternalFn::offline);
	resourceManager->registerNewResourceType(RESOURCE_TYPE_PACKAGE, RESOURCE_VERSION_PACKAGE, PackageResourceInternalFn::load, PackageResourceInternalFn::unload, nullptr, nullptr);
	resourceManager->registerNewResourceType(RESOURCE_TYPE_SHADER, RESOURCE_VERSION_SHADER, ShaderResourceInternalFn::load, ShaderResourceInternalFn::unload, ShaderResourceInternalFn::online, ShaderResourceInternalFn::offline);
	resourceManager->registerNewResourceType(RESOURCE_TYPE_SPRITE, RESOURCE_VERSION_SPRITE, nullptr, nullptr, nullptr, nullptr);
	resourceManager->registerNewResourceType(RESOURCE_TYPE_SPRITE_ANIMATION, RESOURCE_VERSION_SPRITE_ANIMATION, nullptr, nullptr, nullptr, nullptr);
	resourceManager->registerNewResourceType(RESOURCE_TYPE_STATE_MACHINE, RESOURCE_VERSION_STATE_MACHINE, nullptr, nullptr, nullptr, nullptr);
	resourceManager->registerNewResourceType(RESOURCE_TYPE_TEXTURE, RESOURCE_VERSION_TEXTURE, TextureResourceInternalFn::load, TextureResourceInternalFn::unload, TextureResourceInternalFn::online, TextureResourceInternalFn::offline);
	resourceManager->registerNewResourceType(RESOURCE_TYPE_UNIT, RESOURCE_VERSION_UNIT, nullptr, nullptr, nullptr, nullptr);

#if AMSTEL_ENGINE_SOUND
	resourceManager->registerNewResourceType(RESOURCE_TYPE_SOUND, RESOURCE_VERSION_SOUND, nullptr, nullptr, nullptr, nullptr);
#endif // AMSTEL_ENGINE_SOUND

#if AMSTEL_ENGINE_PHYSICS
	resourceManager->registerNewResourceType(RESOURCE_TYPE_PHYSICS, RESOURCE_VERSION_PHYSICS, nullptr, nullptr, nullptr, nullptr);
	resourceManager->registerNewResourceType(RESOURCE_TYPE_PHYSICS_CONFIG, RESOURCE_VERSION_PHYSICS_CONFIG, nullptr, nullptr, nullptr, nullptr);
#endif // AMSTEL_ENGINE_PHYSICS

#if AMSTEL_ENGINE_SCRIPT_LUA
	resourceManager->registerNewResourceType(RESOURCE_TYPE_SCRIPT, RESOURCE_VERSION_SCRIPT, nullptr, nullptr, nullptr, nullptr);
#endif // AMSTEL_ENGINE_SCRIPT_LUA

	// Read config
	{
		TempAllocator1024 tempAllocator1024;
		DynamicString bootDirectoryName(tempAllocator1024);

		if (deviceOptions.bootDirectory != nullptr)
		{
			bootDirectoryName += deviceOptions.bootDirectory;
			bootDirectoryName += '/';
		}
		bootDirectoryName += AMSTEL_ENGINE_BOOT_CONFIG;

		const StringId64 configName(bootDirectoryName.getCStr());

		resourceManager->load(RESOURCE_TYPE_CONFIG, configName);
		resourceManager->flush();
		bootConfiguration.parse((const char*)resourceManager->getResourceData(RESOURCE_TYPE_CONFIG, configName));
		resourceManager->unload(RESOURCE_TYPE_CONFIG, configName);
	}

	// Init all remaining subsystems
	rioRendererAllocator = RIO_NEW(linearAllocator, RioRendererAllocator)(getDefaultAllocator());
	rioRendererCallback = RIO_NEW(linearAllocator, RioRendererCallback)();

	this->display = DisplayFn::create(linearAllocator);

	this->windowWidth = bootConfiguration.windowWidth;
	this->windowHeight = bootConfiguration.windowHeight;

	window = WindowFn::create(linearAllocator);
	window->open(deviceOptions.windowX
		, deviceOptions.windowY
		, this->windowWidth
		, this->windowHeight
		);

	window->setWindowTitle(bootConfiguration.windowTitle.getCStr());
	window->setIsFullscreen(bootConfiguration.isFullscreen);
	window->rioRendererSetup();

	// RIO_RENDERER_PCI_ID_NONE Auto select video adapter
	RioRenderer::init(RioRenderer::RendererType::Count
		, RIO_RENDERER_PCI_ID_NONE
		, 0
		, rioRendererCallback
		, rioRendererAllocator
		);

	RioRenderer::reset(windowWidth, windowHeight, (bootConfiguration.vSync ? RIO_RENDERER_RESET_VSYNC : RIO_RENDERER_RESET_NONE));

	shaderManager = RIO_NEW(this->linearAllocator, ShaderManager)(getDefaultAllocator());

	materialManager = RIO_NEW(this->linearAllocator, MaterialManager)(getDefaultAllocator(), *resourceManager);
	
	inputManager = RIO_NEW(this->linearAllocator, InputManager)(getDefaultAllocator());

	unitManager = RIO_NEW(this->linearAllocator, UnitManager)(getDefaultAllocator());

#if AMSTEL_ENGINE_SCRIPT_LUA
	luaEnvironment  = RIO_NEW(linearAllocator, LuaEnvironment)();
#endif // AMSTEL_ENGINE_SCRIPT_LUA

#if AMSTEL_ENGINE_SOUND
	AudioGlobalFn::init();
#endif // AMSTEL_ENGINE_SOUND

#if AMSTEL_ENGINE_PHYSICS
	PhysicsGlobalFn::init(linearAllocator);
#endif // AMSTEL_ENGINE_PHYSICS

#if AMSTEL_ENGINE_LOAD_RESOURCES_FROM_BOOT_PACKAGE
	ResourcePackage* bootResourcePackage = createResourcePackageById(bootConfiguration.bootPackageName);
	bootResourcePackage->load();
	bootResourcePackage->flush();
#endif // AMSTEL_ENGINE_LOAD_RESOURCES_FROM_BOOT_PACKAGE

#if AMSTEL_ENGINE_SCRIPT_LUA
	luaEnvironment->loadLuaLibraries();
	luaEnvironment->execute((LuaResource*)resourceManager->getResourceData(RESOURCE_TYPE_SCRIPT, bootConfiguration.bootScriptName));
#endif // AMSTEL_ENGINE_SCRIPT_LUA

	pipeline = RIO_NEW(linearAllocator, Pipeline)();
	pipeline->create(windowWidth, windowHeight);

#if AMSTEL_ENGINE_TOOLS
	toolInit();
#endif // AMSTEL_ENGINE_TOOLS

	logInfo(DEVICE, "Initialized");

#if AMSTEL_ENGINE_SCRIPT_LUA
	luaEnvironment->callGlobalFunction("init", 0);
#endif // AMSTEL_ENGINE_SCRIPT_LUA

#if AMSTEL_ENGINE_GAME2D_CPP
	gameBase.init();
#endif // AMSTEL_ENGINE_GAME2D_CPP

	int64_t timeLast = OsFn::getClockTime();
	uint16_t oldWindowWidth = 0;
	uint16_t oldWindowHeight = 0;

	// Main loop
	while (!processEvents(bootConfiguration.vSync) && !quitApplication)
	{
		const int64_t time = OsFn::getClockTime();
		const double clockFrequency = (double)OsFn::getClockFrequency();
		const float dt = float(double(time - timeLast) / clockFrequency);
		timeLast = time;

		ProfilerGlobalFn::clear();

		consoleServer->update();

		RECORD_FLOAT("device.dt", dt);
		RECORD_FLOAT("device.fps", 1.0f/dt);

		if (this->windowWidth != oldWindowWidth || this->windowHeight != oldWindowHeight)
		{
			oldWindowWidth = this->windowWidth;
			oldWindowHeight = this->windowHeight;

			pipeline->reset(this->windowWidth, this->windowHeight);
		}

#if AMSTEL_ENGINE_TOOLS
		toolUpdate(dt);
#endif // AMSTEL_ENGINE_TOOLS

		if (!isApplicationPaused)
		{
			resourceManager->completeLoadRequests();

#if AMSTEL_ENGINE_SCRIPT_LUA
			{
				const int64_t startTime = OsFn::getClockTime();
				luaEnvironment->callGlobalFunction("update", 1, ARGUMENT_FLOAT, dt);
				RECORD_FLOAT("lua.update", float(double(OsFn::getClockTime() - startTime) / clockFrequency));
			}
			{
				const int64_t startTime = OsFn::getClockTime();
				luaEnvironment->callGlobalFunction("render", 1, ARGUMENT_FLOAT, dt);
				RECORD_FLOAT("lua.render", float(double(OsFn::getClockTime() - startTime) / clockFrequency));
			}
#endif // AMSTEL_ENGINE_SCRIPT_LUA

#if AMSTEL_ENGINE_GAME2D_CPP
			gameBase.update(dt);
			gameBase.render(dt);
#endif // AMSTEL_ENGINE_GAME2D_CPP
		}

#if AMSTEL_ENGINE_SCRIPT_LUA
		luaEnvironment->resetTemporaryTypes();
#endif // AMSTEL_ENGINE_SCRIPT_LUA
		
		inputManager->update();

		const RioRenderer::Stats* stats = RioRenderer::getStats();
		RECORD_FLOAT("RioRenderer.gpuTime", float(double(stats->gpuTimeEnd - stats->gpuTimeBegin)*1000.0/stats->gpuTimerFreq));
		RECORD_FLOAT("RioRenderer.cpuTime", float(double(stats->cpuTimeEnd - stats->cpuTimeBegin)*1000.0/stats->cpuTimerFreq));

		ProfilerGlobalFn::flush();

		RioRenderer::frame();
	}

#if AMSTEL_ENGINE_TOOLS
	toolShutdown();
#endif // AMSTEL_ENGINE_TOOLS

#if AMSTEL_ENGINE_SCRIPT_LUA
	luaEnvironment->callGlobalFunction("shutdown", 0);
#endif // AMSTEL_ENGINE_SCRIPT_LUA

#if AMSTEL_ENGINE_LOAD_RESOURCES_FROM_BOOT_PACKAGE
	bootResourcePackage->unload();
	destroyResourcePackage(*bootResourcePackage);
#endif // AMSTEL_ENGINE_LOAD_RESOURCES_FROM_BOOT_PACKAGE

#if AMSTEL_ENGINE_PHYSICS
	PhysicsGlobalFn::shutdown(linearAllocator);
#endif // AMSTEL_ENGINE_PHYSICS

#if AMSTEL_ENGINE_SOUND
	AudioGlobalFn::shutdown();
#endif // AMSTEL_ENGINE_SOUND

	RIO_DELETE(linearAllocator, pipeline);

#if AMSTEL_ENGINE_SCRIPT_LUA
	RIO_DELETE(linearAllocator, luaEnvironment);
#endif // AMSTEL_ENGINE_SCRIPT_LUA
	
	RIO_DELETE(linearAllocator, unitManager);

	RIO_DELETE(linearAllocator, inputManager);

	RIO_DELETE(linearAllocator, materialManager);
	
	RIO_DELETE(linearAllocator, shaderManager);

	RIO_DELETE(linearAllocator, resourceManager);
	
	RIO_DELETE(linearAllocator, resourceLoader);

	pipeline->destroy();

	RioRenderer::shutdown();

	window->close();

	WindowFn::destroy(linearAllocator, *window);
	DisplayFn::destroy(linearAllocator, *display);
	RIO_DELETE(linearAllocator, rioRendererCallback);
	RIO_DELETE(linearAllocator, rioRendererAllocator);

	loggerToFile.shutdown(dataFileSystem);

	RIO_DELETE(linearAllocator, dataFileSystem);

	ProfilerGlobalFn::shutdown();

	linearAllocator.clear();
}

void Device::quit()
{
	quitApplication = true;
}

void Device::pause()
{
	isApplicationPaused = true;
	logInfo(DEVICE, "Paused");
}

void Device::unpause()
{
	isApplicationPaused = false;
	logInfo(DEVICE, "Unpaused");
}

void Device::getMainWindowResolution(uint16_t& width, uint16_t& height)
{
	width = this->windowWidth;
	height = this->windowHeight;
}

#if AMSTEL_ENGINE_WORLD

void Device::render(World& world, UnitId cameraUnitId)
{
	float aspectRatio = (bootConfiguration.aspectRatio == -1.0f
		? (float)windowWidth /(float)windowHeight
		: bootConfiguration.aspectRatio
		);

	world.cameraSetAspect(cameraUnitId, aspectRatio);
	world.cameraSetViewportMetrics(cameraUnitId, 0, 0, windowWidth, windowHeight);

	const Matrix4x4 viewMatrix4x4 = world.getCameraViewMatrix(cameraUnitId);
	const Matrix4x4 projectionMatrix4x4 = world.getCameraProjectionMatrix(cameraUnitId);

	Matrix4x4 orthoProjectionMatrix4x4;
	setToOrthographic(orthoProjectionMatrix4x4, 0, windowWidth, 0, windowHeight, 0.01f, 1.0f);

	uint32_t rgbaColorPacked = 0x303030ff;
	RioRenderer::setViewClear(VIEW_SPRITE_0, RIO_RENDERER_CLEAR_COLOR | RIO_RENDERER_CLEAR_DEPTH, rgbaColorPacked, 1.0f, 0);

	RioRenderer::setViewTransform(VIEW_SPRITE_0, getFloatPtr(viewMatrix4x4), getFloatPtr(projectionMatrix4x4));
	RioRenderer::setViewTransform(VIEW_SPRITE_1, getFloatPtr(viewMatrix4x4), getFloatPtr(projectionMatrix4x4));
	RioRenderer::setViewTransform(VIEW_SPRITE_2, getFloatPtr(viewMatrix4x4), getFloatPtr(projectionMatrix4x4));
	RioRenderer::setViewTransform(VIEW_SPRITE_3, getFloatPtr(viewMatrix4x4), getFloatPtr(projectionMatrix4x4));
	RioRenderer::setViewTransform(VIEW_SPRITE_4, getFloatPtr(viewMatrix4x4), getFloatPtr(projectionMatrix4x4));
	RioRenderer::setViewTransform(VIEW_SPRITE_5, getFloatPtr(viewMatrix4x4), getFloatPtr(projectionMatrix4x4));
	RioRenderer::setViewTransform(VIEW_SPRITE_6, getFloatPtr(viewMatrix4x4), getFloatPtr(projectionMatrix4x4));
	RioRenderer::setViewTransform(VIEW_SPRITE_7, getFloatPtr(viewMatrix4x4), getFloatPtr(projectionMatrix4x4));
	RioRenderer::setViewTransform(VIEW_MESH, getFloatPtr(viewMatrix4x4), getFloatPtr(projectionMatrix4x4));
	RioRenderer::setViewTransform(VIEW_DEBUG, getFloatPtr(viewMatrix4x4), getFloatPtr(projectionMatrix4x4));
	RioRenderer::setViewTransform(VIEW_GUI, getFloatPtr(MATRIX4X4_IDENTITY), getFloatPtr(orthoProjectionMatrix4x4));

	RioRenderer::setViewRect(VIEW_SPRITE_0, 0, 0, windowWidth, windowHeight);
	RioRenderer::setViewRect(VIEW_SPRITE_1, 0, 0, windowWidth, windowHeight);
	RioRenderer::setViewRect(VIEW_SPRITE_2, 0, 0, windowWidth, windowHeight);
	RioRenderer::setViewRect(VIEW_SPRITE_3, 0, 0, windowWidth, windowHeight);
	RioRenderer::setViewRect(VIEW_SPRITE_4, 0, 0, windowWidth, windowHeight);
	RioRenderer::setViewRect(VIEW_SPRITE_5, 0, 0, windowWidth, windowHeight);
	RioRenderer::setViewRect(VIEW_SPRITE_6, 0, 0, windowWidth, windowHeight);
	RioRenderer::setViewRect(VIEW_SPRITE_7, 0, 0, windowWidth, windowHeight);
	RioRenderer::setViewRect(VIEW_MESH, 0, 0, windowWidth, windowHeight);
	RioRenderer::setViewRect(VIEW_DEBUG, 0, 0, windowWidth, windowHeight);
	RioRenderer::setViewRect(VIEW_GUI, 0, 0, windowWidth, windowHeight);

	RioRenderer::setViewMode(VIEW_SPRITE_0, RioRenderer::ViewMode::DepthAscending);
	RioRenderer::setViewMode(VIEW_SPRITE_1, RioRenderer::ViewMode::DepthAscending);
	RioRenderer::setViewMode(VIEW_SPRITE_2, RioRenderer::ViewMode::DepthAscending);
	RioRenderer::setViewMode(VIEW_SPRITE_3, RioRenderer::ViewMode::DepthAscending);
	RioRenderer::setViewMode(VIEW_SPRITE_4, RioRenderer::ViewMode::DepthAscending);
	RioRenderer::setViewMode(VIEW_SPRITE_5, RioRenderer::ViewMode::DepthAscending);
	RioRenderer::setViewMode(VIEW_SPRITE_6, RioRenderer::ViewMode::DepthAscending);
	RioRenderer::setViewMode(VIEW_SPRITE_7, RioRenderer::ViewMode::DepthAscending);
	RioRenderer::setViewMode(VIEW_GUI, RioRenderer::ViewMode::Sequential);

	RioRenderer::setViewFrameBuffer(VIEW_SPRITE_0, pipeline->frameBufferHandle);
	RioRenderer::setViewFrameBuffer(VIEW_SPRITE_1, pipeline->frameBufferHandle);
	RioRenderer::setViewFrameBuffer(VIEW_SPRITE_2, pipeline->frameBufferHandle);
	RioRenderer::setViewFrameBuffer(VIEW_SPRITE_3, pipeline->frameBufferHandle);
	RioRenderer::setViewFrameBuffer(VIEW_SPRITE_4, pipeline->frameBufferHandle);
	RioRenderer::setViewFrameBuffer(VIEW_SPRITE_5, pipeline->frameBufferHandle);
	RioRenderer::setViewFrameBuffer(VIEW_SPRITE_6, pipeline->frameBufferHandle);
	RioRenderer::setViewFrameBuffer(VIEW_SPRITE_7, pipeline->frameBufferHandle);

	RioRenderer::setViewFrameBuffer(VIEW_MESH, pipeline->frameBufferHandle);
	RioRenderer::setViewFrameBuffer(VIEW_DEBUG, pipeline->frameBufferHandle);
	RioRenderer::setViewFrameBuffer(VIEW_GUI, pipeline->frameBufferHandle);

	RioRenderer::touch(VIEW_SPRITE_0);
	RioRenderer::touch(VIEW_SPRITE_1);
	RioRenderer::touch(VIEW_SPRITE_2);
	RioRenderer::touch(VIEW_SPRITE_3);
	RioRenderer::touch(VIEW_SPRITE_4);
	RioRenderer::touch(VIEW_SPRITE_5);
	RioRenderer::touch(VIEW_SPRITE_6);
	RioRenderer::touch(VIEW_SPRITE_7);

	RioRenderer::touch(VIEW_MESH);
	RioRenderer::touch(VIEW_DEBUG);
	RioRenderer::touch(VIEW_GUI);

	world.render(viewMatrix4x4, projectionMatrix4x4);

#if !AMSTEL_ENGINE_TOOLS
	pipeline->render(*shaderManager, StringId32("blit"), 0, this->windowWidth, this->windowHeight);
#endif // !AMSTEL_ENGINE_TOOLS
}
#endif // AMSTEL_ENGINE_WORLD

#if AMSTEL_ENGINE_WORLD

World* Device::createWorld()
{

#if AMSTEL_ENGINE_SCRIPT_LUA

	World* world = RIO_NEW(getDefaultAllocator(), World)(getDefaultAllocator()
		, *resourceManager
		, *shaderManager
		, *materialManager
		, *unitManager
		, *luaEnvironment
		);

#else

	World* world = RIO_NEW(getDefaultAllocator(), World)(getDefaultAllocator()
		, *resourceManager
		, *shaderManager
		, *materialManager
		, *unitManager
		);

#endif // AMSTEL_ENGINE_SCRIPT_LUA

	ArrayFn::pushBack(worldList, world);
	return world;
}

void Device::destroyWorld(World& world)
{
	for (uint32_t i = 0, n = ArrayFn::getCount(worldList); i < n; ++i)
	{
		if (&world == worldList[i])
		{
			RIO_DELETE(getDefaultAllocator(), &world);
			worldList[i] = worldList[n - 1];
			ArrayFn::popBack(worldList);
			return;
		}
	}

	RIO_FATAL("World not found");
}
#endif // AMSTEL_ENGINE_WORLD

ResourcePackage* Device::createResourcePackageById(StringId64 id)
{
	return RIO_NEW(getDefaultAllocator(), ResourcePackage)(id, *resourceManager);
}

void Device::destroyResourcePackage(ResourcePackage& resourcePackage)
{
	RIO_DELETE(getDefaultAllocator(), &resourcePackage);
}

void Device::reload(StringId64 type, StringId64 name)
{
	StringId64 mix;
	mix.id = type.id ^ name.id;

	TempAllocator1024 tempAllocator1024;
	DynamicString path(tempAllocator1024);
	mix.toString(path);

	logInfo(DEVICE, "Reloading #ID(%s)", path.getCStr());

	resourceManager->reload(type, name);
	const void* newResource = resourceManager->getResourceData(type, name);

#if AMSTEL_ENGINE_SCRIPT_LUA
	if (type == RESOURCE_TYPE_SCRIPT)
	{
		luaEnvironment->execute((const LuaResource*)newResource);
	}
#endif // AMSTEL_ENGINE_SCRIPT_LUA

	logInfo(DEVICE, "Reloaded #ID(%s)", path.getCStr());
}

void Device::logToFile(const char* message)
{
	loggerToFile.logToFile(message);
}

char deviceGlobalBuffer[sizeof(Device)];

Device* deviceGlobal = nullptr;

void run(const DeviceOptions& deviceOptions)
{
	RIO_ASSERT(deviceGlobal == nullptr, "RIO Engine already initialized");
	
	ConsoleServerGlobalFn::init();

	deviceGlobal = new (deviceGlobalBuffer) Device(deviceOptions, *getConsoleServerGlobal());
	deviceGlobal->run();
	deviceGlobal->~Device();
	deviceGlobal = nullptr;

	ConsoleServerGlobalFn::shutdown();
}

Device* getDevice()
{
	return Rio::deviceGlobal;
}

} // namespace Rio

#include "Resource/DataCompiler.h"

#include "Config.h"

#include "Core/Containers/HashMap.h"
#include "Core/Containers/Map.h"
#include "Core/Containers/Vector.h"
#include "Core/FileSystem/File.h"
#include "Core/FileSystem/FileSystemDisk.h"
#include "Core/FileSystem/Path.h"
#include "Core/Json/JsonObject.h"
#include "Core/Json/RJson.h"
#include "Core/Memory/Allocator.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Os.h"
#include "Core/Strings/DynamicString.h"
#include "Core/Strings/StringStream.h"
#include "Core/ConsoleServer.h"

#include "Device/DeviceLog.h"

#include "Device/DeviceOptions.h"

#include "Resource/Types.h"

#include "Resource/CompileOptions.h"
#include "Resource/ConfigResource.h"

#include "Resource/Gui/FontResource.h"

#include "Resource/LevelResource.h"
#include "Resource/PackageResource.h"
#include "Resource/UnitResource.h"

#include "Resource/Renderer/TextureResource.h"
#include "Resource/Renderer/MaterialResource.h"
#include "Resource/Renderer/MeshResource.h"
#include "Resource/Renderer/ShaderResource.h"

#include "Resource/Sprite/SpriteResource.h"
#include "Resource/Sprite/StateMachineResource.h"

#if AMSTEL_ENGINE_SOUND
#include "Resource/Audio/SoundResource.h"
#endif // AMSTEL_ENGINE_SOUND

#if AMSTEL_ENGINE_SCRIPT_LUA
#include "Resource/Script/LuaResource.h"
#endif // AMSTEL_ENGINE_SCRIPT_LUA

#if AMSTEL_ENGINE_PHYSICS
#include "Resource/Physics/PhysicsResource.h"
#endif // AMSTEL_ENGINE_PHYSICS

namespace 
{ 
	const Rio::LogInternal::System COMPILER = { "Compiler" }; 
}

namespace Rio
{

struct LineReader
{
	const char* lineString = nullptr;
	const uint32_t length = 0;
	uint32_t position = 0;

	LineReader(const char* lineString)
		: lineString(lineString)
		, length(getStrLen32(lineString))
	{
	}

	void readLine(DynamicString& line)
	{
		const char* currentString = &lineString[position];
		const char* newLine = getStrNl(currentString);
		position += uint32_t(newLine - currentString);
		line.set(currentString, uint32_t(newLine - currentString));
	}

	bool getIsEndOfFile()
	{
		return lineString[position] == '\0';
	}
};

static void consoleCommandCompile(ConsoleServer& consoleServer, TcpSocket clientTcpSocket, const char* json, void* userData)
{
	TempAllocator4096 tempAllocator4096;
	JsonObject consoleCommandJsonObject(tempAllocator4096);
	DynamicString idString(tempAllocator4096);
	DynamicString dataDirectoryString(tempAllocator4096);
	DynamicString platformString(tempAllocator4096);

	RJsonFn::parse(json, consoleCommandJsonObject);
	RJsonFn::parseString(consoleCommandJsonObject["id"], idString);
	RJsonFn::parseString(consoleCommandJsonObject["dataDirectory"], dataDirectoryString);
	RJsonFn::parseString(consoleCommandJsonObject["platform"], platformString);

	{
		TempAllocator512 tempAllocator512;
		StringStream stringStream(tempAllocator512);
		stringStream << "{\"type\":\"compile\",\"id\":\"" << idString.getCStr() << "\",\"start\":true}";
		consoleServer.send(clientTcpSocket, StringStreamFn::getCStr(stringStream));
	}

	logInfo(COMPILER, "Compiling '%s'", idString.getCStr());
	bool compilationSuccess = ((DataCompiler*)userData)->compile(dataDirectoryString.getCStr(), platformString.getCStr());

	if (compilationSuccess == true)
	{
		logInfo(COMPILER, "Compiled '%s'", idString.getCStr());
	}
	else
	{
		logError(COMPILER, "Failed to compile '%s'", idString.getCStr());
	}

	{
		TempAllocator512 tempAllocator512;
		StringStream stringStream(tempAllocator512);
		stringStream << "{\"type\":\"compile\",\"id\":\"" << idString.getCStr() << "\",\"success\":" << (compilationSuccess ? "true" : "false") << "}";
		consoleServer.send(clientTcpSocket, StringStreamFn::getCStr(stringStream));
	}
}

DataCompiler::DataCompiler(ConsoleServer& consoleServer)
	: consoleServer(&consoleServer)
	, sourceFileSystem(getDefaultAllocator())
	, sourceDirectoryList(getDefaultAllocator())
	, resourceTypeDataMap(getDefaultAllocator())
	, fileNameList(getDefaultAllocator())
	, globList(getDefaultAllocator())
	, dataIndexMap(getDefaultAllocator())

#if AMSTEL_ENGINE_FILE_MONITOR_IMPLEMENTED
	, fileMonitor(getDefaultAllocator())
#endif // AMSTEL_ENGINE_FILE_MONITOR_IMPLEMENTED
{
	consoleServer.registerCommand("compile", consoleCommandCompile, this);
}

DataCompiler::~DataCompiler()
{
#if AMSTEL_ENGINE_FILE_MONITOR_IMPLEMENTED
	fileMonitor.stop();
#endif // AMSTEL_ENGINE_FILE_MONITOR_IMPLEMENTED
}

void DataCompiler::addFile(const char* path)
{
	for (uint32_t globIndex = 0; globIndex < VectorFn::getCount(this->globList); ++globIndex)
	{
		if (wildCmp(this->globList[globIndex].getCStr(), path))
		{
			return;
		}
	}

	TempAllocator512 tempAllocator512;
	DynamicString pathString(tempAllocator512);
	pathString.set(path, getStrLen32(path));
	VectorFn::pushBack(this->fileNameList, pathString);

	StringStream stringStream(tempAllocator512);
	stringStream << "{\"type\":\"addFile\",\"path\":\"" << pathString.getCStr() << "\"}";
	consoleServer->send(StringStreamFn::getCStr(stringStream));
}

void DataCompiler::addTree(const char* path)
{
	TempAllocator512 tempAllocator512;
	DynamicString sourceDirectory(tempAllocator512);
	sourceDirectory = MapFn::get(sourceDirectoryList, sourceDirectory, sourceDirectory);

	sourceFileSystem.setPrefix(sourceDirectory.getCStr());
	DataCompiler::scanSourceDirectory(sourceDirectory.getCStr(), path);

	StringStream stringStream(tempAllocator512);
	stringStream << "{\"type\":\"addTree\",\"path\":\"" << path << "\"}";
	consoleServer->send(StringStreamFn::getCStr(stringStream));
}

void DataCompiler::removeFile(const char* path)
{
	for (uint32_t i = 0; i < VectorFn::getCount(this->fileNameList); ++i)
	{
		if (this->fileNameList[i] == path)
		{
			this->fileNameList[i] = this->fileNameList[VectorFn::getCount(this->fileNameList) - 1];
			VectorFn::popBack(this->fileNameList);

			TempAllocator512 tempAllocator512;
			StringStream stringStream(tempAllocator512);
			stringStream << "{\"type\":\"removeFile\",\"path\":\"" << path << "\"}";
			consoleServer->send(StringStreamFn::getCStr(stringStream));
			return;
		}
	}
}

void DataCompiler::removeTree(const char* path)
{
	TempAllocator512 tempAllocator512;
	StringStream stringStream(tempAllocator512);
	stringStream << "{\"type\":\"removeTree\",\"path\":\"" << path << "\"}";
	consoleServer->send(StringStreamFn::getCStr(stringStream));

	for (uint32_t i = 0; i < VectorFn::getCount(this->fileNameList);)
	{
		if (this->fileNameList[i].startsWith(path))
		{
			TempAllocator512 tempAllocator512Local;
			StringStream stringStream(tempAllocator512Local);
			stringStream << "{\"type\":\"removeFile\",\"path\":\"" << this->fileNameList[i].getCStr() << "\"}";
			consoleServer->send(StringStreamFn::getCStr(stringStream));

			this->fileNameList[i] = this->fileNameList[VectorFn::getCount(this->fileNameList) - 1];
			VectorFn::popBack(this->fileNameList);
			continue;
		}

		++i;
	}
}

void DataCompiler::scanSourceDirectory(const char* prefix, const char* currentDirectory)
{
	Vector<DynamicString> fileNameListLocal(getDefaultAllocator());
	sourceFileSystem.getFileList(currentDirectory, fileNameListLocal);

	for (uint32_t i = 0; i < VectorFn::getCount(fileNameListLocal); ++i)
	{
		TempAllocator512 tempAllocator512;
		DynamicString fileNameCurrent(tempAllocator512);

		if (strcmp(currentDirectory, "") != 0)
		{
			fileNameCurrent += currentDirectory;
			fileNameCurrent += '/';
		}
		fileNameCurrent += fileNameListLocal[i];

		if (sourceFileSystem.getIsDirectory(fileNameCurrent.getCStr()))
		{
			DataCompiler::scanSourceDirectory(prefix, fileNameCurrent.getCStr());
		}
		else // Assume a regular file
		{
			DynamicString resourceName(tempAllocator512);
			if (strcmp(prefix, "") != 0)
			{
				resourceName += prefix;
				resourceName += '/';
			}
			resourceName += fileNameCurrent;
			addFile(resourceName.getCStr());
		}
	}
}

void DataCompiler::mapSourceDirectory(const char* name, const char* sourceDirectory)
{
	TempAllocator512 tempAllocator512;
	DynamicString sourceName(tempAllocator512);
	DynamicString sourceDirectoryString(tempAllocator512);
	sourceName.set(name, getStrLen32(name));
	sourceDirectoryString.set(sourceDirectory, getStrLen32(sourceDirectory));
	MapFn::set(sourceDirectoryList, sourceName, sourceDirectoryString);
}

void DataCompiler::getSourceDirectory(const char* resourceName, DynamicString& sourceDirectoryName)
{
	const char* slash = strchr(resourceName, '/');

	TempAllocator512 tempAllocator512;
	DynamicString sourceName(tempAllocator512);

	if (slash != nullptr)
	{
		sourceName.set(resourceName, uint32_t(slash - resourceName));
	}
	else
	{
		sourceName.set("", 0);
	}

	DynamicString deffault(tempAllocator512);
	DynamicString empty(tempAllocator512);
	empty = "";

	deffault = MapFn::get(sourceDirectoryList, empty, empty);
	sourceDirectoryName = MapFn::get(sourceDirectoryList, sourceName, deffault);
}

void DataCompiler::addIgnoreGlobPattern(const char* glob)
{
	TempAllocator256 tempAllocator256;
	DynamicString globString(tempAllocator256);
	globString.set(glob, getStrLen32(glob));
	VectorFn::pushBack(this->globList, globString);
}

void DataCompiler::scan()
{
	// Scan all source directories
	auto currentPair = MapFn::begin(sourceDirectoryList);
	auto endPair = MapFn::end(sourceDirectoryList);

	for (; currentPair != endPair; ++currentPair)
	{
		DynamicString prefix(getDefaultAllocator());
		PathFn::join(prefix, currentPair->pair.second.getCStr(), currentPair->pair.first.getCStr());
		sourceFileSystem.setPrefix(prefix.getCStr());

		if (sourceFileSystem.exists(AMSTEL_ENGINE_DATAIGNORE))
		{
			File& file = *sourceFileSystem.open(AMSTEL_ENGINE_DATAIGNORE, FileOpenMode::READ);
			const uint32_t size = file.getFileSize();
			char* data = (char*)getDefaultAllocator().allocate(size + 1);
			file.read(data, size);
			data[size] = '\0';
			sourceFileSystem.close(file);

			LineReader lineReader(data);

			while (!lineReader.getIsEndOfFile())
			{
				TempAllocator512 tempAllocator512;
				DynamicString line(tempAllocator512);
				lineReader.readLine(line);

				line.trim();

				if (line.getIsEmpty() || line.startsWith("#"))
				{
					continue;
				}

				addIgnoreGlobPattern(line.getCStr());
			}

			getDefaultAllocator().deallocate(data);
		}

		scanSourceDirectory(currentPair->pair.first.getCStr(), "");
	}

#if AMSTEL_ENGINE_FILE_MONITOR_IMPLEMENTED
	fileMonitor.start(MapFn::begin(sourceDirectoryList)->pair.second.getCStr(), true, fileMonitorCallback, this);
#endif // AMSTEL_ENGINE_FILE_MONITOR_IMPLEMENTED
}

bool DataCompiler::compile(const char* dataDirectory, const char* platform)
{
	FileSystemDisk dataFileSystem(getDefaultAllocator());
	dataFileSystem.setPrefix(dataDirectory);
	dataFileSystem.createDirectory("");

	if (!dataFileSystem.exists(RIO_DATA_DIRECTORY))
	{
		dataFileSystem.createDirectory(RIO_DATA_DIRECTORY);
	}

	if (!dataFileSystem.exists(RIO_TEMP_DIRECTORY))
	{
		dataFileSystem.createDirectory(RIO_TEMP_DIRECTORY);
	}

	std::sort(VectorFn::begin(this->fileNameList), VectorFn::end(this->fileNameList));

	bool success = false;

	// Compile all changed resources
	for (uint32_t i = 0; i < VectorFn::getCount(this->fileNameList); ++i)
	{
		const char* filename = this->fileNameList[i].getCStr();
		const char* type = PathFn::getFileExtension(filename);

		if (type == nullptr)
		{
			continue;
		}

		char name[256];
		const uint32_t size = uint32_t(type - filename - 1);
		strncpy(name, filename, size);
		name[size] = '\0';

		TempAllocator1024 tempAllocator1024;
		DynamicString path(tempAllocator1024);
		DynamicString sourcePath(tempAllocator1024);
		DynamicString destinationPath(tempAllocator1024);

		StringId64 resourceType(type);
		StringId64 resourceName(name);

		// Build source file path
		sourcePath += name;
		sourcePath += '.';
		sourcePath += type;

		// Build destination file path
		StringId64 mix;
		mix.id = resourceType.id ^ resourceName.id;
		mix.toString(destinationPath);

		PathFn::join(path, RIO_DATA_DIRECTORY, destinationPath.getCStr());

		logInfo(COMPILER, "%s", sourcePath.getCStr());

		if (!canCompileResource(resourceType))
		{
			logError(COMPILER, "Unknown resource type: '%s'", type);
			logError(COMPILER, "Append extension to " AMSTEL_ENGINE_DATAIGNORE " to ignore the type");
			success = false;
			break;
		}

		Buffer outputBuffer(getDefaultAllocator());
		ArrayFn::reserve(outputBuffer, 4 * 1024 * 1024);

		if (!setjmp(this->jumpBuffer))
		{
			CompileOptions compileOptions(*this, dataFileSystem, sourcePath, outputBuffer, platform);

			HashMapFn::get(this->resourceTypeDataMap, resourceType, ResourceTypeData()).compileFunction(compileOptions);

			File* outputFile = dataFileSystem.open(path.getCStr(), FileOpenMode::WRITE);
			uint32_t size = ArrayFn::getCount(outputBuffer);
			uint32_t written = outputFile->write(ArrayFn::begin(outputBuffer), size);
			dataFileSystem.close(*outputFile);

			success = size == written;
		}
		else
		{
			success = false;
		}

		if (!success)
		{
			logError(COMPILER, "Error");
			break;
		}
		else
		{
			if (!MapFn::has(dataIndexMap, destinationPath))
			{
				MapFn::set(dataIndexMap, destinationPath, sourcePath);
			}
		}
	}

	// Write index
	{
		File* file = dataFileSystem.open("dataIndex.RJson", FileOpenMode::WRITE);
		if (!file)
		{
			return false;
		}

		StringStream stringStream(getDefaultAllocator());

		auto currentData = MapFn::begin(dataIndexMap);
		auto endData = MapFn::end(dataIndexMap);
		for (; currentData != endData; ++currentData)
		{
			stringStream << "\"" << currentData->pair.first.getCStr() << "\" = \"" << currentData->pair.second.getCStr() << "\"\n";
		}

		file->write(StringStreamFn::getCStr(stringStream), getStrLen32(StringStreamFn::getCStr(stringStream)));
		dataFileSystem.close(*file);
	}

	return success;
}

void DataCompiler::registerResourceCompiler(StringId64 type, uint32_t version, CompileFunction compileFunction)
{
	RIO_ASSERT(!HashMapFn::has(this->resourceTypeDataMap, type), "Type already registered");
	RIO_ENSURE(nullptr != compileFunction);

	ResourceTypeData resourceTypeData;
	resourceTypeData.version = version;
	resourceTypeData.compileFunction = compileFunction;

	HashMapFn::set(this->resourceTypeDataMap, type, resourceTypeData);
}

uint32_t DataCompiler::getDataCompilerVersion(StringId64 type)
{
	ResourceTypeData resourceTypeData;
	resourceTypeData.version = COMPILER_NOT_FOUND;
	resourceTypeData.compileFunction = nullptr;

	return HashMapFn::get(this->resourceTypeDataMap, type, resourceTypeData).version;
}

bool DataCompiler::canCompileResource(StringId64 type)
{
	return HashMapFn::has(this->resourceTypeDataMap, type);
}

void DataCompiler::error(const char* message, va_list argumentList)
{
	logErrorVariadic(COMPILER, message, argumentList);
	longjmp(this->jumpBuffer, 1);
}

#if AMSTEL_ENGINE_FILE_MONITOR_IMPLEMENTED
void DataCompiler::fileMonitorCallback(FileMonitorEvent::Enum fileMonitorEvent, bool isDirectory, const char* path, const char* pathRenamed)
{
	TempAllocator512 tempAllocator512;
	DynamicString resourceName(tempAllocator512);
	DynamicString resourceNameRenamed(tempAllocator512);
	DynamicString sourceDirectoryName(tempAllocator512);

	sourceDirectoryName = MapFn::get(sourceDirectoryList, sourceDirectoryName, sourceDirectoryName);
	resourceName = &path[sourceDirectoryName.getLength()+1]; // TODO add PathFn::relative()
	resourceNameRenamed = pathRenamed ? &pathRenamed[sourceDirectoryName.getLength() + 1] : "";

	switch (fileMonitorEvent)
	{
	case FileMonitorEvent::CREATED:
		if (isDirectory == false)
		{
			addFile(resourceName.getCStr());
		}
		else
		{
			addTree(resourceName.getCStr());
		}
		break;

	case FileMonitorEvent::DELETED:
		if (isDirectory == false)
		{
			removeFile(resourceName.getCStr());
		}
		else
		{
			removeTree(resourceName.getCStr());
		}
		break;

	case FileMonitorEvent::RENAMED:
		if (isDirectory == false)
		{
			removeFile(resourceName.getCStr());
			addFile(resourceNameRenamed.getCStr());
		}
		else
		{
			removeTree(resourceName.getCStr());
			addTree(resourceNameRenamed.getCStr());
		}
		break;

	case FileMonitorEvent::CHANGED:
		break;

	default:
		RIO_ASSERT(false, "Unknown FileMonitorEvent: %d", fileMonitorEvent);
		break;
	}
}

void DataCompiler::fileMonitorCallback(void* thiz, FileMonitorEvent::Enum fileMonitorEvent, bool isDirectory, const char* pathOriginal, const char* pathModified)
{
	((DataCompiler*)thiz)->fileMonitorCallback(fileMonitorEvent, isDirectory, pathOriginal, pathModified);
}
#endif // AMSTEL_ENGINE_FILE_MONITOR_IMPLEMENTED

struct InitMemoryGlobals
{
	InitMemoryGlobals()
	{
		Rio::MemoryGlobalFn::init();
	}

	~InitMemoryGlobals()
	{
		Rio::MemoryGlobalFn::shutdown();
	}
};

int dataCompilerMain(int argumentCount, char** argumentList)
{
	InitMemoryGlobals initMemoryGlobals;
	RIO_UNUSED(initMemoryGlobals);

	DeviceOptions deviceOptions(getDefaultAllocator(), argumentCount, (const char**)argumentList);
	if (deviceOptions.parse() == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

	ConsoleServerGlobalFn::init();
	getConsoleServerGlobal()->listen(AMSTEL_ENGINE_DEFAULT_COMPILER_PORT, deviceOptions.waitForConsole);

	DataCompiler* dataCompiler = RIO_NEW(getDefaultAllocator(), DataCompiler)(*getConsoleServerGlobal());

	dataCompiler->registerResourceCompiler(RESOURCE_TYPE_CONFIG, RESOURCE_VERSION_CONFIG, ConfigResourceInternalFn::compile);
	dataCompiler->registerResourceCompiler(RESOURCE_TYPE_FONT, RESOURCE_VERSION_FONT, FontResourceInternalFn::compile);
	dataCompiler->registerResourceCompiler(RESOURCE_TYPE_LEVEL, RESOURCE_VERSION_LEVEL, LevelResourceInternalFn::compile);
	dataCompiler->registerResourceCompiler(RESOURCE_TYPE_MATERIAL, RESOURCE_VERSION_MATERIAL, MaterialResourceInternalFn::compile);
	dataCompiler->registerResourceCompiler(RESOURCE_TYPE_MESH, RESOURCE_VERSION_MESH, MeshResourceInternalFn::compile);
	dataCompiler->registerResourceCompiler(RESOURCE_TYPE_PACKAGE, RESOURCE_VERSION_PACKAGE, PackageResourceInternalFn::compile);
	dataCompiler->registerResourceCompiler(RESOURCE_TYPE_SHADER, RESOURCE_VERSION_SHADER, ShaderResourceInternalFn::compile);
	dataCompiler->registerResourceCompiler(RESOURCE_TYPE_SPRITE, RESOURCE_VERSION_SPRITE, SpriteResourceInternalFn::compile);
	dataCompiler->registerResourceCompiler(RESOURCE_TYPE_SPRITE_ANIMATION, RESOURCE_VERSION_SPRITE_ANIMATION, SpriteAnimationResourceInternalFn::compile);
	dataCompiler->registerResourceCompiler(RESOURCE_TYPE_STATE_MACHINE, RESOURCE_VERSION_STATE_MACHINE, StateMachineInternalFn::compile);
	dataCompiler->registerResourceCompiler(RESOURCE_TYPE_TEXTURE, RESOURCE_VERSION_TEXTURE, TextureResourceInternalFn::compile);
	dataCompiler->registerResourceCompiler(RESOURCE_TYPE_UNIT, RESOURCE_VERSION_UNIT, UnitResourceInternalFn::compile);

#if AMSTEL_ENGINE_SOUND
	dataCompiler->registerResourceCompiler(RESOURCE_TYPE_SOUND, RESOURCE_VERSION_SOUND, SoundResourceInternalFn::compile);
#endif // AMSTEL_ENGINE_SOUND

#if AMSTEL_ENGINE_PHYSICS
	dataCompiler->registerResourceCompiler(RESOURCE_TYPE_PHYSICS, RESOURCE_VERSION_PHYSICS, PhysicsResourceInternalFn::compile);
	dataCompiler->registerResourceCompiler(RESOURCE_TYPE_PHYSICS_CONFIG, RESOURCE_VERSION_PHYSICS_CONFIG, PhysicsConfigResourceInternalFn::compile);
#endif // AMSTEL_ENGINE_PHYSICS

#if AMSTEL_ENGINE_SCRIPT_LUA
	dataCompiler->registerResourceCompiler(RESOURCE_TYPE_SCRIPT, RESOURCE_VERSION_SCRIPT, LuaResourceInternalFn::compile);
#endif // AMSTEL_ENGINE_SCRIPT_LUA

	// Add ignore globs
	dataCompiler->addIgnoreGlobPattern("*.bak");
	dataCompiler->addIgnoreGlobPattern("*.dds");
	dataCompiler->addIgnoreGlobPattern("*.ktx");
	dataCompiler->addIgnoreGlobPattern("*.ogg");
	dataCompiler->addIgnoreGlobPattern("*.png");
	dataCompiler->addIgnoreGlobPattern("*.pvr");
	dataCompiler->addIgnoreGlobPattern("*.tga");
	dataCompiler->addIgnoreGlobPattern("*.tmp");
	dataCompiler->addIgnoreGlobPattern("*.wav");
	dataCompiler->addIgnoreGlobPattern("*~");
	dataCompiler->addIgnoreGlobPattern(".*");

	dataCompiler->mapSourceDirectory("", deviceOptions.sourceDirectory.getCStr());

	if (deviceOptions.sourceDirectoryToMapName)
	{
		dataCompiler->mapSourceDirectory(deviceOptions.sourceDirectoryToMapName, deviceOptions.sourceDirectoryToMapPrefix.getCStr());
	}

	dataCompiler->scan();

	bool compilationSuccess = true;

	if (deviceOptions.runEngineAsServer)
	{
		while (true)
		{
			getConsoleServerGlobal()->update();
			OsFn::sleep(60);
		}
	}
	else
	{
		compilationSuccess = dataCompiler->compile(deviceOptions.destinationDirectory.getCStr(), deviceOptions.platformName);
	}

	RIO_DELETE(getDefaultAllocator(), dataCompiler);

	ConsoleServerGlobalFn::shutdown();

	return compilationSuccess ? EXIT_SUCCESS : EXIT_FAILURE;
}

} // namespace Rio

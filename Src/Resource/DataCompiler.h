#pragma once

#include "Config.h"

#include "Core/Containers/Types.h"
#include "Core/FileSystem/FileSystemDisk.h"
#include "Core/ConsoleServer.h"

#if AMSTEL_ENGINE_FILE_MONITOR_IMPLEMENTED
#include "Core/FileSystem/FileMonitor.h"
#endif // AMSTEL_ENGINE_FILE_MONITOR_IMPLEMENTED

#include "Resource/Types.h"

#include <setjmp.h>

namespace Rio
{

// Compiles source data into binary
struct DataCompiler
{
	using CompileFunction = void (*)(CompileOptions& compileOptions);

	struct ResourceTypeData
	{
		uint32_t version = UINT32_MAX;
		CompileFunction compileFunction = nullptr;
	};

	ConsoleServer* consoleServer = nullptr;
	FileSystemDisk sourceFileSystem;
	Map<DynamicString, DynamicString> sourceDirectoryList;
	HashMap<StringId64, ResourceTypeData> resourceTypeDataMap;
	Vector<DynamicString> fileNameList;
	Vector<DynamicString> globList;
	Map<DynamicString, DynamicString> dataIndexMap;
	jmp_buf jumpBuffer;

#if AMSTEL_ENGINE_FILE_MONITOR_IMPLEMENTED
	FileMonitor fileMonitor;
#endif // AMSTEL_ENGINE_FILE_MONITOR_IMPLEMENTED
	
	void addFile(const char* path);
	void addTree(const char* path);
	void removeFile(const char* path);
	void removeTree(const char* path);
	void scanSourceDirectory(const char* prefix, const char* path);

#if AMSTEL_ENGINE_FILE_MONITOR_IMPLEMENTED
	void fileMonitorCallback(FileMonitorEvent::Enum fileMonitorEvent, bool isDirectory, const char* path, const char* pathRenamed);
	static void fileMonitorCallback(void* thiz, FileMonitorEvent::Enum fileMonitorEvent, bool isDirectory, const char* pathOriginal, const char* pathModified);
#endif // AMSTEL_ENGINE_FILE_MONITOR_IMPLEMENTED

	explicit DataCompiler(ConsoleServer& consoleServer);
	~DataCompiler();

	void mapSourceDirectory(const char* name, const char* sourceDirectoryName);
	void getSourceDirectory(const char* resourceName, DynamicString& sourceDirectoryName);

	// Adds a <glob> pattern to ignore when scanning the source directory
	void addIgnoreGlobPattern(const char* glob);

	// Scans source directory for resources
	void scan();

	// Compiles all the resources found in the source directory and puts them in <dataDirectory>
	// Returns true on success, false otherwise
	bool compile(const char* dataDirectory, const char* platformName);

	// Registers the resource <compileFunction> for the given resource <type> and <version>
	void registerResourceCompiler(StringId64 type, uint32_t version, CompileFunction compileFunction);

	// Returns whether there is a compileFunction for the resource <type>
	bool canCompileResource(StringId64 type);

	// Returns the version of the compileFunction for <type> or COMPILER_NOT_FOUND if no compileFunction is found
	uint32_t getDataCompilerVersion(StringId64 type);

	void error(const char* message, va_list argumentList);

	static const uint32_t COMPILER_NOT_FOUND = UINT32_MAX;
};

int dataCompilerMain(int argumentCount, char** argumentList);

} // namespace Rio

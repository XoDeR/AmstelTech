#include "Resource/ConfigResource.h"

#include "Core/FileSystem/File.h"
#include "Core/Json/JsonObject.h"
#include "Core/Json/RJson.h"
#include "Core/Memory/Allocator.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Strings/DynamicString.h"

#include "Resource/CompileOptions.h"
#include "Resource/Types.h"

#include "Config.h"

namespace Rio
{

namespace ConfigResourceInternalFn
{
	void compile(CompileOptions& compileOptions)
	{
		Buffer buffer = compileOptions.read();

		TempAllocator1024 tempAllocator1024;
		JsonObject bootJsonObject(tempAllocator1024);
		RJsonFn::parse(buffer, bootJsonObject);

#if AMSTEL_ENGINE_SCRIPT_LUA
		const char* bootScriptJson  = bootJsonObject["bootScript"];
		DATA_COMPILER_ASSERT(bootScriptJson != nullptr, compileOptions, "'bootScript' must be specified.");
		DynamicString bootScript(tempAllocator1024);
		RJsonFn::parseString(bootScriptJson, bootScript);
		DATA_COMPILER_ASSERT_RESOURCE_EXISTS("lua", bootScript.getCStr(), compileOptions);
#endif // AMSTEL_ENGINE_SCRIPT_LUA
		
		const char* bootPackageJson = bootJsonObject["bootPackage"];
		DATA_COMPILER_ASSERT(bootPackageJson != nullptr, compileOptions, "'bootPackage' must be specified.");
		DynamicString bootPackage(tempAllocator1024);
		RJsonFn::parseString(bootPackageJson, bootPackage);
		DATA_COMPILER_ASSERT_RESOURCE_EXISTS("package", bootPackage.getCStr(), compileOptions);

		compileOptions.write(buffer);
	}

	void* load(File& file, Allocator& a)
	{
		const uint32_t size = file.getFileSize();
		char* resourceData = (char*)a.allocate(size + 1);
		file.read(resourceData, size);
		resourceData[size] = '\0';
		return resourceData;
	}

	void unload(Allocator& a, void* resourceData)
	{
		a.deallocate(resourceData);
	}

} // namespace ConfigResourceInternalFn

} // namespace Rio

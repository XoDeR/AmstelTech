#include "Resource/ShaderResource.h"

#include "Config.h"
#include "Core/Containers/Map.h"
#include "Core/FileSystem/FileSystem.h"
#include "Core/Json/JsonObject.h"
#include "Core/Json/RJson.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Strings/StringStream.h"

#include "Device/Device.h"

#include "Resource/CompileOptions.h"
#include "Resource/ResourceManager.h"
#include "Resource/ShaderCompiler.h"

#include "World/ShaderManager.h"

namespace Rio
{

namespace ShaderResourceInternalFn
{
	void compile(CompileOptions& compileOptions)
	{
		ShaderCompiler shaderCompiler(compileOptions);
		shaderCompiler.parse(compileOptions.getSourcePath());
		shaderCompiler.compile();
	}

	void* load(File& file, Allocator& a)
	{
		return getDevice()->shaderManager->load(file, a);
	}

	void online(StringId64 id, ResourceManager& resourceManager)
	{
		getDevice()->shaderManager->online(id, resourceManager);
	}

	void offline(StringId64 id, ResourceManager& resourceManager)
	{
		getDevice()->shaderManager->offline(id, resourceManager);
	}

	void unload(Allocator& a, void* resourceData)
	{
		getDevice()->shaderManager->unload(a, resourceData);
	}

} // namespace ShaderResourceInternalFn

} // namespace Rio

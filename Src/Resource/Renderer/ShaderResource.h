#pragma once

#include "Core/Containers/Types.h"
#include "Core/FileSystem/Types.h"
#include "Core/Memory/Types.h"
#include "Core/Strings/StringId.h"
#include "Core/Types.h"

#include "Resource/Types.h"

#include "RioRenderer/RioRenderer.h"

namespace Rio
{

struct ShaderResource
{
	ShaderResource(Allocator& a)
		: shaderResourceData(a)
	{
	}

	struct Sampler
	{
		uint32_t name =	UINT32_MAX;
		uint32_t state = UINT32_MAX;
	};

	struct Data
	{
		StringId32 name;
		uint64_t state = UINT64_MAX;
		Sampler samplerList[4];
		const RioRenderer::Memory* vertexShaderMemory = nullptr;
		const RioRenderer::Memory* fragmentShaderMemory = nullptr;
	};

	Array<Data> shaderResourceData;
};

namespace ShaderResourceInternalFn
{
	void compile(CompileOptions& compileOptions);
	void* load(File& file, Allocator& a);
	void online(StringId64 id, ResourceManager& resourceManager);
	void offline(StringId64 id, ResourceManager& resourceManager);
	void unload(Allocator& a, void* resourceData);

} // namespace ShaderResourceInternalFn

} // namespace Rio

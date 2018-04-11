#pragma once

#include "Core/Containers/Types.h"
#include "Core/FileSystem/Types.h"
#include "Core/Memory/Types.h"
#include "Core/Strings/StringId.h"

#include "Resource/ShaderResource.h"
#include "Resource/Types.h"

#include "RioRenderer/RioRenderer.h"

namespace Rio
{

struct ShaderManager
{
	struct ShaderData
	{
		uint64_t state = UINT64_MAX;
		ShaderResource::Sampler samplerList[4];
		RioRenderer::ProgramHandle program;
	};

	HashMap<StringId32, ShaderData> shaderMap;

	ShaderManager(Allocator& a);

	void* load(File& file, Allocator& a);
	void online(StringId64 id, ResourceManager& resourceManager);
	void offline(StringId64 id, ResourceManager& resourceManager);
	void unload(Allocator& a, void* resourceData);

	void addShader(StringId32 name, uint64_t state, const ShaderResource::Sampler samplerList[4], RioRenderer::ProgramHandle program);
	uint32_t getSamplerState(StringId32 shaderId, StringId32 samplerName);
	void submit(StringId32 shaderId, uint8_t viewId, int32_t depth = 0, uint64_t state = UINT64_MAX);
};

} // namespace Rio

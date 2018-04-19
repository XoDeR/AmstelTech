#include "World/ShaderManager.h"

#include "Core/Containers/Array.h"
#include "Core/Containers/HashMap.h"
#include "Core/FileSystem/ReaderWriter.h"
#include "Core/Strings/String.h"

#include "Resource/ResourceManager.h"
#include "Resource/ShaderResource.h"

namespace Rio
{

ShaderManager::ShaderManager(Allocator& a)
	: shaderMap(a)
{
}

void* ShaderManager::load(File& file, Allocator& a)
{
	BinaryReader binaryReader(file);
	uint32_t version;
	binaryReader.read(version);
	RIO_ASSERT(version == RESOURCE_VERSION_SHADER, "Wrong version");

	uint32_t itemsCount = 0;
	binaryReader.read(itemsCount);

	ShaderResource* shaderResource = RIO_NEW(a, ShaderResource)(a);
	ArrayFn::resize(shaderResource->shaderResourceData, itemsCount);

	for (uint32_t i = 0; i < itemsCount; ++i)
	{
		uint32_t shaderNameIndex = UINT32_MAX;
		binaryReader.read(shaderNameIndex);

		uint64_t renderStateIndex;
		binaryReader.read(renderStateIndex);

		uint32_t samplerListCount = 0;
		binaryReader.read(samplerListCount);
		RIO_ENSURE(samplerListCount < countof(shaderResource->shaderResourceData[i].samplerList));

		for (uint32_t samplerIndex = 0; samplerIndex < samplerListCount; ++samplerIndex)
		{
			uint32_t samplerNameIndex = UINT32_MAX;
			binaryReader.read(samplerNameIndex);

			uint32_t samplerStateIndex = UINT32_MAX;
			binaryReader.read(samplerStateIndex);

			shaderResource->shaderResourceData[i].samplerList[samplerIndex].name = samplerNameIndex;
			shaderResource->shaderResourceData[i].samplerList[samplerIndex].state = samplerStateIndex;
		}

		uint32_t vertexShaderCodeSize = 0;
		binaryReader.read(vertexShaderCodeSize);
		const RioRenderer::Memory* vertexShaderMemory = RioRenderer::alloc(vertexShaderCodeSize);
		binaryReader.read(vertexShaderMemory->data, vertexShaderCodeSize);

		uint32_t fragmentShaderCodeSize = 0;
		binaryReader.read(fragmentShaderCodeSize);
		const RioRenderer::Memory* fragmentShaderMemory = RioRenderer::alloc(fragmentShaderCodeSize);
		binaryReader.read(fragmentShaderMemory->data, fragmentShaderCodeSize);

		shaderResource->shaderResourceData[i].name.id = shaderNameIndex;
		shaderResource->shaderResourceData[i].state = renderStateIndex;
		shaderResource->shaderResourceData[i].vertexShaderMemory = vertexShaderMemory;
		shaderResource->shaderResourceData[i].fragmentShaderMemory = fragmentShaderMemory;
	}

	return shaderResource;
}

void ShaderManager::online(StringId64 id, ResourceManager& resourceManager)
{
	const ShaderResource* shader = (ShaderResource*)resourceManager.getResourceData(RESOURCE_TYPE_SHADER, id);

	for (uint32_t i = 0; i < ArrayFn::getCount(shader->shaderResourceData); ++i)
	{
		const ShaderResource::Data& data = shader->shaderResourceData[i];

		RioRenderer::ShaderHandle vertexShaderHandle = RioRenderer::createShader(data.vertexShaderMemory);
		RIO_ASSERT(RioRenderer::isValid(vertexShaderHandle), "Failed to create vertex shader");
		RioRenderer::ShaderHandle fragmentShaderHandle = RioRenderer::createShader(data.fragmentShaderMemory);
		RIO_ASSERT(RioRenderer::isValid(fragmentShaderHandle), "Failed to create fragment shader");
		RioRenderer::ProgramHandle programHandle = RioRenderer::createProgram(vertexShaderHandle, fragmentShaderHandle, true);
		RIO_ASSERT(RioRenderer::isValid(programHandle), "Failed to create GPU program");

		addShader(data.name, data.state, data.samplerList, programHandle);
	}
}

void ShaderManager::offline(StringId64 id, ResourceManager& resourceManager)
{
	const ShaderResource* shaderResource = (ShaderResource*)resourceManager.getResourceData(RESOURCE_TYPE_SHADER, id);

	for (uint32_t i = 0; i < ArrayFn::getCount(shaderResource->shaderResourceData); ++i)
	{
		const ShaderResource::Data& data = shaderResource->shaderResourceData[i];

		ShaderData shaderData;
		shaderData.state = RIO_RENDERER_STATE_DEFAULT;
		shaderData.program = RIO_RENDERER_INVALID_HANDLE_UINT16_T;
		shaderData = HashMapFn::get(shaderMap, data.name, shaderData);

		RioRenderer::destroy(shaderData.program);

		HashMapFn::remove(shaderMap, data.name);
	}
}

void ShaderManager::unload(Allocator& a, void* resourceData)
{
	RIO_DELETE(a, (ShaderResource*)resourceData);
}

void ShaderManager::addShader(StringId32 name, uint64_t state, const ShaderResource::Sampler samplerList[4], RioRenderer::ProgramHandle program)
{
	ShaderData shaderData;
	shaderData.state = state;
	memcpy(shaderData.samplerList, samplerList, sizeof(shaderData.samplerList));
	shaderData.program = program;
	HashMapFn::set(shaderMap, name, shaderData);
}

uint32_t ShaderManager::getSamplerState(StringId32 shaderId, StringId32 samplerName)
{
	RIO_ASSERT(HashMapFn::has(shaderMap, shaderId), "Shader not found");
	ShaderData shaderData;
	shaderData.state = RIO_RENDERER_STATE_DEFAULT;
	shaderData.program = RIO_RENDERER_INVALID_HANDLE_UINT16_T;
	shaderData = HashMapFn::get(shaderMap, shaderId, shaderData);

	for (uint32_t i = 0; i < countof(shaderData.samplerList); ++i)
	{
		if (shaderData.samplerList[i].name == samplerName.id)
		{
			return shaderData.samplerList[i].state;
		}
	}

	RIO_FATAL("Sampler not found");
	return UINT32_MAX;
}

void ShaderManager::submit(StringId32 shaderId, uint8_t viewId, int32_t depth, uint64_t state)
{
	RIO_ASSERT(HashMapFn::has(shaderMap, shaderId), "Shader not found");
	ShaderData shaderData;
	shaderData.state = RIO_RENDERER_STATE_DEFAULT;
	shaderData.program = RIO_RENDERER_INVALID_HANDLE_UINT16_T;
	shaderData = HashMapFn::get(shaderMap, shaderId, shaderData);

	RioRenderer::setState(state != UINT64_MAX ? state : shaderData.state);
	RioRenderer::submit(viewId, shaderData.program, depth);
}

} // namespace Rio

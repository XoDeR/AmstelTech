#pragma once

#include "Core/FileSystem/Types.h"
#include "Core/Math/Types.h"
#include "Core/Memory/Types.h"
#include "Core/Strings/StringId.h"

#include "Resource/Types.h"

namespace Rio
{

struct MaterialResource
{
	uint32_t version = UINT32_MAX;
	StringId32 shader;
	uint32_t textureListCount = 0;
	uint32_t textureDataOffset = 0;
	uint32_t uniformListCount = 0;
	uint32_t uniformDataOffset = 0;
	uint32_t dynamicDataSize = 0;
	uint32_t dynamicDataOffset = 0;
};

struct TextureData
{
	// Sampler uniform name
	uint32_t samplerNameOffset = 0;
	// Sampler name
	StringId32 name;
	// Resource name
	StringId64 id;
	// Offset into dynamic blob
	uint32_t dataOffset = 0; 
	uint32_t padding01 = 0;
};

struct TextureHandle
{
	uint32_t samplerHandleIndex = 0;
	uint32_t textureHandleIndex = 0;
};

struct UniformType
{
	enum Enum
	{
		FLOAT,
		VECTOR2,
		VECTOR3,
		VECTOR4,

		COUNT
	};
};

struct UniformData
{
	// UniformType::Enum
	uint32_t type = UINT32_MAX;
	// Uniform name
	StringId32 name;
	// Uniform name (string)
	uint32_t nameOffset = 0;
	// Offset into dynamic blob
	uint32_t dataOffset = 0;
};

struct UniformHandle
{
	uint32_t uniformHandleIndex = 0;
};

namespace MaterialResourceInternalFn
{
	void compile(CompileOptions& compileOptions);
	void* load(File& file, Allocator& a);
	void online(StringId64 id, ResourceManager& resourceManager);
	void offline(StringId64 id, ResourceManager& resourceManager);
	void unload(Allocator& a, void* resourceData);

} // namespace MaterialResourceInternalFn

namespace MaterialResourceFn
{
	UniformData* getUniformDataByIndex(const MaterialResource* materialResource, uint32_t i);
	UniformData* getUniformDataByName(const MaterialResource* materialResource, StringId32 name);
	const char* getUniformName(const MaterialResource* materialResource, const UniformData* uniformData);
	TextureData* getTextureDataByIndex(const MaterialResource* materialResource, uint32_t i);
	const char* getTextureName(const MaterialResource* materialResource, const TextureData* textureData);
	UniformHandle* getUniformHandleByIndex(const MaterialResource* materialResource, uint32_t i, char* dynamic);
	UniformHandle* getUniformHandleByName(const MaterialResource* materialResource, StringId32 name, char* dynamic);
	TextureHandle* getTextureHandle(const MaterialResource* materialResource, uint32_t i, char* dynamic);

} // namespace MaterialResourceFn

} // namespace Rio

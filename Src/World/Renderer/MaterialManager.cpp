#include "World/MaterialManager.h"

#include "Core/Containers/SortMap.h"
#include "Core/FileSystem/File.h"
#include "Resource/MaterialResource.h"
#include "Resource/ResourceManager.h"

#include "RioRenderer/RioRenderer.h"

#include <cstring> // memcpy

namespace Rio
{

MaterialManager::MaterialManager(Allocator& a, ResourceManager& resourceManager)
	: allocator(&a)
	, resourceManager(&resourceManager)
	, materialNameMap(a)
{
}

MaterialManager::~MaterialManager()
{
	auto currentMaterialPair = SortMapFn::begin(materialNameMap);
	auto endMaterialPair = SortMapFn::end(materialNameMap);
	for (; currentMaterialPair != endMaterialPair; ++currentMaterialPair)
	{
		allocator->deallocate(currentMaterialPair->second);
	}
}

void* MaterialManager::load(File& file, Allocator& a)
{
	const uint32_t fileSize = file.getFileSize();
	void* resourceData = a.allocate(fileSize);
	file.read(resourceData, fileSize);
	RIO_ASSERT(*(uint32_t*)resourceData == RESOURCE_VERSION_MATERIAL, "Wrong material version");
	return resourceData;
}

void MaterialManager::online(StringId64 id, ResourceManager& resourceManager)
{
	using namespace MaterialResourceFn;

	MaterialResource* materialResource = (MaterialResource*)resourceManager.getResourceData(RESOURCE_TYPE_MATERIAL, id);

	char* base = (char*)materialResource + materialResource->dynamicDataOffset;

	for (uint32_t i = 0; i < materialResource->textureListCount; ++i)
	{
		TextureData* textureData = getTextureDataByIndex(materialResource, i);
		TextureHandle* textureHandle = getTextureHandle(materialResource, i, base);
		textureHandle->samplerHandleIndex = RioRenderer::createUniform(getTextureName(materialResource, textureData), RioRenderer::UniformType::Int1).idx;
	}

	for (uint32_t i = 0; i < materialResource->uniformListCount; ++i)
	{
		UniformData* uniformData = getUniformDataByIndex(materialResource, i);
		UniformHandle* uniformHandle = getUniformHandleByIndex(materialResource, i, base);
		uniformHandle->uniformHandleIndex = RioRenderer::createUniform(getUniformName(materialResource, uniformData), RioRenderer::UniformType::Vec4).idx;
	}
}

void MaterialManager::offline(StringId64 id, ResourceManager& resourceManager)
{
	using namespace MaterialResourceFn;

	MaterialResource* materialResource = (MaterialResource*)resourceManager.getResourceData(RESOURCE_TYPE_MATERIAL, id);

	char* base = (char*)materialResource + materialResource->dynamicDataOffset;

	for (uint32_t i = 0; i < materialResource->textureListCount; ++i)
	{
		TextureHandle* textureHandle = getTextureHandle(materialResource, i, base);
		RioRenderer::UniformHandle rioRendererSamplerHandle;
		rioRendererSamplerHandle.idx = textureHandle->samplerHandleIndex;
		RioRenderer::destroy(rioRendererSamplerHandle);
	}

	for (uint32_t i = 0; i < materialResource->uniformListCount; ++i)
	{
		UniformHandle* uniformHandle = getUniformHandleByIndex(materialResource, i, base);
		RioRenderer::UniformHandle rioRendererUniformHandle;
		rioRendererUniformHandle.idx = uniformHandle->uniformHandleIndex;
		RioRenderer::destroy(rioRendererUniformHandle);
	}
}

void MaterialManager::unload(Allocator& a, void* resourceData)
{
	a.deallocate(resourceData);
}

void MaterialManager::createMaterial(StringId64 id)
{
	if (SortMapFn::has(materialNameMap, id))
	{
		return;
	}

	const MaterialResource* materialResource = (MaterialResource*)resourceManager->getResourceData(RESOURCE_TYPE_MATERIAL, id);

	const uint32_t size = sizeof(Material) + materialResource->dynamicDataSize;
	Material* material = (Material*)allocator->allocate(size);
	material->materialResource = materialResource;
	material->materialData = (char*)&material[1];

	const char* data = (char*)materialResource + materialResource->dynamicDataOffset;
	memcpy(material->materialData, data, materialResource->dynamicDataSize);

	SortMapFn::set(materialNameMap, id, material);
	SortMapFn::sort(materialNameMap);
}

void MaterialManager::destroyMaterial(StringId64 id)
{
	Material* material = SortMapFn::get(materialNameMap, id, (Material*)nullptr);
	allocator->deallocate(material);

	SortMapFn::remove(materialNameMap, id);
	SortMapFn::sort(materialNameMap);
}

Material* MaterialManager::getMaterialById(StringId64 id)
{
	RIO_ASSERT(SortMapFn::has(materialNameMap, id), "Material not found");
	return SortMapFn::get(materialNameMap, id, (Material*)nullptr);
}

} // namespace Rio

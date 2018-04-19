#include "World/Material.h"

#include "Resource/MaterialResource.h"
#include "Resource/ResourceManager.h"
#include "Resource/TextureResource.h"

#include "World/ShaderManager.h"

#include "RioRenderer/RioRenderer.h"

namespace Rio
{

void Material::bind(ResourceManager& resourceManager, ShaderManager& shaderManager, uint8_t view, int32_t depth) const
{
	using namespace MaterialResourceFn;

	// Set samplers
	for (uint32_t i = 0; i < this->materialResource->textureListCount; ++i)
	{
		const TextureData* textureData = getTextureDataByIndex(this->materialResource, i);
		const TextureHandle* textureHandle = getTextureHandle(this->materialResource, i, this->materialData);

		const TextureResource* textureResource = (TextureResource*)resourceManager.getResourceData(RESOURCE_TYPE_TEXTURE, textureData->id);

		RioRenderer::UniformHandle sampler;
		RioRenderer::TextureHandle texture;
		sampler.idx = textureHandle->samplerHandleIndex;
		texture.idx = textureResource->handle.idx;

		RioRenderer::setTexture(i
			, sampler
			, texture
			, shaderManager.getSamplerState(this->materialResource->shader, textureData->name)
			);
	}

	// Set uniforms
	for (uint32_t i = 0; i < this->materialResource->uniformListCount; ++i)
	{
		const UniformHandle* uniformHandle = getUniformHandleByIndex(this->materialResource, i, this->materialData);

		RioRenderer::UniformHandle rioRendererUniformHandle;
		rioRendererUniformHandle.idx = uniformHandle->uniformHandleIndex;
		RioRenderer::setUniform(rioRendererUniformHandle, (char*)uniformHandle + sizeof(uniformHandle->uniformHandleIndex));
	}

	shaderManager.submit(this->materialResource->shader, view, depth);
}

void Material::setFloat(StringId32 name, float value)
{
	char* ptr = (char*)MaterialResourceFn::getUniformHandleByName(this->materialResource, name, this->materialData);
	*(float*)(ptr + sizeof(uint32_t)) = value;
}

void Material::setVector2(StringId32 name, const Vector2& value)
{
	char* ptr = (char*)MaterialResourceFn::getUniformHandleByName(this->materialResource, name, this->materialData);
	*(Vector2*)(ptr + sizeof(uint32_t)) = value;
}

void Material::setVector3(StringId32 name, const Vector3& value)
{
	char* ptr = (char*)MaterialResourceFn::getUniformHandleByName(this->materialResource, name, this->materialData);
	*(Vector3*)(ptr + sizeof(uint32_t)) = value;
}

} // namespace Rio

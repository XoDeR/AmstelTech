#include "World/Renderer/SpriteManager.h"

#include "Core/Containers/HashMap.h"

namespace Rio
{

void SpriteManager::allocate(uint32_t itemListCount)
{
	RIO_ENSURE(itemsCount > this->spriteInstanceData.size);

	const uint32_t bytes = 0
		+ itemListCount * sizeof(UnitId) + alignof(UnitId)
		+ itemListCount * sizeof(SpriteResource*) + alignof(SpriteResource*)
		+ itemListCount * sizeof(StringId64) + alignof(StringId64)
		+ itemListCount * sizeof(uint32_t) + alignof(uint32_t)
		+ itemListCount * sizeof(Matrix4x4) + alignof(Matrix4x4)
		+ itemListCount * sizeof(Aabb) + alignof(Aabb)
		+ itemListCount * sizeof(bool) + alignof(bool)
		+ itemListCount * sizeof(bool) + alignof(bool)
		+ itemListCount * sizeof(uint32_t) + alignof(uint32_t)
		+ itemListCount * sizeof(uint32_t) + alignof(uint32_t)
		+ itemListCount * sizeof(SpriteInstance) + alignof(SpriteInstance)
		;

	SpriteInstanceData newSpriteInstanceData;
	newSpriteInstanceData.size = this->spriteInstanceData.size;
	newSpriteInstanceData.capacity = itemListCount;
	newSpriteInstanceData.buffer = allocator->allocate(bytes);
	newSpriteInstanceData.firstHiddenIndex = this->spriteInstanceData.firstHiddenIndex;

	newSpriteInstanceData.unitIdList = (UnitId*)newSpriteInstanceData.buffer;
	newSpriteInstanceData.spriteResourceList = (const SpriteResource**)Memory::getAlignedToTop(newSpriteInstanceData.unitIdList + itemListCount, alignof(const SpriteResource*));
	newSpriteInstanceData.materialNameList = (StringId64*)Memory::getAlignedToTop(newSpriteInstanceData.spriteResourceList + itemListCount, alignof(StringId64));
	newSpriteInstanceData.frameIdList = (uint32_t*)Memory::getAlignedToTop(newSpriteInstanceData.materialNameList + itemListCount, alignof(uint32_t));
	newSpriteInstanceData.worldMatrix4x4List = (Matrix4x4*)Memory::getAlignedToTop(newSpriteInstanceData.frameIdList + itemListCount, alignof(Matrix4x4));
	newSpriteInstanceData.aabbList = (Aabb*)Memory::getAlignedToTop(newSpriteInstanceData.worldMatrix4x4List + itemListCount, alignof(Aabb));
	newSpriteInstanceData.flipXList = (bool*)Memory::getAlignedToTop(newSpriteInstanceData.aabbList + itemListCount, alignof(bool));
	newSpriteInstanceData.flipYList = (bool*)Memory::getAlignedToTop(newSpriteInstanceData.flipXList + itemListCount, alignof(bool));
	newSpriteInstanceData.layerList = (uint32_t*)Memory::getAlignedToTop(newSpriteInstanceData.flipYList + itemListCount, alignof(uint32_t));
	newSpriteInstanceData.depthList = (uint32_t*)Memory::getAlignedToTop(newSpriteInstanceData.layerList + itemListCount, alignof(uint32_t));
	newSpriteInstanceData.nextSpriteInstanceList = (SpriteInstance*)Memory::getAlignedToTop(newSpriteInstanceData.depthList + itemListCount, alignof(SpriteInstance));

	memcpy(newSpriteInstanceData.unitIdList, this->spriteInstanceData.unitIdList, this->spriteInstanceData.size * sizeof(UnitId));
	memcpy(newSpriteInstanceData.spriteResourceList, this->spriteInstanceData.spriteResourceList, this->spriteInstanceData.size * sizeof(SpriteResource*));
	memcpy(newSpriteInstanceData.materialNameList, this->spriteInstanceData.materialNameList, this->spriteInstanceData.size * sizeof(StringId64));
	memcpy(newSpriteInstanceData.frameIdList, this->spriteInstanceData.frameIdList, this->spriteInstanceData.size * sizeof(uint32_t));
	memcpy(newSpriteInstanceData.worldMatrix4x4List, this->spriteInstanceData.worldMatrix4x4List, this->spriteInstanceData.size * sizeof(Matrix4x4));
	memcpy(newSpriteInstanceData.aabbList, this->spriteInstanceData.aabbList, this->spriteInstanceData.size * sizeof(Aabb));
	memcpy(newSpriteInstanceData.flipXList, this->spriteInstanceData.flipXList, this->spriteInstanceData.size * sizeof(bool));
	memcpy(newSpriteInstanceData.flipYList, this->spriteInstanceData.flipYList, this->spriteInstanceData.size * sizeof(bool));
	memcpy(newSpriteInstanceData.layerList, this->spriteInstanceData.layerList, this->spriteInstanceData.size * sizeof(uint32_t));
	memcpy(newSpriteInstanceData.depthList, this->spriteInstanceData.depthList, this->spriteInstanceData.size * sizeof(uint32_t));
	memcpy(newSpriteInstanceData.nextSpriteInstanceList, this->spriteInstanceData.nextSpriteInstanceList, this->spriteInstanceData.size * sizeof(SpriteInstance));

	allocator->deallocate(this->spriteInstanceData.buffer);
	this->spriteInstanceData = newSpriteInstanceData;
}

void SpriteManager::grow()
{
	allocate(this->spriteInstanceData.capacity * 2 + 1);
}

SpriteInstance SpriteManager::create(UnitId unitId, const SpriteResource* spriteResource, StringId64 materialName, uint32_t layer, uint32_t depth, const Matrix4x4& transformMatrix4x4)
{
	if (this->spriteInstanceData.size == this->spriteInstanceData.capacity)
	{
		grow();
	}

	const uint32_t lastIndex = this->spriteInstanceData.size;

	this->spriteInstanceData.unitIdList[lastIndex] = unitId;
	this->spriteInstanceData.spriteResourceList[lastIndex] = spriteResource;
	this->spriteInstanceData.materialNameList[lastIndex] = materialName;
	this->spriteInstanceData.frameIdList[lastIndex] = 0;
	this->spriteInstanceData.worldMatrix4x4List[lastIndex] = transformMatrix4x4;
	this->spriteInstanceData.aabbList[lastIndex] = Aabb();
	this->spriteInstanceData.flipXList[lastIndex] = false;
	this->spriteInstanceData.flipYList[lastIndex] = false;
	this->spriteInstanceData.layerList[lastIndex] = layer;
	this->spriteInstanceData.depthList[lastIndex] = depth;
	this->spriteInstanceData.nextSpriteInstanceList[lastIndex] = makeSpriteInstance(UINT32_MAX);

	++this->spriteInstanceData.size;
	++this->spriteInstanceData.firstHiddenIndex;

	HashMapFn::set(this->unitIdToSpriteInstanceIndexMap, unitId, lastIndex);
	return makeSpriteInstance(lastIndex);
}

void SpriteManager::destroy(SpriteInstance spriteInstance)
{
	RIO_ASSERT(spriteInstance.index < this->spriteInstanceData.size, "Index out of bounds");

	const uint32_t lastIndex = this->spriteInstanceData.size - 1;
	const UnitId unitId = this->spriteInstanceData.unitIdList[spriteInstance.index];
	const UnitId lastUnitId = this->spriteInstanceData.unitIdList[lastIndex];

	this->spriteInstanceData.unitIdList[spriteInstance.index] = this->spriteInstanceData.unitIdList[lastIndex];
	this->spriteInstanceData.spriteResourceList[spriteInstance.index] = this->spriteInstanceData.spriteResourceList[lastIndex];
	this->spriteInstanceData.materialNameList[spriteInstance.index] = this->spriteInstanceData.materialNameList[lastIndex];
	this->spriteInstanceData.frameIdList[spriteInstance.index] = this->spriteInstanceData.frameIdList[lastIndex];
	this->spriteInstanceData.worldMatrix4x4List[spriteInstance.index] = this->spriteInstanceData.worldMatrix4x4List[lastIndex];
	this->spriteInstanceData.aabbList[spriteInstance.index] = this->spriteInstanceData.aabbList[lastIndex];
	this->spriteInstanceData.flipXList[spriteInstance.index] = this->spriteInstanceData.flipXList[lastIndex];
	this->spriteInstanceData.flipYList[spriteInstance.index] = this->spriteInstanceData.flipYList[lastIndex];
	this->spriteInstanceData.layerList[spriteInstance.index] = this->spriteInstanceData.layerList[lastIndex];
	this->spriteInstanceData.depthList[spriteInstance.index] = this->spriteInstanceData.depthList[lastIndex];
	this->spriteInstanceData.nextSpriteInstanceList[spriteInstance.index] = this->spriteInstanceData.nextSpriteInstanceList[lastIndex];

	--this->spriteInstanceData.size;
	--this->spriteInstanceData.firstHiddenIndex;

	HashMapFn::set(this->unitIdToSpriteInstanceIndexMap, lastUnitId, spriteInstance.index);
	HashMapFn::remove(this->unitIdToSpriteInstanceIndexMap, unitId);
}

bool SpriteManager::has(UnitId unitId)
{
	return getIsValid(getSpriteInstanceByUnitId(unitId));
}

SpriteInstance SpriteManager::getSpriteInstanceByUnitId(UnitId unitId)
{
	return makeSpriteInstance(HashMapFn::get(this->unitIdToSpriteInstanceIndexMap, unitId, UINT32_MAX));
}

void SpriteManager::destroy()
{
	allocator->deallocate(this->spriteInstanceData.buffer);
}

} // namespace Rio
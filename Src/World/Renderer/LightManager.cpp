#include "World/Renderer/LightManager.h"

#include "Core/Containers/HashMap.h"

#include "Core/Math/Vector4.h"
#include "Core/Math/Color4.h"
#include "Core/Math/Matrix4x4.h"

#include "World/Renderer/DebugLine.h"

namespace Rio
{

void LightManager::allocate(uint32_t instanceDataSize)
{
	RIO_ENSURE(instanceDataSize > this->lightInstanceData.size);

	const uint32_t bytes = 0
		+ instanceDataSize * sizeof(UnitId) + alignof(UnitId)
		+ instanceDataSize * sizeof(Matrix4x4) + alignof(Matrix4x4)
		+ instanceDataSize * sizeof(float) + alignof(float)
		+ instanceDataSize * sizeof(float) + alignof(float)
		+ instanceDataSize * sizeof(float) + alignof(float)
		+ instanceDataSize * sizeof(Color4) + alignof(Color4)
		+ instanceDataSize * sizeof(uint32_t) + alignof(uint32_t)
		;

	LightInstanceData newLightInstanceData;
	newLightInstanceData.size = this->lightInstanceData.size;
	newLightInstanceData.capacity = instanceDataSize;
	newLightInstanceData.buffer = allocator->allocate(bytes);

	newLightInstanceData.unitIdList = (UnitId*)newLightInstanceData.buffer;
	newLightInstanceData.worldMatrix4x4List = (Matrix4x4*)Memory::getAlignedToTop(newLightInstanceData.unitIdList + instanceDataSize, alignof(Matrix4x4));
	newLightInstanceData.rangeList = (float*)Memory::getAlignedToTop(newLightInstanceData.worldMatrix4x4List + instanceDataSize, alignof(float));
	newLightInstanceData.intensityList = (float*)Memory::getAlignedToTop(newLightInstanceData.rangeList + instanceDataSize, alignof(float));
	newLightInstanceData.spotAngleList = (float*)Memory::getAlignedToTop(newLightInstanceData.intensityList + instanceDataSize, alignof(float));
	newLightInstanceData.colorList = (Color4*)Memory::getAlignedToTop(newLightInstanceData.spotAngleList + instanceDataSize, alignof(Color4));
	newLightInstanceData.lightTypeList = (uint32_t*)Memory::getAlignedToTop(newLightInstanceData.colorList + instanceDataSize, alignof(uint32_t));

	memcpy(newLightInstanceData.unitIdList, this->lightInstanceData.unitIdList, this->lightInstanceData.size * sizeof(UnitId));
	memcpy(newLightInstanceData.worldMatrix4x4List, this->lightInstanceData.worldMatrix4x4List, this->lightInstanceData.size * sizeof(Matrix4x4));
	memcpy(newLightInstanceData.rangeList, this->lightInstanceData.rangeList, this->lightInstanceData.size * sizeof(float));
	memcpy(newLightInstanceData.intensityList, this->lightInstanceData.intensityList, this->lightInstanceData.size * sizeof(float));
	memcpy(newLightInstanceData.spotAngleList, this->lightInstanceData.spotAngleList, this->lightInstanceData.size * sizeof(float));
	memcpy(newLightInstanceData.colorList, this->lightInstanceData.colorList, this->lightInstanceData.size * sizeof(Color4));
	memcpy(newLightInstanceData.lightTypeList, this->lightInstanceData.lightTypeList, this->lightInstanceData.size * sizeof(uint32_t));

	allocator->deallocate(this->lightInstanceData.buffer);
	this->lightInstanceData = newLightInstanceData;
}

void LightManager::grow()
{
	allocate(this->lightInstanceData.capacity * 2 + 1);
}

LightInstance LightManager::create(UnitId unitId, const LightDesc& lightDesc, const Matrix4x4& transformMatrix4x4)
{
	RIO_ASSERT(!HashMapFn::has(this->unitIdToLightInstanceIndexMap, unitId), "Unit already has light");

	if (this->lightInstanceData.size == this->lightInstanceData.capacity)
	{
		grow();
	}

	const uint32_t lastIndex = this->lightInstanceData.size;

	this->lightInstanceData.unitIdList[lastIndex] = unitId;
	this->lightInstanceData.worldMatrix4x4List[lastIndex] = transformMatrix4x4;
	this->lightInstanceData.rangeList[lastIndex] = lightDesc.range;
	this->lightInstanceData.intensityList[lastIndex] = lightDesc.intensity;
	this->lightInstanceData.spotAngleList[lastIndex] = lightDesc.spotAngle;
	this->lightInstanceData.colorList[lastIndex] = createVector4(lightDesc.color.x, lightDesc.color.y, lightDesc.color.z, 1.0f);
	this->lightInstanceData.lightTypeList[lastIndex] = lightDesc.type;

	++this->lightInstanceData.size;

	HashMapFn::set(this->unitIdToLightInstanceIndexMap, unitId, lastIndex);
	return makeLightInstance(lastIndex);
}

void LightManager::destroy(LightInstance lightInstance)
{
	RIO_ASSERT(lightInstance.index < this->lightInstanceData.size, "Index out of bounds");

	const uint32_t lastIndex = this->lightInstanceData.size - 1;
	const UnitId unitId = this->lightInstanceData.unitIdList[lightInstance.index];
	const UnitId lastUnitId = this->lightInstanceData.unitIdList[lastIndex];

	this->lightInstanceData.unitIdList[lightInstance.index] = this->lightInstanceData.unitIdList[lastIndex];
	this->lightInstanceData.worldMatrix4x4List[lightInstance.index] = this->lightInstanceData.worldMatrix4x4List[lastIndex];
	this->lightInstanceData.rangeList[lightInstance.index] = this->lightInstanceData.rangeList[lastIndex];
	this->lightInstanceData.intensityList[lightInstance.index] = this->lightInstanceData.intensityList[lastIndex];
	this->lightInstanceData.spotAngleList[lightInstance.index] = this->lightInstanceData.spotAngleList[lastIndex];
	this->lightInstanceData.colorList[lightInstance.index] = this->lightInstanceData.colorList[lastIndex];
	this->lightInstanceData.lightTypeList[lightInstance.index] = this->lightInstanceData.lightTypeList[lastIndex];

	--this->lightInstanceData.size;

	HashMapFn::set(this->unitIdToLightInstanceIndexMap, lastUnitId, lightInstance.index);
	HashMapFn::remove(this->unitIdToLightInstanceIndexMap, unitId);
}

bool LightManager::has(UnitId unitId)
{
	return getIsValid(getLightInstanceByUnitId(unitId));
}

LightInstance LightManager::getLightInstanceByUnitId(UnitId unitId)
{
	return makeLightInstance(HashMapFn::get(this->unitIdToLightInstanceIndexMap, unitId, UINT32_MAX));
}

void LightManager::destroy()
{
	allocator->deallocate(this->lightInstanceData.buffer);
}

void LightManager::debugDraw(uint32_t startIndex, uint32_t count, DebugLine& debugLine)
{
	for (uint32_t i = startIndex; i < startIndex + count; ++i)
	{
		const Vector3 position = getTranslation(this->lightInstanceData.worldMatrix4x4List[i]);
		const Vector3 direction = -getZAxis(this->lightInstanceData.worldMatrix4x4List[i]);

		switch (this->lightInstanceData.lightTypeList[i])
		{
		case LightType::DIRECTIONAL:
		{
			const Vector3 end = position + direction * 3.0f;
			debugLine.addLine(position, end, COLOR4_YELLOW);
			debugLine.addCone(position + direction * 2.8f, end, 0.1f, COLOR4_YELLOW);
		}
		break;

		case LightType::OMNI:
			debugLine.addSphere(position, this->lightInstanceData.rangeList[i], COLOR4_YELLOW);
			break;

		case LightType::SPOT:
		{
			const float angle = this->lightInstanceData.spotAngleList[i];
			const float range = this->lightInstanceData.rangeList[i];
			const float radius = getTanFloat(angle) * range;
			debugLine.addCone(position + range * direction, position, radius, COLOR4_YELLOW);
		}
		break;

		default:
			RIO_FATAL("Unknown light type");
			break;
		}
	}
}

} // namespace Rio
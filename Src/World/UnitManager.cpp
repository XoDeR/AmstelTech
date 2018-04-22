#include "World/UnitManager.h"

#include "Core/Containers/Array.h"
#include "Core/Containers/Queue.h"

#include "World/World.h"

#define MINIMUM_FREE_INDICES 1024

namespace Rio
{

UnitManager::UnitManager(Allocator& a)
	: generationList(a)
	, freeIndexList(a)
	, destroyCallbackList(a)
{
}

UnitId UnitManager::makeUnit(uint32_t unitIndex, uint8_t generation)
{
	UnitId unitId = { 0 | unitIndex | static_cast<uint32_t>(generation) << UNIT_INDEX_BITS };
	return unitId;
}

UnitId UnitManager::createNewUnit()
{
	uint32_t unitIndex = UINT32_MAX;

	if (QueueFn::getCount(this->freeIndexList) > MINIMUM_FREE_INDICES)
	{
		unitIndex = QueueFn::getFront(this->freeIndexList);
		QueueFn::popFront(this->freeIndexList);
	}
	else
	{
		ArrayFn::pushBack(this->generationList, static_cast<uint8_t>(0));
		unitIndex = ArrayFn::getCount(this->generationList) - 1;
		RIO_ASSERT(unitIndex < (1 << UNIT_INDEX_BITS), "Unit index is out of bounds");
	}

	return makeUnit(unitIndex, this->generationList[unitIndex]);
}

UnitId UnitManager::createNewUnit(World& world)
{
	return world.createNewEmptyUnit();
}

bool UnitManager::getIsUnitAlive(UnitId unitId) const
{
	return this->generationList[unitId.getUnitIndex()] == unitId.getUnitId();
}

void UnitManager::destroy(UnitId unitId)
{
	const uint32_t unitIndex = unitId.getUnitIndex();
	++(this->generationList[unitIndex]);
	QueueFn::pushBack(this->freeIndexList, unitIndex);

	triggerDestroyCallbackList(unitId);
}

void UnitManager::registerDestroyFunction(DestroyFunction destroyFunction, void* userData)
{
	DestroyData destroyData;
	destroyData.destroy = destroyFunction;
	destroyData.userData = userData;
	ArrayFn::pushBack(this->destroyCallbackList, destroyData);
}

void UnitManager::unregisterDestroyFunction(void* userData)
{
	for (uint32_t i = 0, listCount = ArrayFn::getCount(this->destroyCallbackList); i < listCount; ++i)
	{
		if (this->destroyCallbackList[i].userData == userData)
		{
			this->destroyCallbackList[i] = this->destroyCallbackList[listCount - 1];
			ArrayFn::popBack(this->destroyCallbackList);
			return;
		}
	}

	RIO_FATAL("Unknown destroy function");
}

void UnitManager::triggerDestroyCallbackList(UnitId unitId)
{
	for (uint32_t i = 0; i < ArrayFn::getCount(this->destroyCallbackList); ++i)
	{
		this->destroyCallbackList[i].destroy(unitId, destroyCallbackList[i].userData);
	}
}

} // namespace Rio

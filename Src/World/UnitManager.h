#pragma once

#include "Core/Containers/Types.h"
#include "World/Types.h"

namespace Rio
{

struct UnitManager
{
	using DestroyFunction = void (*)(UnitId unit, void* userData);

	struct DestroyData
	{
		DestroyFunction destroy = nullptr;
		void* userData = nullptr;
	};

	Array<uint8_t> generationList;
	Queue<uint32_t> freeIndexList;
	Array<DestroyData> destroyCallbackList;

	UnitManager(Allocator& a);

private:
	UnitId makeUnit(uint32_t unitIndex, uint8_t generation);

public:
	// Creates a new unit with available unitId
	UnitId createNewUnit();

	// Creates a new unit with available unitId in the given <world>
	UnitId createNewUnit(World& world);

	// Returns whether the unit <unitId> is alive
	bool getIsUnitAlive(UnitId unitId) const;

	// Destroys the unit <unitId>
	void destroy(UnitId unitId);

	void registerDestroyFunction(DestroyFunction destroyFunction, void* userData);

	void unregisterDestroyFunction(void* userData);

	void triggerDestroyCallbackList(UnitId unitId);
};

} // namespace Rio

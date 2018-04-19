#pragma once

#include "Core/Containers/Types.h"
#include "Core/Math/Types.h"
#include "Core/Memory/Types.h"

#include "Resource/Types.h"

#include "World/Types.h"

namespace Rio
{

// Game level
struct Level
{
	uint32_t marker = LEVEL_MARKER;
	Allocator* allocator = nullptr;
	UnitManager* unitManager = nullptr;
	World* world = nullptr;
	const LevelResource* levelResource = nullptr;
	Array<UnitId> unitLookupList;

	Level(Allocator& a, UnitManager& unitManager, World& world, const LevelResource& levelResource);
	~Level();
	void load(const Vector3& position, const Quaternion& rotation);
};

} // namespace Rio

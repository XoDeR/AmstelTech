#include "World/Level.h"

#include "Resource/LevelResource.h"
#include "Resource/UnitResource.h"

#include "World/UnitManager.h"
#include "World/World.h"

namespace Rio
{

Level::Level(Allocator& a, UnitManager& unitManager, World& world, const LevelResource& levelResource)
	: allocator(&a)
	, unitManager(&unitManager)
	, world(&world)
	, levelResource(&levelResource)
	, unitLookupList(a)
{
}

Level::~Level()
{
	marker = 0;
}

void Level::load(const Vector3& position, const Quaternion& rotation)
{
	// Spawn units
	const UnitResource* unitResource = LevelResourceFn::getUnitResourceListOffset(this->levelResource);

	ArrayFn::resize(this->unitLookupList, unitResource->unitListCount);
	for (uint32_t i = 0; i < unitResource->unitListCount; ++i)
	{
		this->unitLookupList[i] = unitManager->create();
	}

	spawnUnitList(*this->world, *unitResource, position, rotation, ArrayFn::begin(this->unitLookupList));

#if AMSTEL_ENGINE_SOUND
	// Play sounds
	const uint32_t soundListCount = LevelResourceFn::getSoundListCount(this->levelResource);
	for (uint32_t i = 0; i < soundListCount; ++i)
	{
		const LevelSound* levelSound = LevelResourceFn::getSoundByIndex(this->levelResource, i);
		this->world->playSound(levelSound->name
			, levelSound->loop
			, levelSound->volume
			, levelSound->position
			, levelSound->range
			);
	}
#endif // AMSTEL_ENGINE_SOUND
}

} // namespace Rio

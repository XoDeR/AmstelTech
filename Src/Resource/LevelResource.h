#pragma once

#include "Core/FileSystem/Types.h"
#include "Core/Math/Types.h"
#include "Core/Memory/Types.h"
#include "Core/Strings/StringId.h"

#include "Resource/Types.h"

namespace Rio
{

struct LevelResource
{
	uint32_t version = UINT32_MAX;
	uint32_t unitListOffset = 0;

#if AMSTEL_ENGINE_SOUND
	uint32_t soundListCount = 0;
	uint32_t soundListOffset = 0;
#endif // AMSTEL_ENGINE_SOUND
};

#if AMSTEL_ENGINE_SOUND
struct LevelSound
{
	StringId64 name;
	Vector3 position;
	float volume = 0.0f;
	float range = 0.0f;
	bool loop = false;
	char padding00[3] = { 0, 0, 0 };
};
#endif // AMSTEL_ENGINE_SOUND

namespace LevelResourceInternalFn
{
	void compile(CompileOptions& compileOptions);

} // namespace LevelResourceInternalFn

namespace LevelResourceFn
{
	// Returns the unit resource in the level
	const UnitResource* getUnitResourceListOffset(const LevelResource* levelResource);

#if AMSTEL_ENGINE_SOUND
	// Returns the number of sounds in the level resource
	uint32_t getSoundListCount(const LevelResource* levelResource);

	// Returns the sound <i>
	const LevelSound* getSoundByIndex(const LevelResource* levelResource, uint32_t i);
#endif // AMSTEL_ENGINE_SOUND

} // namespace LevelResourceFn

} // namespace Rio

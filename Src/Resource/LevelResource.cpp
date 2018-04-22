#include "Resource/LevelResource.h"

#include "Core/Containers/Array.h"
#include "Core/Containers/Map.h"
#include "Core/FileSystem/File.h"
#include "Core/FileSystem/FileSystem.h"
#include "Core/Json/JsonObject.h"
#include "Core/Json/RJson.h"
#include "Core/Memory/Memory.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Strings/DynamicString.h"

#include "Resource/CompileOptions.h"
#include "Resource/UnitCompiler.h"

namespace Rio
{

namespace LevelResourceInternalFn
{
	void compile(CompileOptions& compileOptions)
	{
		Buffer buffer = compileOptions.read();
		TempAllocator4096 tempAllocator4096;
		JsonObject levelJsonObject(tempAllocator4096);
		RJsonFn::parse(buffer, levelJsonObject);

#if AMSTEL_ENGINE_SOUND
		Array<LevelSound> levelSoundList(getDefaultAllocator());
		{
			JsonObject soundListJsonObject(tempAllocator4096);
			RJsonFn::parseObject(levelJsonObject["soundList"], soundListJsonObject);

			auto currentSoundJson = JsonObjectFn::begin(soundListJsonObject);
			auto endSoundJson = JsonObjectFn::end(soundListJsonObject);
			for (; currentSoundJson != endSoundJson; ++currentSoundJson)
			{
				JsonObject soundJsonObject(tempAllocator4096);
				RJsonFn::parseObject(currentSoundJson->pair.second, soundJsonObject);

				DynamicString soundNameString(tempAllocator4096);
				RJsonFn::parseString(soundJsonObject["name"], soundNameString);
				DATA_COMPILER_ASSERT_RESOURCE_EXISTS("sound"
					, soundNameString.getCStr()
					, compileOptions
					);

				LevelSound levelSound;
				levelSound.name = RJsonFn::parseResourceId(soundJsonObject["name"]);
				levelSound.position = RJsonFn::parseVector3(soundJsonObject["position"]);
				levelSound.volume = RJsonFn::parseFloat(soundJsonObject["volume"]);
				levelSound.range = RJsonFn::parseFloat(soundJsonObject["range"]);
				levelSound.loop = RJsonFn::parseBool(soundJsonObject["loop"]);

				ArrayFn::pushBack(levelSoundList, levelSound);
			}
		}
#endif // AMSTEL_ENGINE_SOUND

		UnitCompiler unitCompiler(compileOptions);
		unitCompiler.compileUnitListFromJson(levelJsonObject["unitList"]);
		Buffer unitBlob = unitCompiler.getBlob();

		// Write
		LevelResource levelResource;
		levelResource.version = RESOURCE_VERSION_LEVEL;

#if AMSTEL_ENGINE_SOUND
		levelResource.soundListCount = ArrayFn::getCount(levelSoundList);
#endif // AMSTEL_ENGINE_SOUND

		levelResource.unitListOffset = sizeof(LevelResource);

#if AMSTEL_ENGINE_SOUND
		levelResource.soundListOffset = levelResource.unitListOffset + ArrayFn::getCount(unitBlob);
#endif // AMSTEL_ENGINE_SOUND

		compileOptions.write(levelResource.version);
		compileOptions.write(levelResource.unitListOffset);

#if AMSTEL_ENGINE_SOUND
		compileOptions.write(levelResource.soundListCount);
		compileOptions.write(levelResource.soundListOffset);
#endif // AMSTEL_ENGINE_SOUND

		compileOptions.write(unitBlob);

#if AMSTEL_ENGINE_SOUND
		for (uint32_t i = 0; i < ArrayFn::getCount(levelSoundList); ++i)
		{
			compileOptions.write(levelSoundList[i].name);
			compileOptions.write(levelSoundList[i].position);
			compileOptions.write(levelSoundList[i].volume);
			compileOptions.write(levelSoundList[i].range);
			compileOptions.write(levelSoundList[i].loop);
			compileOptions.write(levelSoundList[i].padding00[0]);
			compileOptions.write(levelSoundList[i].padding00[1]);
			compileOptions.write(levelSoundList[i].padding00[2]);
		}
#endif // AMSTEL_ENGINE_SOUND
	}

} // namespace LevelResourceInternalFn

namespace LevelResourceFn
{
	const UnitResource* getUnitResourceListOffset(const LevelResource* levelResource)
	{
		return (const UnitResource*)((char*)levelResource + levelResource->unitListOffset);
	}

#if AMSTEL_ENGINE_SOUND
	uint32_t getSoundListCount(const LevelResource* levelResource)
	{
		return levelResource->soundListCount;
	}

	const LevelSound* getSoundByIndex(const LevelResource* levelResource, uint32_t i)
	{
		RIO_ASSERT(i < getSoundListCount(levelResource), "Index out of bounds");
		const LevelSound* begin = (LevelSound*)((char*)levelResource + levelResource->soundListOffset);
		return &begin[i];
	}
#endif // AMSTEL_ENGINE_SOUND

} // namespace LevelResourceFn

} // namespace Rio

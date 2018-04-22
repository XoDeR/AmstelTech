#include "Resource/Sprite/SpriteResource.h"

#include "Config.h"
#include "Core/Containers/Array.h"
#include "Core/FileSystem/ReaderWriter.h"
#include "Core/Json/JsonObject.h"
#include "Core/Json/RJson.h"
#include "Core/Math/Aabb.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Strings/String.h"

#include "Resource/CompileOptions.h"
#include "Resource/ResourceManager.h"

namespace Rio
{

namespace SpriteResourceInternalFn
{
	struct SpriteFrame
	{
		StringId32 name;
		Vector4 region; // [x, y, w, h]
		Vector2 pivot;  // [x, y]
	};

	void parseSpriteFrame(const char* json, SpriteFrame& spriteFrame)
	{
		TempAllocator512 tempAllocator512;
		JsonObject spriteFrameJsonObject(tempAllocator512);
		RJsonFn::parse(json, spriteFrameJsonObject);

		spriteFrame.name = RJsonFn::parseStringId(spriteFrameJsonObject["name"]);
		spriteFrame.region = RJsonFn::parseVector4(spriteFrameJsonObject["region"]);
		spriteFrame.pivot = RJsonFn::parseVector2(spriteFrameJsonObject["pivot"]);
	}

	void compile(CompileOptions& compileOptions)
	{
		Buffer buffer = compileOptions.read();

		TempAllocator4096 tempAllocator4096;
		JsonObject spriteJsonObject(tempAllocator4096);
		RJsonFn::parse(buffer, spriteJsonObject);

		JsonArray frameListJsonArray(tempAllocator4096);
		RJsonFn::parseArray(spriteJsonObject["frameList"], frameListJsonArray);

		// Read width/height
		const float width = RJsonFn::parseFloat(spriteJsonObject["width" ]);
		const float height = RJsonFn::parseFloat(spriteJsonObject["height"]);
		const uint32_t frameListCount = ArrayFn::getCount(frameListJsonArray);

		Array<float> vertexList(getDefaultAllocator());
		for (uint32_t i = 0; i < frameListCount; ++i)
		{
			SpriteFrame spriteFrame;
			parseSpriteFrame(frameListJsonArray[i], spriteFrame);

			// Compute uv coords
			float u0 = spriteFrame.region.x / width;
			float v0 = (spriteFrame.region.y + spriteFrame.region.w) / height;
			float u1 = (spriteFrame.region.x + spriteFrame.region.z) / width;
			float v1 = spriteFrame.region.y / height;

			// Compute positions
			float x0 = spriteFrame.region.x - spriteFrame.pivot.x;
			float y0 = -(spriteFrame.region.y + spriteFrame.region.w - spriteFrame.pivot.y);
			float x1 = spriteFrame.region.x + spriteFrame.region.z - spriteFrame.pivot.x;
			float y1 = -(spriteFrame.region.y - spriteFrame.pivot.y);

			x0 /= RIO_DEFAULT_PIXELS_PER_METER;
			y0 /= RIO_DEFAULT_PIXELS_PER_METER;
			x1 /= RIO_DEFAULT_PIXELS_PER_METER;
			y1 /= RIO_DEFAULT_PIXELS_PER_METER;

			ArrayFn::pushBack(vertexList, x0);
			ArrayFn::pushBack(vertexList, y0);
			ArrayFn::pushBack(vertexList, u0);
			ArrayFn::pushBack(vertexList, v0);

			ArrayFn::pushBack(vertexList, x1);
			ArrayFn::pushBack(vertexList, y0);
			ArrayFn::pushBack(vertexList, u1);
			ArrayFn::pushBack(vertexList, v0);

			ArrayFn::pushBack(vertexList, x1);
			ArrayFn::pushBack(vertexList, y1);
			ArrayFn::pushBack(vertexList, u1);
			ArrayFn::pushBack(vertexList, v1);

			ArrayFn::pushBack(vertexList, x0);
			ArrayFn::pushBack(vertexList, y1);
			ArrayFn::pushBack(vertexList, u0);
			ArrayFn::pushBack(vertexList, v1);
		}

		Aabb aabb;
		AabbFn::reset(aabb);

		for (uint32_t i = 0; i < ArrayFn::getCount(vertexList); i += 4)
		{
			Vector3 vector3;
			vector3.x = vertexList[i + 0];
			vector3.y = 0.0f;
			vector3.z = vertexList[i + 1];
			AabbFn::addPoints(aabb, 1, &vector3);
		}

		aabb.min.y = -0.25f;
		aabb.max.y = 0.25f;

		Obb obb;
		obb.transformMatrix = createMatrix4x4(QUATERNION_IDENTITY, AabbFn::getCenter(aabb));
		obb.halfExtents.x = (aabb.max.x - aabb.min.x) * 0.5f;
		obb.halfExtents.y = (aabb.max.y - aabb.min.y) * 0.5f;
		obb.halfExtents.z = (aabb.max.z - aabb.min.z) * 0.5f;

		const uint32_t vertexListCount = ArrayFn::getCount(vertexList) / 4; // 4 components per vertex

		// Write
		SpriteResource spriteResource;
		spriteResource.version = RESOURCE_VERSION_SPRITE;
		spriteResource.obb = obb;
		spriteResource.vertexListCount = vertexListCount;

		compileOptions.write(spriteResource.version);
		compileOptions.write(spriteResource.obb);

		compileOptions.write(spriteResource.vertexListCount);
		for (uint32_t i = 0; i < ArrayFn::getCount(vertexList); ++i)
		{
			compileOptions.write(vertexList[i]);
		}
	}

} // namespace SpriteResourceInternalFn

namespace SpriteResourceFn
{
	const float* getFrameData(const SpriteResource* spriteResource, uint32_t frameIndex)
	{
		return ((float*)&spriteResource[1]) + 16 * frameIndex;
	}

} // namespace SpriteResourceFn

namespace SpriteAnimationResourceInternalFn
{
	void compile(CompileOptions& compileOptions)
	{
		Buffer buffer = compileOptions.read();

		TempAllocator4096 tempAllocator4096;
		JsonObject spriteAnimationJsonObject(tempAllocator4096);
		JsonArray frameListJsonArray(tempAllocator4096);

		Array<uint32_t> frameList(getDefaultAllocator());
		float totalTime = 0.0f;

		RJsonFn::parse(buffer, spriteAnimationJsonObject);
		RJsonFn::parseArray(spriteAnimationJsonObject["frameList"], frameListJsonArray);

		ArrayFn::resize(frameList, ArrayFn::getCount(frameListJsonArray));
		for (uint32_t i = 0; i < ArrayFn::getCount(frameListJsonArray); ++i)
		{
			frameList[i] = (uint32_t)RJsonFn::parseFloat(frameListJsonArray[i]);
		}

		totalTime = RJsonFn::parseFloat(spriteAnimationJsonObject["totalTime"]);

		// Write
		SpriteAnimationResource spriteAnimationResource;
		spriteAnimationResource.version = RESOURCE_VERSION_SPRITE_ANIMATION;
		spriteAnimationResource.frameListCount = ArrayFn::getCount(frameList);
		spriteAnimationResource.totalTime = totalTime;

		compileOptions.write(spriteAnimationResource.version);
		compileOptions.write(spriteAnimationResource.frameListCount);
		compileOptions.write(spriteAnimationResource.totalTime);

		for (uint32_t i = 0; i < ArrayFn::getCount(frameList); i++)
		{
			compileOptions.write(frameList[i]);
		}
	}

} // namespace SpriteAnimationResourceInternalFn

namespace SpriteAnimationResourceFn
{
	const uint32_t* getSpriteAnimationFrameList(const SpriteAnimationResource* spriteAnimationResource)
	{
		return (uint32_t*)&spriteAnimationResource[1];
	}

} // namespace SpriteAnimationResourceFn

} // namespace Rio

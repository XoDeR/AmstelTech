#pragma once

#include "Core/Containers/Types.h"
#include "Core/Math/Types.h"
#include "Core/Strings/StringId.h"

#include "Resource/Types.h"

#include "World/Types.h"

#include "RioRenderer/RioRenderer.h"

namespace Rio
{
	struct SpriteManager
	{
		struct SpriteInstanceData
		{
			uint32_t size = 0;
			uint32_t capacity = 0;
			void* buffer = nullptr;

			uint32_t firstHiddenIndex = 0;

			UnitId* unitIdList = nullptr;
			const SpriteResource** spriteResourceList = nullptr;
			StringId64* materialNameList = nullptr;
			uint32_t* frameIdList = nullptr;
			Matrix4x4* worldMatrix4x4List = nullptr;
			Aabb* aabbList = nullptr;
			bool* flipXList = nullptr;
			bool* flipYList = nullptr;
			uint32_t* layerList = nullptr;
			uint32_t* depthList = nullptr;
			SpriteInstance* nextSpriteInstanceList = nullptr;
		};

		Allocator* allocator = nullptr;
		HashMap<UnitId, uint32_t> unitIdToSpriteInstanceIndexMap;
		SpriteInstanceData spriteInstanceData;

		SpriteManager(Allocator& a)
			: allocator(&a)
			, unitIdToSpriteInstanceIndexMap(a)
		{
			memset(&spriteInstanceData, 0, sizeof(spriteInstanceData));
		}

		SpriteInstance create(UnitId unitId, const SpriteResource* spriteResource, StringId64 materialName, uint32_t layer, uint32_t depth, const Matrix4x4& transformMatrix4x4);
		void destroy(SpriteInstance spriteInstance);
		bool has(UnitId unitId);
		SpriteInstance getSpriteInstanceByUnitId(UnitId unitId);
		void allocate(uint32_t itemListCount);
		void grow();
		void destroy();

		SpriteInstance makeSpriteInstance(uint32_t index)
		{
			SpriteInstance spriteInstance = { index };
			return spriteInstance;
		}
	};


} // namespace Rio
#pragma once

#include "Core/Containers/Types.h"
#include "Core/Math/Types.h"
#include "Core/Strings/StringId.h"

#include "Resource/Types.h"

#include "World/Types.h"

#include "RioRenderer/RioRenderer.h"

namespace Rio
{

struct LightManager
{
	struct LightInstanceData
	{
		uint32_t size = 0;
		uint32_t capacity = 0;
		void* buffer = nullptr;

		UnitId* unitIdList = nullptr;
		Matrix4x4* worldMatrix4x4List = nullptr;
		float* rangeList = nullptr;
		float* intensityList = nullptr;
		float* spotAngleList = nullptr;
		Color4* colorList = nullptr;
		uint32_t* lightTypeList = nullptr; // LightType::Enum
	};

	Allocator* allocator = nullptr;
	HashMap<UnitId, uint32_t> unitIdToLightInstanceIndexMap;
	LightInstanceData lightInstanceData;

	LightManager(Allocator& a)
		: allocator(&a)
		, unitIdToLightInstanceIndexMap(a)
	{
		memset(&lightInstanceData, 0, sizeof(lightInstanceData));
	}

	LightInstance create(UnitId unitId, const LightDesc& lightDesc, const Matrix4x4& transformMatrix4x4);
	void destroy(LightInstance lightInstance);
	bool has(UnitId unitId);
	LightInstance getLightInstanceByUnitId(UnitId unitId);
	void debugDraw(uint32_t startIndex, uint32_t count, DebugLine& debugLine);
	void allocate(uint32_t instanceDataSize);
	void grow();
	void destroy();

	LightInstance makeLightInstance(uint32_t index)
	{
		LightInstance lightInstance = { index };
		return lightInstance;
	}
};


} // namespace Rio
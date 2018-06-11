#pragma once

#include "Config.h"
#include <cstdint> // uint32_t

namespace Rio
{

#define UNIT_INDEX_BITS 22
#define UNIT_INDEX_MASK 0x003fffff
#define UNIT_ID_BITS 8
#define UNIT_ID_MASK 0x3fc00000

	struct UnitId
	{
		uint32_t unitIndex = UINT32_MAX;

		uint32_t getUnitIndex() const
		{
			return unitIndex & UNIT_INDEX_MASK;
		}

		uint32_t getUnitId() const
		{
			return (unitIndex >> UNIT_INDEX_BITS) & UNIT_ID_MASK;
		}

		bool getIsUnitValid()
		{
			return unitIndex != UINT32_MAX;
		}
	};

	inline bool operator==(const UnitId& a, const UnitId& b)
	{
		return a.unitIndex == b.unitIndex;
	}

	const UnitId UNIT_INVALID = { UINT32_MAX };

	template <>
	struct Hash<UnitId>
	{
		uint32_t operator()(const UnitId& id) const
		{
			return id.unitIndex;
		}
	};
} // namespace Rio
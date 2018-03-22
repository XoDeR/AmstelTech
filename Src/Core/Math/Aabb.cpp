#include "Core/Math/Aabb.h"

namespace Rio
{

namespace AabbFn
{
	void addPoints(Aabb& b, uint32_t pointsCount, uint32_t stride, const void* points)
	{
		const char* pointsMemory = (const char*)points;
		for (uint32_t i = 0; i < pointsCount; ++i, pointsMemory += stride)
		{
			const Vector3& currentPoint = *(const Vector3*)pointsMemory;

			b.min.x = getMinFloat(b.min.x, currentPoint.x);
			b.min.y = getMinFloat(b.min.y, currentPoint.y);
			b.min.z = getMinFloat(b.min.z, currentPoint.z);

			b.max.x = getMaxFloat(b.max.x, currentPoint.x);
			b.max.y = getMaxFloat(b.max.y, currentPoint.y);
			b.max.z = getMaxFloat(b.max.z, currentPoint.z);
		}
	}

	void addBoxes(Aabb& b, uint32_t boxesCount, const Aabb* boxes)
	{
		for (uint32_t i = 0; i < boxesCount; ++i)
		{
			const Aabb& currentBox = boxes[i];

			b.min.x = getMinFloat(b.min.x, currentBox.min.x);
			b.min.y = getMinFloat(b.min.y, currentBox.min.y);
			b.min.z = getMinFloat(b.min.z, currentBox.min.z);

			b.max.x = getMaxFloat(b.max.x, currentBox.max.x);
			b.max.y = getMaxFloat(b.max.y, currentBox.max.y);
			b.max.z = getMaxFloat(b.max.z, currentBox.max.z);
		}
	}

} // namespace AabbFn

} // namespace Rio

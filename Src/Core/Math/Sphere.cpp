#include "Core/Math/Sphere.h"

namespace Rio
{

namespace SphereFn
{
	void addPoints(Sphere& s, uint32_t pointsCount, uint32_t stride, const void* points)
	{
		float radiusSquared = s.r*s.r;

		const char* pointsMemory = (const char*)points;
		for (uint32_t i = 0; i < pointsCount; ++i, pointsMemory += stride)
		{
			const Vector3& currentPoint = *(const Vector3*)pointsMemory;

			radiusSquared = getMaxFloat(radiusSquared, getLengthSquared(currentPoint - s.c));
		}

		s.r = getSqrtFloat(radiusSquared);
	}

	void addSpheres(Sphere& s, uint32_t spheresCount, const Sphere* spheres)
	{
		for (uint32_t i = 0; i < spheresCount; ++i)
		{
			const Sphere& currentSphere = spheres[i];
			const float distance = getLengthSquared(currentSphere.c - s.c);

			if (distance < (currentSphere.r + s.r) * (currentSphere.r + s.r))
			{
				if (currentSphere.r*currentSphere.r > s.r*s.r)
				{
					s.r = getSqrtFloat(distance + currentSphere.r*currentSphere.r);
				}
			}
		}
	}

} // namespace SphereFn

} // namespace Rio

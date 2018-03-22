#pragma once

#include "Core/Math/Math.h"
#include "Core/Math/Types.h"
#include "Core/Math/Vector3.h"

namespace Rio
{

namespace SphereFn
{
	// Resets the sphere <s>
	void reset(Sphere& s);

	// Returns the volume of the sphere <s>
	float getVolume(const Sphere& s);

	// Adds <pointsCount> <points> to the sphere <s>, expanding its bounds if necessary
	void addPoints(Sphere& s, uint32_t pointsCount, uint32_t stride, const void* points);

	// Adds <pointsCount> <points> to the sphere expanding if necessary
	void addPoints(Sphere& s, uint32_t pointsCount, const Vector3* points);

	// Adds <spheresCount> <spheres> expanding if necessary
	void addSpheres(Sphere& s, uint32_t spheresCount, const Sphere* spheres);

	// Returns whether point <p> is contained into the sphere
	bool containsPoint(const Sphere& s, const Vector3& p);

} // namespace SphereFn

namespace SphereFn
{
	inline void reset(Sphere& s)
	{
		s.c = VECTOR3_ZERO;
		s.r = 0.0f;
	}

	inline float getVolume(const Sphere& s)
	{
		return (4.0f/3.0f*PI) * (s.r*s.r*s.r);
	}

	inline void addPoints(Sphere& s, uint32_t pointsCount, const Vector3* points)
	{
		addPoints(s, pointsCount, sizeof(Vector3), points);
	}

	inline bool containsPoint(const Sphere& s, const Vector3& p)
	{
		float distance = getLengthSquared(p - s.c);
		return distance < s.r*s.r;
	}

} // namespace SphereFn

} // namespace Rio

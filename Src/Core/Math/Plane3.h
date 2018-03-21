#pragma once

#include "Core/Math/Math.h"
#include "Core/Math/Types.h"
#include "Core/Math/Vector3.h"

namespace Rio
{

namespace Plane3Fn
{
	// Returns the plane defined by the <point> and the <normal>
	Plane3 createFromPointAndNormal(const Vector3& point, const Vector3& normal);

	// Normalizes the plane <p> and returns its result
	Plane3& getNormalized(Plane3& p);

	// Returns the signed distance between plane <p> and point <point>
	float getDistanceToPoint(const Plane3& p, const Vector3& point);

} // namespace Plane3Fn

namespace Plane3Fn
{
	inline Plane3 createFromPointAndNormal(const Vector3& point, const Vector3& normal)
	{
		Plane3 p;

		p.n = normal;
		p.d = -getDotProduct(normal, point);

		return p;
	}

	inline Plane3& getNormalized(Plane3& p)
	{
		const float length = getLength(p.n);

		if (getAreEqualFloat(length, 0.0f))
		{
			return p;
		}

		const float invertedLength = 1.0f / length;

		p.n *= invertedLength;
		p.d *= invertedLength;

		return p;
	}

	inline float getDistanceToPoint(const Plane3& p, const Vector3& point)
	{
		return getDotProduct(p.n, point) + p.d;
	}

} // namespace Plane3Fn

} // namespace Rio

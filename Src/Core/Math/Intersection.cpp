#include "Core/Math/Intersection.h"

#include "Core/Math/Aabb.h"
#include "Core/Math/Plane3.h"
#include "Core/Math/Sphere.h"
#include "Core/Math/Vector3.h"

namespace Rio
{

float getRayPlaneIntersection(const Vector3& from, const Vector3& direction, const Plane3& p)
{
	const float tempFloat = getDotProduct(from, p.n);
	const float denominator = getDotProduct(direction, p.n);

	if (getAreEqualFloat(denominator, 0.0f))
	{
		return -1.0f;
	}

	return (-p.d - tempFloat) / denominator;
}

float getRayDiscIntersection(const Vector3& from, const Vector3& direction, const Vector3& center, float radius, const Vector3& normal)
{
	const Plane3 p = Plane3Fn::createFromPointAndNormal(center, normal);
	const float t = getRayPlaneIntersection(from, direction, p);

	if (t == -1.0f)
	{
		return -1.0f;
	}

	const Vector3 intersection_point = from + direction * t;
	if (getDistanceSquared(intersection_point, center) < radius*radius)
	{
		return t;
	}

	return -1.0f;
}

float getRaySphereIntersection(const Vector3& from, const Vector3& direction, const Sphere& s)
{
	const Vector3 v = s.c - from;
	const float b   = getDotProduct(v, direction);
	const float rr  = s.r * s.r;
	const float bb  = b * b;
	const float det = rr - getDotProduct(v, v) + bb;

	if (det < 0.0f || b < s.r)
	{
		return -1.0f;
	}

	return b - getSqrtFloat(det);
}

float getRayObbIntersection(const Vector3& from, const Vector3& direction, const Matrix4x4& tm, const Vector3& halfExtents)
{
	float tMin = 0.0f;
	float tMax = 999999999.9f;

	const Vector3 obbPosition = createVector3(tm.t.x, tm.t.y, tm.t.z);
	const Vector3 delta = obbPosition - from;

	{
		const Vector3 xAxis = createVector3(tm.x.x, tm.x.y, tm.x.z);
		const float e = getDotProduct(xAxis, delta);
		const float f = getDotProduct(direction, xAxis);

		if (getAbsFloat(f) > 0.001f)
		{
			float t1 = (e-halfExtents.x)/f;
			float t2 = (e+halfExtents.x)/f;

			if (t1 > t2) 
			{ 
				// swap
				float w = t1;
				t1 = t2;
				t2 = w; 
			}

			if (t2 < tMax)
			{
				tMax = t2;
			}
			if (t1 > tMin)
			{
				tMin = t1;
			}

			if (tMax < tMin)
			{
				return -1.0f;
			}

		}
		else
		{
			if (-e - halfExtents.x > 0.0f || -e + halfExtents.x < 0.0f)
			{
				return -1.0f;
			}
		}
	}

	{
		const Vector3 yAxis = createVector3(tm.y.x, tm.y.y, tm.y.z);
		const float e = getDotProduct(yAxis, delta);
		const float f = getDotProduct(direction, yAxis);

		if (getAbsFloat(f) > 0.001f)
		{
			float t1 = (e-halfExtents.y)/f;
			float t2 = (e+halfExtents.y)/f;

			if (t1 > t2) 
			{
				float w=t1;t1=t2;t2=w; 
			}

			if (t2 < tMax)
			{
				tMax = t2;
			}

			if (t1 > tMin)
			{
				tMin = t1;
			}

			if (tMin > tMax)
			{
				return -1.0f;
			}
		}
		else
		{
			if (-e - halfExtents.y > 0.0f || -e + halfExtents.y < 0.0f)
			{
				return -1.0f;
			}
		}
	}

	{
		const Vector3 zAxis = createVector3(tm.z.x, tm.z.y, tm.z.z);
		const float e = getDotProduct(zAxis, delta);
		const float f = getDotProduct(direction, zAxis);

		if (getAbsFloat(f) > 0.001f)
		{
			float t1 = (e-halfExtents.z)/f;
			float t2 = (e+halfExtents.z)/f;

			if (t1 > t2) 
			{ 
				float w = t1;
				t1 = t2;
				t2 = w; 
			}

			if (t2 < tMax)
			{
				tMax = t2;
			}

			if (t1 > tMin)
			{
				tMin = t1;
			}

			if (tMin > tMax)
			{
				return -1.0f;
			}

		}
		else
		{
			if (-e - halfExtents.z > 0.0f || -e + halfExtents.z < 0.0f)
			{
				return -1.0f;
			}
		}
	}

	return tMin;
}

float getRayTriangleIntersection(const Vector3& from, const Vector3& direction, const Vector3& v0, const Vector3& v1, const Vector3& v2)
{
	const Vector3 vertexList[] = { v0, v1, v2 };
	const uint16_t indexList[] = { 0, 1, 2 };
	return getRayMeshIntersection(from, direction, MATRIX4X4_IDENTITY, vertexList, sizeof(Vector3), indexList, 3);
}

float getRayMeshIntersection(const Vector3& from, const Vector3& direction, const Matrix4x4& tm, const void* vertices, uint32_t stride, const uint16_t* indices, uint32_t count)
{
	bool hit = false;
	float tMin = 999999999.9f;

	for (uint32_t i = 0; i < count; i += 3)
	{
		const uint32_t i0 = indices[i + 0];
		const uint32_t i1 = indices[i + 1];
		const uint32_t i2 = indices[i + 2];

		const Vector3& v0 = *(const Vector3*)((const char*)vertices + i0*stride) * tm;
		const Vector3& v1 = *(const Vector3*)((const char*)vertices + i1*stride) * tm;
		const Vector3& v2 = *(const Vector3*)((const char*)vertices + i2*stride) * tm;

		// Trumbore intersection algorithm

		// Find vectors for two edges sharing v0
		const Vector3 e1 = v1 - v0;
		const Vector3 e2 = v2 - v0;

		// Begin calculating determinant - also used to calculate u parameter
		const Vector3 P = getCrossProduct(direction, e2);

		// If determinant is near zero, ray lies in plane of triangle
		const float det = getDotProduct(e1, P);
		if (getAreEqualFloat(det, 0.0f))
		{
			continue;
		}

		const float invertedDeterminant = 1.0f / det;

		// Distance from v0 to ray origin
		const Vector3 T = from - v0;

		// u parameter and test bound
		const float u = getDotProduct(T, P) * invertedDeterminant;

		// The intersection lies outside of the triangle
		if (u < 0.0f || u > 1.0f)
		{
			continue;
		}

		// Prepare to test v parameter
		const Vector3 Q = getCrossProduct(T, e1);

		// v parameter and test bound
		const float v = getDotProduct(direction, Q) * invertedDeterminant;

		// The intersection lies outside of the triangle
		if (v < 0.0f || u + v  > 1.0f)
		{
			continue;
		}

		const float t = getDotProduct(e2, Q) * invertedDeterminant;

		// Ray intersection
		if (t > FLOAT_EPSILON)
		{
			hit = true;
			tMin = getMinFloat(t, tMin);
		}
	}

	return hit ? tMin : -1.0f;
}

bool getPlanesIntersection(const Plane3& a, const Plane3& b, const Plane3& c, Vector3& intersectionPoint)
{
	const Vector3 na = a.n;
	const Vector3 nb = b.n;
	const Vector3 nc = c.n;
	const float denominator = -getDotProduct(getCrossProduct(na, nb), nc);

	if (getAreEqualFloat(denominator, 0.0f))
	{
		return false;
	}

	const float invertedDenominator = 1.0f / denominator;

	const Vector3 nbnc = a.d * getCrossProduct(nb, nc);
	const Vector3 ncna = b.d * getCrossProduct(nc, na);
	const Vector3 nanb = c.d * getCrossProduct(na, nb);

	intersectionPoint = (nbnc + ncna + nanb) * invertedDenominator;

	return true;
}

bool getFrustumSphereIntersection(const Frustum& f, const Sphere& s)
{
	if (Plane3Fn::getDistanceToPoint(f.planeLeft, s.c) < -s.r ||
		Plane3Fn::getDistanceToPoint(f.planeRight, s.c) < -s.r)
	{
		return false;
	}

	if (Plane3Fn::getDistanceToPoint(f.planeBottom, s.c) < -s.r ||
		Plane3Fn::getDistanceToPoint(f.planeTop, s.c) < -s.r)
	{
		return false;
	}

	if (Plane3Fn::getDistanceToPoint(f.planeNear, s.c) < -s.r ||
		Plane3Fn::getDistanceToPoint(f.planeFar, s.c) < -s.r)
	{
		return false;
	}

	return true;
}

bool getFrustumBoxIntersection(const Frustum& f, const Aabb& b)
{
	const Vector3 v0 = AabbFn::getVertexByIndex(b, 0);
	const Vector3 v1 = AabbFn::getVertexByIndex(b, 1);
	const Vector3 v2 = AabbFn::getVertexByIndex(b, 2);
	const Vector3 v3 = AabbFn::getVertexByIndex(b, 3);
	const Vector3 v4 = AabbFn::getVertexByIndex(b, 4);
	const Vector3 v5 = AabbFn::getVertexByIndex(b, 5);
	const Vector3 v6 = AabbFn::getVertexByIndex(b, 6);
	const Vector3 v7 = AabbFn::getVertexByIndex(b, 7);

	uint8_t out = 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeLeft, v0) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeLeft, v1) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeLeft, v2) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeLeft, v3) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeLeft, v4) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeLeft, v5) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeLeft, v6) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeLeft, v7) < 0.0f) ? 1 : 0;
	if (out == 8)
	{
		return false;
	}

	out = 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeRight, v0) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeRight, v1) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeRight, v2) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeRight, v3) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeRight, v4) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeRight, v5) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeRight, v6) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeRight, v7) < 0.0f) ? 1 : 0;
	if (out == 8)
	{
		return false;
	}

	out = 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeBottom, v0) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeBottom, v1) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeBottom, v2) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeBottom, v3) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeBottom, v4) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeBottom, v5) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeBottom, v6) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeBottom, v7) < 0.0f) ? 1 : 0;
	if (out == 8)
	{
		return false;
	}

	out = 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeTop, v0) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeTop, v1) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeTop, v2) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeTop, v3) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeTop, v4) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeTop, v5) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeTop, v6) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeTop, v7) < 0.0f) ? 1 : 0;
	if (out == 8)
	{
		return false;
	}

	out = 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeNear, v0) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeNear, v1) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeNear, v2) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeNear, v3) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeNear, v4) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeNear, v5) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeNear, v6) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeNear, v7) < 0.0f) ? 1 : 0;
	if (out == 8)
	{
		return false;
	}

	out = 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeFar, v0) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeFar, v1) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeFar, v2) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeFar, v3) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeFar, v4) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeFar, v5) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeFar, v6) < 0.0f) ? 1 : 0;
	out += (Plane3Fn::getDistanceToPoint(f.planeFar, v7) < 0.0f) ? 1 : 0;
	if (out == 8)
	{
		return false;
	}

	// If we are here, it is because either the box intersects or it is contained in the frustum
	return true;
}

} // namespace Rio

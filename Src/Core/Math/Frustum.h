#pragma once

#include "Core/Math/Aabb.h"
#include "Core/Math/Intersection.h"
#include "Core/Math/Plane3.h"
#include "Core/Math/Types.h"

namespace Rio
{

namespace FrustumFn
{
	// Creates the frustum <f> from the view matrix <m>
	void createFromMatrix(Frustum& f, const Matrix4x4& m);

	// Returns whether the frustum <f> contains the point <p>
	bool containsPoint(const Frustum& f, const Vector3& p);

	// Returns the corner <index> of the frustum <f>
	// Index to corner table:
	// 0 -- near bottom left
	// 1 -- near bottom right
	// 2 -- near top right
	// 3 -- near top left
	// 4 -- far bottom left
	// 5 -- far bottom right
	// 6 -- far top right
	// 7 -- far top left
	Vector3 getVertexByIndex(const Frustum& f, uint32_t index);

	// Returns the Aabb enclosing the frustum <f>
	Aabb getEnclosingAabb(const Frustum& f);

} // namespace FrustumFn

namespace FrustumFn
{
	inline void createFromMatrix(Frustum& f, const Matrix4x4& m)
	{
		f.planeLeft.n.x = m.x.w + m.x.x;
		f.planeLeft.n.y = m.y.w + m.y.x;
		f.planeLeft.n.z = m.z.w + m.z.x;
		f.planeLeft.d = m.t.w + m.t.x;

		f.planeRight.n.x = m.x.w - m.x.x;
		f.planeRight.n.y = m.y.w - m.y.x;
		f.planeRight.n.z = m.z.w - m.z.x;
		f.planeRight.d = m.t.w - m.t.x;

		f.planeBottom.n.x = m.x.w + m.x.y;
		f.planeBottom.n.y = m.y.w + m.y.y;
		f.planeBottom.n.z = m.z.w + m.z.y;
		f.planeBottom.d = m.t.w + m.t.y;

		f.planeTop.n.x = m.x.w - m.x.y;
		f.planeTop.n.y = m.y.w - m.y.y;
		f.planeTop.n.z = m.z.w - m.z.y;
		f.planeTop.d = m.t.w - m.t.y;

		f.planeNear.n.x = m.x.z;
		f.planeNear.n.y = m.y.z;
		f.planeNear.n.z = m.z.z;
		f.planeNear.d = m.t.z;

		f.planeFar.n.x = m.x.w - m.x.z;
		f.planeFar.n.y = m.y.w - m.y.z;
		f.planeFar.n.z = m.z.w - m.z.z;
		f.planeFar.d = m.t.w - m.t.z;

		Plane3Fn::getNormalized(f.planeLeft);
		Plane3Fn::getNormalized(f.planeRight);
		Plane3Fn::getNormalized(f.planeBottom);
		Plane3Fn::getNormalized(f.planeTop);
		Plane3Fn::getNormalized(f.planeNear);
		Plane3Fn::getNormalized(f.planeFar);
	}

	inline bool containsPoint(const Frustum& f, const Vector3& p)
	{
		return !(Plane3Fn::getDistanceToPoint(f.planeLeft, p) < 0.0f
			|| Plane3Fn::getDistanceToPoint(f.planeRight, p) < 0.0f
			|| Plane3Fn::getDistanceToPoint(f.planeBottom, p) < 0.0f
			|| Plane3Fn::getDistanceToPoint(f.planeTop, p) < 0.0f
			|| Plane3Fn::getDistanceToPoint(f.planeNear, p) < 0.0f
			|| Plane3Fn::getDistanceToPoint(f.planeFar, p) < 0.0f
			);
	}

	inline Aabb getEnclosingAabb(const Frustum& f)
	{
		Vector3 vertices[8];

		vertices[0] = getVertexByIndex(f, 0);
		vertices[1] = getVertexByIndex(f, 1);
		vertices[2] = getVertexByIndex(f, 2);
		vertices[3] = getVertexByIndex(f, 3);
		vertices[4] = getVertexByIndex(f, 4);
		vertices[5] = getVertexByIndex(f, 5);
		vertices[6] = getVertexByIndex(f, 6);
		vertices[7] = getVertexByIndex(f, 7);

		Aabb r;

		AabbFn::reset(r);
		AabbFn::addPoints(r, 8, vertices);

		return r;
	}

} // namespace FrustumFn

} // namespace Rio

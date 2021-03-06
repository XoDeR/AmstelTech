#pragma once

#include "Core/Math/Types.h"

namespace Rio
{

// Returns the distance along ray <from> <direction> to intersection point with plane <p> or -1.0 if no intersection
float getRayPlaneIntersection(const Vector3& from, const Vector3& direction, const Plane3& p);

// Returns the distance along ray <from> <direction> to intersection point with disc 
// defined by <center>, <radius> and <normal> or -1.0 if there is no intersection
float getRayDiscIntersection(const Vector3& from, const Vector3& direction, const Vector3& center, float radius, const Vector3& normal);

// Returns the distance along ray <from> <direction> to intersection point with sphere <s> or -1.0 if no intersection
float getRaySphereIntersection(const Vector3& from, const Vector3& direction, const Sphere& s);

// Returns the distance along ray <from> <direction> to intersection point with the oriented bounding box <tm> <halfExtents> or -1.0 if no intersection
float getRayObbIntersection(const Vector3& from, const Vector3& direction, const Matrix4x4& tm, const Vector3& halfExtents);

// Returns the distance along ray <from> <direction> to intersection point with the triangle <v0> <v1> <v2> or -1.0 if no intersection
float getRayTriangleIntersection(const Vector3& from, const Vector3& direction, const Vector3& v0, const Vector3& v1, const Vector3& v2);

// Returns the distance along ray <from> <direction> 
// to intersection point with the triangle mesh defined by <vertices>, <stride>, <indices>, <count> or -1.0 if no intersection
float getRayMeshIntersection(const Vector3& from, const Vector3& direction, const Matrix4x4& tm, const void* vertices, uint32_t stride, const uint16_t* indices, uint32_t count);

// Returns whether the planes <a>, <b> and <c> intersects and if so fills <intersectionPoint> with the intersection point
bool getPlanesIntersection(const Plane3& a, const Plane3& b, const Plane3& c, Vector3& intersectionPoint);

// Returns whether the frustum <f> and the sphere <s> intersects
bool getFrustumSphereIntersection(const Frustum& f, const Sphere& s);

// Returns whether the frustum <f> and the Aabb <b> intersects
bool getFrustumBoxIntersection(const Frustum& f, const Aabb& b);

} // namespace Rio

#pragma once

#include "Core/Math/Math.h"
#include "Core/Math/Types.h"

namespace Rio
{

// Returns a new vector from individual elements
inline Vector3 createVector3(float x, float y, float z)
{
	Vector3 v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

// Adds the vector <a> to <b> and returns the result
inline Vector3& operator+=(Vector3& a, const Vector3& b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	return a;
}

// Subtracts the vector <b> from <a> and returns the result
inline Vector3& operator-=(Vector3& a, const Vector3& b)
{
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	return a;
}

// Multiplies the vector <a> by the scalar <k> and returns the result
inline Vector3& operator*=(Vector3& a, float k)
{
	a.x *= k;
	a.y *= k;
	a.z *= k;
	return a;
}

// Negates <a> and returns the result
inline Vector3 operator-(const Vector3& a)
{
	Vector3 v;
	v.x = -a.x;
	v.y = -a.y;
	v.z = -a.z;
	return v;
}

// Adds the vector <a> to <b> and returns the result
inline Vector3 operator+(Vector3 a, const Vector3& b)
{
	a += b;
	return a;
}

// Subtracts the vector <b> from <a> and returns the result
inline Vector3 operator-(Vector3 a, const Vector3& b)
{
	a -= b;
	return a;
}

// Multiplies the vector <a> by the scalar <k> and returns the result
inline Vector3 operator*(Vector3 a, float k)
{
	a *= k;
	return a;
}

// Multiplies the vector <a> by the scalar <k> and returns the result
inline Vector3 operator*(float k, Vector3 a)
{
	a *= k;
	return a;
}

// Returns true whether the vectors <a> and <b> are equal
inline bool operator==(const Vector3& a, const Vector3& b)
{
	return getAreEqualFloat(a.x, b.x) && getAreEqualFloat(a.y, b.y) && getAreEqualFloat(a.z, b.z);
}

// Returns the dot product between the vectors <a> and <b>
inline float getDotProduct(const Vector3& a, const Vector3& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Returns the cross product between the vectors <a> and <b>
inline Vector3 getCrossProduct(const Vector3& a, const Vector3& b)
{
	Vector3 v;
	v.x = a.y * b.z - a.z * b.y;
	v.y = a.z * b.x - a.x * b.z;
	v.z = a.x * b.y - a.y * b.x;
	return v;
}

// Returns the squared length of <a>
inline float getLengthSquared(const Vector3& a)
{
	return getDotProduct(a, a);
}

// Returns the length of <a>
inline float getLength(const Vector3& a)
{
	return getSqrtFloat(getLengthSquared(a));
}

// Normalizes <a> and returns the result
inline Vector3 getNormalized(Vector3& a)
{
	const float len = getLength(a);
	const float invertedLength = 1.0f / len;
	a.x *= invertedLength;
	a.y *= invertedLength;
	a.z *= invertedLength;
	return a;
}

// Sets the length of <a> to <length>
inline void setLength(Vector3& a, float length)
{
	getNormalized(a);
	a.x *= length;
	a.y *= length;
	a.z *= length;
}

// Returns the squared distance between the points <a> and <b>
inline float getDistanceSquared(const Vector3& a, const Vector3& b)
{
	return getLengthSquared(b - a);
}

// Returns the distance between the points <a> and <b>
inline float getDistance(const Vector3& a, const Vector3& b)
{
	return getLength(b - a);
}

// Returns the angle between the vectors <a> and <b>
inline float getAngle(const Vector3& a, const Vector3& b)
{
	return getArcCosFloat(getDotProduct(a, b) / (getLength(a) * getLength(b)));
}

// Returns a vector that contains the largest value for each element from <a> and <b>
inline Vector3 getMax(const Vector3& a, const Vector3& b)
{
	Vector3 v;
	v.x = getMaxFloat(a.x, b.x);
	v.y = getMaxFloat(a.y, b.y);
	v.z = getMaxFloat(a.z, b.z);
	return v;
}

// Returns a vector that contains the smallest value for each element from <a> and <b>
inline Vector3 getMin(const Vector3& a, const Vector3& b)
{
	Vector3 v;
	v.x = getMinFloat(a.x, b.x);
	v.y = getMinFloat(a.y, b.y);
	v.z = getMinFloat(a.z, b.z);
	return v;
}

// Returns the linearly interpolated vector between <a> and <b> at time <t> in [0.0f, 1.0f]
inline Vector3 getLinearlyInterpolated(const Vector3& a, const Vector3& b, float t)
{
	Vector3 v;
	v.x = getLinearlyInterpolated(a.x, b.x, t);
	v.y = getLinearlyInterpolated(a.y, b.y, t);
	v.z = getLinearlyInterpolated(a.z, b.z, t);
	return v;
}

// Returns the pointer to the data of <a>
inline float* getFloatPtr(Vector3& a)
{
	return &a.x;
}

// Returns the pointer to the data of <a>
inline const float* getFloatPtr(const Vector3& a)
{
	return &a.x;
}

// Returns the Vector2 portion of <a> by truncating z
inline Vector2 getVector2Truncated(const Vector3& a)
{
	Vector2 v;
	v.x = a.x;
	v.y = a.y;
	return v;
}

} // namespace Rio

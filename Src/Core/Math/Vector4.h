#pragma once

#include "Core/Math/Math.h"
#include "Core/Math/Types.h"

namespace Rio
{

// Returns a new vector from individual elements
inline Vector4 createVector4(float x, float y, float z, float w)
{
	Vector4 v;
	v.x = x;
	v.y = y;
	v.z = z;
	v.w = w;
	return v;
}

// Adds the vector <a> to <b> and returns the result
inline Vector4& operator+=(Vector4& a, const Vector4& b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	a.w += b.w;
	return a;
}

// Subtracts the vector <b> from <a> and returns the result
inline Vector4& operator-=(Vector4& a, const Vector4& b)
{
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	a.w -= b.w;
	return a;
}

// Multiplies the vector <a> by the scalar <k> and returns the result
inline Vector4& operator*=(Vector4& a, float k)
{
	a.x *= k;
	a.y *= k;
	a.z *= k;
	a.w *= k;
	return a;
}

// Negates <a> and returns the result
inline Vector4 operator-(const Vector4& a)
{
	Vector4 v;
	v.x = -a.x;
	v.y = -a.y;
	v.z = -a.z;
	v.w = -a.w;
	return v;
}

// Adds the vector <a> to <b> and returns the result
inline Vector4 operator+(Vector4 a, const Vector4& b)
{
	a += b;
	return a;
}

// Subtracts the vector <b> from <a> and returns the result
inline Vector4 operator-(Vector4 a, const Vector4& b)
{
	a -= b;
	return a;
}

// Multiplies the vector <a> by the scalar <k> and returns the result
inline Vector4 operator*(Vector4 a, float k)
{
	a *= k;
	return a;
}

// Multiplies the vector <a> by the scalar <k> and returns the result
inline Vector4 operator*(float k, Vector4 a)
{
	a *= k;
	return a;
}

// Returns true whether the vectors <a> and <b> are equal
inline bool operator==(const Vector4& a, const Vector4& b)
{
	return getAreEqualFloat(a.x, b.x)
		&& getAreEqualFloat(a.y, b.y)
		&& getAreEqualFloat(a.z, b.z)
		&& getAreEqualFloat(a.w, b.w)
		;
}

// Returns the dot product between the vectors <a> and <b>
inline float getDotProduct(const Vector4& a, const Vector4& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

// Returns the squared length of <a>
inline float getLengthSquared(const Vector4& a)
{
	return getDotProduct(a, a);
}

// Returns the length of <a>
inline float getLength(const Vector4& a)
{
	return getSqrtFloat(getLengthSquared(a));
}

// Normalizes <a> and returns the result
inline Vector4 getNormalized(Vector4& a)
{
	const float length = getLength(a);
	const float invertedLength = 1.0f / length;
	a.x *= invertedLength;
	a.y *= invertedLength;
	a.z *= invertedLength;
	a.w *= invertedLength;
	return a;
}

// Sets the length of <a> to <length>
inline void setLength(Vector4& a, float length)
{
	getNormalized(a);
	a.x *= length;
	a.y *= length;
	a.z *= length;
	a.w *= length;
}

// Returns the squared distance between the points <a> and <b>
inline float getDistanceSquared(const Vector4& a, const Vector4& b)
{
	return getLengthSquared(b - a);
}

// Returns the distance between the points <a> and <b>
inline float getDistance(const Vector4& a, const Vector4& b)
{
	return getLength(b - a);
}

// Returns the angle between the vectors <a> and <b>
inline float getAngle(const Vector4& a, const Vector4& b)
{
	return getArcCosFloat(getDotProduct(a, b) / (getLength(a) * getLength(b)));
}

// Returns a vector that contains the largest value for each element from <a> and <b>
inline Vector4 getMax(const Vector4& a, const Vector4& b)
{
	Vector4 v;
	v.x = getMaxFloat(a.x, b.x);
	v.y = getMaxFloat(a.y, b.y);
	v.z = getMaxFloat(a.z, b.z);
	v.w = getMaxFloat(a.w, b.w);
	return v;
}

// Returns a vector that contains the smallest value for each element from <a> and <b>
inline Vector4 getMin(const Vector4& a, const Vector4& b)
{
	Vector4 v;
	v.x = getMinFloat(a.x, b.x);
	v.y = getMinFloat(a.y, b.y);
	v.z = getMinFloat(a.z, b.z);
	v.w = getMinFloat(a.w, b.w);
	return v;
}

// Returns the linearly interpolated vector between <a> and <b> at time <t> in [0.0f, 1.0f]
inline Vector4 getLinearlyInterpolated(const Vector4& a, const Vector4& b, float t)
{
	Vector4 v;
	v.x = getLinearlyInterpolated(a.x, b.x, t);
	v.y = getLinearlyInterpolated(a.y, b.y, t);
	v.z = getLinearlyInterpolated(a.z, b.z, t);
	v.w = getLinearlyInterpolated(a.w, b.w, t);
	return v;
}

// Returns the pointer to the data of <a>
inline float* getFloatPtr(Vector4& a)
{
	return &a.x;
}

// Returns the pointer to the data of <a>
inline const float* getFloatPtr(const Vector4& a)
{
	return &a.x;
}

// Returns the Vector3 portion of <a> by truncating w
inline Vector3 getVector3Truncated(const Vector4& a)
{
	Vector3 v;
	v.x = a.x;
	v.y = a.y;
	v.z = a.z;
	return v;
}

} // namespace Rio

#pragma once

#include "Core/Math/Math.h"
#include "Core/Math/Types.h"

namespace Rio
{

// Returns a new vector from individual elements
inline Vector2 createVector2(float x, float y)
{
	Vector2 v;
	v.x = x;
	v.y = y;
	return v;
}

// Adds the vector <a> to <b> and returns the result
inline Vector2& operator+=(Vector2& a, const Vector2& b)
{
	a.x += b.x;
	a.y += b.y;
	return a;
}

// Subtracts the vector <b> from <a> and returns the result
inline Vector2& operator-=(Vector2& a, const Vector2& b)
{
	a.x -= b.x;
	a.y -= b.y;
	return a;
}

// Multiplies the vector <a> by the scalar <k> and returns the result
inline Vector2& operator*=(Vector2& a, float k)
{
	a.x *= k;
	a.y *= k;
	return a;
}

// Negates <a> and returns the result
inline Vector2 operator-(const Vector2& a)
{
	Vector2 v;
	v.x = -a.x;
	v.y = -a.y;
	return v;
}

// Adds the vector <a> to <b> and returns the result
inline Vector2 operator+(Vector2 a, const Vector2& b)
{
	a += b;
	return a;
}

// Subtracts the vector <b> from <a> and returns the result
inline Vector2 operator-(Vector2 a, const Vector2& b)
{
	a -= b;
	return a;
}

// Multiplies the vector <a> by the scalar <k> and returns the result
inline Vector2 operator*(Vector2 a, float k)
{
	a *= k;
	return a;
}

// Multiplies the vector <a> by the scalar <k> and returns the result
inline Vector2 operator*(float k, Vector2 a)
{
	a *= k;
	return a;
}

// Returns true whether the vectors <a> and <b> are equal
inline bool operator==(const Vector2& a, const Vector2& b)
{
	return getAreEqualFloat(a.x, b.x) && getAreEqualFloat(a.y, b.y);
}

// Returns the dot product between the vectors <a> and <b>
inline float getDotProduct(const Vector2& a, const Vector2& b)
{
	return a.x * b.x + a.y * b.y;
}

// Returns the squared length of <a>
inline float getLengthSquared(const Vector2& a)
{
	return getDotProduct(a, a);
}

// Returns the length of <a>
inline float getLength(const Vector2& a)
{
	return getSqrtFloat(getLengthSquared(a));
}

// Normalizes <a> and returns the result
inline Vector2 getNormalized(Vector2& a)
{
	const float len = getLength(a);
	const float invertedLength = 1.0f / len;
	a.x *= invertedLength;
	a.y *= invertedLength;
	return a;
}

// Sets the length of <a> to <len>
inline void setLength(Vector2& a, float len)
{
	getNormalized(a);
	a.x *= len;
	a.y *= len;
}

// Returns the squared distance between the points <a> and <b>
inline float getDistanceSquared(const Vector2& a, const Vector2& b)
{
	return getLengthSquared(b - a);
}

// Returns the distance between the points <a> and <b>
inline float getDistance(const Vector2& a, const Vector2& b)
{
	return getLength(b - a);
}

// Returns the angle between the vectors <a> and <b>
inline float getAngle(const Vector2& a, const Vector2& b)
{
	return getArcCosFloat(getDotProduct(a, b) / (getLength(a) * getLength(b)));
}

// Returns a vector that contains the largest value for each element from <a> and <b>
inline Vector2 getMax(const Vector2& a, const Vector2& b)
{
	Vector2 v;
	v.x = getMaxFloat(a.x, b.x);
	v.y = getMaxFloat(a.y, b.y);
	return v;
}

// Returns a vector that contains the smallest value for each element from <a> and <b>
inline Vector2 getMin(const Vector2& a, const Vector2& b)
{
	Vector2 v;
	v.x = getMinFloat(a.x, b.x);
	v.y = getMinFloat(a.y, b.y);
	return v;
}

// Returns the linearly interpolated vector between <a> and <b> at time <t> in the interval [0.0f, 1.0f]
inline Vector2 getLinearlyInterpolated(const Vector2& a, const Vector2& b, float t)
{
	Vector2 v;
	v.x = getLinearlyInterpolated(a.x, b.x, t);
	v.y = getLinearlyInterpolated(a.y, b.y, t);
	return v;
}

// Returns the pointer to the data of <a>
inline float* getFloatPtr(Vector2& a)
{
	return &a.x;
}

// Returns the pointer to the data of <a>
inline const float* getFloatPtr(const Vector2& a)
{
	return &a.x;
}

} // namespace Rio

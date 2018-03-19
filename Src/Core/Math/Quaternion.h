#pragma once

#include "Core/Math/Math.h"
#include "Core/Math/Matrix3x3.h"
#include "Core/Math/Types.h"

namespace Rio
{

// Returns a new quaternion from individual elements
inline Quaternion createQuaternion(float x, float y, float z, float w)
{
	Quaternion q;

	q.x = x;
	q.y = y;
	q.z = z;
	q.w = w;

	return q;
}

// Returns a new quaternion from <axis> and <angle>
inline Quaternion createQuaternion(const Vector3& axis, float angle)
{
	const float ha = angle * 0.5f;
	const float sa = getSinFloat(ha);
	const float ca = getCosFloat(ha);

	Quaternion q;

	q.x = axis.x * sa;
	q.y = axis.y * sa;
	q.z = axis.z * sa;
	q.w = ca;

	return q;
}

// Returns a new quaternion from matrix <m>
Quaternion createQuaternion(const Matrix3x3& m);

// Multiplies the quaternions <a> by <b> and returns the result
// Rotates first by <a> then by <b>
inline Quaternion& operator*=(Quaternion& a, const Quaternion& b)
{
	const float tx = a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y;
	const float ty = a.w*b.y + a.y*b.w + a.z*b.x - a.x*b.z;
	const float tz = a.w*b.z + a.z*b.w + a.x*b.y - a.y*b.x;
	const float tw = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z;
	a.x = tx;
	a.y = ty;
	a.z = tz;
	a.w = tw;
	return a;
}

// Negates the quaternion <q> and returns the result
inline Quaternion operator-(const Quaternion& q)
{
	Quaternion r;

	r.x = -q.x;
	r.y = -q.y;
	r.z = -q.z;
	r.w = -q.w;

	return r;
}

// Multiplies the quaternions <a> by <b> and returns the result
// (Rotates first by <a> then by <b>)
inline Quaternion operator*(Quaternion a, const Quaternion& b)
{
	a *= b;
	return a;
}

// Multiplies the quaternion <a> by the scalar <k>
inline Quaternion operator*(const Quaternion& q, float k)
{
	Quaternion r;

	r.x = q.x * k;
	r.y = q.y * k;
	r.z = q.z * k;
	r.w = q.w * k;

	return r;
}

// Returns the dot product between quaternions <a> and <b>
inline float getDotProduct(const Quaternion& a, const Quaternion& b)
{
	return a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;
}

// Returns the length of <q>
inline float getLength(const Quaternion& q)
{
	return getSqrtFloat(getDotProduct(q, q));
}

// Normalizes the quaternion <q> and returns the result
inline Quaternion& getNormalized(Quaternion& q)
{
	const float length = getLength(q);
	const float invertedLength = 1.0f / length;
	q.x *= invertedLength;
	q.y *= invertedLength;
	q.z *= invertedLength;
	q.w *= invertedLength;
	return q;
}

// Returns the conjugate of quaternion <q>
inline Quaternion getConjugateCopy(const Quaternion& q)
{
	Quaternion r;

	r.x = -q.x;
	r.y = -q.y;
	r.z = -q.z;
	r.w = q.w;

	return r;
}

// Returns the inverse of quaternion <q>
inline Quaternion getInvertedCopy(const Quaternion& q)
{
	const float length = getLength(q);
	const float invertedLength = 1.0f / length;
	return getConjugateCopy(q) * invertedLength;
}

// Returns the quaternion <q> raised to the power of <exp>
inline Quaternion getPowerOf(const Quaternion& q, float exp)
{
	if (getAbsFloat(q.w) < 0.9999)
	{
		const float alpha = getArcCosFloat(q.w); // alpha = theta/2
		const float newAlpha = alpha * exp;
		const float mult = getSinFloat(newAlpha) / getSinFloat(alpha);

		Quaternion r;

		r.w = getCosFloat(newAlpha);
		r.x = q.x * mult;
		r.y = q.y * mult;
		r.z = q.z * mult;

		return r;
	}

	return q;
}

// Returns the quaternion describing the rotation needed to face towards <direction>
inline Quaternion getToLookAt(const Vector3& direction, const Vector3& up = VECTOR3_YAXIS)
{
	const Vector3 right = getCrossProduct(direction, up);
	const Vector3 nup = getCrossProduct(right, direction);

	Matrix3x3 m;

	m.x = -right;
	m.y = nup;
	m.z = direction;

	return createQuaternion(m);
}

// Returns the right axis of the rotation represented by <q>
inline Vector3 getRightAxis(const Quaternion& q)
{
	const Matrix3x3 m = createMatrix3x3(q);
	return m.x;
}

// Returns the up axis of the rotation represented by <q>
inline Vector3 getUpAxis(const Quaternion& q)
{
	const Matrix3x3 m = createMatrix3x3(q);
	return m.y;
}

// Returns the forward axis of the rotation represented by <q>
inline Vector3 getForwardAxis(const Quaternion& q)
{
	const Matrix3x3 m = createMatrix3x3(q);
	return m.z;
}

// Returns the linearly interpolated quaternion between <a> and <b> at time <t> in [0.0f, 1.0f]
// Uses NLerp
inline Quaternion getLinearlyInterpolated(const Quaternion& a, const Quaternion& b, float t)
{
	const float t1 = 1.0f - t;

	Quaternion r;

	if (getDotProduct(a, b) < 0.0f)
	{
		r.x = t1*a.x + t*-b.x;
		r.y = t1*a.y + t*-b.y;
		r.z = t1*a.z + t*-b.z;
		r.w = t1*a.w + t*-b.w;
	}
	else
	{
		r.x = t1*a.x + t*b.x;
		r.y = t1*a.y + t*b.y;
		r.z = t1*a.z + t*b.z;
		r.w = t1*a.w + t*b.w;
	}

	return getNormalized(r);
}

} // namespace Rio

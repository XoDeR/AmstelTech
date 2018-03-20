#pragma once

#include "Core/Math/Math.h"
#include "Core/Math/Matrix3x3.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Types.h"
#include "core/math/vector4.h"

namespace Rio
{

// Returns a new matrix from individual elements
inline Matrix4x4 createMatrix4x4(float xx, float xy, float xz, float xw
	, float yx, float yy, float yz, float yw
	, float zx, float zy, float zz, float zw
	, float tx, float ty, float tz, float tw
	)
{
	Matrix4x4 m;

	m.x.x = xx;
	m.x.y = xy;
	m.x.z = xz;
	m.x.w = xw;

	m.y.x = yx;
	m.y.y = yy;
	m.y.z = yz;
	m.y.w = yw;

	m.z.x = zx;
	m.z.y = zy;
	m.z.z = zz;
	m.z.w = zw;

	m.t.x = tx;
	m.t.y = ty;
	m.t.z = tz;
	m.t.w = tw;

	return m;
}

// Returns a new matrix from individual elements
inline Matrix4x4 createMatrix4x4(const float a[16])
{
	Matrix4x4 m;

	m.x.x = a[0];
	m.x.y = a[1];
	m.x.z = a[2];
	m.x.w = a[3];

	m.y.x = a[4];
	m.y.y = a[5];
	m.y.z = a[6];
	m.y.w = a[7];

	m.z.x = a[8];
	m.z.y = a[9];
	m.z.z = a[10];
	m.z.w = a[11];

	m.t.x = a[12];
	m.t.y = a[13];
	m.t.z = a[14];
	m.t.w = a[15];

	return m;
}

// Returns a new matrix from axes <x>, <y> and <z> and translation <t>
inline Matrix4x4 createMatrix4x4(const Vector3& x, const Vector3& y, const Vector3& z, const Vector3& t)
{
	Matrix4x4 m;

	m.x.x = x.x;
	m.x.y = x.y;
	m.x.z = x.z;
	m.x.w = 0.0f;

	m.y.x = y.x;
	m.y.y = y.y;
	m.y.z = y.z;
	m.y.w = 0.0f;

	m.z.x = z.x;
	m.z.y = z.y;
	m.z.z = z.z;
	m.z.w = 0.0f;

	m.t.x = t.x;
	m.t.y = t.y;
	m.t.z = t.z;
	m.t.w = 1.0f;

	return m;
}

// Returns a new matrix from rotation <r> and translation <t>
inline Matrix4x4 createMatrix4x4(const Quaternion& r, const Vector3& t)
{
	Matrix4x4 m;

	m.x.x = 1.0f - 2.0f * r.y * r.y - 2.0f * r.z * r.z;
	m.x.y = 2.0f * r.x * r.y + 2.0f * r.w * r.z;
	m.x.z = 2.0f * r.x * r.z - 2.0f * r.w * r.y;
	m.x.w = 0.0f;

	m.y.x = 2.0f * r.x * r.y - 2.0f * r.w * r.z;
	m.y.y = 1.0f - 2.0f * r.x * r.x - 2.0f * r.z * r.z;
	m.y.z = 2.0f * r.y * r.z + 2.0f * r.w * r.x;
	m.y.w = 0.0f;

	m.z.x = 2.0f * r.x * r.z + 2.0f * r.w * r.y;
	m.z.y = 2.0f * r.y * r.z - 2.0f * r.w * r.x;
	m.z.z = 1.0f - 2.0f * r.x * r.x - 2.0f * r.y * r.y;
	m.z.w = 0.0f;

	m.t.x = t.x;
	m.t.y = t.y;
	m.t.z = t.z;
	m.t.w = 1.0f;

	return m;
}

// Returns a new matrix from translation <t>
inline Matrix4x4 createMatrix4x4(const Vector3& t)
{
	Matrix4x4 m;

	m.x.x = 1.0f;
	m.x.y = 0.0f;
	m.x.z = 0.0f;
	m.x.w = 0.0f;

	m.y.x = 0.0f;
	m.y.y = 1.0f;
	m.y.z = 0.0f;
	m.y.w = 0.0f;

	m.z.x = 0.0f;
	m.z.y = 0.0f;
	m.z.z = 1.0f;
	m.z.w = 0.0f;

	m.t.x = t.x;
	m.t.y = t.y;
	m.t.z = t.z;
	m.t.w = 1.0f;

	return m;
}

// Returns a new matrix from rotation matrix <r>
inline Matrix4x4 createMatrix4x4(const Matrix3x3& r)
{
	Matrix4x4 m;

	m.x.x = r.x.x;
	m.x.y = r.x.y;
	m.x.z = r.x.z;
	m.x.w = 0.0f;

	m.y.x = r.y.x;
	m.y.y = r.y.y;
	m.y.z = r.y.z;
	m.y.w = 0.0f;

	m.z.x = r.z.x;
	m.z.y = r.z.y;
	m.z.z = r.z.z;
	m.z.w = 0.0f;

	m.t.x = 0.0f;
	m.t.y = 0.0f;
	m.t.z = 0.0f;
	m.t.w = 1.0f;

	return m;
}

// Adds the matrix <a> to <b> and returns the result
inline Matrix4x4& operator+=(Matrix4x4& a, const Matrix4x4& b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	a.t += b.t;

	return a;
}

// Subtracts the matrix <b> from <a> and returns the result
inline Matrix4x4& operator-=(Matrix4x4& a, const Matrix4x4& b)
{
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	a.t -= b.t;

	return a;
}

// Multiplies the matrix <a> by the scalar <k> and returns the result
inline Matrix4x4& operator*=(Matrix4x4& a, float k)
{
	a.x *= k;
	a.y *= k;
	a.z *= k;
	a.t *= k;

	return a;
}

// Multiplies the matrix <a> by <b> and returns the result
// (Transforms first by <a> then by <b>)
inline Matrix4x4& operator*=(Matrix4x4& a, const Matrix4x4& b)
{
	Matrix4x4 tmp;

	tmp.x.x = a.x.x*b.x.x + a.x.y*b.y.x + a.x.z*b.z.x + a.x.w*b.t.x;
	tmp.x.y = a.x.x*b.x.y + a.x.y*b.y.y + a.x.z*b.z.y + a.x.w*b.t.y;
	tmp.x.z = a.x.x*b.x.z + a.x.y*b.y.z + a.x.z*b.z.z + a.x.w*b.t.z;
	tmp.x.w = a.x.x*b.x.w + a.x.y*b.y.w + a.x.z*b.z.w + a.x.w*b.t.w;

	tmp.y.x = a.y.x*b.x.x + a.y.y*b.y.x + a.y.z*b.z.x + a.y.w*b.t.x;
	tmp.y.y = a.y.x*b.x.y + a.y.y*b.y.y + a.y.z*b.z.y + a.y.w*b.t.y;
	tmp.y.z = a.y.x*b.x.z + a.y.y*b.y.z + a.y.z*b.z.z + a.y.w*b.t.z;
	tmp.y.w = a.y.x*b.x.w + a.y.y*b.y.w + a.y.z*b.z.w + a.y.w*b.t.w;

	tmp.z.x = a.z.x*b.x.x + a.z.y*b.y.x + a.z.z*b.z.x + a.z.w*b.t.x;
	tmp.z.y = a.z.x*b.x.y + a.z.y*b.y.y + a.z.z*b.z.y + a.z.w*b.t.y;
	tmp.z.z = a.z.x*b.x.z + a.z.y*b.y.z + a.z.z*b.z.z + a.z.w*b.t.z;
	tmp.z.w = a.z.x*b.x.w + a.z.y*b.y.w + a.z.z*b.z.w + a.z.w*b.t.w;

	tmp.t.x = a.t.x*b.x.x + a.t.y*b.y.x + a.t.z*b.z.x + a.t.w*b.t.x;
	tmp.t.y = a.t.x*b.x.y + a.t.y*b.y.y + a.t.z*b.z.y + a.t.w*b.t.y;
	tmp.t.z = a.t.x*b.x.z + a.t.y*b.y.z + a.t.z*b.z.z + a.t.w*b.t.z;
	tmp.t.w = a.t.x*b.x.w + a.t.y*b.y.w + a.t.z*b.z.w + a.t.w*b.t.w;

	a = tmp;
	return a;
}

// Adds the matrix <a> to <b> and returns the result
inline Matrix4x4 operator+(Matrix4x4 a, const Matrix4x4& b)
{
	a += b;
	return a;
}

// Subtracts the matrix <b> from <a> and returns the result
inline Matrix4x4 operator-(Matrix4x4 a, const Matrix4x4& b)
{
	a -= b;
	return a;
}

// Multiplies the matrix <a> by the scalar <k> and returns the result
inline Matrix4x4 operator*(Matrix4x4 a, float k)
{
	a *= k;
	return a;
}

// Multiplies the matrix <a> by the scalar <k> and returns the result
inline Matrix4x4 operator*(float k, Matrix4x4 a)
{
	a *= k;
	return a;
}

// Multiplies the matrix <a> by the vector <v> and returns the result
inline Vector3 operator*(const Vector3& v, const Matrix4x4& a)
{
	Vector3 r;

	r.x = v.x*a.x.x + v.y*a.y.x + v.z*a.z.x + a.t.x;
	r.y = v.x*a.x.y + v.y*a.y.y + v.z*a.z.y + a.t.y;
	r.z = v.x*a.x.z + v.y*a.y.z + v.z*a.z.z + a.t.z;

	return r;
}

// Multiplies the matrix <a> by the vector <v> and returns the result
inline Vector4 operator*(const Vector4& v, const Matrix4x4& a)
{
	Vector4 r;

	r.x = v.x*a.x.x + v.y*a.y.x + v.z*a.z.x + v.w*a.t.x;
	r.y = v.x*a.x.y + v.y*a.y.y + v.z*a.z.y + v.w*a.t.y;
	r.z = v.x*a.x.z + v.y*a.y.z + v.z*a.z.z + v.w*a.t.z;
	r.w = v.x*a.x.w + v.y*a.y.w + v.z*a.z.w + v.w*a.t.w;

	return r;
}

// Multiplies the matrix <a> by <b> and returns the result
// (Transforms first by <a> then by <b>)
inline Matrix4x4 operator*(Matrix4x4 a, const Matrix4x4& b)
{
	a *= b;
	return a;
}

// Sets the matrix <m> to perspective
inline void setToPerspective(Matrix4x4& m, float fovY, float aspect, float nearValue, float farValue)
{
	const float height = 1.0f / getTanFloat(fovY * 0.5f);
	const float width = height * 1.0f / aspect;
	const float aa = farValue / (farValue - nearValue);
	const float bb = -nearValue * aa;

	m.x.x = width;
	m.x.y = 0.0f;
	m.x.z = 0.0f;
	m.x.w = 0.0f;

	m.y.x = 0.0f;
	m.y.y = height;
	m.y.z = 0.0f;
	m.y.w = 0.0f;

	m.z.x = 0.0f;
	m.z.y = 0.0f;
	m.z.z = aa;
	m.z.w = 1.0f;

	m.t.x = 0.0f;
	m.t.y = 0.0f;
	m.t.z = bb;
	m.t.w = 0.0f;
}

// Sets the matrix <m> to orthographic
inline void setToOrthographic(Matrix4x4& m, float left, float right, float bottom, float top, float nearValue, float farValue)
{
	m.x.x = 2.0f / (right - left);
	m.x.y = 0.0f;
	m.x.z = 0.0f;
	m.x.w = 0.0f;

	m.y.x = 0.0f;
	m.y.y = 2.0f / (top - bottom);
	m.y.z = 0.0f;
	m.y.w = 0.0f;

	m.z.x = 0.0f;
	m.z.y = 0.0f;
	m.z.z = 1.0f / (farValue - nearValue);
	m.z.w = 0.0f;

	m.t.x = (left + right) / (left - right);
	m.t.y = (top + bottom) / (bottom - top);
	m.t.z = nearValue / (nearValue - farValue);
	m.t.w = 1.0f;
}

// Transposes the matrix <m> and returns the result
inline Matrix4x4& getTransposed(Matrix4x4& m)
{
	float tmp;

	tmp = m.x.y;
	m.x.y = m.y.x;
	m.y.x = tmp;

	tmp = m.x.z;
	m.x.z = m.z.x;
	m.z.x = tmp;

	tmp = m.x.w;
	m.x.w = m.t.x;
	m.t.x = tmp;

	tmp = m.y.z;
	m.y.z = m.z.y;
	m.z.y = tmp;

	tmp = m.y.w;
	m.y.w = m.t.y;
	m.t.y = tmp;

	tmp = m.z.w;
	m.z.w = m.t.z;
	m.t.z = tmp;

	return m;
}

// Returns the transposed of the matrix <m>
inline Matrix4x4 getTransposedCopy(Matrix4x4 m)
{
	getTransposed(m);
	return m;
}

// Sets the matrix <m> to look
inline void setToLook(Matrix4x4& m, const Vector3& position, const Vector3& target, const Vector3& up)
{
	Vector3 zAxis = position - target;
	getNormalized(zAxis);
	const Vector3 xAxis = getCrossProduct(up, zAxis);
	const Vector3 yAxis = getCrossProduct(zAxis, xAxis);

	m.x.x = xAxis.x;
	m.x.y = yAxis.x;
	m.x.z = zAxis.x;
	m.x.w = 0.0f;

	m.y.x = xAxis.y;
	m.y.y = yAxis.y;
	m.y.z = zAxis.y;
	m.y.w = 0.0f;

	m.z.x = xAxis.z;
	m.z.y = yAxis.z;
	m.z.z = zAxis.z;
	m.z.w = 0.0f;

	m.t.x = -getDotProduct(position, xAxis);
	m.t.y = -getDotProduct(position, yAxis);
	m.t.z = -getDotProduct(position, zAxis);
	m.t.w = 1.0f;
}

// Inverts the matrix <m> and returns the result
Matrix4x4& getInverted(Matrix4x4& m);

// Returns the inverse of the matrix <m>
inline Matrix4x4 getInvertedCopy(Matrix4x4 m)
{
	getInverted(m);
	return m;
}

// Sets the matrix <m> to identity
inline void setToIdentity(Matrix4x4& m)
{
	m.x.x = 1.0f;
	m.x.y = 0.0f;
	m.x.z = 0.0f;
	m.x.w = 0.0f;

	m.y.x = 0.0f;
	m.y.y = 1.0f;
	m.y.z = 0.0f;
	m.y.w = 0.0f;

	m.z.x = 0.0f;
	m.z.y = 0.0f;
	m.z.z = 1.0f;
	m.z.w = 0.0f;

	m.t.x = 0.0f;
	m.t.y = 0.0f;
	m.t.z = 0.0f;
	m.t.w = 1.0f;
}

// Returns the x axis of the matrix <m>
inline Vector3 getXAxis(const Matrix4x4& m)
{
	Vector3 v;

	v.x = m.x.x;
	v.y = m.x.y;
	v.z = m.x.z;

	return v;
}

// Returns the y axis of the matrix <m>
inline Vector3 getYAxis(const Matrix4x4& m)
{
	Vector3 v;

	v.x = m.y.x;
	v.y = m.y.y;
	v.z = m.y.z;
	
	return v;
}

// Returns the z axis of the matrix <m>
inline Vector3 getZAxis(const Matrix4x4& m)
{
	Vector3 v;
	
	v.x = m.z.x;
	v.y = m.z.y;
	v.z = m.z.z;
	
	return v;
}

// Sets the x axis of the matrix <m>
inline void setXAxis(Matrix4x4& m, const Vector3& x)
{
	m.x.x = x.x;
	m.x.y = x.y;
	m.x.z = x.z;
}

// Sets the y axis of the matrix <m>
inline void setYAxis(Matrix4x4& m, const Vector3& y)
{
	m.y.x = y.x;
	m.y.y = y.y;
	m.y.z = y.z;
}

// Sets the z axis of the matrix <m>
inline void setZAxis(Matrix4x4& m, const Vector3& z)
{
	m.z.x = z.x;
	m.z.y = z.y;
	m.z.z = z.z;
}

// Returns the translation portion of the matrix <m>
inline Vector3 getTranslation(const Matrix4x4& m)
{
	Vector3 v;

	v.x = m.t.x;
	v.y = m.t.y;
	v.z = m.t.z;
	
	return v;
}

// Sets the translation portion of the matrix <m>
inline void setTranslation(Matrix4x4& m, const Vector3& translation)
{
	m.t.x = translation.x;
	m.t.y = translation.y;
	m.t.z = translation.z;
}

// Returns the rotation portion of the matrix <m> as a Matrix3x3
inline Matrix3x3 getRotationMatrix3x3Truncated(const Matrix4x4& m)
{
	Matrix3x3 result;

	result.x.x = m.x.x;
	result.x.y = m.x.y;
	result.x.z = m.x.z;

	result.y.x = m.y.x;
	result.y.y = m.y.y;
	result.y.z = m.y.z;

	result.z.x = m.z.x;
	result.z.y = m.z.y;
	result.z.z = m.z.z;

	return result;
}

// Returns the rotation portion of the matrix <m> as a Quaternion
inline Quaternion getRotationQuaternion(const Matrix4x4& m)
{
	return createQuaternion(getRotationMatrix3x3Truncated(m));
}

// Sets the rotation portion of the matrix <m>
inline void setRotation(Matrix4x4& m, const Matrix3x3& rot)
{
	m.x.x = rot.x.x;
	m.x.y = rot.x.y;
	m.x.z = rot.x.z;

	m.y.x = rot.y.x;
	m.y.y = rot.y.y;
	m.y.z = rot.y.z;

	m.z.x = rot.z.x;
	m.z.y = rot.z.y;
	m.z.z = rot.z.z;
}

// Sets the rotation portion of the matrix <m>
inline void setRotation(Matrix4x4& m, const Quaternion& rotation)
{
	setRotation(m, createMatrix3x3(rotation));
}

// Returns the scale of the matrix <m>
inline Vector3 getScale(const Matrix4x4& m)
{
	const float sx = getLength(getVector3Truncated(m.x));
	const float sy = getLength(getVector3Truncated(m.y));
	const float sz = getLength(getVector3Truncated(m.z));

	Vector3 v;

	v.x = sx;
	v.y = sy;
	v.z = sz;

	return v;
}

// Sets the scale of the matrix <m>
inline void setScale(Matrix4x4& m, const Vector3& s)
{
	Matrix3x3 rot = getRotationMatrix3x3Truncated(m);
	setScale(rot, s);
	setRotation(m, rot);
}

// Returns the pointer to the matrix's data
inline float* getFloatPtr(Matrix4x4& m)
{
	return getFloatPtr(m.x);
}

// Returns the pointer to the matrix's data
inline const float* getFloatPtr(const Matrix4x4& m)
{
	return getFloatPtr(m.x);
}

} // namespace Rio

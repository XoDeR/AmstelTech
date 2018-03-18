#include "Core/Math/Matrix3x3.h"

namespace Rio
{

Matrix3x3& getInverted(Matrix3x3& m)
{
	const float xx = m.x.x;
	const float xy = m.x.y;
	const float xz = m.x.z;
	const float yx = m.y.x;
	const float yy = m.y.y;
	const float yz = m.y.z;
	const float zx = m.z.x;
	const float zy = m.z.y;
	const float zz = m.z.z;

	float det = 0.0f;
	det += m.x.x * (m.y.y * m.z.z - m.z.y * m.y.z);
	det -= m.y.x * (m.x.y * m.z.z - m.z.y * m.x.z);
	det += m.z.x * (m.x.y * m.y.z - m.y.y * m.x.z);

	const float invertedDeterminant = 1.0f / det;

	m.x.x = + (yy*zz - zy*yz) * invertedDeterminant;
	m.x.y = - (xy*zz - zy*xz) * invertedDeterminant;
	m.x.z = + (xy*yz - yy*xz) * invertedDeterminant;

	m.y.x = - (yx*zz - zx*yz) * invertedDeterminant;
	m.y.y = + (xx*zz - zx*xz) * invertedDeterminant;
	m.y.z = - (xx*yz - yx*xz) * invertedDeterminant;

	m.z.x = + (yx*zy - zx*yy) * invertedDeterminant;
	m.z.y = - (xx*zy - zx*xy) * invertedDeterminant;
	m.z.z = + (xx*yy - yx*xy) * invertedDeterminant;

	return m;
}

} // namespace Rio

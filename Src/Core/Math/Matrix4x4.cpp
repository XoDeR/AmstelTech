#include "Core/Math/Matrix4x4.h"

namespace Rio
{

Matrix4x4& getInverted(Matrix4x4& m)
{
	const float xx = m.x.x;
	const float xy = m.x.y;
	const float xz = m.x.z;
	const float xw = m.x.w;
	const float yx = m.y.x;
	const float yy = m.y.y;
	const float yz = m.y.z;
	const float yw = m.y.w;
	const float zx = m.z.x;
	const float zy = m.z.y;
	const float zz = m.z.z;
	const float zw = m.z.w;
	const float tx = m.t.x;
	const float ty = m.t.y;
	const float tz = m.t.z;
	const float tw = m.t.w;

	float det = 0.0f;

	det += xx * (yy * (zz*tw - tz*zw) - zy * (yz*tw - tz*yw) + ty * (yz*zw - zz*yw));
	det -= yx * (xy * (zz*tw - tz*zw) - zy * (xz*tw - tz*xw) + ty * (xz*zw - zz*xw));
	det += zx * (xy * (yz*tw - tz*yw) - yy * (xz*tw - tz*xw) + ty * (xz*yw - yz*xw));
	det -= tx * (xy * (yz*zw - zz*yw) - yy * (xz*zw - zz*xw) + zy * (xz*yw - yz*xw));

	const float invertedDeterminant = 1.0f / det;

	m.x.x = + (yy * (zz*tw - tz*zw) - zy * (yz*tw - tz*yw) + ty * (yz*zw - zz*yw)) * invertedDeterminant;
	m.x.y = - (xy * (zz*tw - tz*zw) - zy * (xz*tw - tz*xw) + ty * (xz*zw - zz*xw)) * invertedDeterminant;
	m.x.z = + (xy * (yz*tw - tz*yw) - yy * (xz*tw - tz*xw) + ty * (xz*yw - yz*xw)) * invertedDeterminant;
	m.x.w = - (xy * (yz*zw - zz*yw) - yy * (xz*zw - zz*xw) + zy * (xz*yw - yz*xw)) * invertedDeterminant;

	m.y.x = - (yx * (zz*tw - tz*zw) - zx * (yz*tw - tz*yw) + tx * (yz*zw - zz*yw)) * invertedDeterminant;
	m.y.y = + (xx * (zz*tw - tz*zw) - zx * (xz*tw - tz*xw) + tx * (xz*zw - zz*xw)) * invertedDeterminant;
	m.y.z = - (xx * (yz*tw - tz*yw) - yx * (xz*tw - tz*xw) + tx * (xz*yw - yz*xw)) * invertedDeterminant;
	m.y.w = + (xx * (yz*zw - zz*yw) - yx * (xz*zw - zz*xw) + zx * (xz*yw - yz*xw)) * invertedDeterminant;

	m.z.x = + (yx * (zy*tw - ty*zw) - zx * (yy*tw - ty*yw) + tx * (yy*zw - zy*yw)) * invertedDeterminant;
	m.z.y = - (xx * (zy*tw - ty*zw) - zx * (xy*tw - ty*xw) + tx * (xy*zw - zy*xw)) * invertedDeterminant;
	m.z.z = + (xx * (yy*tw - ty*yw) - yx * (xy*tw - ty*xw) + tx * (xy*yw - yy*xw)) * invertedDeterminant;
	m.z.w = - (xx * (yy*zw - zy*yw) - yx * (xy*zw - zy*xw) + zx * (xy*yw - yy*xw)) * invertedDeterminant;

	m.t.x = - (yx * (zy*tz - ty*zz) - zx * (yy*tz - ty*yz) + tx * (yy*zz - zy*yz)) * invertedDeterminant;
	m.t.y = + (xx * (zy*tz - ty*zz) - zx * (xy*tz - ty*xz) + tx * (xy*zz - zy*xz)) * invertedDeterminant;
	m.t.z = - (xx * (yy*tz - ty*yz) - yx * (xy*tz - ty*xz) + tx * (xy*yz - yy*xz)) * invertedDeterminant;
	m.t.w = + (xx * (yy*zz - zy*yz) - yx * (xy*zz - zy*xz) + zx * (xy*yz - yy*xz)) * invertedDeterminant;

	return m;
}

} // namespace Rio

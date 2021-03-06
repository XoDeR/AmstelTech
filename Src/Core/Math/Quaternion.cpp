#include "Core/Error/Error.h"
#include "Core/Math/Quaternion.h"
#include "Core/Types.h"

namespace Rio
{

// Returns the rotation portion of the matrix <m> as Quaternion
Quaternion createQuaternion(const Matrix3x3& m)
{
	const float ww = m.x.x + m.y.y + m.z.z;
	const float xx = m.x.x - m.y.y - m.z.z;
	const float yy = m.y.y - m.x.x - m.z.z;
	const float zz = m.z.z - m.x.x - m.y.y;

	float max = ww;
	uint32_t index = 0;

	if (xx > max)
	{
		max = xx;
		index = 1;
	}

	if (yy > max)
	{
		max = yy;
		index = 2;
	}

	if (zz > max)
	{
		max = zz;
		index = 3;
	}

	const float biggest = getSqrtFloat(max + 1.0f) * 0.5f;
	const float mult = 0.25f / biggest;

	Quaternion tmp;
	switch (index)
	{
	case 0:
		tmp.w = biggest;
		tmp.x = (m.y.z - m.z.y) * mult;
		tmp.y = (m.z.x - m.x.z) * mult;
		tmp.z = (m.x.y - m.y.x) * mult;
		break;

	case 1:
		tmp.x = biggest;
		tmp.w = (m.y.z - m.z.y) * mult;
		tmp.y = (m.x.y + m.y.x) * mult;
		tmp.z = (m.z.x + m.x.z) * mult;
		break;

	case 2:
		tmp.y = biggest;
		tmp.w = (m.z.x - m.x.z) * mult;
		tmp.x = (m.x.y + m.y.x) * mult;
		tmp.z = (m.y.z + m.z.y) * mult;
		break;

	case 3:
		tmp.z = biggest;
		tmp.w = (m.x.y - m.y.x) * mult;
		tmp.x = (m.z.x + m.x.z) * mult;
		tmp.y = (m.y.z + m.z.y) * mult;
		break;

	default:
		RIO_FATAL("Fatal");
		break;
	}

	getNormalized(tmp);
	return tmp;
}

} // namespace Rio

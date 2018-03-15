#include "Core/Math/Math.h"
#include "Core/Types.h"

#include <math.h>

namespace Rio
{

bool getAreEqualFloat(float a, float b, float epsilon)
{
	return b <= (a + epsilon)
		&& b >= (a - epsilon)
		;
}

float getMinFloat(float a, float b)
{
	return a < b ? a : b;
}

float getMaxFloat(float a, float b)
{
	return a < b ? b : a;
}

float getClampedFloat(float min, float max, float val)
{
	return getMinFloat(getMaxFloat(min, val), max);
}

float getFractFloat(float a)
{
	return a - floorf(a);
}

float getAbsFloat(float a)
{
	return ::fabsf(a);
}

float getSinFloat(float a)
{
	return sinf(a);
}

float getCosFloat(float a)
{
	return cosf(a);
}

float getArcCosFloat(float a)
{
	return ::acosf(a);
}

float getTanFloat(float a)
{
	return tanf(a);
}

float getSqrtFloat(float a)
{
	return sqrtf(a);
}

float getRadiansFromDegreesFloat(float degrees)
{
	return degrees * PI / 180.0f;
}

float getDegreesFromRadiansFloat(float radians)
{
	return radians * 180.0f / PI;
}

float getLinearlyInterpolated(const float p0, const float p1, float t)
{
	return (1.0f - t) * p0 + t * p1;
}

} // namespace Rio

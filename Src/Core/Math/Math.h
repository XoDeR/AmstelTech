#pragma once

#include "Core/Types.h"

namespace Rio
{

static const float PI = 3.14159265358979323846f;
static const float PI_TWO = 6.28318530717958647693f;
static const float PI_HALF = 1.57079632679489661923f;
static const float FLOAT_EPSILON = 1.0e-7f;

// Returns whether <a> and <b> are equal according to <epsilon>
bool getAreEqualFloat(float a, float b, float epsilon = FLOAT_EPSILON);

// Returns the minimum of <a> and <b>
float getMinFloat(float a, float b);

// Returns the maximum of <a> and <b>
float getMaxFloat(float a, float b);

// Clamps <val> to <min> and <max>
float getClampedFloat(float min, float max, float val);

// Returns the fractional part of <a>
float getFractFloat(float a);

// Returns the absolute value of <a>
float getAbsFloat(float a);

// Returns the sine of <a>
float getSinFloat(float a);

// Returns the cosine of <a>
float getCosFloat(float a);

// Returns the arc cosine of <a>
float getArcCosFloat(float a);

// Returns the tangent of <a>
float getTanFloat(float a);

// Returns the nonnegative square root of <a>
float getSqrtFloat(float a);

// Returns <degrees> in radians
float getRadiansFromDegreesFloat(float degrees);

// Returns <radians> in degrees
float getDegreesFromRadiansFloat(float radians);

// Returns the linear interpolated value between <p0> and <p1> at time <t>
float getLinearlyInterpolated(const float p0, const float p1, float t);

} // namespace Rio

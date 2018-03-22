#pragma once

#include "Core/Types.h"

namespace Rio
{

// Pseudo-random number generator
// Uses LCG algorithm -- fast and compatible with the standard C rand()
struct Random
{
	int32_t seed = 0;

	// Initializes the generator with the given <seed>
	Random(int32_t seed);

	// Returns a pseudo-random integer in the range [0, 32767]
	int32_t getRandomInt32();

	// Returns a pseudo-random integer in the range [0, max)
	int32_t getRandomInt32LessThan(int32_t max);

	// Returns a pseudo-random float in the range [0.0f, 1.0f]
	float getRandomUnitFloat();
};

inline Random::Random(int32_t seed)
	: seed(seed)
{
}

inline int32_t Random::getRandomInt32()
{
	seed = 214013 * seed + 13737667;
	return (seed >> 16) & 0x7fff;
}

inline int32_t Random::getRandomInt32LessThan(int32_t max)
{
	return (max == 0) ? 0 : getRandomInt32() % max;
}

inline float Random::getRandomUnitFloat()
{
	return getRandomInt32() / (float)0x7fff;
}

} // namespace Rio

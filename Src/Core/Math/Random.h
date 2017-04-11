// Copyright (c) 2016, 2017 Volodymyr Syvochka
#pragma once

#include "Core/Base/Types.h"
#include "Core/Base/RioCommon.h"
#include "Core/Math/MathUtils.h"

namespace RioCore
{
	// George Marsaglia's MWC
	class RngMwc
	{
	private:
		uint32_t m_z;
		uint32_t m_w;
	public:
		RngMwc(uint32_t z = 12345, uint32_t w = 65435)
			: m_z(z)
			, m_w(w)
		{
		}

		void reset(uint32_t z = 12345, uint32_t w = 65435)
		{
			m_z = z;
			m_w = w;
		}

		uint32_t gen()
		{
			m_z = 36969 * (m_z & 65535) + (m_z >> 16);
			m_w = 18000 * (m_w & 65535) + (m_w >> 16);
			return (m_z << 16) + m_w;
		}
	};

	// George Marsaglia's FIB
	class RngFib
	{
	private:
		uint32_t m_a = 9983651;
		uint32_t m_b = 95746118;
	public:
		RngFib()
		{
		}

		void reset()
		{
			m_a = 9983651;
			m_b = 95746118;
		}

		uint32_t gen()
		{
			m_b = m_a + m_b;
			m_a = m_b - m_a;
			return m_a;
		}
	};

	// George Marsaglia's SHR3
	class RngShr3
	{
	private:
		uint32_t m_jsr;
	public:
		RngShr3(uint32_t jsr = 34221)
			: m_jsr(jsr)
		{
		}

		void reset(uint32_t jsr = 34221)
		{
			m_jsr = jsr;
		}

		uint32_t gen()
		{
			m_jsr ^= m_jsr << 17;
			m_jsr ^= m_jsr >> 13;
			m_jsr ^= m_jsr << 5;
			return m_jsr;
		}
	};

	// Returns random number between 0.0f and 1.0f
	template <typename Ty>
	inline float frnd(Ty* rng)
	{
		uint32_t rndLocal = rng->gen() & UINT16_MAX;
		return float(rndLocal) * 1.0f / float(UINT16_MAX);
	}

	// Returns random number between -1.0f and 1.0f
	template <typename Ty>
	inline float frndh(Ty* rng)
	{
		return 2.0f * RioCore::frnd(rng) - 1.0f;
	}

	// Generate random point on unit sphere
	template <typename Ty>
	static inline void randUnitSphere(float result[3], Ty* rng)
	{
		float rand0 = frnd(rng) * 2.0f - 1.0f;
		float rand1 = frnd(rng) * PI * 2.0f;

		float sqrtf1 = sqrtf(1.0f - rand0*rand0);
		result[0] = sqrtf1 * cosf(rand1);
		result[1] = sqrtf1 * sinf(rand1);
		result[2] = rand0;
	}

	// Generate random point on unit hemisphere
	template <typename Ty>
	static inline void randUnitHemisphere(float result[3], Ty* rng, const float normal[3])
	{
		float dir[3];
		randUnitSphere(dir, rng);

		float DdotN = dir[0] * normal[0]
			+ dir[1] * normal[1]
			+ dir[2] * normal[2]
			;

		if (0.0f > DdotN)
		{
			dir[0] = -dir[0];
			dir[1] = -dir[1];
			dir[2] = -dir[2];
		}

		result[0] = dir[0];
		result[1] = dir[1];
		result[2] = dir[2];
	}

	// Sampling with Hammersley and Halton Points
	static inline void generateSphereHammersley(void* _data, uint32_t _stride, uint32_t _num, float _scale = 1.0f)
	{
		uint8_t* data = (uint8_t*)_data;

		for (uint32_t ii = 0; ii < _num; ii++)
		{
			float tt = 0.0f;
			float pp = 0.5;
			for (uint32_t jj = ii; jj; jj >>= 1)
			{
				tt += (jj & 1) ? pp : 0.0f;
				pp *= 0.5f;
			}

			tt = 2.0f * tt - 1.0f;

			const float phi = (ii + 0.5f) / _num;
			const float phirad = phi * 2.0f * PI;
			const float st = sqrtf(1.0f - tt*tt) * _scale;

			float* xyz = (float*)data;
			data += _stride;

			xyz[0] = st * cosf(phirad);
			xyz[1] = st * sinf(phirad);
			xyz[2] = tt * _scale;
		}
	}


	// Pseudo-random number generator
	// Uses LCG algorithm: fast and compatible with the standard C rand()
	class Random
	{
	public:
		// Initializes the generator with the given <seed>
		Random(int32_t seed);
		// Returns a pseudo-random integer in the range [0, 32767]
		int32_t getTnteger();
		// Returns a pseudo-random integer in the range [0, max)
		int32_t getTnteger(int32_t max);
		// Returns a pseudo-random float in the range [0.0, 1.0]
		float getUnitFloat();
	private:
		int32_t seed;
	};

	inline Random::Random(int32_t seed)
		: seed(seed)
	{
	}

	inline int32_t Random::getTnteger()
	{
		seed = 214013 * seed + 13737667;
		return (seed >> 16) & 0x7fff;
	}

	inline int32_t Random::getTnteger(int32_t max)
	{
		return (max == 0) ? 0 : getTnteger() % max;
	}

	inline float Random::getUnitFloat()
	{
		return getTnteger() / (float)0x7fff;
	}

} // namespace RioCore
// Copyright (c) 2016, 2017 Volodymyr Syvochka
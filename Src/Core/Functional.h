#pragma once

#include "Core/Murmur.h"
#include "Core/Types.h"

namespace Rio
{

template<typename T>
struct EqualTo
{
	bool operator()(const T& a, const T& b) const
	{
		return a == b;
	};
};

template<typename T>
struct NotEqualTo
{
	bool operator()(const T& a, const T& b) const
	{
		return a != b;
	};
};

template <typename T>
struct Greater
{
	bool operator()(const T& a, const T& b) const
	{
		return a > b;
	};
};

template<typename T>
struct Less
{
	bool operator()(const T& a, const T& b) const
	{
		return a < b;
	};
};

template<typename T>
struct GreaterEqual
{
	bool operator()(const T& a, const T& b) const
	{
		return a >= b;
	};
};

template<typename T>
struct LessEqual
{
	bool operator()(const T& a, const T& b) const
	{
		return a <= b;
	};
};

// Hash functions
template <typename T>
struct Hash;

template<>
struct Hash<bool>
{
	uint32_t operator()(const bool val) const
	{
		return (uint32_t)val;
	}
};

template<>
struct Hash<int8_t>
{
	uint32_t operator()(const int8_t val) const
	{
		return (uint32_t)val;
	}
};

template<>
struct Hash<uint8_t>
{
	uint32_t operator()(const uint8_t val) const
	{
		return (uint32_t)val;
	}
};

template<>
struct Hash<int16_t>
{
	uint32_t operator()(const int16_t val) const
	{
		return (uint32_t)val;
	}
};

template<>
struct Hash<uint16_t>
{
	uint32_t operator()(const uint16_t val) const
	{
		return (uint32_t)val;
	}
};

template<>
struct Hash<int32_t>
{
	uint32_t operator()(const int32_t val) const
	{
		return (uint32_t)val;
	}
};

template<>
struct Hash<uint32_t>
{
	uint32_t operator()(const uint32_t val) const
	{
		return (uint32_t)val;
	}
};

template<>
struct Hash<int64_t>
{
	uint32_t operator()(const int64_t val) const
	{
		return (uint32_t)val;
	}
};

template<>
struct Hash<uint64_t>
{
	uint32_t operator()(const uint64_t val) const
	{
		return (uint32_t)val;
	}
};

template<>
struct Hash<float>
{
	uint32_t operator()(const float val) const
	{
		return val == 0.0f ? 0 : murmur32(&val, sizeof(val), 0);
	}
};

template<>
struct Hash<double>
{
	uint32_t operator()(const double val) const
	{
		return val == 0.0 ? 0 : murmur32(&val, sizeof(val), 0);
	}
};

} // namespace Rio

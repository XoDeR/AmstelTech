#pragma once

#include "Core/Strings/String.h"

#include <algorithm> // std::max

namespace Rio
{

struct FixedString
{
	uint32_t length = 0;
	const char* data = nullptr;

	FixedString()
	{
	}

	FixedString(const char* str)
		: length(getStrLen32(str))
		, data(str)
	{
	}

	FixedString(const char* str, uint32_t length)
		: length(length)
		, data(str)
	{
	}

	FixedString& operator=(const char* str)
	{
		length = getStrLen32(str);
		data = str;
		return *this;
	}

	uint32_t getLength() const
	{
		return length;
	}

	const char* getCStr() const
	{
		return data;
	}
};

inline bool operator==(const FixedString& a, const char* str)
{
	const uint32_t len = getStrLen32(str);
	return a.length == len
		&& strncmp(a.data, str, len) == 0
		;
}

inline bool operator==(const FixedString& a, const FixedString& b)
{
	return a.length == b.length
		&& strncmp(a.data, b.data, a.length) == 0
		;
}

inline bool operator<(const FixedString& a, const FixedString& b)
{
	const uint32_t len = (std::max)(a.length, b.length);
	return strncmp(a.data, b.data, len) < 0;
}

} // namespace Rio

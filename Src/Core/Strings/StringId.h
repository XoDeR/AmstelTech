#pragma once

#include "Core/Strings/Types.h"
#include "Core/Types.h"

namespace Rio
{

// Hashed string
struct StringId32
{
	uint32_t id = 0;

	StringId32() 
	{
	}

	explicit StringId32(uint32_t id)
		: id(id)
	{
	}

	explicit StringId32(const char* str);
	explicit StringId32(const char* str, uint32_t len);

	void hashString(const char* str, uint32_t len);

	// Fills <s> with the string representation of the id
	void toString(DynamicString& s);
};

// Hashed string
struct StringId64
{
	uint64_t id = 0;

	StringId64() 
	{
	}

	explicit StringId64(uint64_t id)
		: id(id)
	{
	}

	explicit StringId64(const char* str);
	explicit StringId64(const char* str, uint32_t len);

	void hashString(const char* str, uint32_t len);

	// Fills <s> with the string representation of the id
	void toString(DynamicString& s);
};

inline bool operator==(const StringId32& a, const StringId32& b)
{
	return a.id == b.id;
}

inline bool operator!=(const StringId32& a, const StringId32& b)
{
	return a.id != b.id;
}

inline bool operator<(const StringId32& a, const StringId32& b)
{
	return a.id < b.id;
}

inline bool operator==(const StringId64& a, const StringId64& b)
{
	return a.id == b.id;
}

inline bool operator!=(const StringId64& a, const StringId64& b)
{
	return a.id != b.id;
}

inline bool operator<(const StringId64& a, const StringId64& b)
{
	return a.id < b.id;
}

template <>
struct Hash<StringId32>
{
	uint32_t operator()(const StringId32& id) const
	{
		return id.id;
	}
};

template <>
struct Hash<StringId64>
{
	uint32_t operator()(const StringId64& id) const
	{
		return (uint32_t)id.id;
	}
};

} // namespace Rio

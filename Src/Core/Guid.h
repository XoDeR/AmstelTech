#pragma once

#include "Core/Strings/Types.h"
#include "Core/Types.h"

#include <cstring> // memcmp

namespace Rio
{

// Globally unique identifier
struct Guid
{
	uint32_t data1 = 0u;
	uint16_t data2 = 0u;
	uint16_t data3 = 0u;
	uint64_t data4 = 0u;
};

namespace GuidFn
{
	// Returns a new randomly generated Guid
	Guid getNewGuid();

	// Parses the guid from <str>
	Guid parseFromString(const char* str);

	// Parses the <guid> from <str> and returns true if success
	bool tryParseFromString(Guid& guid, const char* str);

	// Fills <str> with the string representation of the <guid>
	void toString(const Guid& guid, DynamicString& str);

} // namespace GuidFn

// Returns whether Guid <a> and <b> are equal
inline bool operator==(const Guid& a, const Guid& b)
{
	return memcmp(&a, &b, sizeof(a)) == 0;
}

// Returns whether Guid <a> is less than <b>
inline bool operator<(const Guid& a, const Guid& b)
{
	return memcmp(&a, &b, sizeof(a)) < 0;
}

static const Guid GUID_ZERO = 
{ 
	0u, 
	0u, 
	0u, 
	0u 
};

} // namespace Rio

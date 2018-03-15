#pragma once

#include "Core/Containers/Types.h"
#include "Core/Strings/FixedString.h"

namespace Rio
{

struct JsonValueType
{
	enum Enum
	{
		NIL,
		BOOL,
		NUMBER,
		STRING,
		ARRAY,
		OBJECT
	};
};

// Array of pointers to json-encoded data
using JsonArray = Array<const char*>;

// Map from key to pointers to json-encoded data
struct JsonObject
{
	Map<FixedString, const char*> jsonMap;

	JsonObject(Allocator& a);

	const char* operator[](const char* key) const;
	const char* operator[](const FixedString& key) const;
};

} // namespace Rio

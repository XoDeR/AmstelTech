#pragma once

#include "Core/Containers/Map.h"
#include "Core/Json/Types.h"

namespace Rio
{

namespace JsonObjectFn
{
	// Returns the number of keys in the object <jsonObject>
	inline uint32_t getSize(const JsonObject& jsonObject)
	{
		return MapFn::getCount(jsonObject.jsonMap);
	}

	// Returns whether the object <jsonObject> has the <key>
	inline bool has(const JsonObject& jsonObject, const char* key)
	{
		return MapFn::has(jsonObject.jsonMap, FixedString(key));
	}

	// Returns a pointer to the first item in the object <jsonObject>
	inline const Map<FixedString, const char*>::Node* begin(const JsonObject& jsonObject)
	{
		return MapFn::begin(jsonObject.jsonMap);
	}

	// Returns a pointer to the item following the last item in the object <jsonObject>
	inline const Map<FixedString, const char*>::Node* end(const JsonObject& jsonObject)
	{
		return MapFn::end(jsonObject.jsonMap);
	}

} // namespace JsonObjectFn

inline JsonObject::JsonObject(Allocator& a)
	: jsonMap(a)
{
}

// Returns the value of the <key> or nullptr
inline const char* JsonObject::operator[](const char* key) const
{
	return MapFn::get(jsonMap, FixedString(key), (const char*)nullptr);
}

// Returns the value of the <key> or nullptr
inline const char* JsonObject::operator[](const FixedString& key) const
{
	return MapFn::get(jsonMap, key, (const char*)nullptr);
}

}

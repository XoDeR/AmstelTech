#pragma once

#include "Core/Json/Types.h"
#include "Core/Strings/Types.h"

namespace Rio
{

namespace JsonFn
{
	// Returns the data type of the JSON string <json>
	JsonValueType::Enum getJsonType(const char* json);

	// Returns the JSON number <json> as int32
	int32_t parseInt32(const char* json);

	// Returns the JSON number <json> as float
	float parseFloat(const char* json);

	// Returns the JSON boolean <json> as bool
	bool parseBool(const char* json);

	// Parses the JSON string <json> and puts it into a <string>
	void parseString(const char* json, DynamicString& string);

	// Parses the JSON array <json> and puts it into <array> 
	// as pointers to the corresponding items into the original <json> string
	void parseArray(const char* json, JsonArray& array);

	// Parses the JSON object <json> and puts it into an <object> 
	// as a map from a key to the pointer to the corresponding value into the original string <json>
	void parseObject(const char* json, JsonObject& object);

	// Parses the JSON-encoded <json>
	void parse(const char* json, JsonObject& object);

	// Parses the JSON-encoded <json>
	void parse(Buffer& json, JsonObject& object);

} // namespace JsonFn

} // namespace Rio

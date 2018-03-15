#include "Core/Json/Json.h"

#include "Core/Containers/Map.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Strings/DynamicString.h"
#include "Core/Strings/String.h"

namespace Rio
{

namespace JsonFn
{
	static const char* skipExpectedToNext(const char* json, const char c = 0)
	{
		RIO_ENSURE(nullptr != json);

		if (c && c != *json)
		{
			RIO_ASSERT(false, "Expected '%c' got '%c'", c, *json);
		}

		return ++json;
	}

	static const char* skipString(const char* json)
	{
		RIO_ENSURE(nullptr != json);

		while (*++json)
		{
			if (*json == '"')
			{
				++json;
				return json;
			}
			else if (*json == '\\')
			{
				++json;
			}
		}

		return json;
	}

	static const char* skipValue(const char* json)
	{
		RIO_ENSURE(nullptr != json);

		switch (*json)
		{
		case '"': json = skipString(json); break;
		case '[': json = skipBlock(json, '[', ']'); break;
		case '{': json = skipBlock(json, '{', '}'); break;
		default: for (; *json != ',' && *json != '}' && *json != ']'; ++json) ; break;
		}

		return json;
	}

	JsonValueType::Enum getJsonType(const char* json)
	{
		RIO_ENSURE(nullptr != json);

		switch (*json)
		{
		case '"': return JsonValueType::STRING;
		case '{': return JsonValueType::OBJECT;
		case '[': return JsonValueType::ARRAY;
		case '-': return JsonValueType::NUMBER;
		default: return (isdigit(*json)) ? JsonValueType::NUMBER : (*json == 'n' ? JsonValueType::NIL : JsonValueType::BOOL);
		}
	}

	static double parseNumber(const char* json)
	{
		RIO_ENSURE(nullptr != json);

		TempAllocator512 tempAllocator512;
	 	Array<char> number(tempAllocator512);

		if (*json == '-')
		{
			ArrayFn::pushBack(number, '-');
			++json;
		}
		while (isdigit(*json))
		{
			ArrayFn::pushBack(number, *json);
			++json;
		}

		if (*json == '.')
		{
			ArrayFn::pushBack(number, '.');
			while (*++json && isdigit(*json))
			{
				ArrayFn::pushBack(number, *json);
			}
		}

		if (*json == 'e' || *json == 'E')
		{
			ArrayFn::pushBack(number, *json);
			++json;

			if (*json == '-' || *json == '+')
			{
				ArrayFn::pushBack(number, *json);
				++json;
			}
			while (isdigit(*json))
			{
				ArrayFn::pushBack(number, *json);
				++json;
			}
		}

		ArrayFn::pushBack(number, '\0');

		double result = 0.0;
		int ok = sscanf(ArrayFn::begin(number), "%lf", &result);
		RIO_ASSERT(ok == 1, "Failed to parse double: %s", ArrayFn::begin(number));
		RIO_UNUSED(ok);
		return result;
	}

	int32_t parseInt32(const char* json)
	{
		return static_cast<int32_t>(parseNumber(json));
	}

	float parseFloat(const char* json)
	{
		return static_cast<float>(parseNumber(json));
	}

	bool parseBool(const char* json)
	{
		RIO_ENSURE(nullptr != json);

		switch (*json)
		{
		case 't':
			json = skipExpectedToNext(json, 't');
			json = skipExpectedToNext(json, 'r');
			json = skipExpectedToNext(json, 'u');
			skipExpectedToNext(json, 'e');
			return true;

		case 'f':
			json = skipExpectedToNext(json, 'f');
			json = skipExpectedToNext(json, 'a');
			json = skipExpectedToNext(json, 'l');
			json = skipExpectedToNext(json, 's');
			skipExpectedToNext(json, 'e');
			return false;

		default:
			RIO_FATAL("Bad boolean");
			return false;
		}
	}

	void parseString(const char* json, DynamicString& string)
	{
		RIO_ENSURE(nullptr != json);

		if (*json == '"')
		{
			while (*++json)
			{
				// Empty string
				if (*json == '"')
				{
					++json;
					return;
				}
				else if (*json == '\\')
				{
					++json;

					switch (*json)
					{
						case '"': string += '"'; break;
						case '\\': string += '\\'; break;
						case '/': string += '/'; break;
						case 'b': string += '\b'; break;
						case 'f': string += '\f'; break;
						case 'n': string += '\n'; break;
						case 'r': string += '\r'; break;
						case 't': string += '\t'; break;
						default:
						{
							RIO_FATAL("Bad escape character");
							break;
						}
					}
				}
				else
				{
					string += *json;
				}
			}
		}

		RIO_FATAL("Bad string");
	}

	void parseArray(const char* json, JsonArray& array)
	{
		RIO_ENSURE(nullptr != json);

		if (*json == '[')
		{
			json = skipSpaces(++json);

			if (*json == ']')
			{
				return;
			}

			while (*json)
			{
				ArrayFn::pushBack(array, json);

				json = skipValue(json);
				json = skipSpaces(json);

				if (*json == ']')
				{
					return;
				}

				json = skipExpectedToNext(json, ',');
				json = skipSpaces(json);
			}
		}

		RIO_FATAL("Bad array");
	}

	void parseObject(const char* json, JsonObject& object)
	{
		RIO_ENSURE(nullptr != json);

		if (*json == '{')
		{
			json = skipSpaces(++json);

			if (*json == '}')
			{
				return;
			}

			while (*json)
			{
				const char* keyBegin = *json == '"' ? (json + 1) : json;

				TempAllocator256 ta;
				DynamicString key(ta);
				parseString(json, key);

				FixedString keyStr(keyBegin, key.getLength());

				json = skipString(json);
				json = skipSpaces(json);
				json = skipExpectedToNext(json, ':');
				json = skipSpaces(json);

				MapFn::set(object.jsonMap, keyStr, json);

				json = skipValue(json);
				json = skipSpaces(json);

				if (*json == '}')
				{
					return;
				}

				json = skipExpectedToNext(json, ',');
				json = skipSpaces(json);
			}
		}

		RIO_FATAL("Bad object");
	}

	void parse(const char* json, JsonObject& object)
	{
		RIO_ENSURE(nullptr != json);
		parseObject(json, object);
	}

	void parse(Buffer& json, JsonObject& object)
	{
		ArrayFn::pushBack(json, '\0');
		ArrayFn::popBack(json);
		parse(ArrayFn::begin(json), object);
	}

} // namespace JsonFn

} // namespace Rio
